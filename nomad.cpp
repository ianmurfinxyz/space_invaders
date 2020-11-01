

#include "nomad.h"

namespace nomad
{

//===============================================================================================//
// ##>LOG                                                                                        //
//===============================================================================================//

Log::Log()
{
  _os.open(filename, std::ios_base::trunc);
  if(!_os){
    log(ERROR, logstr::fail_open_log);
    log(INFO, logstr::info_stderr_log);
  }
}

Log::~Log()
{
  if(_os)
    _os.close();
}

void Log::log(Level level, const char* error, const std::string& addendum)
{
  std::ostream& o {_os ? _os : std::cerr}; 
  o << lvlstr[level] << delim << error;
  if(!addendum.empty())
    o << delim << addendum;
  o << std::endl;
}

std::unique_ptr<Log> log {nullptr};

//===============================================================================================//
// ##>INPUT                                                                                      //
//===============================================================================================//

Input::Input()
{
  for(auto& key : _keyStates)
    key = KeyState::UP;
}

void Input::onKeyEvent(const SDL_Event& event)
{
  assert(event.type == SDL_KEYDOWN || event.type == SDL_KEYUP);

  KeyCode key = convertSdlKeyCode(event.key.keysym.sym);

  if(key == KEY_COUNT) 
    return;

  if(event.type == SDL_KEYDOWN)
    _keyStates[key] = KeyState::PRESSED;
  else
    _keyStates[key] = KeyState::RELEASED;
}

void Input::onUpdate()
{
  for(auto& state : _keyStates){
    if(state == KeyState::PRESSED)
      state = KeyState::DOWN;
    else if(state == KeyState::RELEASED)
      state = KeyState::UP;
  }
}

Input::KeyCode Input::convertSdlKeyCode(int sdlCode)
{
  switch(sdlCode) {
    case SDLK_a: return KEY_a;
    case SDLK_b: return KEY_b;
    case SDLK_c: return KEY_c;
    case SDLK_d: return KEY_d;
    case SDLK_e: return KEY_e;
    case SDLK_f: return KEY_f;
    case SDLK_g: return KEY_g;
    case SDLK_h: return KEY_h;
    case SDLK_i: return KEY_i;
    case SDLK_j: return KEY_j;
    case SDLK_k: return KEY_k;
    case SDLK_l: return KEY_l;
    case SDLK_m: return KEY_m;
    case SDLK_n: return KEY_n;
    case SDLK_o: return KEY_o;
    case SDLK_p: return KEY_p;
    case SDLK_q: return KEY_q;
    case SDLK_r: return KEY_r;
    case SDLK_s: return KEY_s;
    case SDLK_t: return KEY_t;
    case SDLK_v: return KEY_v;
    case SDLK_w: return KEY_w;
    case SDLK_x: return KEY_x;
    case SDLK_y: return KEY_y;
    case SDLK_LEFT: return KEY_LEFT;
    case SDLK_RIGHT: return KEY_RIGHT;
    case SDLK_DOWN: return KEY_DOWN;
    case SDLK_UP: return KEY_UP;
    case SDLK_SPACE: return KEY_SPACE;
    default: return KEY_COUNT;
  }
}

std::unique_ptr<Input> input {nullptr};

//===============================================================================================//
// ##>RESOURCES                                                                                  //
//===============================================================================================//

Dataset::Property::Property(int32_t key, std::string name, Value_t default_, Value_t min, Value_t max) :
  _key{key}, _name{name}, _default{default_}, _min{min}, _max{max}
{
  assert(_default.index() == _min.index() && _default.index() == _max.index());
  _type = static_cast<Type>(_default.index());
}

Dataset::Dataset(std::initializer_list<Property> properties)
{
  for(const auto& property : properties)
    _properties.emplace(std::make_pair(property._key, property));
}

int32_t Dataset::load(const char* filename)
{
  std::ifstream file {filename};
  if(!file){
    log->log(Log::WARN, logstr::warn_cannot_open_dataset, std::string{filename});
    log->log(Log::INFO, logstr::info_using_property_defaults);
    for(auto& pair : _properties)
      pair.second._value = pair.second._default;
    return -1;
  }

  auto lineNoToString = [](int32_t l){return std::string{" ["} + std::to_string(l) + "] ";};
  auto isSpace = [](char c){return std::isspace<char>(c, std::locale::classic());};

  int32_t lineNo {0};
  int32_t nErrors {0};

  for(std::string line; std::getline(file, line);){
    ++lineNo;

    line.erase(std::remove_if(line.begin(), line.end(), isSpace), line.end());

    if(line.front() == comment) 
      continue;
     
    int32_t count {0};
    count = std::count(line.begin(), line.end(), seperator);
    if(count != 1){
      log->log(Log::WARN, logstr::warn_malformed_dataset, std::string{filename}); 
      log->log(Log::INFO, logstr::info_on_line, lineNoToString(lineNo) + line);
      log->log(Log::INFO, logstr::info_unexpected_seperators, std::string{seperator});
      log->log(Log::INFO, logstr::info_ignoring_line);
      ++nErrors;
      continue;
    }

    std::size_t pos = line.find_first_of(seperator);
    std::string name {line.substr(0, pos)};
    std::string value {line.substr(pos + 1)};

    if(name.empty() || value.empty()){
      log->log(Log::ERROR, logstr::warn_malformed_dataset, std::string{filename}); 
      log->log(Log::INFO, logstr::info_on_line, lineNoToString(lineNo) + line);
      log->log(Log::INFO, logstr::info_incomplete_property);
      log->log(Log::INFO, logstr::info_ignoring_line);
      ++nErrors;
      continue;
    }

    Property* p {nullptr};
    for(auto& pair : _properties){
      if(pair.second._name == name){
        p = &pair.second;
        break;
      }
    }

    if(p == nullptr){
      log->log(Log::WARN, logstr::warn_malformed_dataset, std::string{filename}); 
      log->log(Log::INFO, logstr::info_on_line, lineNoToString(lineNo) + line);
      log->log(Log::INFO, logstr::info_unknown_dataset_property, name);
      log->log(Log::INFO, logstr::info_ignoring_line);
      continue;
    }

    switch(p->_type){
      case INT_PROPERTY:
        {
          int32_t result {0};
          if(!parseInt(value, result)){
            log->log(Log::WARN, logstr::warn_malformed_dataset, std::string{filename}); 
            log->log(Log::INFO, logstr::info_on_line, lineNoToString(lineNo) + line);
            log->log(Log::INFO, logstr::info_expected_integer, value);
            log->log(Log::INFO, logstr::info_ignoring_line);
            ++nErrors;
            continue;
          }
          p->_value = std::clamp(result, std::get<int32_t>(p->_min), std::get<int32_t>(p->_max));
          if(std::get<int32_t>(p->_value) != result){
            log->log(Log::INFO, logstr::info_property_clamped, name);
          }
          log->log(Log::INFO, logstr::info_property_set, std::string{filename} + lineNoToString(lineNo) + line);
          break;
        }
      case FLOAT_PROPERTY:
        {
          float result {0.f};
          if(!parseFloat(value, result)){
            log->log(Log::WARN, logstr::warn_malformed_dataset, std::string{filename}); 
            log->log(Log::INFO, logstr::info_on_line, lineNoToString(lineNo) + line);
            log->log(Log::INFO, logstr::info_expected_float, value);
            log->log(Log::INFO, logstr::info_ignoring_line);
            ++nErrors;
            continue;
          }
          p->_value = std::clamp(result, std::get<float>(p->_min), std::get<float>(p->_max));
          if(std::get<float>(p->_value) != result){
            log->log(Log::INFO, logstr::info_property_clamped, name);
          }
          log->log(Log::INFO, logstr::info_property_set, std::string{filename} + lineNoToString(lineNo) + line);
          break;
        }
      case BOOL_PROPERTY:
        {
          bool result {false};
          if(!parseBool(value, result)){
            log->log(Log::WARN, logstr::warn_malformed_dataset, std::string{filename}); 
            log->log(Log::INFO, logstr::info_on_line, lineNoToString(lineNo) + line);
            log->log(Log::INFO, logstr::info_expected_bool, value);
            log->log(Log::INFO, logstr::info_ignoring_line);
            ++nErrors;
            continue;
          }
          p->_value = result;
          log->log(Log::INFO, logstr::info_property_set, std::string{filename} + lineNoToString(lineNo) + line);
          break;
        }
    }
  }

  for(auto& pair : _properties){
    if(pair.second._value == Value_t{}){
      log->log(Log::WARN, logstr::warn_property_not_set, pair.second._name);
      log->log(Log::INFO, logstr::info_using_property_default);
      pair.second._value = pair.second._default;
      ++nErrors;
    }
  }

  return nErrors;
}

int32_t Dataset::write(const char* filename, bool genComments)
{
  std::ofstream file {filename, std::ios_base::out | std::ios_base::trunc};
  if(!file){
    log->log(Log::WARN, logstr::warn_cannot_create_dataset, std::string{filename});
    return -1;
  }

  for(const auto& pair : _properties){
    const Property& p = pair.second;
    
    std::stringstream ss {};

    if(genComments){
      ss << comment;
      ss << " default=";
      printValue(p._default, ss);
      ss << " min=";
      printValue(p._min, ss);
      ss << " max=";
      printValue(p._max, ss);
      ss << '\n';
      file << ss.str();
    }

    std::stringstream().swap(ss);

    ss << p._name << "=";
    printValue(p._value, ss);
    ss << '\n';
    file << ss.str();
  }
}

int32_t Dataset::getIntValue(int32_t key) const
{
  assert(_properties.at(key)._type == INT_PROPERTY);
  return std::get<int32_t>(_properties.at(key)._value);
}

float Dataset::getFloatValue(int32_t key) const
{
  assert(_properties.at(key)._type == FLOAT_PROPERTY);
  return std::get<float>(_properties.at(key)._value);
}

bool Dataset::getBoolValue(int32_t key) const
{
  assert(_properties.at(key)._type == BOOL_PROPERTY);
  return std::get<bool>(_properties.at(key)._value);
}

void Dataset::setIntValue(int32_t key, int32_t value)
{
  assert(_properties.at(key)._type == INT_PROPERTY);
  _properties[key]._value = value;
}

void Dataset::setFloatValue(int32_t key, float value)
{
  assert(_properties.at(key)._type == FLOAT_PROPERTY);
  _properties[key]._value = value;
}

void Dataset::setBoolValue(int32_t key, bool value)
{
  assert(_properties.at(key)._type == BOOL_PROPERTY);
  _properties[key]._value = value;
}

void Dataset::scaleIntValue(int32_t key, int32_t scale)
{
  assert(_properties.at(key)._type == INT_PROPERTY);
  int32_t value = std::get<int32_t>(_properties[key]._value);
  _properties[key]._value = value * scale;
}

void Dataset::scaleFloatValue(int32_t key, float scale)
{
  assert(_properties.at(key)._type == FLOAT_PROPERTY);
  float value = std::get<float>(_properties[key]._value);
  _properties[key]._value = value * scale;
}

bool Dataset::parseInt(const std::string& value, int32_t& result)
{
  auto isDigit = [](unsigned char c){return std::isdigit(c);};
  auto isSign = [](unsigned char c){return c == '+' || c == '-';};

  int32_t nSigns = std::count_if(value.begin(), value.end(), isSign);
  if(nSigns > 1){
    return false;
  }
  else if(nSigns == 1 && isSign(value.front()) == false){
      return false;
  }

  int32_t count = std::count_if(value.begin(), value.end(), isDigit);
  if(count != value.length() - nSigns){
    return false;
  }

  result = std::stoi(value);
  return true;
}

bool Dataset::parseFloat(const std::string& value, float& result)
{
  auto isDigit = [](unsigned char c){return std::isdigit(c);};
  auto isSign = [](unsigned char c){return c == '+' || c == '-';};

  int32_t nSigns = std::count_if(value.begin(), value.end(), isSign);
  if(nSigns > 1){
    return false;
  }
  else if(nSigns == 1 && isSign(value.front()) == false){
      return false;
  }

  int32_t nPoints = std::count(value.begin(), value.end(), '.');
  if(nPoints > 1)
    return false;

  int32_t count = std::count_if(value.begin(), value.end(), isDigit);
  if(count != value.length() - nPoints - nSigns)
    return false;

  result = std::stof(value);
  return true;
}

bool Dataset::parseBool(const std::string& value, bool& result)
{
  if(value == "true"){
    result = true;
    return true;
  }
  else if(value == "false"){
    result = false;
    return true;
  }

  return false;
}

void Dataset::printValue(const Value_t& value, std::ostream& os)
{
  switch(value.index()){
    case 0:
      os << std::get<int32_t>(value);
      break;
    case 1:
      os << std::get<float>(value);
      break;
    case 2:
      os << (std::get<bool>(value) ? "true" : "false");
      break;
  }
}

void Assets::loadBitmaps(std::vector<std::typle<Key_t, Name_t, Scale_t>> manifest)
{
  for(const auto& tuple : manifest){
    Key_t key = std::get<0>(typle);
    Name_t name = std::get<1>(typle);
    Scale_t scale = std::get<2>(typle);

    assert(scale < maxScale);

    auto search = _fonts.find(key);
    if(search == _fonts.end()){
      auto pair = _fonts.insert(std::make_pair(key, {}));
      assert(pair->second);
      search = pair->first;
    }

    if((search->second)[scale] != nullptr){
      std::string addendum = std::string{key} + std::string{" : scale:"} + std::to_string(scale)); 
      log->log(Log::WARN, logstr::warn_bitmap_already_loaded, std::move(addendum));
      log->log(Log::INFO, logstr::info_skipping_bitmap_loading);
      continue;
    }

    (search->second)[scale] = std::move(std::make_unique<Bitmap>(loadBitmap(name, scale)));
  }
}

Bitmap Assets::loadBitmap(std::string name, Scale_t scale)
{
  assert(scale < maxScale);

  std::string bitpath {};
  bitpath += bitmaps_path;
  bitpath += path_seperator;
  bitpath += name;
  bitpath += bitmaps_extension;

  log->log(Log::INFO, logstr::info_loading_bitmap, bitpath);

  std::ifstream file {bitpath};
  if(!file){
    log->log(Log::WARN, logstr::warn_missing_bitmap, bitpath);
    log->log(Log::INFO, logstr::using_error_bitmap);
    return generateErrorBitmap(scale);
  }

  auto isSpace = [](char c){return std::isspace<char>(c, std::locale::classic());};

  std::vector<std::string> rows {};

  for(std::string row; std::getline(file, row);){
    row.erase(std::remove_if(row.begin(), row.end(), isSpace), row.end());
    if(row.length() > 0)
      rows.emplace_back(std::move(row));
  }

  if(rows.size() == 0){
    log->log(Log::WARN, logstr::warn_no_bitmap_data, bitpath);
    log->log(Log::INFO, logstr::using_error_bitmap);
    return generateErrorBitmap(scale);
  }

  auto isBinary = [](char ch){return ch == '0' || ch == '1';};

  int32_t rowNo {0};
  for(auto& row : rows){
    ++rowNo;

    int32_t count = std::count_if(row.begin(), row.end(), isBinary);
    if(count != row.length()){
      log->log(Log::WARN, logstr::warn_malformed_bitmap);
      std::string addendum = std::string{" ["} + std::to_string(rowNo) + std::string{"] "} + row;
      log->log(Log::INFO, logstr::info_on_row, addendum);
      return generateErrorBitmap(scale);
    }
  }

  std::reverse(rows.begin(), rows.end()); 

  Bitmap bitmap {};
  bitmap.initialize(std::move(rows), scale);

  return bitmap;
}

Bitmap Assets::generateErrorBitmap(Scale_t scale)
{
  std::vector<std::string> rows {};

  for(auto row : errorBits){
    rows.push_back(row); 
  }

  Bitmap bitmap();
  bitmap.initialize(rows, scale);

  return bitmap;
}

void Assets::loadFonts(std::vector<std::typle<Key_t, Name_t, Scale_t>> manifest)
{
  for(const auto& tuple : manifest){
    Key_t key = std::get<0>(typle);
    Name_t name = std::get<1>(typle);
    Scale_t scale = std::get<2>(typle);

    assert(scale < maxScale);

    auto search = _fonts.find(key);
    if(search == _fonts.end()){
      auto pair = _fonts.insert(std::make_pair(key, {}));
      assert(pair->second);
      search = pair->first;
    }

    if((search->second)[scale] != nullptr){
      std::string addendum = std::string{key} + std::string{" : scale:"} + std::to_string(scale)); 
      log->log(Log::WARN, logstr::warn_font_already_loaded, std::move(addendum));
      log->log(Log::INFO, logstr::info_skipping_font_loading);
      continue;
    }

    (search->second)[scale] = std::move(std::make_unique<Font>(loadFont(name, scale)));
  }
}

Font Assets::loadFont(std::string name, Scale_t scale)
{
  Font::Meta meta;
  std::vector<Font::Glyph> glyphs {};

  std::string basepath {};
  basepath += fonts_path;
  basepath += path_seperator;
  basepath += name;
  basepath += path_seperator;

  std::string fontpath {basepath};
  fontpath += name;
  fontpath += fonts_extension;

  log->log(Log::INFO, logstr::info_loading_font, fontpath);

  FontData fontdata {};
  if((int32_t r = fontdata.load(fontpath)) != 0){
    const char* error = (r < 0) ? logstr::fail_open_font : logstr::fail_parse_font;
    log->log(Log::WARN, error, fontpath);
    log->log(Log::info, logstr::info_using_error_font);
    return generateErrorFont(scale);
  }

  meta._lineSpace = fontdata.getIntValue(KEY_LINE_SPACE) * scale;
  meta._wordSpace = fontdata.getIntValue(KEY_WORD_SPACE) * scale;
  meta._glyphSpace = fontdata.getIntValue(KEY_GLYPH_SPACE) * scale;
  meta._size = fontdata.getIntValue(KEY_SIZE) * scale;

  GlyphData glyphdata {};

  for(int i = 0; i < asciiCharCount; ++i){
    int32_t ascii = static_cast<int32_t>('!') + i;

    std::string glyphpath {basepath};
    glyphpath += glyphFilenames[i];
    glyphpath += glyphs_extension;

    if((int32_t r = glyphdata.load(glyphpath)) != 0){
      const char* error = (r < 0) ? logstr::fail_open_glyph : logstr::fail_parse_glyph;
      log->log(Log::WARN, error, glyphpath);
      log->log(Log::info, logstr::info_using_error_glyph);
      log->log(Log::info, logstr::info_ascii_code, std::to_string(ascii));
      glyphs.emplace_back(generateErrorGlyph(ascii, scale));
      continue;
    }

    Glyph glyph {};

    glyph._asciiCode = ascii;
    glyph._offsetX = glyphdata.getIntValue(KEY_OFFSET_X) * scale;
    glyph._advance = glyphdata.getIntValue(KEY_ADVANCE) * scale;
    glyph._width = glyphdata.getIntValue(KEY_WIDTH) * scale;
    glyph._height = glyphdata.getIntValue(KEY_HEIGHT) * scale;

    std::string bitpath {basepath};
    bitpath += glyphFilenames[i];
    bitpath += bitmap_extension;
    glyph._bitmap = loadBitmap(bitpath, scale);

    glyphs.emplace_back(std::move(glyph));
  }

  Font font;
  font.initialize(meta, std::move(glyphs));

  return font;
}

Font Assets::generateErrorFont(Scale_t scale)
{
  Font::Meta meta;
  std::vector<Font::Glyph> glyphs {};

  FontData fontdata {};

  meta._lineSpace = fontdata.getIntValue(KEY_LINE_SPACE) * scale;
  meta._wordSpace = fontdata.getIntValue(KEY_WORD_SPACE) * scale;
  meta._glyphSpace = fontdata.getIntValue(KEY_GLYPH_SPACE) * scale;
  meta._size = fontdata.getIntValue(KEY_SIZE) * scale;

  for(int i = 0; i < asciiCharCount; ++i){
    int32_t ascii = static_cast<int32_t>('!') + i;
    glyphs.emplace_back(generateErrorGlyph(ascii, scale));
  }

  Font font {};
  font.initialize(meta, std::move(glyphs));
  return font;
}

Glyph Assets::generateErrorGlyph(int32_t asciiCode, Scale_t scale)
{
  GlyphData glyphdata {};
  Glyph glyph {};

  glyph._asciiCode = asciiCode;
  glyph._offsetX = glyphdata.getIntValue(KEY_OFFSET_X) * scale;
  glyph._advance = glyphdata.getIntValue(KEY_ADVANCE) * scale;
  glyph._width = glyphdata.getIntValue(KEY_WIDTH) * scale;
  glyph._height = glyphdata.getIntValue(KEY_HEIGHT) * scale;

  std::vector<std::string> bits {};
  for(auto row : errorBits)
    bits.push_back(row);

  glyph.bitmap.initialize(std::move(bits), scale);

  return glyph;
}

const Bitmap& Assets::getBitmap(Key_t key, Scale_t scale) const
{
  return (_bitmaps.at(key))[scale];
}

const Font& Assets::getFont(Key_t key, Scale_t scale) const
{
  return (_fonts.at(key))[scale];
}

std::unique_ptr<Assets> assets {nullptr};


//===============================================================================================//
// ##>GRAPHICS                                                                                   //
//===============================================================================================//

void Bitmap::initialize(std::vector<std::string> bits, int32_t scale)
  // predicate: bit strings must contain only 0's and 1's
{
  // strip redundant data
  for(auto& row : bits){
    while(row.back() == '0'){
      row.pop_back();
    }
  }

  // scale bits
  if(scale > 1){
    std::vector<std::string> sbits {};
    std::string srow {};
    for(auto& row : bits){
      for(auto c : row){
        for(int i = 0; i < scale; ++i){
          srow.push_back(c);
        }
      }
      row.shrink_to_fit();
      for(int i = 0; i < scale; ++i){
        sbits.push_back(srow);
      }
      srow.clear();
    }
    bits = std::move(sbits);
  }

  // compute bitmap dimensions
  size_t w {0};
  for(const auto& row : bits){
    w = std::max(w, row.length());
  }
  _width = w;
  _height = bits.size();

  // generate the bit data
  std::vector<bool> brow {};
  brow.reserve(w);
  for(const auto& row : bits){
    brow.clear();
    for(char c : row){
      bool bit = !(c == '0'); 
      brow.push_back(bit); 
    }
    int32_t n = w - row.size();
    while(n-- > 0){
      brow.push_back(false); // pad to make all rows equal length
    }
    brow.shrink_to_fit();
    _bits.push_back(brow);
  }

  generateBytes();
}

bool Bitmap::getBit(int32_t row, int32_t col)
{
  assert(0 <= row && row < _width);
  assert(0 <= col && col < _height);
  return _bits[row][col];
}

void Bitmap::setBit(int32_t row, int32_t col, bool value, bool regen)
{
  assert(0 <= row && row < _width);
  assert(0 <= col && col < _height);
  _bits[row][col] = value;
  if(regen)
    generateBytes();
}

void Bitmap::print(std::ostream& out) const
{
  for(auto iter = _bits.rbegin(); iter != _bits.rend(); ++iter){
    for(bool bit : *iter){
      out << bit;
    }
    out << '\n';
  }
  out << std::endl;
}

void Bitmap::generateBytes()
{
  _bytes.clear();
  uint8_t byte {0};
  int32_t bitNo {0};
  for(const auto& row : _bits){
    for(bool bit : row){
      if(bitNo > 7){
        _bytes.push_back(byte);
        byte = 0;
        bitNo = 0;
      }
      if(bit) 
        byte |= 0x01 << (7 - bitNo);
      bitNo++;
    }
    _bytes.push_back(byte);
    byte = 0;
    bitNo = 0;
  }
}

void Font::initialize(Meta meta, std::vector<Glyph> glyphs)
  // predicate: expects glyphs for all ascii codes in the range 33-126 inclusive.
  // predicate: expects glyphs to be in ascending order of ascii code.
{
  _meta = meta;
  _glyphs = std::move(glyphs);
}

Renderer::Renderer(const Config& config)
{
  _config = config;

  uint32_t flags = SDL_WINDOW_OPENGL;
  if(_config._fullscreen){
    flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    log->log(Log::INFO, logstr::info_fullscreen_mode);
  }

  std::stringstream ss {};
  ss << "{w:" << _config._windowWidth << ",h:" << _config._windowHeight << "}";
  log->log(Log::INFO, logstr::info_creating_window, std::string{ss.str()});

  _window = SDL_CreateWindow(
      _config._windowTitle.c_str(), 
      SDL_WINDOWPOS_UNDEFINED,
      SDL_WINDOWPOS_UNDEFINED,
      _config._windowWidth,
      _config._windowHeight,
      flags
  );

  if(_window == nullptr){
    log->log(Log::FATAL, logstr::fail_create_window, std::string{SDL_GetError()});
    exit(EXIT_FAILURE);
  }

  Vector2i wndsize = getWindowSize();
  std::stringstream().swap(ss);
  ss << "{w:" << wndsize._x << ",h:" << wndsize._y << "}";
  log->log(Log::INFO, logstr::info_created_window, std::string{ss.str()});

  _glContext = SDL_GL_CreateContext(_window);
  if(_glContext == nullptr){
    log->log(Log::FATAL, logstr::fail_create_opengl_context, std::string{SDL_GetError()});
    exit(EXIT_FAILURE);
  }

  if(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, _config._openglVersionMajor) < 0){
    nomad::log->log(Log::FATAL, logstr::fail_set_opengl_attribute, std::string{SDL_GetError()});
    exit(EXIT_FAILURE);
  }
  if(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, _config._openglVersionMinor) < 0){
    nomad::log->log(Log::FATAL, logstr::fail_set_opengl_attribute, std::string{SDL_GetError()});
    exit(EXIT_FAILURE);
  }

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  setViewport(iRect{0, 0, _config._windowWidth, _config._windowHeight});
}

Renderer::~Renderer()
{
  SDL_GL_DeleteContext(_glContext);
  SDL_DestroyWindow(_window);
}

void Renderer::setViewport(iRect viewport)
{
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0.0, viewport._w, 0.0, viewport._h, -1.0, 1.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glViewport(viewport._x, viewport._y, viewport._w, viewport._h);
  _viewport = viewport;
}

void Renderer::blitText(Vector2f position, const std::string& text, const Color3f& color)
{
}

void Renderer::blitBitmap(Vector2f position, const Bitmap& bitmap, const Color3f& color)
{
  glColor3f(color.getRed(), color.getGreen(), color.getBlue());  
  glRasterPos2f(position._x, position._y);
  glBitmap(bitmap.getWidth(), bitmap.getHeight(), 0, 0, 0, 0, bitmap.getBytes().data());
}

void Renderer::clearWindow(const Color3f& color)
{
  glClearColor(color.getRed(), color.getGreen(), color.getBlue(), 1.f);
  glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer::clearViewport(const Color3f& color)
{
  glEnable(GL_SCISSOR_TEST);
  glScissor(_viewport._x, _viewport._y, _viewport._w, _viewport._h);
  glClearColor(color.getRed(), color.getGreen(), color.getBlue(), 1.f);
  glClear(GL_COLOR_BUFFER_BIT);
  glDisable(GL_SCISSOR_TEST);
}

void Renderer::show()
{
  SDL_GL_SwapWindow(_window);
}

Vector2i Renderer::getWindowSize() const
{
  Vector2i size;
  SDL_GL_GetDrawableSize(_window, &size._x, &size._y);
  return size;
}

std::unique_ptr<Renderer> renderer {nullptr};

//===============================================================================================//
// ##>APPLICATION                                                                                //
//===============================================================================================//

void Application::onWindowResize(int32_t windowWidth, int32_t windowHeight)
{
  Vector2i worldSize = getWorldSize();

  if((windowWidth < worldSize._x) || (windowHeight < worldSize._y)){
    _isWindowTooSmall = true;
    _engine->pause();
    _viewport._x = 0;
    _viewport._y = 0;
    _viewport._w = worldSize._x;
    _viewport._h = worldSize._y;
  }
  else{
    _isWindowTooSmall = false;
    _engine->unpause();
    _viewport._x = (windowWidth - worldSize._x) / 2;
    _viewport._y = (windowHeight - worldSize._y) / 2;
    _viewport._w = worldSize._x;
    _viewport._h = worldSize._y;
  }
}

bool Application::initialize(Engine* engine, int32_t windowWidth, int32_t windowHeight)
{
  _engine = engine;
  return true;
}

void Application::onUpdate(double now, float dt)
{
  assert(_activeState != nullptr);

  if(nomad::input->getKeyState(pauseKey) == Input::KeyState::PRESSED)
    _engine->togglePause();

  (*_activeState)->onUpdate(now, dt);
}

void Application::onDraw(double now, float dt)
{
  assert(_activeState != nullptr);

  nomad::renderer->setViewport(_viewport);

  if(_isWindowTooSmall)
    drawWindowTooSmall();
  else
    (*_activeState)->onDraw(now, dt);
}

void Application::addState(std::unique_ptr<ApplicationState>&& state)
{
  _states.emplace(std::make_pair(state->getName(), std::move(state)));
}

void Application::switchState(const std::string& name)
{
  assert(_states.find(name) != _states.end());
  _activeState = &_states[name];
}

void Application::drawWindowTooSmall()
{
  nomad::renderer->clearViewport(colors::red);
}

//===============================================================================================//
// ##>ENGINE                                                                                     //
//===============================================================================================//

Engine::Duration_t Engine::RealClock::update()
{
  _now1 = Clock_t::now();
  _dt = _now1 - _now0;
  _now1 = _now0;
  return _dt;
}

Engine::Duration_t Engine::GameClock::update(Duration_t realDt)
{
  if(_isPaused) return Duration_t::zero();
  _dt = Duration_t{static_cast<int64_t>(realDt.count() * _scale)};
  _now += _dt;
  return _dt;
}

int64_t Engine::Metronome::doTicks(Duration_t gameNow)
{
  int ticks {0};
  while(_lastTickNow + _tickPeriod < gameNow) {
    _lastTickNow += _tickPeriod;
    ++ticks;
  }
  _totalTicks += ticks;
  return ticks;
}

void Engine::TPSMeter::recordTicks(Duration_t realDt, int32_t ticks)
{
  _timer += realDt;
  _ticks += ticks;
  if(_timer > oneSecond){
    _tps = _ticks;
    _ticks = 0;
    _timer = Duration_t::zero();
  }
}

void Engine::initialize(std::unique_ptr<Application>&& app)
{
  log = std::make_unique<Log>();

  _app = std::move(app);

  if(!_config.load(Config::filename))
    _config.write(Config::filename); // generate a default file if one doesn't exist.

  if(SDL_Init(SDL_INIT_VIDEO) < 0){
    log->log(Log::FATAL, logstr::fail_sdl_init, std::string{SDL_GetError()});
    exit(EXIT_FAILURE);
  }

  LoopTick* tick = &_loopTicks[LOOPTICK_UPDATE];
  tick->_onTick = &Engine::onUpdateTick;
  tick->_metronome.setTickPeriod(Duration_t{static_cast<int64_t>(1.0e9 / 60.0_hz)});
  tick->_ticksAccumulated = 0;
  tick->_ticksDoneThisFrame = 0;
  tick->_maxTicksPerFrame = 5;
  tick->_tickPeriod = 1.0 / 60.0_hz;

  tick = &_loopTicks[LOOPTICK_DRAW];
  tick->_onTick = &Engine::onDrawTick;
  tick->_metronome.setTickPeriod(Duration_t{static_cast<int64_t>(1.0e9 / 60.0_hz)});
  tick->_ticksAccumulated = 0;
  tick->_ticksDoneThisFrame = 0;
  tick->_maxTicksPerFrame = 1;
  tick->_tickPeriod = 1.0 / 60.0_hz;

  input = std::make_unique<Input>();
  assets = std::make_unique<Assets>();

  std::stringstream ss {};
  ss << _app->getName() 
     << " version:" 
     << _app->getVersionMajor() 
     << "." 
     << _app->getVersionMinor();

  Renderer::Config rconfig {
    std::string{ss.str()},
    _config.getIntValue(Config::KEY_WINDOW_WIDTH),
    _config.getIntValue(Config::KEY_WINDOW_HEIGHT),
    _config.getIntValue(Config::KEY_OPENGL_MAJOR),
    _config.getIntValue(Config::KEY_OPENGL_MINOR),
    _config.getBoolValue(Config::KEY_FULLSCREEN)
  };

  renderer = std::make_unique<Renderer>(rconfig);

  Vector2i windowSize = nomad::renderer->getWindowSize();

  _app->initialize(this, windowSize._x, windowSize._y);

  _isSleeping = true;
  _isDrawingPerformanceStats = false;
  _isDone = false;
}

void Engine::run()
{
  _realClock.start();
  while(!_isDone) mainloop();
}

void Engine::mainloop()
{
  auto now0 = Clock_t::now();
  auto realDt = _realClock.update();
  auto gameDt = _gameClock.update(realDt);
  auto gameNow = _gameClock.getNow();

  SDL_Event event;
  while(SDL_PollEvent(&event) != 0){
    switch(event.type){
      case SDL_QUIT:
        _isDone = true;
        return;
      case SDL_WINDOWEVENT:
        if(event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
          _app->onWindowResize(event.window.data1, event.window.data2);
        break;
      case SDL_KEYDOWN:
        if(event.key.keysym.sym == SDLK_LEFTBRACKET){
          _gameClock.incrementScale(-0.1);
          break;
        }
        else if(event.key.keysym.sym == SDLK_RIGHTBRACKET){
          _gameClock.incrementScale(0.1);
          break;
        }
        else if(event.key.keysym.sym == SDLK_KP_HASH){
          _gameClock.setScale(1.f);
          break;
        }
        else if(event.key.keysym.sym == SDLK_BACKQUOTE){
          _isDrawingPerformanceStats = !_isDrawingPerformanceStats;
          break;
        }
      case SDL_KEYUP:
        nomad::input->onKeyEvent(event);
        break;
    }
  }

  for(int32_t i = LOOPTICK_UPDATE; i < LOOPTICK_COUNT; ++i){
    LoopTick& tick = _loopTicks[i];
    tick._ticksAccumulated += tick._metronome.doTicks(gameNow);
    tick._ticksDoneThisFrame = 0;
    while(tick._ticksAccumulated > 0 && tick._ticksDoneThisFrame < tick._maxTicksPerFrame){
      ++tick._ticksDoneThisFrame;
      --tick._ticksAccumulated;
      (this->*tick._onTick)(gameNow, gameDt, realDt, tick._tickPeriod);
    }
    tick._tpsMeter.recordTicks(realDt, tick._ticksDoneThisFrame);
  }

  nomad::input->onUpdate();
  
  if(_isSleeping){
    auto now1 {Clock_t::now()};
    auto framePeriod {now1 - now0};
    if(framePeriod < minFramePeriod)
      std::this_thread::sleep_for(minFramePeriod - framePeriod); 
  }
}

void Engine::drawPerformanceStats(Duration_t realDt, Duration_t gameDt)
{

}

void Engine::drawPauseDialog()
{

}

void Engine::onUpdateTick(Duration_t gameNow, Duration_t gameDt, Duration_t realDt, float tickDt)
{
  double now = durationToSeconds(gameNow);
  _app->onUpdate(now, tickDt);
}

void Engine::onDrawTick(Duration_t gameNow, Duration_t gameDt, Duration_t realDt, float tickDt)
{
  // TODO - temp - clear the game viewport only in the game and menu states - only clear window
  // when toggle perf stats
  nomad::renderer->clearWindow(colors::black);

  double now = durationToSeconds(gameNow);

  _app->onDraw(now, tickDt);

  if(_gameClock.isPaused())
    drawPauseDialog();

  if(_isDrawingPerformanceStats)
    drawPerformanceStats(realDt, gameDt);

  nomad::renderer->show();
}

double Engine::durationToSeconds(Duration_t d)
{
  return static_cast<double>(d.count()) / static_cast<double>(oneSecond.count());
}

} // namespace nomad


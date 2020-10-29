

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
// ##>GRAPHICS                                                                                   //
//===============================================================================================//

Bitmap::Bitmap(std::vector<std::string> bits, int32 scale)
{
  assert(0 < scale && scale <= scaleMax);
  for(auto& str : bits){
    for(char c : str){
      assert(c == '0' || c == '1');
    }
    while(str.back() == '0'){
      str.pop_back();
    }
  }
  if(scale > 1){
    std::vector<std::string> sbits {};
    std::string row {};
    for(auto& str : bits){
      for(auto c : str){
        for(int i = 0; i < scale; ++i){
          row.push_back(c);
        }
      }
      row.shrink_to_fit();
      for(int i = 0; i < scale; ++i){
        sbits.push_back(row);
      }
      row.clear();
    }
    bits = std::move(sbits);
  }
  int32 w {0};
  for(const auto& str : bits){
    w = std::max(w, static_cast<int32>(str.length()));
  }
  _width = w;
  _height = bits.size();
  std::vector<bool> row {};
  row.reserve(w);
  for(const auto& str : bits){
    row.clear();
    for(char c : str){
      bool bit = !(c == '0'); 
      row.push_back(bit); 
    }
    int32 n {w - row.size()};
    while(n-- > 0){
      row.push_back(false);
    }
    row.shrink_to_fit();
    _bits.push_back(row);
  }
  regenBytes();
}

void Bitmap::regenBytes()
{
  _bytes.clear();
  uint8 byte {0};
  int32 bitNo {0};
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

bool Bitmap::getBit(int32 row, int32 col)
{
  assert(0 <= row && row < _width);
  assert(0 <= col && col < _height);
  return _bits[row][col];
}

void Bitmap::setBit(int32 row, int32 col, bool value, bool regen)
{
  assert(0 <= row && row < _width);
  assert(0 <= col && col < _height);
  _bits[row][col] = value;
  if(regen)
    regenBytes();
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

Font::Font(const std::vector<Glyph>& glyphs, Meta meta) : 
  _glyphs{glyphs}, _meta{meta}
{
  static constexpr int checkSum {7505}; // sum of all ascii codes from 32 to 126 inclusive.

  int sum{0};
  for(const auto& g : _glyphs)
    sum += g._asciiCode;
  assert(sum == checkSum);

  auto gcompare = [](const Glyph& g0, const Glyph& g1){return g0._asciiCode < g1._asciiCode;};
  std::sort(_glyphs.begin(), _glyphs.end(), gcompare);
}

Renderer::Renderer(const Config& config)
{
  _config = config;

  uint32 flags = SDL_WINDOW_OPENGL;
  if(_config._fullscreen)
    flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;

  std::stringstream ss {};
  ss << "{w:" << _config._windowWidth << ",h:" << _config._windowHeight << "}";
  nomad::log->log(Log::info, Log::creating_window, std::string{ss.str()});

  _window = SDL_CreateWindow(
      _config._windowTitle.c_str(), 
      SDL_WINDOWPOS_UNDEFINED,
      SDL_WINDOWPOS_UNDEFINED,
      _config._windowWidth,
      _config._windowHeight,
      flags
  );

  if(_window == nullptr){
    nomad::log->log(Log::fatal, Log::failed_to_create_window, std::string{SDL_GetError()});
    exit(EXIT_FAILURE);
  }

  _glContext = SDL_GL_CreateContext(_window);
  if(_glContext == nullptr){
    nomad::log->log(Log::fatal, Log::failed_to_create_opengl_context, std::string{SDL_GetError()});
    exit(EXIT_FAILURE);
  }

  if(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, _config._openglVersionMajor) < 0){
    nomad::log->log(Log::fatal, Log::opengl_set_attribute_fail, std::string{SDL_GetError()});
    exit(EXIT_FAILURE);
  }
  if(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, _config._openglVersionMinor) < 0){
    nomad::log->log(Log::fatal, Log::opengl_set_attribute_fail, std::string{SDL_GetError()});
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
// ##>RESOURCES                                                                                  //
//===============================================================================================//

Configuration::Configuration(std::initializer_list<Property> properties)
{
  for(const auto& property : properties)
    _properties.emplace(std::make_pair(property.key, property));
}

void Configuration::clearProperties()
{
  _properties.clear();
}

bool removeProperty(int32_t key)
{
  return _properties.erase(key);
}

void Configuration::addProperty(const Property& property)
{
  auto& search = _properties.find(property.key);
  if(search == _properties.end()){
    _properties.emplace(std::make_pair(property.key, property));
    return true;
  }
  return false;
}

bool Configuration::hasProperty(int32_t key) const
{
  return _properties.find(key) == _properties.end();
}

bool Configuration::load(const std::string& filename, bool loadComments)
{
  std::ifstream file {filename};
  if(!file){
    log->log(Log::ERROR, logstr::fail_open_config, std::string{filename});
    return false;
  }

  auto lineNoToString [](int32_t l){return std::string{"["} + std::to_string(l) + "]";}
  auto isSpace = [](char c){return std::isspace<char>(c, std::locale::classic());};

  int32_t lineNo {0};

  for(std::string line; std::getline(file, line);){
    ++lineNo;

    line.erase(std::remove_if(line.begin(), line.end(), isSpace), line.end());

    if(line.front() == comment) 
      continue;
     
    int32 count {0};
    count = std::count(line.begin(), line.end(), seperator);
    if(count != 1){
      log->log(Log::ERROR, logstr::fail_malformed_config, lineNoToString(lineNo) + line);
      log->log(Log::INFO, logstr::info_unexpected_seperators, std::string{seperator});
      continue;
    }

    std::size_t pos = line.find_first_of(seperator);
    std::string keyword {line.substr(0, pos)};
    std::string value {line.substr(pos + 1)};

    if(keyword.empty() || value.empty()){
      log->log(Log::ERROR, logstr::fail_malformed_config, lineNoToString(lineNo) + line);
      log->log(Log::INFO, logstr::info_incomplete_property);
      continue;
    }

    Property* p {nullptr};
    for(auto& pair : _properties){
      if(pair.second._keyword == keyword){
        p = &pair.second;
        break;
      }
    }

    if(p == nullptr){
      log->log(Log::ERROR, logstr::unkown_config_property, lineNoToString(lineNo) + keyword);
      continue;
    }

    switch(p->_type){
      case INT_PROPERTY:
        {
          int32_t result {0};
          if(!parseInt(value, result)){
            log->log(Log::ERROR, logstr::fail_malformed_dataset, lineNoToString(lineNo) + line);
            log->log(Log::INFO, logstr::expected_integer, value);
            continue;
          }
          p->_value = result;
          break;
        }
      case FLOAT_PROPERTY:
        {
          float result {0.f};
          if(!parseFloat(value, result)){
            log->log(Log::ERROR, logstr::fail_malformed_dataset, lineNoToString(lineNo) + line);
            log->log(Log::INFO, logstr::expected_float, value);
            continue;
          }
          p->_value = result;
          break;
        }
      case BOOL_PROPERTY:
        {
          bool result {0.f};
          if(!parseBool(value, result)){
            log->log(Log::ERROR, logstr::fail_malformed_dataset, lineNoToString(lineNo) + line);
            log->log(Log::INFO, logstr::expected_bool, value);
            continue;
          }
          p->_value = result;
          break;
        }
    };

  }

  return true;
}

bool Configuration::write(const std::string& filename)
{
}

int32_t Configuration::lookupIntValue(int32_t key) const
{
}

int32_t Configuration::lookupIntDefault(int32_t key) const
{
}

int32_t Configuration::lookupIntMax(int32_t key) const
{
}

int32_t Configuration::lookupIntMin(int32_t key) const
{
}

float Configuration::lookupFloatValue(int32_t key) const
{
}

float Configuration::lookupFloatDefault(int32_t key) const
{
}

float Configuration::lookupFloatMax(int32_t key) const
{
}

float Configuration::lookupFloatMin(int32_t key) const
{
}

bool Configuration::lookupBoolValue(int32_t key) const
{
}

bool Configuration::lookupBoolDefault(int32_t key) const
{
}

void Configuration::setComment(int32_t key, const std::string& comment)
{
}

void Configuration::setIntValue(int32_t key)
{
}

void Configuration::setIntDefault(int32_t key)
{
}

void Configuration::setIntMax(int32_t key)
{
}

void Configuration::setIntMin(int32_t key)
{
}

void Configuration::setFloatValue(int32_t key)
{
}

void Configuration::setFloatDefault(int32_t key)
{
}

void Configuration::setFloatMax(int32_t key)
{
}

void Configuration::setFloatMin(int32_t key)
{
}

void Configuration::setBoolValue(int32_t key)
{
}

void Configuration::setBoolDefault(int32_t key)
{
}

bool Configuration::parseInt(const std::string& value, int32_t& result)
{
  auto isDigit = [](unsigned char c){return std::isdigit(c);};
  auto isSign = [](unsigned char c){return c == '+' || c == '-';}

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

bool Configuration::parseFloat(const std::string& value, float& result)
{
  auto isDigit = [](unsigned char c){return std::isdigit(c);};
  auto isSign = [](unsigned char c){return c == '+' || c == '-';}

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

  count = std::count_if(value.begin(), value.end(), isDigit);
  if(count != value.length() - nPoints - nSigns)
    return false;

  result = std::stof(value);
  return true;
}

bool Configuration::parseBool(const std::string& value, bool& result)
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

void Assets::loadBitmaps(const std::vector<std::string>& manifest, int32 scale)
{
  auto isSpace = [](char ch){return std::isspace<char>(ch, std::locale::classic());};
  auto isBinary = [](char ch){return ch == '0' || ch == '1';};

  bool fail {false};
  std::string path {};
  for(const auto& key : manifest){
    if(_bitmaps.find(key) != _bitmaps.end())
      continue;

    path.clear();
    path += bitmaps_path;
    path += key;
    path += bitmaps_extension;
    std::ifstream bitmap {path};
    if(!bitmap){
      nomad::log->log(Log::fatal, Log::missing_asset, path);
      fail = true;
      continue;
    }

    bool malformed {false};
    std::vector<std::string> rows {};
    for(std::string row; std::getline(bitmap, row);){
      row.erase(std::remove_if(row.begin(), row.end(), isSpace), row.end());
      int32 count = std::count_if(row.begin(), row.end(), isBinary);
      if(count != row.length()){
        nomad::log->log(Log::fatal, Log::malformed_bitmap, path);
        malformed = true;
        break;
      }
      rows.push_back(row);
    }
    if(malformed) continue;
    std::reverse(rows.begin(), rows.end()); 
    
    _bitmaps.emplace(std::make_pair(key, Bitmap{rows, scale}));
  }

  if(fail)
    exit(EXIT_FAILURE);
}

const Font& Assets::getFont(const std::string& key, int32 scale) const
{

}

std::unique_ptr<Assets> assets {nullptr};

//===============================================================================================//
// ##>APPLICATION                                                                                //
//===============================================================================================//

void Application::onWindowResize(int32 windowWidth, int32 windowHeight)
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

bool Application::initialize(Engine* engine, int32 windowWidth, int32 windowHeight)
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
  _dt = Duration_t{static_cast<int64>(realDt.count() * _scale)};
  _now += _dt;
  return _dt;
}

int64 Engine::Metronome::doTicks(Duration_t gameNow)
{
  int ticks {0};
  while(_lastTickNow + _tickPeriod < gameNow) {
    _lastTickNow += _tickPeriod;
    ++ticks;
  }
  _totalTicks += ticks;
  return ticks;
}

void Engine::TPSMeter::recordTicks(Duration_t realDt, int32 ticks)
{
  _timer += realDt;
  _ticks += ticks;
  if(_timer > oneSecond){
    _tps = _ticks;
    _ticks = 0;
    _timer = Duration_t::zero();
  }
}

void Engine::Config::load()
{
  Dataset ds;
  if(!ds.load(filename)){
    log->log(Log::INFO, logstr::info_using_default_config);
    return;
  }

  std::string key{};
  for(auto& property : _properties){
    key += property._key;
    if(ds.hasProperty(key)){
      property._value = ds.getProperty(key);
      ds.eraseProperty(key);
      log->log(Log::INFO, logstr::info_set_config_property, key);
    }
    key.clear();
  }

  for(const auto& pair : ds.getProperties()){
    log->log(Log::INFO, logstr::info_unkown_config_property, pair.first);
  }
}

void Engine::initialize(std::unique_ptr<Application>&& app)
{
  nomad::log = std::make_unique<Log>();

  _app = std::move(app);

  _config.load();

  if(SDL_Init(SDL_INIT_VIDEO) < 0){
    nomad::log->log(Log::fatal, Log::sdl2_init_failed, std::string{SDL_GetError()});
    exit(EXIT_FAILURE);
  }

  LoopTick* tick = &_loopTicks[LOOPTICK_UPDATE];
  tick->_onTick = &Engine::onUpdateTick;
  tick->_metronome.setTickPeriod(Duration_t{static_cast<int64>(1.0e9 / 60.0_hz)});
  tick->_ticksAccumulated = 0;
  tick->_ticksDoneThisFrame = 0;
  tick->_maxTicksPerFrame = 5;
  tick->_tickPeriod = 1.0 / 60.0_hz;

  tick = &_loopTicks[LOOPTICK_DRAW];
  tick->_onTick = &Engine::onDrawTick;
  tick->_metronome.setTickPeriod(Duration_t{static_cast<int64>(1.0e9 / 60.0_hz)});
  tick->_ticksAccumulated = 0;
  tick->_ticksDoneThisFrame = 0;
  tick->_maxTicksPerFrame = 1;
  tick->_tickPeriod = 1.0 / 60.0_hz;

  nomad::input = std::make_unique<Input>();
  nomad::assets = std::make_unique<Assets>();

  std::stringstream ss {};
  ss << _app->getName() 
     << " version:" 
     << _app->getVersionMajor() 
     << "." 
     << _app->getVersionMinor();

  Renderer::Config rconfig {
    std::string{ss.str()},
    _config[Config::WINDOW_WIDTH],
    _config[Config::WINDOW_HEIGHT],
    _config[Config::OPENGL_MAJOR],
    _config[Config::OPENGL_MINOR],
    _config[Config::FULLSCREEN]
  };

  nomad::renderer = std::make_unique<Renderer>(rconfig);

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

  for(int32 i = LOOPTICK_UPDATE; i < LOOPTICK_COUNT; ++i){
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

void Engine::generateDefaultConfiguration()
{
  _config.clear();
  _config.addProperties({
    {CKEY_WINDOW_WIDTH, "windowWidth", "", {0}, {500}, {300}, {1000}, Configuration::INT_PROPERTY},
    {CKEY_WINDOW_HEIGHT, {500}, {300}, {1000}, Configuration::INT_PROPERTY},
    {CKEY_FULLSCREEN, {true}, {false}, {true}, Configuration::INT_PROPERTY},
    {CKEY_WINDOW_WIDTH, {500}, {300}, {1000}, Configuration::INT_PROPERTY},
    {CKEY_WINDOW_WIDTH, {500}, {300}, {1000}, Configuration::INT_PROPERTY}
  });
}

} // namespace nomad


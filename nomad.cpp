

#include "nomad.h"

namespace nomad
{

//===============================================================================================//
//                                                                                               //
// ##>LOG                                                                                        //
//                                                                                               //
//===============================================================================================//

Log::Log()
{
  _os.open(filename, std::ios_base::trunc);
  if(!_os)
    log(error, failed_to_open_log_file);
}

Log::~Log()
{
  if(_os)
    _os.close();
}

void Log::log(const char* prefix, const char* error, const std::string& addendum)
{
  std::ostream& o {_os ? _os : std::cerr}; 
  o << prefix << delim << error;
  if(!addendum.empty())
    o << delim << addendum;
  o << std::endl;
}

std::unique_ptr<Log> log {nullptr};

//===============================================================================================//
//                                                                                               //
// ##>INPUT MANAGER                                                                              //
//                                                                                               //
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
//                                                                                               //
// ##>BITMAP                                                                                     //
//                                                                                               //
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

//===============================================================================================//
//                                                                                               //
// ##>FONT                                                                                       //
//                                                                                               //
//===============================================================================================//

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

//===============================================================================================//
//                                                                                               //
// ##>ASSETS                                                                                     //
//                                                                                               //
//===============================================================================================//

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
//                                                                                               //
// ##>RENDERER                                                                                   //
//                                                                                               //
//===============================================================================================//

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
//                                                                                               //
// ##>APPLICATION                                                                                //
//                                                                                               //
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
//                                                                                               //
// ##>ENGINE                                                                                     //
//                                                                                               //
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

const std::string Engine::Config::filename {"config"};
const std::string Engine::Config::key_window_width {"window_width"};
const std::string Engine::Config::key_window_height {"window_height"};
const std::string Engine::Config::key_fullscreen {"fullscreen"};
const std::string Engine::Config::key_opengl_major {"opengl_major"};
const std::string Engine::Config::key_opengl_minor {"opengl_minor"};

Engine::Config::Config()
{
  _properties.emplace(std::make_pair(key_window_width, defaultWindowWidth));
  _properties.emplace(std::make_pair(key_window_height, defaultWindowHeight));
  _properties.emplace(std::make_pair(key_fullscreen, defaultFullscreen));
  _properties.emplace(std::make_pair(key_opengl_major, defaultOpenglMajor));
  _properties.emplace(std::make_pair(key_opengl_minor, defaultOpenglMinor));
}

void Engine::Config::load()
{
  std::ifstream config {filename};
  if(!config){
    nomad::log->log(Log::error, Log::failed_to_open_config_file, std::string{filename});
    return;
  }

  auto isSpace = [](char c){return std::isspace<char>(c, std::locale::classic());};
  auto isDigit = [](unsigned char c){return std::isdigit(c);};

  for(std::string line; std::getline(config, line);){
    line.erase(std::remove_if(line.begin(), line.end(), isSpace), line.end());

    if(line.front() == comment) 
      continue;
     
    int32 count {0};
    count = std::count(line.begin(), line.end(), seperator);
    if(count != 1){
      nomad::log->log(Log::warning, Log::malformed_config_line, line);
      continue;
    }

    std::size_t pos = line.find_first_of(seperator);
    std::string key {line.substr(0, pos)};
    std::string value {line.substr(pos + 1)};

    if(key.empty() || value.empty()){
      nomad::log->log(Log::warning, Log::malformed_config_line, line);
      continue; 
    }

    auto iter = _properties.find(key);
    if(iter == _properties.end()){
      nomad::log->log(Log::warning, Log::unkown_config_key, key);
      continue;
    }
     
    count = std::count_if(value.begin(), value.end(), isDigit);
    if(count != value.length()){
      nomad::log->log(Log::warning, Log::malformed_config_value, value);
      continue;
    }

    iter->second = std::stoi(value);
    nomad::log->log(Log::info, Log::set_config_property, line);
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
    _config.getProperty(Config::key_window_width),
    _config.getProperty(Config::key_window_height),
    _config.getProperty(Config::key_opengl_major),
    _config.getProperty(Config::key_opengl_minor),
    _config.getProperty(Config::key_fullscreen),
  };

  nomad::renderer = std::make_unique<Renderer>(rconfig);

  Vector2i windowSize = nomad::renderer->getWindowSize();

  //
  //
  // TODO - remove this -- temp!!!
  //
  //
  //
  //
  std::cout << "w: " << windowSize._x << " h: " << windowSize._y << std::endl;

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

} // namespace nomad


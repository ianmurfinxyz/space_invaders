
#include <chrono>
#include <thread>
#include <cstdint>
#include <iostream>
#include <algorithm>
#include <cassert>
#include <string>
#include <sstream>
#include <cmath>
#include <vector>
#include <memory>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

using int8 = std::int8_t;
using int16 = std::int16_t;
using int32 = std::int32_t;
using int64 = std::int64_t;
using uint8 = std::uint8_t;
using uint16 = std::uint16_t;
using uint32 = std::uint32_t;
using uint64 = std::uint64_t;

//===============================================================================================//
//                                                                                               //
// ##>MATH                                                                                       //
//                                                                                               //
//===============================================================================================//

constexpr long double operator"" _hz(long double hz){return hz;}

template<typename T>
struct Vector2
{
  T _x;
  T _y;

  Vector2() : _x{0}, _y{0} {}
  Vector2(T x, T y) : _x{x}, _y{y} {}
  void zero() {_x = _y = 0;}
  bool isZero() {return _x == 0 && _y == 0;}
  Vector2 operator+(const Vector2& v) const {return Vector2{_x + v._x, _y + v._y};}
  void operator+=(const Vector2& v) {_x += v._x; _y += v._y;}
  Vector2 operator-(const Vector2& v) const {return Vector2{_x - v._x, _y - v._y};}
  void operator-=(const Vector2& v) {_x -= v._x; _y -= v._y;}
  Vector2 operator*(float scale) const {return Vector2{_x * scale, _y * scale};}
  void operator*=(float scale) {_x *= scale; _y *= scale;}
  float dot(const Vector2& v) {return (_x * v._x) + (_y * v._y);}
  float cross(const Vector2& v) const {return (_x * v._y) - (_y * v._x);}
  float length() const {return std::hypot(_x, _y);}
  float lengthSquared() const {return (_x * _x) + (_y * _y);}

  Vector2 normalized() const
  {
    Vector2 v = *this;
    v.normalize();
    return v;
  }

  void normalize()
  {
    float l = (_x * _x) + (_y * _y);
    if(l) {
      l = std::sqrt(l);
      _x /= l;
      _y /= l;
    }
  }
};

using Vector2f = Vector2<float>;
using Vector2i = Vector2<int32>;

template<typename T>
struct Rect
{
  T _x;
  T _y;
  T _w;
  T _h;
};

using iRect = Rect<int32>;
using fRect = Rect<float>;

//===============================================================================================//
//                                                                                               //
// ##>COLORS                                                                                     //
//                                                                                               //
//===============================================================================================//

class Color3f
{
  constexpr static float lo {0.f};
  constexpr static float hi {1.f};

public:
  Color3f(float r, float g, float b) : 
    _r{std::clamp(r, lo, hi)},
    _g{std::clamp(g, lo, hi)},
    _b{std::clamp(b, lo, hi)}
  {}

  float getRed() const {return _r;}
  float getGreen() const {return _g;}
  float getBlue() const {return _b;}
  void setRed(float r){_r = std::clamp(r, lo, hi);}
  void setGreen(float g){_g = std::clamp(g, lo, hi);}
  void setBlue(float b){_b = std::clamp(b, lo, hi);}

private:
  float _r;
  float _g;
  float _b;
};

namespace colors
{
  const Color3f white {1.f, 1.f, 1.f};
  const Color3f black {0.f, 0.f, 0.f};
  const Color3f red {1.f, 0.f, 0.f};
  const Color3f green {0.f, 1.f, 0.f};
  const Color3f blue {0.f, 0.f, 1.f};
  const Color3f cyan {0.f, 1.f, 1.f};
  const Color3f magenta {1.f, 0.f, 1.f};
  const Color3f yellow {1.f, 1.f, 0.f};
}

//===============================================================================================//
//                                                                                               //
// ##>BITMAP                                                                                     //
//                                                                                               //
//===============================================================================================//

// Bits in the bitmap are accessible via a [row][col] position mapped to screen space like:
//
//          row                               
//           ^
//           |                              y
//         7 | | | |█|█| | | |              ^
//         6 | | |█|█|█|█| | |              |      screen-space
//         5 | |█|█|█|█|█|█| |              |         axes
//         4 |█|█| |█|█| |█|█|       ==>    |
//         3 |█|█|█|█|█|█|█|█|              |
//         2 | | |█| | |█| | |              +----------> x
//         1 | |█| |█|█| |█| |
//         0 |█| |█| | |█| |█|           i.e bit[0][0] is the bottom-left most bit.
//           +-----------------> col
//            0 1 2 3 4 5 6 7
class Bitmap
{
public:
  constexpr static int32 scaleMax {8};

  Bitmap() : _width{0}, _height{0} {};
  Bitmap(std::vector<std::string> bits, int32 scale = 1);
  Bitmap(const Bitmap&) = default;
  Bitmap& operator=(const Bitmap&) = default;
  bool getBit(int32 row, int32 col);
  void setBit(int32 row, int32 col, bool value, bool regen = true);
  int32 getWidth() const {return _width;}
  int32 getHeight() const {return _height;}
  const std::vector<uint8>& getBytes() const {return _bytes;}
  void print(std::ostream& out) const;

private:
  void regenBytes();

private:
  std::vector<std::vector<bool>> _bits;  // used for bit manipulation ops
  std::vector<uint8> _bytes;             // used for rendering
  int32 _width;
  int32 _height;
};

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

#include "bitmaps.h"

//===============================================================================================//
//                                                                                               //
// ##>RENDERER                                                                                   //
//                                                                                               //
//===============================================================================================//

class Renderer
{
public:
  struct Config
  {
    std::string _windowTitle;
    int32 _windowWidth;
    int32 _windowHeight;
    int32 openglVersionMajor;
    int32 openglVersionMinor;
  };
  
public:
  Renderer(const Config& config);
  Renderer(const Renderer&) = delete;
  Renderer* operator=(const Renderer&) = delete;
  ~Renderer();
  void setViewport(iRect viewport);
  void blitText(Vector2f position, const std::string& text, const Color3f& color);
  void blitBitmap(Vector2f position, const Bitmap& bitmap, const Color3f& color);
  void clearWindow(const Color3f& color);
  void clearViewport(const Color3f& color);
  void show();

private:
  SDL_Window* _window;
  SDL_GLContext _glContext;
  Config _config;
  iRect _viewport;
};

Renderer::Renderer(const Config& config)
{
  _config = config;

  _window = SDL_CreateWindow(
      _config._windowTitle.c_str(), 
      SDL_WINDOWPOS_UNDEFINED,
      SDL_WINDOWPOS_UNDEFINED,
      _config._windowWidth,
      _config._windowHeight,
      SDL_WINDOW_OPENGL
  );

  if(_window == nullptr){
    // TODO: log error
    exit(EXIT_FAILURE);
  }

  _glContext = SDL_GL_CreateContext(_window);
  if(_glContext == nullptr){
    // TODO: log error
    exit(EXIT_FAILURE);
  }

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, _config.openglVersionMajor);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, _config.openglVersionMinor);
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

//===============================================================================================//
//                                                                                               //
// ##>APPLICATION                                                                                //
//                                                                                               //
//===============================================================================================//

class Application
{
public:
  using Clock_t = std::chrono::steady_clock;
  using TimePoint_t = std::chrono::time_point<Clock_t>;
  using Duration_t = std::chrono::nanoseconds;

  constexpr static Duration_t oneSecond {1'000'000};
  constexpr static Duration_t minFramePeriod {2'000'000};

  class RealClock
  {
  public:
    explicit RealClock() : _start{}, _now0{}, _now1{}, _dt{}{}
    ~RealClock() = default;
    void start(){_now0 = Clock_t::now();}

    Duration_t update()
    {
      _now1 = Clock_t::now();
      _dt = _now1 - _now0;
      _now1 = _now0;
      return _dt;
    }

    Duration_t getDt() const {return _dt;}
    Duration_t getTimeSinceStart() {return Clock_t::now() - _start;}

  private:
    TimePoint_t _start;
    TimePoint_t _now0;
    TimePoint_t _now1;
    Duration_t _dt;
  };

  class GameClock
  {
  public:
    explicit GameClock() : _now{}, _scale{1.f}, _isPaused{false}{}
    ~GameClock() = default;

    Duration_t update(Duration_t realDt)
    {
      if(_isPaused) return Duration_t::zero();
      _dt = Duration_t{static_cast<int64>(realDt.count() * _scale)};
      _now += _dt;
      return _dt;
    }

    Duration_t getNow() const {return _now;}
    Duration_t getDt() const {return _dt;}
    void incrementScale(float increment){_scale += increment;}
    void setScale(float scale){_scale = scale;}
    float getScale() const {return _scale;}
    void pause(){_isPaused = true;}
    void unpause(){_isPaused = false;}
    void togglePause(){_isPaused = !_isPaused;}
    bool isPaused() const {return _isPaused;}

  private:
    Duration_t _now;
    Duration_t _dt;
    float _scale;
    bool _isPaused;
  };

  class Metronome
  {
  public:
    explicit Metronome() : _lastTickNow{}, _tickPeriod{}, _totalTicks{0}{}
    ~Metronome() = default;

    int64 doTicks(Duration_t gameNow)
    {
      int ticks {0};
      while(_lastTickNow + _tickPeriod < gameNow) {
        _lastTickNow += _tickPeriod;
        ++ticks;
      }
      _totalTicks += ticks;
      return ticks;
    }

    void setTickPeriod(Duration_t period) {_tickPeriod += period;}
    Duration_t getTickPeriod() const {return _tickPeriod;}
    int64 getTotalTicks() const {return _totalTicks;}

  private:
    Duration_t _lastTickNow;
    Duration_t _tickPeriod;
    int64 _totalTicks;
  };

  class TPSMeter
  {
  public:
    explicit TPSMeter() : _timer{}, _ticks{0}, _tps{0} {}
    ~TPSMeter() = default;

    void recordTicks(Duration_t realDt, int32 ticks)
    {
      _timer += realDt;
      _ticks += ticks;
      if(_timer > oneSecond){
        _tps = _ticks;
        _ticks = 0;
        _timer = Duration_t::zero();
      }
    }

    int32 getTPS() const {return _tps;}
    
  private:
    Duration_t _timer;
    int32 _ticks;
    int32 _tps;
  };

  enum class ApplicationStates {MENU, GAME};

  class ApplicationState
  {
  public:
    ApplicationState(Application* app) : _app(app) {}
    virtual ~ApplicationState() = default;
    virtual void onUpdate(double now, float dt) = 0;
    virtual void onDraw(double now, float dt) = 0;
    virtual void onReset() = 0;

  protected:
    Application* _app;
  };

  class GameState : public ApplicationState
  {
  public:
    GameState(Application* app) : ApplicationState{app}{}
    virtual ~GameState() = default;
    virtual const std::string& getName() = 0;
    virtual int32 getVersionMajor() = 0;
    virtual int32 getVersionMinor() = 0;
  };

  enum LoopTicks {LOOPTICK_UPDATE, LOOPTICK_DRAW, LOOPTICK_COUNT};

  struct LoopTick
  {
    explicit LoopTick() : _tpsMeter{}, _metronome{}, _ticksAccumulated{0}, _ticksDoneThisFrame{0},
      _maxTicksPerFrame{0}, _tickPeriod{0.f} {}

    void (Application::*_onTick)(Duration_t, Duration_t, Duration_t, float);
    TPSMeter _tpsMeter;
    Metronome _metronome;
    int64 _ticksAccumulated;
    int32 _ticksDoneThisFrame;
    int32 _maxTicksPerFrame;
    float _tickPeriod;
  };

public:
  Application() = default;
  ~Application() = default;
  void initialize(std::unique_ptr<ApplicationState>&& menu, std::unique_ptr<GameState>&& game);
  void shutdown();
  void run();

  void transitionToAppState(ApplicationStates newState);

  std::unique_ptr<Renderer>& getRenderer() {return _renderer;}

private:
  void mainloop();
  void drawPerformanceStats(Duration_t realDt, Duration_t gameDt);
  void onUpdateTick(Duration_t gameNow, Duration_t gameDt, Duration_t realDt, float tickDt);
  void onDrawTick(Duration_t gameNow, Duration_t gameDt, Duration_t realDt, float tickDt);

private:
  RealClock _realClock;
  GameClock _gameClock;

  ApplicationStates _activeAppState;
  std::unique_ptr<ApplicationState> _menu;
  std::unique_ptr<GameState> _game;

  std::array<LoopTick, LOOPTICK_COUNT> _loopTicks;

  std::unique_ptr<Renderer> _renderer;

  bool _isSleeping;
  bool _isDrawingPerformanceStats;
  bool _isDone;
};

void Application::initialize(std::unique_ptr<ApplicationState>&& menu, std::unique_ptr<GameState>&& game)
{
  if(SDL_Init(SDL_INIT_VIDEO) < 0){
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to initiaize SDL: %s", SDL_GetError());
    exit(EXIT_FAILURE);
  }

  _menu = std::move(menu);
  _game = std::move(game);

  _activeAppState = ApplicationStates::MENU;

  LoopTick* tick = &_loopTicks[LOOPTICK_UPDATE];
  tick->_onTick = &Application::onUpdateTick;
  tick->_metronome.setTickPeriod(Duration_t{static_cast<int64>(1.0e9 / 60.0_hz)});
  tick->_maxTicksPerFrame = 5;
  tick->_tickPeriod = 1.0 / 60.0_hz;

  tick = &_loopTicks[LOOPTICK_DRAW];
  tick->_onTick = &Application::onDrawTick;
  tick->_metronome.setTickPeriod(Duration_t{static_cast<int64>(1.0e9 / 60.0_hz)});
  tick->_maxTicksPerFrame = 1;
  tick->_tickPeriod = 1.0 / 60.0_hz;

  std::stringstream ss {};
  ss << _game->getName() << " version:" << _game->getVersionMajor() << "." << _game->getVersionMinor();
  Renderer::Config rencfg {
    std::string{ss.str()},
    800,
    600,   // TODO: load from config file? or something
    2,
    1
  };
  _renderer = std::move(std::unique_ptr<Renderer>{new Renderer(rencfg)});

  _isSleeping = true;
  _isDrawingPerformanceStats = false;
  _isDone = false;
}

void Application::shutdown()
{

}

void Application::run()
{
  _realClock.start();
  while(!_isDone) mainloop();
}

void Application::mainloop()
{
  auto now0 = Clock_t::now();
  auto realDt = _realClock.update();
  auto gameDt = _gameClock.update(realDt);
  auto gameNow = _gameClock.getNow();

  // handle user input events

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
  
  if(_isSleeping){
    auto now1 {Clock_t::now()};
    auto framePeriod {now1 - now0};
    if(framePeriod < minFramePeriod)
      std::this_thread::sleep_for(minFramePeriod - framePeriod); 
  }
}

void Application::drawPerformanceStats(Duration_t realDt, Duration_t gameDt)
{

}

void Application::onUpdateTick(Duration_t gameNow, Duration_t gameDt, Duration_t realDt, float tickDt)
{
  double now = static_cast<double>(gameNow.count()) / static_cast<double>(oneSecond.count());

  switch(_activeAppState){
    case ApplicationStates::MENU:
      _menu->onUpdate(now, tickDt);
      break;

    case ApplicationStates::GAME:
      _game->onUpdate(now, tickDt);
      break;
  }
}

void Application::onDrawTick(Duration_t gameNow, Duration_t gameDt, Duration_t realDt, float tickDt)
{
  // TODO - temp - clear the game viewport only in the game and menu states - only clear window
  // when toggle perf stats
  _renderer->clearWindow(colors::red);

  double now = static_cast<double>(gameNow.count()) / static_cast<double>(oneSecond.count());

  switch(_activeAppState){
    case ApplicationStates::MENU:
      _menu->onDraw(now, tickDt);
      break;

    case ApplicationStates::GAME:
      _game->onDraw(now, tickDt);
      break;
  }

  if(_isDrawingPerformanceStats)
    drawPerformanceStats(realDt, gameDt);

  _renderer->show();
}

//===============================================================================================//
//                                                                                               //
// ##>GAME                                                                                       //
//                                                                                               //
//===============================================================================================//

class Game final : public Application::GameState
{
  constexpr static int32 versionMajor {0};
  constexpr static int32 versionMinor {1};

  const static std::string name;

public:
  Game(Application* app) : Application::GameState{app}{}
  ~Game() = default;

  void onUpdate(double now, float dt);
  void onDraw(double now, float dt);
  void onReset();

  const std::string& getName() {return name;}
  int32 getVersionMajor() {return versionMajor;}
  int32 getVersionMinor() {return versionMinor;}

private:
};

const std::string Game::name {"Space Invaders"};

void Game::onUpdate(double now, float dt)
{
}

void Game::onDraw(double now, float dt)
{

}

void Game::onReset()
{
}

//===============================================================================================//
//                                                                                               //
// ##>MENU                                                                                       //
//                                                                                               //
//===============================================================================================//

class Menu final : public Application::ApplicationState
{
public:
  Menu(Application* app) : Application::ApplicationState{app}{}
  ~Menu() = default;
  void onUpdate(double now, float dt);
  void onDraw(double now, float dt);
  void onReset();

private:
};

void Menu::onUpdate(double now, float dt)
{
}

void Menu::onDraw(double now, float dt)
{
  std::unique_ptr<Renderer>& renderer = _app->getRenderer();
  renderer->blitBitmap(Vector2f{100.f, 100.f}, bitmaps::cannon0, colors::cyan);
  renderer->blitBitmap(Vector2f{150.f, 100.f}, bitmaps::squid0, colors::green);
}

void Menu::onReset()
{
}

//===============================================================================================//
//                                                                                               //
// ##>MAIN                                                                                       //
//                                                                                               //
//===============================================================================================//

static Application* app {nullptr};

void boot()
{
  ::app = new Application{};

  std::unique_ptr<Application::ApplicationState> menu{new Menu{::app}};
  std::unique_ptr<Application::GameState> game{new Game{::app}};

  ::app->initialize(std::move(menu), std::move(game));
}

int main(int argc, char* arv[])
{
  boot();
  ::app->run();
  ::app->shutdown();
  delete ::app;
}

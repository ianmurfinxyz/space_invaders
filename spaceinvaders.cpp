
#include <chrono>
#include <cstdint>
#include <iostream>
#include <algorithm>
#include <cassert>

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

template<typename T>
struct Vector2
{
  T _x;
  T _y;

  Vector2() : _x{0}, _y{0} {}
  Vector2(T x, T y) : _x{x}, _y{y} {}
  void zero() {_x = _y = 0}
  bool isZero() {return _x == 0 && y == 0;}
  Vector2 operator+(const Vector2& v) const {return Vector2{_x + v._x, _y + v._y};}
  void operator+=(const Vector2& v) {_x += v._x; _y += v._y;}
  Vector2 operator-(const Vector2& v) const {return Vector2{_x - v._x, _y - v._y};}
  void operator-=(const Vector2& v) {_x -= v._x; _y -= v._y;}
  Vector2 operator*(float scale) const {return Vector2{_x * scale, _y * scale};}
  void operator*=(float scale) {_x *= scale; _y *= scale;}
  float dot(const Vector2& v) {return (_x * v._x) + (_y * v._y);}
  float cross(const Vector2& v) const {return (_x * v._y) - (_y * v._x);}
  float length() const {return std::hypt(_x, _y);}
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

template<typename T, T lo, T hi>
class Color3
{
public:
  Color3(T r, T g, T b) : 
    _r{std::clamp(r, lo, hi)},
    _g{std::clamp(g, lo, hi)},
    _b{std::clamp(b, lo, hi)}
  {}

  T getRed() const {return _r;}
  T getGreen() const {return _g;}
  T getBlue() const {return _b;}
  void setRed(T r){_r = std::clamp(r, lo, hi);}
  void setGreen(T g){_g = std::clamp(g, lo, hi);}
  void setBlue(T b){_b = std::clamp(b, lo, hi);}

private:
  T _r;
  T _g;
  T _b;
};

using Color3f = Color3<float, 0.f, 1.f>;
using Colori = Color<int32, 0, UINT8_MAX>;

namespace Colors
{
  Color3f white {1.f, 1.f, 1.f};
  Color3f black {0.f, 0.f, 0.f};
  Color3f red {1.f, 0.f, 0.f};
  Color3f green {0.f, 1.f, 0.f};
  Color3f blue {0.f, 0.f, 1.f};
  Color3f cyan {0.f, 1.f, 1.f};
  Color3f magenta {1.f, 0.f, 1.f};
  Color3f yellow {1.f, 1.f, 0.f};
}

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
  Renderer(Config config);
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
  SDL_GL_Context _glContext;
  Config _config;
  iRect _viewport;
};

Renderer::Renderer(Config config)
{
  _config = config;

  _window = SDL_CreateWindow(
      _config._windowTitle, 
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

  _glcontext = SDL_GL_CreateContext(_window);
  if(_glcontext == nullptr){
    // TODO: log error
    exit(EXIT_FAILURE);
  }

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, _config.openglVersionMajor);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, _config.openglVersionMinor);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  _viewport = iRect{0, 0, _config._windowWidth, _config._windowHeight};
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

  class RealClock
  {
  public:
    explicit RealClock() : _start{}, _now0{}, _now1{}, _dt{}{}
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

    Duration_t update(Duration_t realDt)
    {
      if(_isPaused) return Duration_t::zero();
      _dt = Duration_t{static_cast<int64>(realDt.count() * _scale)};
      _now += dt;
      return dt;
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
    explicit TPSMeter() : _timer{}, _ticks{0}, _tps{0.f} {}

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

  enum ApplicationStates {APPSTATE_MENU, APPSTATE_GAME, APPSTATE_COUNT};

  class ApplicationState
  {
  public:
    ApplicationState(Application* app) : _app(app) {}
    virtual void onUpdate(double now, float dt) = 0;
    virtual void onDraw(double now, float dt) = 0;
    virtual void onReset();

  private:
    Application* _app;
  };

  class GameState : ApplicationState
  {
  public:
    virtual const std::string& getName() = 0;
    virtual int32 getVersionMajor() = 0;
    virtual int32 getVersionMinor() = 0;
  };

  enum LoopTicks {LOOPTICK_UPDATE, LOOPTICK_DRAW, LOOPTICK_COUNT};

  struct LoopTick
  {
    explicit LoopTick() : _tpsMeter{}, _metronome{}, _ticksAccumulated{0}, _ticksDoneThisFrame{0},
      _maxTicksPerFrame{0}, _tickPeriod{0.f}, _tickFrequency{0.f} {}

    void (ApplicationState*::_onTick)(double, float);
    TpsMeter _tpsMeter;
    Metronome _metronome;
    int64 _ticksAccumulated;
    int32 _ticksDoneThisFrame;
    int32 _maxTicksPerFrame;
    float _tickPeriod;
  };

public:
  Application() = default;
  void initialize(std::unique_ptr<ApplicationState> menu, std::unique_ptr<ApplicationState* game);
  void shutdown();
  void run();

  void transitionToAppState(ApplicationStates newState);

private:
  void mainloop();
  void drawPerformanceStats();

private:
  RealClock _realClock;
  GameClock _gameClock;

  std::array<std::unique_ptr<ApplicationState>, APPSTATE_COUNT> _appStates;
  ApplicationStates _activeAppState;

  std::array<LoopTick, LOOPTICK_COUNT> _loopTicks;

  bool _isDrawingPerformanceStats;
  bool _isDone;
};

void Application::initialize(std::unique_ptr<ApplicationState> menu, std::unique_ptr<ApplicationState* game)
{
  if(SDL_Init(SDL_INIT_VIDEO) < 0){
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to initiaize SDL: %s", SDL_GetError());
    exit(EXIT_FAILURE);
  }

  _appStates[APPSTATE_MENU] = std::move{menu};
  _appStates[APPSTATE_GAME] = std::move{game};

  _activeAppState = APPSTATE_MENU;

  LoopTick* tick = _loopTicks[LOOPTICK_UPDATE];
  tick->_onTick = &ApplicationState::onUpdate;
  tick->_metronome.setTickPeriod(Duration_t{static_cast<int64>(1.0e9 / 60.0_hz)});
  tick->_maxTicksPerFrame = 5;
  tick->_tickPeriod = 1.0 / 60.0_hz;

  tick = _loopTicks[LOOPTICK_DRAW];
  tick->_onTick = &ApplicationState::onDraw;
  tick->_metronome.setTickPeriod(Duration_t{static_cast<int64>(1.0e9 / 60.0_hz)});
  tick->_maxTicksPerFrame = 1;
  tick->_tickPeriod = 1.0 / 60.0_hz;

  _isDrawingPeformanceStats = false;
  _isDone = false;
}

void Application::run()
{
  _realClock.start();
  while(!_isDone) mainloop();
}

void Application::mainloop()
{
  auto realDt = _realClock.update();
  auto gameDt = _gameClock.update(realDt);
  auto gameNow = _gameClock.getNow();

  // handle user input events

  for(int32 i = LOOPTICK_UPDATE; i < LOOPTICK_COUNT; ++i){
    LoopTick* tick = _loopTicks[i];
    tick._ticksAccumulated += tick._metronome.doTicks(gameNow);
    tick._ticksDoneThisFrame = 0;
    while(tick._ticksAccumulated > 0 && tick._ticksDoneThisFrame < tick._maxTicksPerFrame){
      ++tick._ticksDoneThisFrame;
      --tick._ticksAccumulated;
      (_appStates[_activeAppState].get()->*tick.on_tick)(gameNow, tick._tickPeriod);

      if(_isDrawingPeformanceStats && i == LOOPTICK_DRAW) 
        drawPerformanceStats(realDt, gameDt);
    }
    tick._tpsMeter.recordTicks(realDt, tick._ticksDoneThisFrame);
  }

  // handle sleeping
}

//===============================================================================================//
//                                                                                               //
// ##>GAME                                                                                       //
//                                                                                               //
//===============================================================================================//

class Game : public Application::GameState
{
public:
  Game(Application* app) : Application::ApplicationState{app}{}

  void onUpdate(double now, float dt);
  void onDraw(double now, float dt);
  void onReset();

  const std::string& getName() {return _name;}
  int32 getVersionMajor() {return _versionMajor;}
  int32 getVersionMinor() {return _versionMinor;}

private:
  std::string _name;
  int32 _versionMajor;
  int32 _versionMinor;
};

//===============================================================================================//
//                                                                                               //
// ##>MENU                                                                                       //
//                                                                                               //
//===============================================================================================//

class Menu : public Application::ApplicationState
{
public:
  Menu(Application* app) : Application::ApplicationState{app}{}
  void onUpdate(double now, float dt);
  void onDraw(double now, float dt);
  void onReset();

private:

};

//===============================================================================================//
//                                                                                               //
// ##>MAIN                                                                                       //
//                                                                                               //
//===============================================================================================//

int main(int argc, char* arv[])
{
  Application app;

  Menu* menu = new Menu(app);
  Game* game = new Game(app);

  app.initialize(std::unique_ptr{menu}, std::unique_ptr{game});
}

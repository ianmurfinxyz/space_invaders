#ifndef _NOMAD_H_
#define _NOMAD_H_

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
#include <fstream>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

namespace nomad
{

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

  constexpr Vector2() : _x{0}, _y{0} {}
  constexpr Vector2(T x, T y) : _x{x}, _y{y} {}
  void zero() {_x = _y = 0;}
  bool isZero() {return _x == 0 && _y == 0;}
  Vector2 operator+(const Vector2& v) const {return Vector2{_x + v._x, _y + v._y};}
  void operator+=(const Vector2& v) {_x += v._x; _y += v._y;}
  Vector2 operator-(const Vector2& v) const {return Vector2{_x - v._x, _y - v._y};}
  void operator-=(const Vector2& v) {_x -= v._x; _y -= v._y;}
  Vector2 operator*(float scale) const {return Vector2{_x * scale, _y * scale};}
  Vector2 operator*(int32 scale) const {return Vector2{_x * scale, _y * scale};}
  void operator*=(float scale) {_x *= scale; _y *= scale;}
  void operator*=(int32 scale) {_x *= scale; _y *= scale;}
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

} // namespace nomad::colors

//===============================================================================================//
//                                                                                               //
// ##>LOG                                                                                        //
//                                                                                               //
//===============================================================================================//

class Log
{
public:
  static constexpr const char* filename {"log"};
  static constexpr const char* delim {" : "};
  static constexpr const char* fatal {"fatal"};
  static constexpr const char* error {"error"};
  static constexpr const char* warning {"warning"};
  static constexpr const char* info {"info"};
  static constexpr const char* sdl2_init_failed {"failed to initialize SDL2"};
  static constexpr const char* failed_to_create_window {"failed to create sdl window"};
  static constexpr const char* creating_window {"creating window"};
  static constexpr const char* failed_to_create_opengl_context {"failed to create opengl context"};
  static constexpr const char* failed_to_open_log_file {"failed to open log file : redirecting to standard error"};
  static constexpr const char* failed_to_open_config_file {"failed to open config file : using default configuration"};
  static constexpr const char* opengl_set_attribute_fail {"failed to set opengl attribute"};
  static constexpr const char* malformed_config_line {"malformed line in config file; expected key=value pair; ignoring line"};
  static constexpr const char* malformed_config_value {"malformed config property value : integer expected"};
  static constexpr const char* unkown_config_key {"unrecognised config property key"};
  static constexpr const char* set_config_property {"set config property"};
  static constexpr const char* failed_to_open_asset_manifest {"failed to open assets manifest"};
  static constexpr const char* missing_asset {"missing asset"};
  static constexpr const char* malformed_bitmap {"malformed bitmap : expected 1's and 0's"};
  static constexpr const char* failed_to_initialize_app_state {"failed to initialize app state"};

public:
  Log();
  ~Log();
  void log(const char* prefix, const char* error, const std::string& addendum = std::string{});

private:
  std::ofstream _os;
};

extern std::unique_ptr<Log> log;

//===============================================================================================//
//                                                                                               //
// ##>INPUT MANAGER                                                                              //
//                                                                                               //
//===============================================================================================//

class Input
{
public:
  enum KeyCode { 
    KEY_a, KEY_b, KEY_c, KEY_d, KEY_e, KEY_f, KEY_g, KEY_h, KEY_i, KEY_j, KEY_k, KEY_l, KEY_m, 
    KEY_n, KEY_o, KEY_p, KEY_q, KEY_r, KEY_s, KEY_t, KEY_u, KEY_v, KEY_w, KEY_x, KEY_y, KEY_z,
    KEY_SPACE, KEY_ENTER, KEY_LEFT, KEY_RIGHT, KEY_UP, KEY_DOWN, KEY_COUNT 
  };

  enum class KeyState { DOWN, PRESSED, UP, RELEASED };

public:
  Input();
  ~Input() = default;
  void onKeyEvent(const SDL_Event& event);
  void onUpdate();
  KeyState getKeyState(KeyCode key){return _keyStates[key];}

private:
  KeyCode convertSdlKeyCode(int sdlCode);

private:
  std::array<KeyState, KEY_COUNT> _keyStates;
};

extern std::unique_ptr<Input> input;

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

  explicit Bitmap() = default;
  explicit Bitmap(std::vector<std::string> bits, int32 scale = 1);
  Bitmap(const Bitmap&) = default;
  Bitmap& operator=(const Bitmap&) = default;
  Bitmap(Bitmap&&) = default;
  Bitmap& operator=(Bitmap&&) = default;
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

//===============================================================================================//
//                                                                                               //
// ##>FONT                                                                                       //
//                                                                                               //
//===============================================================================================//

class Font
{
public:
  struct Glyph
  {
    Bitmap _bitmap;
    int32 _asciiCode;
    int32 _offsetX;
    int32 _advance;
    int32 _width;
    int32 _height;
  };

  struct Meta
  {
    int32 _lineSpace;
    int32 _wordSpace;
    int32 _glyphSpace;
    int32 _size;
  };

public:
  Font(const std::vector<Glyph>& glyphs, Meta meta);
  Font(const Font&) = default;
  Font& operator=(const Font&) = default;
  const Glyph& getGlyph(char c) const;
  int32 getLineSpace() const {return _meta._lineSpace;}
  int32 getWordSpace() const {return _meta._wordSpace;}
  int32 getGlyphSpace() const {return _meta._glyphSpace;}
  int32 getSize() const {return _meta._size;}

private:
  std::vector<Glyph> _glyphs;
  Meta _meta;
};

//===============================================================================================//
//                                                                                               //
// ##>ASSETS                                                                                     //
//                                                                                               //
//===============================================================================================//

class Assets
{
public:
  static constexpr const char* bitmaps_path {"assets/bitmaps/"};
  static constexpr const char* bitmaps_extension {".bitmap"};

public:
  Assets() = default;
  ~Assets() = default;
  void loadBitmaps(const std::vector<std::string>& manifest, int32 scale);
  void loadFonts(const std::vector<std::string>& manifest, const std::vector<int32>& scales);
  const Bitmap& getBitmap(const std::string& key) {return _bitmaps[key];}
  const Font& getFont(const std::string& key, int32 scale) const;

private:
  std::unordered_map<std::string, Bitmap> _bitmaps;
  std::unordered_map<std::string, std::pair<Font, int32>> _fonts;
};

extern std::unique_ptr<Assets> assets;

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
    int32 _openglVersionMajor;
    int32 _openglVersionMinor;
    bool _fullscreen;
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
  Vector2i getWindowSize() const;

private:
  SDL_Window* _window;
  SDL_GLContext _glContext;
  Config _config;
  iRect _viewport;
};

extern std::unique_ptr<Renderer> renderer;

//===============================================================================================//
//                                                                                               //
// ##>APPLICATION                                                                                //
//                                                                                               //
//===============================================================================================//

class Application;

class ApplicationState
{
public:
  ApplicationState(Application* app) : _app(app) {}
  virtual ~ApplicationState() = default;
  virtual void initialize(Vector2i worldSize) = 0;
  virtual void onUpdate(double now, float dt) = 0;
  virtual void onDraw(double now, float dt) = 0;
  virtual void onReset() = 0;
  virtual const std::string& getName() = 0;

protected:
  Application* _app;
};

class Engine;

class Application
{
public:
  static constexpr Input::KeyCode pauseKey {Input::KEY_p};

public:
  Application() = default;
  virtual ~Application() = default;
  
  virtual bool initialize(Engine* engine, int32 windowWidth, int32 windowHeight);

  virtual const std::string& getName() const = 0;
  virtual int32 getVersionMajor() const = 0;
  virtual int32 getVersionMinor() const = 0;

  void onWindowResize(int32 windowWidth, int32 windowHeight);
  void onUpdate(double now, float dt);
  void onDraw(double now, float dt);

  void switchState(const std::string& name);

protected:
  virtual Vector2i getWorldSize() const = 0;

  void addState(std::unique_ptr<ApplicationState>&& state);

private:
  void drawWindowTooSmall();

private:
  std::unordered_map<std::string, std::unique_ptr<ApplicationState>> _states;
  std::unique_ptr<ApplicationState>* _activeState;
  iRect _viewport;
  Engine* _engine;
  bool _isWindowTooSmall;
};

//===============================================================================================//
//                                                                                               //
// ##>ENGINE                                                                                     //
//                                                                                               //
//===============================================================================================//

class Engine
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
    Duration_t update();
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
    Duration_t update(Duration_t realDt);
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
    int64 doTicks(Duration_t gameNow);
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
    void recordTicks(Duration_t realDt, int32 ticks);
    int32 getTPS() const {return _tps;}
    
  private:
    Duration_t _timer;
    int32 _ticks;
    int32 _tps;
  };

  enum LoopTicks {LOOPTICK_UPDATE, LOOPTICK_DRAW, LOOPTICK_COUNT};

  struct LoopTick
  {
    void (Engine::*_onTick)(Duration_t, Duration_t, Duration_t, float);
    TPSMeter _tpsMeter;
    Metronome _metronome;
    int64 _ticksAccumulated;
    int32 _ticksDoneThisFrame;
    int32 _maxTicksPerFrame;
    float _tickPeriod;
  };

  class Config
  {
  public:
    static constexpr char comment {'#'};
    static constexpr char seperator {'='};

    static const std::string filename;
    static const std::string key_window_width;
    static const std::string key_window_height;
    static const std::string key_fullscreen;
    static const std::string key_opengl_major;
    static const std::string key_opengl_minor;

    static constexpr int32 defaultWindowWidth {800};
    static constexpr int32 defaultWindowHeight {600};
    static constexpr int32 defaultFullscreen {0};
    static constexpr int32 defaultOpenglMajor {2};
    static constexpr int32 defaultOpenglMinor {1};

  public:
    Config();
    void load();
    void setProperty(const std::string& key, int32 value){_properties[key] = value;}
    int32 getProperty(const std::string& key){return _properties[key];}

  private:
    std::unordered_map<std::string, int32> _properties;
  };

public:
  Engine() = default;
  ~Engine() = default;
  void initialize(std::unique_ptr<Application>&& app);
  void run();
  void pause(){_gameClock.pause();}
  void unpause(){_gameClock.unpause();}
  void togglePause(){_gameClock.togglePause();}

private:
  void mainloop();
  void drawPerformanceStats(Duration_t realDt, Duration_t gameDt);
  void drawPauseDialog();
  void onUpdateTick(Duration_t gameNow, Duration_t gameDt, Duration_t realDt, float tickDt);
  void onDrawTick(Duration_t gameNow, Duration_t gameDt, Duration_t realDt, float tickDt);
  void loadConfig();
  double durationToSeconds(Duration_t d);

private:
  std::array<LoopTick, LOOPTICK_COUNT> _loopTicks;
  std::unique_ptr<Application> _app;
  RealClock _realClock;
  GameClock _gameClock;
  Config _config;
  bool _isPaused;
  bool _isSleeping;
  bool _isDrawingPerformanceStats;
  bool _isDone;
};

} // namespace nomad

#endif

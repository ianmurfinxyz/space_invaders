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
#include <initializer_list>
#include <memory>
#include <fstream>
#include <variant>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

namespace nomad
{

//===============================================================================================//
// ##>MATH                                                                                       //
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
  Vector2 operator*(int32_t scale) const {return Vector2{_x * scale, _y * scale};}
  void operator*=(float scale) {_x *= scale; _y *= scale;}
  void operator*=(int32_t scale) {_x *= scale; _y *= scale;}
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
using Vector2i = Vector2<int32_t>;

template<typename T>
struct Rect
{
  T _x;
  T _y;
  T _w;
  T _h;
};

using iRect = Rect<int32_t>;
using fRect = Rect<float>;

//===============================================================================================//
// ##>LOG                                                                                        //
//===============================================================================================//

namespace logstr
{
  constexpr const char* fail_open_log = "failed to open log";
  constexpr const char* fail_sdl_init = "failed to initialize SDL";
  constexpr const char* fail_create_opengl_context = "failed to create opengl context";
  constexpr const char* fail_set_opengl_attribute = "failed to set opengl attribute";
  constexpr const char* fail_create_window = "failed to create window";
  constexpr const char* fail_missing_asset = "missing asset";
  constexpr const char* fail_malformed_bitmap = "malformed bitmap";

  constexpr const char* warn_cannot_open_dataset = "failed to open data file";
  constexpr const char* warn_malformed_dataset = "malformed data file";
  constexpr const char* warn_property_not_set = "property not set";

  constexpr const char* info_stderr_log = "logging to standard error";
  constexpr const char* info_using_default_config = "using default engine configuration";
  constexpr const char* info_fullscreen_mode = "activating fullscreen mode";
  constexpr const char* info_creating_window = "creating window";
  constexpr const char* info_created_window = "window created";
  constexpr const char* info_on_line = "on line";
  constexpr const char* info_unknown_dataset_property = "unkown property";
  constexpr const char* info_unexpected_seperators = "expected a single seperator character";
  constexpr const char* info_incomplete_property = "missing property key or value";
  constexpr const char* info_expected_integer = "expected integer value";
  constexpr const char* info_expected_float = "expected float value";
  constexpr const char* info_expected_bool = "expected bool value";
  constexpr const char* info_ignoring_line = "ignoring line";
  constexpr const char* info_property_set = "dataset property set";
  constexpr const char* info_property_clamped = "property value clamped to min-max range";
  constexpr const char* info_using_property_default = "using property default value";
}; 

class Log
{
public:
  static constexpr const char* filename {"log"};
  static constexpr const char* delim {" : "};

  static constexpr std::array<const char*, 4> lvlstr {"fatal", "error", "warning", "info"};

public:
  enum Level
  {
    FATAL,
    ERROR,
    WARN,
    INFO
  };

public:
  Log();
  ~Log();

  void log(Level level, const char* error, const std::string& addendum = std::string{});

private:
  std::ofstream _os;
};

extern std::unique_ptr<Log> log;


//===============================================================================================//
// ##>INPUT                                                                                      //
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
// ##>GRAPHICS                                                                                   //
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
  constexpr static int32_t scaleMax {8};

  explicit Bitmap() = default;
  explicit Bitmap(std::vector<std::string> bits, int32_t scale = 1);
  Bitmap(const Bitmap&) = default;
  Bitmap& operator=(const Bitmap&) = default;
  Bitmap(Bitmap&&) = default;
  Bitmap& operator=(Bitmap&&) = default;
  bool getBit(int32_t row, int32_t col);
  void setBit(int32_t row, int32_t col, bool value, bool regen = true);
  int32_t getWidth() const {return _width;}
  int32_t getHeight() const {return _height;}
  const std::vector<uint8_t>& getBytes() const {return _bytes;}
  void print(std::ostream& out) const;

private:
  void regenBytes();

private:
  std::vector<std::vector<bool>> _bits;  // used for bit manipulation ops
  std::vector<uint8_t> _bytes;             // used for rendering
  int32_t _width;
  int32_t _height;
};

class Font
{
public:
  struct Glyph
  {
    Bitmap _bitmap;
    int32_t _asciiCode;
    int32_t _offsetX;
    int32_t _advance;
    int32_t _width;
    int32_t _height;
  };

  struct Meta
  {
    int32_t _lineSpace;
    int32_t _wordSpace;
    int32_t _glyphSpace;
    int32_t _size;
  };

public:
  Font(const std::vector<Glyph>& glyphs, Meta meta);
  Font(const Font&) = default;
  Font& operator=(const Font&) = default;
  const Glyph& getGlyph(char c) const;
  int32_t getLineSpace() const {return _meta._lineSpace;}
  int32_t getWordSpace() const {return _meta._wordSpace;}
  int32_t getGlyphSpace() const {return _meta._glyphSpace;}
  int32_t getSize() const {return _meta._size;}

private:
  std::vector<Glyph> _glyphs;
  Meta _meta;
};

class Color3f
{
  constexpr static float lo {0.f};
  constexpr static float hi {1.f};

public:
  constexpr Color3f(float r, float g, float b) : 
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
constexpr Color3f white {1.f, 1.f, 1.f};
constexpr Color3f black {0.f, 0.f, 0.f};
constexpr Color3f red {1.f, 0.f, 0.f};
constexpr Color3f green {0.f, 1.f, 0.f};
constexpr Color3f blue {0.f, 0.f, 1.f};
constexpr Color3f cyan {0.f, 1.f, 1.f};
constexpr Color3f magenta {1.f, 0.f, 1.f};
constexpr Color3f yellow {1.f, 1.f, 0.f};
};

class Renderer
{
public:
  struct Config
  {
    std::string _windowTitle;
    int32_t _windowWidth;
    int32_t _windowHeight;
    int32_t _openglVersionMajor;
    int32_t _openglVersionMinor;
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
// ##>RESOURCES                                                                                  //
//===============================================================================================//

class Dataset
{
public:
  enum Type {INT_PROPERTY, FLOAT_PROPERTY, BOOL_PROPERTY};

  struct Property
  {
    using Value_t = std::variant<int32_t, float, bool>;

    Property() = default;
    Property(int32_t key, std::string name, Value_t default_, Value_t min, Value_t max); 

    int32_t _key;
    std::string _name;
    Value_t _value;
    Value_t _default;
    Value_t _min;
    Value_t _max;
    Type _type;
  };

public:
  static constexpr char comment {'#'};
  static constexpr char seperator {'='};

public:
  bool load(const char* filename);
  bool write(const char* filename, bool genComments = true);

  int32_t getIntValue(int32_t key) const;
  float getFloatValue(int32_t key) const;
  bool getBoolValue(int32_t key) const;

  void setIntValue(int32_t key, int32_t value);
  void setFloatValue(int32_t key, float value);
  void setBoolValue(int32_t key, bool value);

  void scaleIntValue(int32_t key, int32_t scale);
  void scaleFloatValue(int32_t key, float scale);

protected:
  Dataset(std::initializer_list<Property> properties);

  Dataset() = delete;
  Dataset(const Dataset&) = delete;
  Dataset(Dataset&&) = delete;
  Dataset& operator=(const Dataset&) = delete;
  Dataset& operator=(Dataset&&) = delete;

private:
  bool parseInt(const std::string& value, int32_t& result);
  bool parseFloat(const std::string& value, float& result);
  bool parseBool(const std::string& value, bool& result);
  
private:
  std::unordered_map<int32_t, Property> _properties;
};

class Assets
{
public:
  static constexpr const char* bitmaps_path {"assets/bitmaps/"};
  static constexpr const char* bitmaps_extension {".bitmap"};

public:
  Assets() = default;
  ~Assets() = default;
  void loadBitmaps(const std::vector<std::string>& manifest, int32_t scale);
  void loadFonts(const std::vector<std::string>& manifest, const std::vector<int32_t>& scales);
  const Bitmap& getBitmap(const std::string& key) {return _bitmaps[key];}
  const Font& getFont(const std::string& key, int32_t scale) const;

private:
  std::unordered_map<std::string, Bitmap> _bitmaps;
  std::unordered_map<std::string, std::pair<Font, int32_t>> _fonts;
};

extern std::unique_ptr<Assets> assets;

//===============================================================================================//
// ##>APPLICATION                                                                                //
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
  
  virtual bool initialize(Engine* engine, int32_t windowWidth, int32_t windowHeight);

  virtual const std::string& getName() const = 0;
  virtual int32_t getVersionMajor() const = 0;
  virtual int32_t getVersionMinor() const = 0;

  void onWindowResize(int32_t windowWidth, int32_t windowHeight);
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
// ##>ENGINE                                                                                     //
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
    int64_t doTicks(Duration_t gameNow);
    void setTickPeriod(Duration_t period) {_tickPeriod += period;}
    Duration_t getTickPeriod() const {return _tickPeriod;}
    int64_t getTotalTicks() const {return _totalTicks;}

  private:
    Duration_t _lastTickNow;
    Duration_t _tickPeriod;
    int64_t _totalTicks;
  };

  class TPSMeter
  {
  public:
    explicit TPSMeter() : _timer{}, _ticks{0}, _tps{0} {}
    ~TPSMeter() = default;
    void recordTicks(Duration_t realDt, int32_t ticks);
    int32_t getTPS() const {return _tps;}
    
  private:
    Duration_t _timer;
    int32_t _ticks;
    int32_t _tps;
  };

  enum LoopTicks {LOOPTICK_UPDATE, LOOPTICK_DRAW, LOOPTICK_COUNT};

  struct LoopTick
  {
    void (Engine::*_onTick)(Duration_t, Duration_t, Duration_t, float);
    TPSMeter _tpsMeter;
    Metronome _metronome;
    int64_t _ticksAccumulated;
    int32_t _ticksDoneThisFrame;
    int32_t _maxTicksPerFrame;
    float _tickPeriod;
  };
  
  class Config final : public Dataset
  {
  public:
    static constexpr const char* filename = "engine.config";
    
    enum Key
    {
      KEY_WINDOW_WIDTH, 
      KEY_WINDOW_HEIGHT, 
      KEY_FULLSCREEN, 
      KEY_OPENGL_MAJOR, 
      KEY_OPENGL_MINOR
    };

    Config() : Dataset({
      {KEY_WINDOW_WIDTH,  "windowWidth",  {500},   {300},   {1000}},
      {KEY_WINDOW_HEIGHT, "windowHeight", {500},   {300},   {1000}},
      {KEY_FULLSCREEN,    "fullscreen",   {false}, {false}, {true}},
      {KEY_OPENGL_MAJOR,  "openglMajor",  {2},     {2},     {2},  },
      {KEY_OPENGL_MINOR,  "openglMinor",  {2},     {2},     {2},  }
    }){}
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
  double durationToSeconds(Duration_t d);

private:
  Config _config;
  std::array<LoopTick, LOOPTICK_COUNT> _loopTicks;
  std::unique_ptr<Application> _app;
  RealClock _realClock;
  GameClock _gameClock;
  bool _isPaused;
  bool _isSleeping;
  bool _isDrawingPerformanceStats;
  bool _isDone;
};

} // namespace nomad

#endif

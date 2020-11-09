#ifndef _NOMAD_H_
#define _NOMAD_H_

#include <chrono>
#include <thread>
#include <cstdint>
#include <iostream>
#include <iomanip>
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
#include <tuple>
#include <random>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

namespace nomad
{
//===============================================================================================//
// ##>UTILITY                                                                                    //
//===============================================================================================//

template<typename T>
inline T wrap(T value, T lo, T hi)
{
  return (value < lo) ? hi : (value > hi) ? lo : value;
}

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

template<typename T, typename Dist>
class RandBasic
{
public:
  RandBasic(T lo, T hi) : d{lo, hi}
  {
    std::random_device r;
    e.seed(r());
  }

  RandBasic(const RandBasic&) = delete;
  RandBasic(RandBasic&&) = delete;
  RandBasic& operator=(const RandBasic&) = delete;
  RandBasic& operator=(RandBasic&&) = delete;

  T operator()() {return d(e);}

private:
    std::default_random_engine e;
    Dist d;
};

using RandInt = RandBasic<int32_t, std::uniform_int_distribution<int32_t>>;
using RandReal = RandBasic<double, std::uniform_real_distribution<double>>;

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

  constexpr const char* warn_cannot_open_dataset = "failed to open data file";
  constexpr const char* warn_cannot_create_dataset = "failed to create data file";
  constexpr const char* warn_malformed_dataset = "malformed data file";
  constexpr const char* warn_property_not_set = "property not set";
  constexpr const char* warn_malformed_bitmap = "malformed bitmap";
  constexpr const char* warn_bitmap_already_loaded = "bitmap already loaded";
  constexpr const char* warn_empty_bitmap_file = "empty bitmap file";
  constexpr const char* warn_font_already_loaded = "font already loaded";
  constexpr const char* warn_cannot_open_asset = "failed to open asset file";
  constexpr const char* warn_asset_parse_errors = "asset file parsing errors";

  constexpr const char* info_stderr_log = "logging to standard error";
  constexpr const char* info_using_default_config = "using default engine configuration";
  constexpr const char* info_fullscreen_mode = "activating fullscreen mode";
  constexpr const char* info_creating_window = "creating window";
  constexpr const char* info_created_window = "window created";
  constexpr const char* info_on_line = "on line";
  constexpr const char* info_on_row = "on row";
  constexpr const char* info_unknown_dataset_property = "unkown property";
  constexpr const char* info_unexpected_seperators = "expected a single seperator character";
  constexpr const char* info_incomplete_property = "missing property key or value";
  constexpr const char* info_expected_integer = "expected integer value";
  constexpr const char* info_expected_float = "expected float value";
  constexpr const char* info_expected_bool = "expected bool value";
  constexpr const char* info_ignoring_line = "ignoring line";
  constexpr const char* info_property_set = "dataset property set";
  constexpr const char* info_property_clamped = "property value clamped to min-max range";
  constexpr const char* info_using_property_defaults = "using property default values";
  constexpr const char* info_using_error_bitmap = "substituting with blank bitmap";
  constexpr const char* info_using_error_font = "substituting with blank font";
  constexpr const char* info_using_error_glyph = "substituting with blank glyph";
  constexpr const char* info_loading_asset = "loading asset";
  constexpr const char* info_skipping_asset_loading = "skipping asset loading";
  constexpr const char* info_ascii_code = "ascii code";
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

  struct KeyLog
  {
    bool _isDown;
    bool _isPressed;
    bool _isReleased;
  };

public:
  Input();
  ~Input() = default;

  void onKeyEvent(const SDL_Event& event);
  void onUpdate();

  bool isKeyDown(KeyCode key) {return _keys[key]._isDown;}
  bool isKeyPressed(KeyCode key) {return _keys[key]._isPressed;}
  bool isKeyReleased(KeyCode key) {return _keys[key]._isReleased;}

private:
  KeyCode convertSdlKeyCode(int sdlCode);

private:
  std::array<KeyLog, KEY_COUNT> _keys;
};

extern std::unique_ptr<Input> input;

//===============================================================================================//
// ##>RESOURCES                                                                                  //
//===============================================================================================//

class Bitmap;
class Font;
struct Glyph;

class Dataset
{
public:
  using Value_t = std::variant<int32_t, float, bool>;

  constexpr static Value_t unsetValue {std::numeric_limits<int32_t>::min()};

  enum Type {INT_PROPERTY, FLOAT_PROPERTY, BOOL_PROPERTY};

  struct Property
  {
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
  int32_t load(const std::string& filename);
  int32_t write(const std::string&, bool genComments = true);

  int32_t getIntValue(int32_t key) const;
  float getFloatValue(int32_t key) const;
  bool getBoolValue(int32_t key) const;

  void setIntValue(int32_t key, int32_t value);
  void setFloatValue(int32_t key, float value);
  void setBoolValue(int32_t key, bool value);

  void scaleIntValue(int32_t key, int32_t scale);
  void scaleFloatValue(int32_t key, float scale);

  void applyDefaults();

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

  void printValue(const Value_t& value, std::ostream& os);
  
private:
  std::unordered_map<int32_t, Property> _properties;
};

class Assets
{
public:
  using Key_t = int32_t;
  using Name_t = const char*;
  using Scale_t = size_t;
  using Manifest_t = std::vector<std::tuple<Key_t, Name_t, Scale_t>>;
  
public:
  static constexpr const int32_t maxScale {8};

public:
  Assets() = default;
  ~Assets() = default;

  Assets(const Assets&) = delete;
  Assets(Assets&&) = delete;

  Assets& operator=(const Assets&) = delete;
  Assets& operator=(Assets&&) = delete;

  void loadBitmaps(const Manifest_t& manifest);
  void loadFonts(const Manifest_t& manifest);

  const Bitmap& getBitmap(Key_t key, Scale_t scale) const;
  const Font& getFont(Key_t key, Scale_t scale) const;

private:
  static constexpr const char path_seperator = '/';

  static constexpr const char* bitmaps_path = "assets/bitmaps";
  static constexpr const char* fonts_path = "assets/fonts";
  static constexpr const char* bitmaps_extension = ".bitmap";
  static constexpr const char* fonts_extension = ".font";
  static constexpr const char* glyphs_extension = ".glyph";

  static constexpr std::array<const char*, 8> errorBits {
    "11111111", "11111111", "11111111", "11111111", "11111111", "11111111", "11111111", "11111111"
  };

  static constexpr int32_t asciiCharCount {94};

  static constexpr const std::array<const char*, asciiCharCount> glyphFilenames {
    "emark", "dquote", "hash", "dollar", "percent", "ampersand", "squote", "lrbracket", "rrbracket",
    "asterix", "plus", "comma", "minus", "dot", "fslash", "0", "1", "2", "3", "4", "5", "6", "7", 
    "8", "9", "colon", "scolon", "lcroc", "equals", "rcroc", "qmark", "at", "A", "B", "C", "D", "E", 
    "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", 
    "Y", "Z", "lsbracket", "bslash", "rsbracket", "carrot", "underscore", "backtick", "a", "b", "c", 
    "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", 
    "w", "x", "y", "z", "lcbracket", "pipe", "rcbracket", "tilde"
  };

  class FontData : public Dataset
  {
  public:
    enum Key { KEY_LINE_SPACE, KEY_WORD_SPACE, KEY_GLYPH_SPACE, KEY_SIZE };

    FontData() : Dataset({
      // key            name          default   min    max
      {KEY_LINE_SPACE , "lineSpace" , {10}    , {0}  , {1000}},
      {KEY_WORD_SPACE , "wordSpace" , {5}     , {0}  , {1000}},
      {KEY_GLYPH_SPACE, "glyphSpace", {2}     , {0}  , {1000}},
      {KEY_SIZE       , "size"      , {8}     , {0}  , {1000}}
    }){}
  };

  class GlyphData : public Dataset
  {
  public:
    enum Key { KEY_ASCII_CODE, KEY_OFFSET_X, KEY_OFFSET_Y, KEY_ADVANCE, KEY_WIDTH, KEY_HEIGHT };

    GlyphData() : Dataset({
      // key           name         default  min       max
      {KEY_ASCII_CODE, "asciiCode", {0}    , {0}     , {1000}},
      {KEY_OFFSET_X  , "offsetX"  , {0}    , {-1000} , {1000}},
      {KEY_OFFSET_Y  , "offsetY"  , {0}    , {-1000} , {1000}},
      {KEY_ADVANCE   , "advance"  , {8}    , {0}     , {1000}},
      {KEY_WIDTH     , "width"    , {8}    , {0}     , {1000}},
      {KEY_HEIGHT    , "height"   , {8}    , {0}     , {1000}}
    }){}
  };

private:
  Bitmap loadBitmap(std::string path, std::string name, Scale_t scale = 1);
  Font loadFont(std::string name, Scale_t scale = 1);
  Bitmap generateErrorBitmap(Scale_t scale);
  Font generateErrorFont(Scale_t scale);
  Glyph generateErrorGlyph(int32_t asciiCode, Scale_t scale);

private:
  std::unordered_map<Key_t, std::array<std::unique_ptr<Bitmap>, maxScale>> _bitmaps;
  std::unordered_map<Key_t, std::array<std::unique_ptr<Font>, maxScale>> _fonts;
};

extern std::unique_ptr<Assets> assets;

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
class Bitmap final
{
  friend Assets;

public:
  Bitmap(const Bitmap&) = default;
  Bitmap(Bitmap&&) = default;
  Bitmap& operator=(const Bitmap&) = default;
  Bitmap& operator=(Bitmap&&) = default;
  ~Bitmap() = default;

  bool getBit(int32_t row, int32_t col);
  int32_t getWidth() const {return _width;}
  int32_t getHeight() const {return _height;}
  const std::vector<uint8_t>& getBytes() const {return _bytes;}

  void setBit(int32_t row, int32_t col, bool value, bool regen = true);

  void print(std::ostream& out) const;

private:
  Bitmap() = default;

  void initialize(std::vector<std::string> bits, int32_t scale = 1);
  void generateBytes();

private:
  std::vector<std::vector<bool>> _bits;  // used for bit manipulation ops
  std::vector<uint8_t> _bytes;           // used for rendering
  int32_t _width;
  int32_t _height;
};

struct Glyph // note -- cannot nest in font as it needs to be forward declared.
{
  Bitmap _bitmap;
  int32_t _asciiCode;
  int32_t _offsetX;
  int32_t _offsetY;
  int32_t _advance;
  int32_t _width;
  int32_t _height;
};

class Font
{
  friend class Assets;

public:
  struct Meta
  {
    int32_t _lineSpace;
    int32_t _wordSpace;
    int32_t _glyphSpace;
    int32_t _size;
  };

public:
  Font(const Font&) = default;
  Font(Font&&) = default;
  Font& operator=(const Font&) = default;
  Font& operator=(Font&&) = default;

  const Glyph& getGlyph(char c) const {return _glyphs[static_cast<int32_t>(c - '!')];}
  int32_t getLineSpace() const {return _meta._lineSpace;}
  int32_t getWordSpace() const {return _meta._wordSpace;}
  int32_t getGlyphSpace() const {return _meta._glyphSpace;}
  int32_t getSize() const {return _meta._size;}

private:
  Font() = default;
  void initialize(Meta meta, std::vector<Glyph> glyphs);

private:
  std::vector<Glyph> _glyphs;
  Meta _meta;
};

class Color3f
{
  constexpr static float lo {0.f};
  constexpr static float hi {1.f};

public:
  Color3f() : _r{0.f}, _g{0.f}, _b{0.f}{}

  constexpr Color3f(float r, float g, float b) : 
    _r{std::clamp(r, lo, hi)},
    _g{std::clamp(g, lo, hi)},
    _b{std::clamp(b, lo, hi)}
  {}

  Color3f(const Color3f&) = default;
  Color3f(Color3f&&) = default;
  Color3f& operator=(const Color3f&) = default;
  Color3f& operator=(Color3f&&) = default;

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

// greys - more grays: https://en.wikipedia.org/wiki/Shades_of_gray 

constexpr Color3f gainsboro {0.88f, 0.88f, 0.88f};
constexpr Color3f lightgray {0.844f, 0.844f, 0.844f};
constexpr Color3f silver {0.768f, 0.768f, 0.768f};
constexpr Color3f mediumgray {0.76f, 0.76f, 0.76f};
constexpr Color3f spanishgray {0.608f, 0.608f, 0.608f};
constexpr Color3f gray {0.512f, 0.512f, 0.512f};
constexpr Color3f dimgray {0.42f, 0.42f, 0.42f};
constexpr Color3f davysgray {0.34f, 0.34f, 0.34f};
constexpr Color3f jet {0.208f, 0.208f, 0.208f};
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
  void blitText(Vector2f position, const std::string& text, const Font& font, const Color3f& color);
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
// ##>APPLICATION                                                                                //
//===============================================================================================//

class Application;

class ApplicationState
{
public:
  ApplicationState(Application* app) : _app(app) {}
  virtual ~ApplicationState() = default;
  virtual void initialize(Vector2i worldSize, int32_t worldScale) = 0;
  virtual void onUpdate(double now, float dt) = 0;
  virtual void onDraw(double now, float dt) = 0;
  virtual void onReset() = 0;
  virtual std::string getName() = 0;

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

  virtual std::string getName() const = 0;
  virtual int32_t getVersionMajor() const = 0;
  virtual int32_t getVersionMinor() const = 0;

  void onWindowResize(int32_t windowWidth, int32_t windowHeight);
  void onUpdate(double now, float dt);
  void onDraw(double now, float dt);

  void switchState(const std::string& name);

  bool isWindowTooSmall() const {return _isWindowTooSmall;}

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

  constexpr static Duration_t oneMillisecond {1'000'000};
  constexpr static Duration_t oneSecond {1'000'000'000};
  constexpr static Duration_t oneMinute {60'000'000'000};
  constexpr static Duration_t minFramePeriod {1'000'000};

  constexpr static Assets::Key_t debugFontKey {0}; // engine reserves this font key for itself.
  constexpr static Assets::Name_t debugFontName {"debug"};
  constexpr static Assets::Scale_t debugFontScale {1};

  class RealClock
  {
  public:
    RealClock() : _start{}, _now0{}, _now1{}, _dt{}{}
    ~RealClock() = default;
    void start(){_now0 = _start = Clock_t::now();}
    Duration_t update();
    Duration_t getDt() const {return _dt;}
    Duration_t getNow() {return Clock_t::now() - _start;}

  private:
    TimePoint_t _start;
    TimePoint_t _now0;
    TimePoint_t _now1;
    Duration_t _dt;
  };

  class GameClock
  {
  public:
    GameClock() : _now{}, _scale{1.f}, _isPaused{false}{}
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
    Metronome() : _lastTickNow{}, _tickPeriod{}, _totalTicks{0}{}
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
    TPSMeter() : _timer{}, _ticks{0}, _tps{0} {}
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
      KEY_OPENGL_MINOR,
    };

    Config() : Dataset({
      //    key               name        default   min      max
      {KEY_WINDOW_WIDTH,  "windowWidth",  {500},   {300},   {1000}},
      {KEY_WINDOW_HEIGHT, "windowHeight", {500},   {300},   {1000}},
      {KEY_FULLSCREEN,    "fullscreen",   {false}, {false}, {true}},
      {KEY_OPENGL_MAJOR,  "openglMajor",  {2},     {2},     {2},  },
      {KEY_OPENGL_MINOR,  "openglMinor",  {1},     {1},     {1},  }
    }){}
  };

public:
  Engine() = default;
  ~Engine() = default;
  void initialize(std::unique_ptr<Application> app);
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

  double durationToMilliseconds(Duration_t d);
  double durationToSeconds(Duration_t d);
  double durationToMinutes(Duration_t d);

private:
  Config _config;
  std::array<LoopTick, LOOPTICK_COUNT> _loopTicks;
  TPSMeter _fpsMeter;
  int64_t _frameNo;
  RealClock _realClock;
  GameClock _gameClock;
  std::unique_ptr<Application> _app;
  bool _isSleeping;
  bool _isDrawingPerformanceStats;
  bool _isDone;
};

} // namespace nomad

#endif

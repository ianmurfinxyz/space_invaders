
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

static Log log {};

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
  void update();
  KeyState getKeyState(KeyCode key);

private:
  KeyCode convertSdlKeyCode(int sdlCode);

private:
  std::array<KeyState, KEY_COUNT> _keyStates;
};

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

void Input::update()
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

  Bitmap(std::vector<std::string> bits, int32 scale = 1);
  Bitmap(const Bitmap&) = default;
  Bitmap& operator=(const Bitmap&) = default;
  Bitmap(const Bitmap&&) = default;
  Bitmap& operator=(const Bitmap&&) = default;
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
  int32 getLineSpace() const {return _meta._lineSpace;}
  int32 getWordSpace() const {return _meta._wordSpace;}
  int32 getGlyphSpace() const {return _meta._glyphSpace;}
  int32 getSize() const {return _meta._size;}

  const Glyph& getGlyph(char c) const
  {
    assert(' ' <= c && c <= '~');
    return _glyphs[c - ' '];
  }

private:
  std::vector<Glyph> _glyphs;
  Meta _meta;
};

Font::Font(const std::vector<Glyph>& glyphs, Meta meta) : 
  _glyphs{glyphs}, _meta{meta}
{
  static constexpr int checkSum {7505}; // sum of all ascii codes from 32 to 126 inclusive.

  int sum{0};
  for(const auto& g : _glyphs)
    sum += g._asciiCode;
  assert(sum == checkSum);

  auto gcompare = [](const Glyph& g0, const Glyph& g1){return g0.asciiCode < g1.asciiCode;};
  std::sort(_glyphs.begin(), _glyphs.end(), gcompare);
}

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
  Assets(int32 scale) = default;
  ~Assets() = default;
  void loadBitmaps(const std::vector<std::string>& manifest, int32 scale);
  void loadFonts(const std::vector<std::string>& manifest, const std::vector<int32>& scales);
  const Bitmap& getBitmap(const std::string& key) const {return _bitmaps[key];}
  const Font& getFont(std::string key, int32 scale) const;

private:
  std::unordered_map<std::string, Bitmap> _bitmaps;
  std::unordered_map<std::string, std::pair<Font, int32>> _fonts;
};

void Assets::loadBitmaps(const std::vector<std::string>& manifest, int32 scale);
{
  auto isSpace = [](char ch){return std::isspace<char>(ch, std::locale::classic());};
  auto isBinary = [](char ch){return ch == '0' || ch == '1';};

  bool fail {false};
  std::string path {};
  for(const auto& key : manifest){
    if(_bitmaps.find(key) == _bitmaps.end())
      continue;

    path.clear();
    path += bitmaps_path;
    path += key;
    path += bitmaps_extension;
    std::ifstream bitmap {path};
    if(!bitmap){
      ::log.log(Log::fatal, Log::missing_asset, path);
      fail = true;
      continue;
    }

    bool malformed {false};
    std::vector<std::string> rows {};
    for(std::string row; std::getline(bitmap, row);){
      row.erase(std::remove_if(row.begin(), row.end(), isSpace), row.end());
      int32 count = std::count(row.begin(), row.end(), isBinary);
      if(count != row.length()){
        ::log.log(Log::fatal, Log::malformed_bitmap, path);
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

const Font& Assets::getFont(std::string key, int32 scale) const
{

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

Renderer::Renderer(const Config& config)
{
  uint32 flags = SDL_WINDOW_OPENGL;
  if(config._fullscreen)
    flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;

  _window = SDL_CreateWindow(
      _config._windowTitle.c_str(), 
      SDL_WINDOWPOS_UNDEFINED,
      SDL_WINDOWPOS_UNDEFINED,
      _config._windowWidth,
      _config._windowHeight,
      flags
  );

  if(_window == nullptr){
    ::log.log(Log::fatal, Log.failed_to_create_window, std::string{SDL_GetError()});
    exit(EXIT_FAILURE);
  }

  _glContext = SDL_GL_CreateContext(_window);
  if(_glContext == nullptr){
    ::log.log(Log::fatal, Log.failed_to_create_opengl_context, std::string{SDL_GetError()});
    exit(EXIT_FAILURE);
  }

  if(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, _config._openglVersionMajor) < 0){
    ::log.log(Log::fatal, Log.opengl_set_attribute_fail, std::string{SDL_GetError()});
    exit(EXIT_FAILURE);
  }
  if(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, _config._openglVersionMinor) < 0){
    ::log.log(Log::fatal, Log.opengl_set_attribute_fail, std::string{SDL_GetError()});
    exit(EXIT_FAILURE);
  }

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  setViewport(iRect{0, 0, _config._windowWidth, _config._windowHeight});
  _config = config;
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

Vector2i getWindowSize() const
{
  Vector2i size;
  SDL_GL_GetDrawableSize(_window, &size.x, &size.y);
  return size;
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

  class ApplicationState
  {
  public:
    ApplicationState(Application* app) : _app(app) {}
    virtual ~ApplicationState() = default;
    virtual bool initialize(int32 windowWidth, int32 windowHeight) = 0;
    virtual void onWindowResize(int32 windowWidth, int32 windowHeight) = 0;
    virtual void onUpdate(double now, float dt) = 0;
    virtual void onDraw(double now, float dt) = 0;
    virtual void onReset() = 0;
    virtual const char* getApplicationName() = 0;
    virtual const char* getBootStateName() = 0;
    virtual const char* getStateName() = 0;
    virtual int32 getVersionMajor() = 0;
    virtual int32 getVersionMinor() = 0;

  protected:
    Application* _app;
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

  class AppConfig
  {
  public:
    static constexpr const char* filename {"appcfg"};
    static constexpr const char comment {'#'};
    static constexpr const char seperator {'='};

    static constexpr const char* key_window_width {"window_width"};
    static constexpr const char* key_window_height {"window_height"};
    static constexpr const char* key_fullscreen {"fullscreen"};
    static constexpr const char* key_opengl_major {"opengl_major"};
    static constexpr const char* key_opengl_minor {"opengl_minor"};

  public:
    AppConfig()
    {
      _properties.insert(std::make_pair{key_window_width, 800});
      _properties.insert(std::make_pair{key_window_height, 600});
      _properties.insert(std::make_pair{key_fullscreen, 0});
      _properties.insert(std::make_pair{key_opengl_major, 2});
      _properties.insert(std::make_pair{key_opengl_minor, 1});
    }

    void load()
    {
      std::ifstream config {filename};
      if(!config){
        ::log->log(Log::error, Log::failed_to_open_config_file, std::string{filename});
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
          ::log->log(Log::warning, Log::malformed_config_line, line);
          continue;
        }
    
        std::size_t pos = line.find_first_of(seperator);
        std::string key {line.substr(0, pos)};
        std::string value {line.substr(pos + 1)};
    
        if(key.empty() || value.empty()){
          ::log->log(Log::warning, Log::malformed_config_line, line);
          continue; 
        }

        auto iter = _properties.find();
        if(iter == _properties.end()){
          ::log->log(Log::warning, Log::unkown_config_key, key);
          continue;
        }
         
        count = std::count_if(value.begin(), value.end(), isDigit);
        if(count != value.length()){
          ::log->log(Log::warning, Log::malformed_config_value, value);
          continue;
        }

        iter->second = std::stoi(value);
        ::log->log(Log::info, Log::set_config_property, line);
      }
    }

    void setProperty(const char* key, int32 value){_properties[key]._value = value;}
    int32 getProperty(const char* key){return _properties[key]._value;}

  private:
    std::unordered_map<const char*, int32> _properties;
  };

public:
  Application() = default;
  ~Application() = default;
  void initialize(std::vector<std::unique_ptr<ApplicationState>>&& states);
  void shutdown();
  void run();

  void transitionToAppState(const char* stateName);

  std::unique_ptr<Renderer>& getRenderer() {return _renderer;}
  std::unique_ptr<Input>& getInput() {return _input;}

private:
  void mainloop();
  void drawPerformanceStats(Duration_t realDt, Duration_t gameDt);
  void onUpdateTick(Duration_t gameNow, Duration_t gameDt, Duration_t realDt, float tickDt);
  void onDrawTick(Duration_t gameNow, Duration_t gameDt, Duration_t realDt, float tickDt);
  void loadConfig();

  inline double durationToSeconds(Duration_t d);

private:
  AppConfig _appconfig;

  RealClock _realClock;
  GameClock _gameClock;

  std::unique_ptr<ApplicationState>* _activeState;
  std::unordered_map<const char*, std::unique_ptr<ApplicationState>> _states;

  std::array<LoopTick, LOOPTICK_COUNT> _loopTicks;

  std::unique_ptr<Input> _input;
  std::unique_ptr<Renderer> _renderer;

  bool _isSleeping;
  bool _isDrawingPerformanceStats;
  bool _isDone;
};

void Application::initialize(std::vector<std::unique_ptr<ApplicationState>>&& states)
{
  _appconfig.load();

  if(SDL_Init(SDL_INIT_VIDEO) < 0){
    ::log->log(Log::fatal, Log::sdl2_init_failed, std::string{SDL_GetError()});
    exit(EXIT_FAILURE);
  }

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

  _input = std::move(std::unique_ptr<Input>{new Input()});

  std::stringstream ss {};
  ss << _game->getName() << " version:" << _game->getVersionMajor() << "." << _game->getVersionMinor();
  Renderer::Config rencfg {
    std::string{ss.str()},
    _appconfig.getProperty(AppConfig::key_window_width),
    _appconfig.getProperty(AppConfig::key_window_height),
    _appconfig.getProperty(AppConfig::key_opengl_major),
    _appconfig.getProperty(AppConfig::key_opengl_minor),
    _appconfig.getProperty(AppConfig::key_fullscreen),
  };
  _renderer = std::move(std::unique_ptr<Renderer>{new Renderer(rencfg)});

  Vector2i windowSize = _renderer->getWindowSize();

  assert(states.size() > 0);

  const char* bootName = states[0].getBootStateName();
  bool fail {false};
  for(auto& state : states){
    fail = state->initialize(windowSize._x, windowSize._y);
    if(fail){
      ::log->log(Log::fatal, Log::failed_to_initialize_app_state, std::string{state->getStateName()});
      exit(EXIT_FAILURE);
    }
    _states.emplace(std::make_pair(state->getName(), std::move(state)));
  }
  _activeState = _states[bootName];

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

  SDL_Event event;
  while(SDL_PollEvent(&event) != 0){
    switch(event.type){
      case SDL_QUIT:
        _isDone = true;
        return;
      case SDL_KEYDOWN:
      case SDL_KEYUP:
        _input->onKeyEvent(event);
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

  _input->update();
  
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
  double now = durationToSeconds(gameNow);
  (*_activeState)->onUpdate(now, tickDt);
}

void Application::onDrawTick(Duration_t gameNow, Duration_t gameDt, Duration_t realDt, float tickDt)
{
  // TODO - temp - clear the game viewport only in the game and menu states - only clear window
  // when toggle perf stats
  _renderer->clearWindow(colors::red);
  double now = durationToSeconds(gameNow);
  (*_activeState)->onUpdate(now, tickDt);
  if(_isDrawingPerformanceStats)
    drawPerformanceStats(realDt, gameDt);
  _renderer->show();
}

double Application::durationToSeconds(Duration_t d)
{
  return static_cast<double>(d.count()) / static_cast<double>(oneSecond.count());
}

//===============================================================================================//
//                                                                                               //
// ##>SPACE INVADERS STATE                                                                       //
//                                                                                               //
//===============================================================================================//

// implement app pausing so can pause the game if window is too small - put in app and provide
// functions for the states to use to pause and upause - pause can simply stop updates for the
// update tick


class SpaceInvaders : public Application::ApplicationState
{
public:
  static constexpr const char* app_name {"space invaders"};
  static constexpr const char* boot_name {"menu"};
  static constexpr int32 version_major {0};
  static constexpr int32 version_minor {1};
  static constexpr Vector2i baseWorldSize {300, 300};

public:
  SpaceInvaders(Application* app) : Application {app}{}
  final const std::string& getApplicationName() {return app_name;}
  final int32 getVersionMajor() const {return version_major;}
  final int32 getVersionMinor() const {return version_minor;}
  final const char* getBootStateName() {return boot_name;}

  bool initialize(int32 windowWidth, int32 windowHeight)
  {
    // calc world size and scale
    // set window too small if too small
  }

  void onWindowResize(int32 windowWidth, int32 windowHeight)
  {
    // check if window too small and set flag accordingly - and check if window no longer too
    // small and set flag accordingly

    _viewport._x = (windowWidth - worldWidth) / 2;
    _viewport._y = (windowHeight - worldHeight) / 2;
    _viewport._w = worldWidth;
    _viewport._h = worldHeight;
  }

protected:
  void drawWindowTooSmall()
  {

  }

protected:
  static iRect _viewport;
  static int32 _worldWidth;
  static int32 _worldHeight;
  static int32 _worldScale;
  static bool _isWindowTooSmall;
};

// define state members here

//===============================================================================================//
//                                                                                               //
// ##>GAME                                                                                       //
//                                                                                               //
//===============================================================================================//

class Game final : public SpaceInvaders
{
public:
  static constexpr const char* game_name {"game"};

public:
  Game(Application* app) : SpaceInvaders{app}{}
  ~Game() = default;

  bool initialize(int32 windowWidth, int32 windowHeight);
  void onUpdate(double now, float dt);
  void onDraw(double now, float dt);
  void onReset();
  const char* getStateName() {return game;}

private:
};

void Game::initialize(int32 windowWidth, int32 windowHeight)
{
  // pass the asset manifest to the assets instance. - will have to access the 
  // application pointer which it should already have as it inherits from app state.
  _app->getAssets()->load(/*manifest vector*/);
}

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

class Menu final : public SpaceInvaders
{
public:
  Menu(Application* app) : SpaceInvaders{app}{}
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

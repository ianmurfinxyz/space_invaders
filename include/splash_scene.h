#ifndef _SI_SPLASH_SCENE_H_
#define _SI_SPLASH_SCENE_H_

class SplashState final : public ApplicationState
{
public:
  static constexpr const char* name = "splash";

public:
  SplashState(Application* app);
  ~SplashState() = default;
  void initialize(Vector2i worldSize, int32_t worldScale);
  void onUpdate(double now, float dt);
  void onDraw(double now, float dt);
  void onEnter();
  std::string getName() {return name;}

private:
  enum EventId
  {
    EVENT_SHOW_SPACE_SIGN,
    EVENT_TRIGGER_SPACE_SIGN,
    EVENT_SHOW_INVADERS_SIGN,
    EVENT_TRIGGER_INVADERS_SIGN,
    EVENT_SHOW_PART_II,
    EVENT_SHOW_HUD,
    EVENT_END,
    EVENT_COUNT
  };

  struct SequenceNode
  {
    float _time;
    EventId _event;
  };

  template<int32_t W, int32_t H>
  class Sign
  {
  public:
    Sign(std::array<std::array<int8_t, W>, H> blocks, 
         std::unique_ptr<Bitmap> blockBitmap,
         Vector2i position, 
         Color3f topColor, 
         Color3f bottomColor, 
         float blockLag, 
         int32_t blockSpace, 
         int32_t blockSize) 
      :
         _blocks{blocks}, 
         _blockBitmap{std::move(blockBitmap)}, 
         _position{position}, 
         _topColor{topColor},
         _bottomColor{bottomColor},
         _blockClock(0.f),
         _blockLag{blockLag}, 
         _blockGap{blockSize + blockSpace},
         _row{0},
         _col{0},
         _isDone{false}
    {}

    Sign(const Sign&) = delete;
    Sign(Sign&&) = default;
    Sign& operator=(const Sign&) = delete;
    Sign& operator=(Sign&&) = default;

    void reset()
    {
      // Must run the update to its conclusion first else the sign data will be corrupted.
      if(!_isDone) 
        return;
      _row = 0;
      _col = 0;
      _isDone = false;
      _blockClock = 0.f;
      for(auto& row : _blocks)
        for(auto& block : row)
          ++block;
    }

    void updateBlocks(float dt)
    {
      if(_isDone)
        return;
      _blockClock += dt;
      while(_blockClock > _blockLag && !_isDone){
        --(_blocks[_row][_col]);
        _col++;
        if(_col >= W){
          _col = 0;
          _row++;
          if(_row >= H)
            _isDone = true;
        }
        _blockClock -= _blockLag;
      }
    }

    void draw()
    {
      for(int32_t row = 0; row < H; ++row){
        for(int32_t col = 0; col < W; ++col){
          if(_blocks[row][col] == 0)
            continue;
          Vector2i position {};
          position._x = _position._x + (col * _blockGap);
          position._y = _position._y - (row * _blockGap);
          const Color3f& color = (row < (H / 2)) ? _topColor : _bottomColor;
          pxr::renderer->blitBitmap(position, *_blockBitmap, color);
        }
      }
    }

    bool isDone() const {return _isDone;}

  private:
    std::array<std::array<int8_t, W>, H> _blocks;
    std::unique_ptr<Bitmap> _blockBitmap;
    Vector2i _position;                           // Position of the top-left corner.
    Color3f _topColor;
    Color3f _bottomColor;
    float _blockClock;                            // Unit: seconds.
    float _blockLag;                              // Unit: seconds.
    int32_t _blockGap;                            // Unit: pixels.
    int32_t _row;
    int32_t _col;
    bool _isDone;
  };

private:
  void doEvents();

private:
  Vector2i _worldSize;
  int32_t _worldScale;

  float _masterClock;
  std::array<SequenceNode, EVENT_COUNT> _sequence;
  int32_t _nextNode;

  int32_t _blockSize;
  int32_t _blockSpace;
  int32_t _signX;
  int32_t _spaceY;
  int32_t _invadersY;

  static constexpr int32_t spaceW {48};
  static constexpr int32_t spaceH {16};
  std::unique_ptr<Sign<spaceW, spaceH>> _spaceSign;
  bool _spaceTriggered;
  bool _spaceVisible;

  static constexpr int32_t invadersW {48};
  static constexpr int32_t invadersH {8};
  std::unique_ptr<Sign<invadersW, invadersH>> _invadersSign;
  bool _invadersTriggered;
  bool _invadersVisible;

  Vector2i _partiiPosition;
  Color3f _partiiColor;
  bool _partiiVisible;

  HUD::uid_t _uidAuthor;
};

#endif

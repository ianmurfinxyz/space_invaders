#ifndef _SPACEINVADERS_H_
#define _SPACEINVADERS_H_

#include "pixiretro.h"

using namespace pxr;

//===============================================================================================//
// ##>SPACE INVADERS                                                                             //
//===============================================================================================//

class SpaceInvaders final : public Application
{
public:
  static constexpr const char* name = "Space Invaders";

  static constexpr int32_t version_major {0};
  static constexpr int32_t version_minor {1};

  static constexpr Vector2i baseWorldSize {224, 256};

public:
  enum BitmapKey : Assets::Key_t
  {
    BMK_CANNON0, BMK_SQUID0, BMK_SQUID1, BMK_CRAB0, BMK_CRAB1, BMK_OCTOPUS0, BMK_OCTOPUS1, 
    BMK_CUTTLE0, BMK_CUTTLE1, BMK_CUTTLETWIN, BMK_SAUCER, BMK_SCHRODINGER, BMK_UFOBOOM, 
    BMK_SAUCERSCORE, BMK_SCHRODINGERSCORE, BMK_CROSS0, BMK_CROSS1, BMK_CROSS2, BMK_CROSS3, 
    BMK_ZIGZAG0, BMK_ZIGZAG1, BMK_ZIGZAG2, BMK_ZIGZAG3, BMK_ZAGZIG0, BMK_ZAGZIG1, 
    BMK_ZAGZIG2, BMK_ZAGZIG3, BMK_LASER0, BMK_CANNONBOOM0, BMK_CANNONBOOM1, BMK_CANNONBOOM2, 
    BMK_HITBAR, BMK_ALIENBOOM, BMK_BOMBBOOMBOTTOM, BMK_BOMBBOOMMIDAIR, BMK_BUNKER, BMK_PARTII,
    BMK_CONTROLS, BMK_MENU, BMK_COUNT
  };

  static constexpr std::array<Assets::Name_t, BMK_COUNT> _bitmapNames {
    "cannon0", "squid0", "squid1", "crab0", "crab1", "octopus0", "octopus1", "cuttle0", "cuttle1",
    "cuttletwin", "saucer", "schrodinger", "ufoboom", "saucerscore", "schrodingerscore",
    "cross0", "cross1", "cross2", "cross3", "zigzag0", "zigzag1", 
    "zigzag2", "zigzag3", "zagzig0", "zagzig1", "zagzig2", "zagzig3", "laser0",
    "cannonboom0", "cannonboom1", "cannonboom2", "hitbar", "alienboom", "bombboombottom", 
    "bombboommidair", "bunker", "partii", "controls", "menu"
  };

  static constexpr Assets::Key_t fontKey {1};
  static constexpr Assets::Name_t fontName {"space"};

  enum SoundKey : Mixer::Key_t
  {
    SK_EXPLOSION, SK_SHOOT, SK_INVADER_KILLED, SK_INVADER_MORPHED, SK_UFO_HIGH_PITCH, 
    SK_UFO_LOW_PITCH, SK_FAST1, SK_FAST2, SK_FAST3, SK_FAST4, SK_COUNT
  };

  static constexpr std::array<Mixer::Name_t, SK_COUNT> _soundNames {
    "explosion", "shoot", "invaderkilled", "invadermorphed", "ufo_highpitch", "ufo_lowpitch", "fastinvader1", 
    "fastinvader2", "fastinvader3", "fastinvader4"
  };

  static constexpr size_t hiscoreNameLen = sizeof(int32_t);  
  static constexpr size_t hiscoreCount = 10;
                                            
  static constexpr int32_t nameToInt(const std::array<char, hiscoreNameLen>& name)
  {
    static_assert(hiscoreNameLen == sizeof(int32_t));
    int32_t iname {0};
    for(size_t i{hiscoreNameLen}; i >= 1; --i)
      iname |= static_cast<int32_t>(name[i - 1]) << ((i - 1) * 8);
    return iname;
  }

  static constexpr std::array<char, hiscoreNameLen> intToName(int32_t iname)
  {
    static_assert(hiscoreNameLen == sizeof(int32_t));
    std::array<char, hiscoreNameLen> name {};
    for(size_t i{hiscoreNameLen}; i >= 1; --i)
      name[i - 1] = static_cast<char>((iname & (0xff << (8 * (i - 1)))) >> (8 * (i - 1)));
    return name;
  };

  struct Score
  {
    //
    // names are stored in the hiscores file packed into an int32, thus names can have up to
    // 4 characters. The mapping between the bytes of the int32, the elements of _name and the
    // order of the characters in the name is as follows,
    //
    //         MSB                             LSB
    //    | 00000000 | 00000000 | 00000000 | 00000000 |  i32
    //      _name[3]                         _name[0]
    //       RM char                          LM char
    //
    // where for a name like Adam,
    //              LM char--^  ^--RM char
    //                      
    std::array<char, hiscoreNameLen> _name;
    int32_t _value;
  };

  class ScoreData final : public pxr::Dataset
  {
  public:
    static constexpr const char* filename = "hiscores";

    enum Key
    {
      NAME0, SCORE0, NAME1, SCORE1, NAME2, SCORE2, NAME3, SCORE3, NAME4, SCORE4, 
      NAME5, SCORE5, NAME6, SCORE6, NAME7, SCORE7, NAME8, SCORE8, NAME9, SCORE9
    };

    ScoreData() : Dataset({
      // key
      {NAME0 , "name0" , nameToInt({'A','D','A','M'}), 0, std::numeric_limits<int32_t>::max()},
      {SCORE0, "score0", 1230                        , 0, std::numeric_limits<int32_t>::max()},
      {NAME1 , "name1" , nameToInt({'I','A','N','_'}), 0, std::numeric_limits<int32_t>::max()},
      {SCORE1, "score1", 3410                        , 0, std::numeric_limits<int32_t>::max()},
      {NAME2 , "name2" , nameToInt({'N','O','O','B'}), 0, std::numeric_limits<int32_t>::max()},
      {SCORE2, "score2", 300                         , 0, std::numeric_limits<int32_t>::max()},
      {NAME3 , "name3" , nameToInt({'T','I','M','_'}), 0, std::numeric_limits<int32_t>::max()},
      {SCORE3, "score3", 460                         , 0, std::numeric_limits<int32_t>::max()},
      {NAME4 , "name4" , nameToInt({'I','A','N','_'}), 0, std::numeric_limits<int32_t>::max()},
      {SCORE4, "score4", 880                         , 0, std::numeric_limits<int32_t>::max()},
      {NAME5 , "name5" , nameToInt({'M','O','O','N'}), 0, std::numeric_limits<int32_t>::max()},
      {SCORE5, "score5", 480                         , 0, std::numeric_limits<int32_t>::max()},
      {NAME6 , "name6" , nameToInt({'B','E','E','F'}), 0, std::numeric_limits<int32_t>::max()},
      {SCORE6, "score6", 1180                        , 0, std::numeric_limits<int32_t>::max()},
      {NAME7 , "name7" , nameToInt({'P','E','E','K'}), 0, std::numeric_limits<int32_t>::max()},
      {SCORE7, "score7", 1440                        , 0, std::numeric_limits<int32_t>::max()},
      {NAME8 , "name8" , nameToInt({'_','I','T','_'}), 0, std::numeric_limits<int32_t>::max()},
      {SCORE8, "score8", 1610                        , 0, std::numeric_limits<int32_t>::max()},
      {NAME9 , "name9" , nameToInt({'_','W','I','N'}), 0, std::numeric_limits<int32_t>::max()},
      {SCORE9, "score9", 240                         , 0, std::numeric_limits<int32_t>::max()},
    }){}
  };

public:
  SpaceInvaders() = default;
  ~SpaceInvaders() = default;

  std::string getName() const {return name;}
  int32_t getVersionMajor() const {return version_major;}
  int32_t getVersionMinor() const {return version_minor;}

  bool initialize(Engine* engine, int32_t windowWidth, int32_t windowHeight);
  void onUpdate(double now, float dt);
  void onDraw(double now, float dt);

  HUD& getHud() {return _hud;}
  void hideHud(){_isHudVisible = false;}   // faster way to hide entire hud.
  void showHud(){_isHudVisible = true;}
  bool isHudVisible() const {return _isHudVisible;}
  void hideHudTop();                     // slower but hides part of hud.
  void showHudTop();

  void setScore(int32_t score){_score = score;}
  void addScore(int32_t score){_score += score;}
  int32_t getScore() const {return _score;}

  void setRound(int32_t round){_round = round;}
  void addRound(int32_t round){_round += round;}
  int32_t getRound() const {return _round;}

  void setCredit(int32_t credit){_credit = credit;}
  void addCredit(int32_t credit){_credit += credit;}
  int32_t getCredit() const {return _credit;}

  void setLives(int32_t lives){_lives = lives;}
  void addLives(int32_t lives){_lives += lives;}
  int32_t getLives() const {return _lives;}

  void resetGameStats();

  Vector2i getWorldSize() const {return _worldSize;}
  int32_t getWorldScale() const {return _worldScale;}

  void loadHiscores();
  void writeHiscores();
  bool isHiscore(int32_t scoreValue);
  bool registerHiscore(const Score& score);
  std::pair<int, int> findScoreBoardPosition(int32_t scoreValue);
  const std::array<Score, hiscoreCount>& getHiscores() const {return _hiscores;}

private:
  static constexpr float flashPeriod {0.1f};  // Inverse frequency of HUD label flashing.
  static constexpr float phasePeriod {0.1f};  // Inverse frequency of HUD label letter phase in.

  std::array<Score, hiscoreCount> _hiscores;

  Vector2i _worldSize;
  int32_t _worldScale;

  HUD _hud;
  HUD::uid_t _uidScoreText;
  HUD::uid_t _uidScoreValue;
  HUD::uid_t _uidHiScoreText;
  HUD::uid_t _uidHiScoreValue;
  HUD::uid_t _uidRoundText;
  HUD::uid_t _uidRoundValue;
  HUD::uid_t _uidCreditText;
  HUD::uid_t _uidCreditValue;
  int32_t _score;
  int32_t _hiscore;
  int32_t _round;
  int32_t _credit;
  int32_t _lives;
  bool _isHudVisible;
};

#endif

//===============================================================================================//
// ##>SPLASH STATE                                                                               //
//===============================================================================================//

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
  void onReset();
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

//===============================================================================================//
// ##>GAME STATE                                                                                 //
//===============================================================================================//

class GameState final : public ApplicationState
{
public:
  static constexpr const char* name = "game";

public:
  GameState(Application* app);
  ~GameState() = default;
  void initialize(Vector2i worldSize, int32_t worldScale);
  void onUpdate(double now, float dt);
  void onDraw(double now, float dt);
  void onReset();
  std::string getName(){return name;}

private:

  class BeatBox
  {
  public:
    static constexpr int beatCount = 4;
    BeatBox() = default;
    BeatBox(std::array<Mixer::Key_t, beatCount> beats, float beatFreq_hz);
    void doBeats(float dt);
    void setBeatFreq(float freq_hz){_beatFreq_hz = freq_hz;}
    void addBeatFreq(float freq_hz){_beatFreq_hz += freq_hz;}
    float getBeatFreq() const {return _beatFreq_hz;}
    void pause(){_isPaused = true;}
    void togglePause(){_isPaused = !_isPaused;}
    bool isPaused() const {return _isPaused;}
  private:
    std::array<Mixer::Key_t, beatCount> _beats;
    int _nextBeat;
    float _beatFreq_hz;
    float _beatPeriod_s;
    float _beatClock_s;
    bool _isPaused;
  };

  struct GridIndex
  {
    int32_t _row;
    int32_t _col;
  };

  enum AlienClassId { SQUID, CRAB, OCTOPUS, CUTTLE, CUTTLETWIN };

  static constexpr int32_t alienFramesCount {2};
  struct AlienClass
  {
    int32_t _width;
    int32_t _height;
    int32_t _scoreValue;
    int32_t _colorIndex;
    std::array<Assets::Key_t, alienFramesCount> _bitmapKeys;
  };

  struct Alien
  {
    AlienClassId _classId;
    Vector2i _position;
    int32_t _row;
    int32_t _col;
    bool _frame;
    bool _isAlive;
  };

  enum UfoClassId { SAUCER, SCHRODINGER };

  struct UfoClass
  {
    int32_t _width;
    int32_t _height;
    int32_t _scoreValue;
    int32_t _colorIndex;
    Assets::Key_t _shipKey;
    Assets::Key_t _boomKey;
    Assets::Key_t _scoreKey;
  };

  struct Ufo
  {
    UfoClassId _classId;
    Vector2f _position;
    float _age;
    float _phaseClock;
    bool _phase;
    bool _isAlive;
  };

  enum BombClassId { CROSS, ZIGZAG, ZAGZIG };

  static constexpr int32_t bombFramesCount {4};
  struct BombClass
  {
    int32_t _width;
    int32_t _height;
    float _speed;                                             // Unit: pixels per second.
    int32_t _colorIndex;
    int32_t _frameInterval;                                   // Beats between draw frames.
    std::array<Assets::Key_t, bombFramesCount> _bitmapKeys; 
  };

  struct Bomb
  {
    BombClassId _classId;
    Vector2f _position;
    int32_t _frameClock;       // unit: Cycle beats.
    int32_t _frame;            // Constraint: value=[0, 4).
    bool _isAlive;
  };

  enum BombHit { BOMBHIT_BOTTOM, BOMBHIT_MIDAIR };

  struct BombBoom
  {
    BombHit _hit;
    Vector2i _position;
    int32_t _colorIndex;
    float _boomClock;     // Unit: seconds. 
    bool _isAlive;
  };

  struct Laser
  {
    Vector2f _position;
    int32_t _width;
    int32_t _height;
    int32_t _colorIndex;
    float _speed;             // Unit: pixels per second.
    bool _isAlive;
    Assets::Key_t _bitmapKey;
  };

  static constexpr int32_t cannonBoomFramesCount {3};
  struct Cannon
  {
    Vector2f _spawnPosition;
    Vector2f _position;
    int32_t _colorIndex;
    int32_t _width;
    int32_t _height;
    int32_t _moveDirection;          // -1 == left, 0 == still, +1 == right.
    float _speed;                    // Unit: pixels per second.
    float _boomDuration;             // Unit: seconds - total length of boom animation.
    float _boomClock;                // Unit: seconds.
    float _boomFrameDuration;        // Unit: seconds - how many beats per frame.
    int32_t _boomFrame;              // Current boom animation frame.
    float _boomFrameClock;           // Unit: seconds.
    bool _isBooming;
    bool _isAlive;
    bool _isFrozen;
    Assets::Key_t _cannonKey;
    std::array<Assets::Key_t, cannonBoomFramesCount> _boomKeys;
  };

  struct Hitbar
  {
    Hitbar(const Bitmap& b, int32_t w, int32_t h, int32_t y, int32_t c) : 
      _bitmap{b}, _width{w}, _height{h}, _positionY{y}, _colorIndex{c}{}

    Bitmap _bitmap;
    int32_t _width;
    int32_t _height;
    int32_t _positionY;
    int32_t _colorIndex;
  };

  struct Bunker
  {
    Bunker(const Bitmap& b, Vector2f p) : _bitmap{b}, _position{p}{}

    Bitmap _bitmap;
    Vector2f _position;
  };

  struct Level
  {
    int32_t _spawnDrops;      // Number of times the aliens drop upon spawning.
    int32_t _formationIndex;  // The grid formation used for this level.
    int32_t _ufoSpawnRate;    // Unit: alien deaths - spawn every rate deaths.
    bool _isCuttlesOn;        // Do cuttle fish spawn from crabs in this level?
    bool _isSchrodingerOn;
  };

  // TEMP - TODO - replace this with UI elements or something once the UI is done. Preferable
  // one in which the text can draw over time for the game over label.
  
  // struct TextLabel
  // {
  //   Vector2f _position;
  //   std::string _message;
  //   int32_t _colorIndex;
  // };

  // struct IntLabel
  // {
  //   Vector2f _position;
  //   int32_t* _value;
  //   int32_t _colorIndex;
  // };

  // struct BitmapLabel
  // {
  //   Vector2f _position;
  //   Assets::Key_t _bitmapKey;
  //   int32_t _colorIndex;
  // };

private:
  void startNextLevel();
  void updateActiveCycle();
  void endSpawning();
  void spawnCannon(bool takeLife);
  void spawnBomb(Vector2f position, BombClassId classId);
  void spawnBoom(Vector2i position, BombHit hit, int32_t colorIndex); 
  void spawnBunker(Vector2f position, Assets::Key_t bitmapKey);
  void spawnUfo(UfoClassId classId);
  void morphAlien(Alien& alien);
  void boomCannon();
  void boomBomb(Bomb& bomb, bool makeBoom = false, Vector2i boomPosition = {}, BombHit hit = BOMBHIT_MIDAIR);
  void boomAlien(Alien& alien);
  void boomLaser(bool makeBoom, BombHit hit = BOMBHIT_MIDAIR);
  void boomBunker(Bunker& bunker, Vector2i hitPixel);
  void doUfoSpawning();
  void doAlienMorphing(float dt);
  void doCannonMoving(float dt);
  void doCannonBooming(float dt);
  void doCannonFiring();
  void doAlienMoving(int32_t beats);
  void doBombMoving(int32_t beats, float dt);
  void doLaserMoving(float dt);
  void doUfoMoving(float dt);
  void doAlienBombing(int32_t beats);
  void doAlienBooming(float dt);
  void doUfoBooming(float dt);
  void doBombBoomBooming(float dt);
  void doUfoReinforcing(float dt);
  void doUfoAging(float dt);
  void doCollisionsBombsHitbar();
  void doCollisionsBombsCannon();
  void doCollisionsLaserAliens();
  void doCollisionsLaserSky();
  bool doCollisionsAliensBorders();
  void doCollisionsBunkersBombs();
  void doCollisionsBunkersLaser();
  void doCollisionsBunkersAliens();
  bool incrementGridIndex(GridIndex& index);
  void drawGrid();
  void drawUfo();
  void drawCannon();
  void drawBombs();
  void drawBombBooms();
  void drawLaser();
  void drawHitbar();
  void drawBunkers();
  //void drawHud();

  // Predicates.
  static bool isBombAlive(const Bomb& bomb) {return bomb._isAlive;}
  static bool isBombBoomAlive(const BombBoom& boom) {return boom._isAlive;}

private:
  const Font* _font;

  BeatBox _beatBox;

  Vector2i _worldSize;
  int32_t _worldScale;

  static constexpr int32_t paletteSize {7}; 
  std::array<Color3f, paletteSize> _colorPallete;

  static constexpr int32_t gridWidth {11};
  static constexpr int32_t gridHeight {5};
  using GridRow = std::array<Alien, gridWidth>;
  std::array<GridRow, gridHeight> _grid; 
  Vector2i _alienShiftDisplacement;
  Vector2i _alienDropDisplacement;
  Vector2i _aliensSpawnPosition;
  int32_t _alienXSeperation;
  int32_t _alienYSeperation;
  int32_t _worldMargin;
  int32_t _worldLeftBorderX;
  int32_t _worldRightBorderX;
  int32_t _worldTopBorderY;
  int32_t _alienMoveDirection;            // Limited to values -1 for left, +1 for right.
  int32_t _dropsDone;
  int32_t _alienPopulation;               // Count of the number of aliens alive.
  GridIndex _nextMover;                   // The alien to move in the next tick.
  Alien* _alienBoomer;                    // The alien going BOOM! (not your dad).
  float _alienBoomClock;                  // Unit: seconds.
  float _alienBoomDuration;               // Unit: seconds.
  Alien* _alienMorpher;                   // The alien morphing into a cuttle fish.
  float _alienMorphClock;
  float _alienMorphDuration;              // Unit: seconds. How long it takes to morph into a cuttle fish.
  bool _isAliensMorphing;
  bool _isAliensBooming;
  bool _isAliensSpawning;
  bool _isAliensDropping;
  bool _isAliensFrozen;

  static constexpr int32_t ufoClassCount {2};
  std::array<UfoClass, ufoClassCount> _ufoClasses;
  Ufo _ufo;
  int32_t _ufoSpawnNo;
  int32_t _schrodingerSpawnNo;           // Which ufo becomes the schrodinger? spawn 0? spawn 4?
  int32_t _ufoLastSpawnPop;              // Population of grid when last ufo spawned.
  int32_t _ufoDirection;                 // Constraint: value=-1 (left) or value=1 (right).
  float _ufoSpawnY;                      // Height of ufos.
  float _ufoLifetime;                    // Unit: seconds.
  float _ufoSpeed;                       // Unit: pixels per second.
  float _ufoBoomDuration;                // Unit: seconds.
  float _ufoScoreDuration;               // Unit: seconds.
  float _ufoPhaseDuration;               // Unit: seconds.
  float _ufoBoomScoreClock;
  bool _isUfoBooming;
  bool _isUfoScoring;                    // Is the score displaying after the ufo was destroyed?

  static constexpr int32_t cycleCount {13};
  static constexpr int32_t cycleLength {4};
  static constexpr int32_t cycleStart {0};
  static constexpr int32_t cycleEnd {-1};
  using Cycle = std::array<int32_t, cycleLength>;
  std::array<Cycle, cycleCount> _cycles;
  std::array<int32_t, cycleCount> _cycleTransitions;
  int32_t _activeCycle;
  int32_t _activeBeat;  // A beat is an element of a cycle.

  static constexpr int32_t alienClassCount {5};
  std::array<AlienClass, alienClassCount> _alienClasses;

  static constexpr int32_t formationCount {2};
  using Formation = std::array<std::array<AlienClassId, gridWidth>, gridHeight>;
  std::array<Formation, formationCount> _formations;

  float _bombIntervalDeviation;                    // Max deviation in bomb drop beat count.
  std::array<int32_t, cycleCount> _bombIntervals;  // Beats between bomb drops.
  int32_t _bombInterval;                           // Base beat count between firing.
  int32_t _bombClock;                              // Unit: beats - used to time the firing.
  std::array<int32_t, gridWidth> _columnPops;      // Populations of alive aliens in each column.
  std::array<int32_t, gridHeight> _rowPops;        // Populations of alive aliens in each row.

  static constexpr int32_t bombClassCount {3};
  std::array<BombClass, bombClassCount> _bombClasses;

  static constexpr int32_t maxBombs {20};
  std::array<Bomb, maxBombs> _bombs;
  int32_t _bombCount;

  std::array<BombBoom, maxBombs> _bombBooms;
  std::array<Assets::Key_t, 2> _bombBoomKeys;
  int32_t _bombBoomWidth;
  int32_t _bombBoomHeight;
  float _bombBoomDuration;                          // Unit: seconds.

  Laser _laser;
  Cannon _cannon;

  std::unique_ptr<Hitbar> _hitbar;

  std::vector<std::unique_ptr<Bunker>> _bunkers;
  int32_t _bunkerColorIndex;
  int32_t _bunkerSpawnX;
  int32_t _bunkerSpawnY;
  int32_t _bunkerSpawnGapX;
  int32_t _bunkerSpawnCount;
  int32_t _bunkerWidth;
  int32_t _bunkerHeight;
  int32_t _bunkerDeleteThreshold; // The bunker will be deleted if it has threshold or fewer pixels.

  static constexpr int32_t levelCount {10};
  std::array<Level, levelCount> _levels;
  int32_t _levelIndex;                        // Active level (index into _levels data).
  // int32_t _round;
  // int32_t _score;
  // int32_t _lives;
  // int32_t _credit;
  bool _isGameOver;
  float _gameOverDuration;                    // Unit: seconds. Time to display game over message.
  float _gameOverClock;                       // Unit: seconds.

  //TextLabel _gameOverLabel;
  //TextLabel _scoreLabel;
  //TextLabel _recordLabel;
  //TextLabel _roundLabel;
  //TextLabel _creditLabel;
  //IntLabel _scoreValueLabel;
  //IntLabel _recordValueLabel;
  //IntLabel _roundValueLabel;
  //IntLabel _creditValueLabel;
  //IntLabel _lifeValueLabel;
  //BitmapLabel _lifeCannonLabel;
  //int32_t _lifeCannonSpacingX;
  //bool _showHud;
};

//===============================================================================================//
// ##>MENU STATE                                                                                 //
//===============================================================================================//

class MenuState final : public ApplicationState
{
public:
  static constexpr const char* name = "menu";

public:
  MenuState(Application* app) : ApplicationState{app} {}
  ~MenuState() = default;

  void initialize(Vector2i worldSize, int32_t worldScale);
  void onUpdate(double now, float dt);
  void onDraw(double now, float dt);
  void onReset();

  std::string getName(){return name;}

private:
  void populateHud();
  void depopulateHud();

private:
  int32_t _worldScale;

  HUD::uid_t _uidMenuText;
  HUD::uid_t _uidMenuBitmap;
  HUD::uid_t _uidControlsText;
  HUD::uid_t _uidControlsBitmap;
  HUD::uid_t _uidTablesText;
  HUD::uid_t _uidSchroBitmap;
  HUD::uid_t _uidSaucerBitmap;
  HUD::uid_t _uidSquidBitmap;
  HUD::uid_t _uidCuttleBitmap;
  HUD::uid_t _uidCrabBitmap;
  HUD::uid_t _uidOctopusBitmap;
  HUD::uid_t _uid500PointsText;
  HUD::uid_t _uidMysteryPointsText;
  HUD::uid_t _uid30PointsText;
  HUD::uid_t _uid20PointsText;
  HUD::uid_t _uid10PointsText;
};


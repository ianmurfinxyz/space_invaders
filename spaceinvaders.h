#ifndef _SPACEINVADERS_H_
#define _SPACEINVADERS_H_

#include "nomad.h"

using namespace nomad;

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
    BMK_SAUCER0, BMK_CROSS0, BMK_CROSS1, BMK_CROSS2, BMK_CROSS3, BMK_ZIGZAG0, BMK_ZIGZAG1, 
    BMK_ZIGZAG2, BMK_ZIGZAG3, BMK_ZAGZIG0, BMK_ZAGZIG1, BMK_ZAGZIG2, BMK_ZAGZIG3, BMK_LASER0,
    BMK_CANNONBOOM0, BMK_CANNONBOOM1, BMK_CANNONBOOM2, BMK_HITBAR, BMK_ALIENBOOM, 
    BMK_BOMBBOOMTOP, BMK_BOMBBOOMBOTTOM, BMK_BOMBBOOMMIDAIR, BMK_COUNT
  };

  constexpr static std::array<Assets::Name_t, BMK_COUNT> _bitmapNames {
    "cannon0", "squid0", "squid1", "crab0", "crab1", "octopus0", "octopus1", 
    "saucer0", "cross0", "cross1", "cross2", "cross3", "zigzag0", "zigzag1", 
    "zigzag2", "zigzag3", "zagzig0", "zagzig1", "zagzig2", "zagzig3", "laser0",
    "cannonboom0", "cannonboom1", "cannonboom2", "hitbar", "alienboom", "bombboomtop",
    "bombboombottom", "bombboommidair"
  };

public:
  SpaceInvaders() = default;
  ~SpaceInvaders() = default;

  std::string getName() const {return name;}
  int32_t getVersionMajor() const {return version_major;}
  int32_t getVersionMinor() const {return version_minor;}

  bool initialize(Engine* engine, int32_t windowWidth, int32_t windowHeight);

private:
  Vector2i getWorldSize() const {return _worldSize;}
  int32_t getWorldScale() const {return _worldScale;}

private:
  Vector2i _worldSize;
  int32_t _worldScale;
};

#endif

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
  struct GridIndex
  {
    int32_t _row;
    int32_t _col;
  };

  enum AlienClassId { SQUID, CRAB, OCTOPUS, GAP };

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

  enum BombHit { BOMBHIT_TOP, BOMBHIT_BOTTOM, BOMBHIT_MIDAIR };

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
    int32_t _moveDirection;            // -1 == left, 0 == still, +1 == right.
    float _speed;                      // Unit: pixels per second.
    int32_t _boomInterval;             // Unit: beats - total length of boom animation.
    int32_t _boomClock;                // Unit: beats.
    int32_t _boomFrameInterval;        // Unit: beats - how many beats per frame.
    int32_t _boomFrame;                // Current boom animation frame.
    int32_t _boomFrameClock;           // Unit: beats.
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

  struct Level
  {
    int32_t _startCycle;      // The start rate of beats (start game speed).
    int32_t _spawnDrops;      // Number of times the aliens drop upon spawning.
    int32_t _formationIndex;  // The grid formation used for this level.
  };

private:
  void startNextLevel();
  void endSpawning();
  void spawnCannon();
  void spawnBomb(Vector2f position, BombClassId classId);
  void spawnBoom(Vector2i position, BombHit hit, int32_t colorIndex); 
  void boomCannon();
  void boomBomb(Bomb& bomb, bool makeBoom = false, Vector2i boomPosition = {}, BombHit hit = BOMBHIT_MIDAIR);
  void boomAlien(Alien& alien);
  void boomLaser(bool makeBoom, BombHit hit = BOMBHIT_MIDAIR);
  void doCannonMoving(float dt);
  void doCannonBooming(int32_t beats);
  void doCannonFiring();
  void doAlienMoving(int32_t beats);
  void doAlienBombing(int32_t beats);
  void doAlienBooming(float dt);
  void doBombMoving(int32_t beats, float dt);
  void doLaserMoving(float dt);
  void doBombBoomBooming(float dt);
  void doCollisionsBombsHitbar();
  void doCollisionsBombsCannon();
  void doCollisionsLaserAliens();
  void doCollisionsLaserSky();
  bool doCollisionsAliensBorders();
  bool incrementGridIndex(GridIndex& index);
  void drawGrid();
  void drawCannon();
  void drawBombs();
  void drawBombBooms();
  void drawLaser();
  void drawHitbar();

  // Predicates.
  static bool isBombAlive(const Bomb& bomb) {return bomb._isAlive;}
  static bool isBombBoomAlive(const BombBoom& boom) {return boom._isAlive;}

private:
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
  bool _isAliensBooming;
  bool _isAliensSpawning;
  bool _isAliensDropping;
  bool _isAliensFrozen;

  static constexpr int32_t cycleCount {15};
  static constexpr int32_t cycleLength {4};
  static constexpr int32_t cycleStart {0};
  static constexpr int32_t cycleEnd {-1};
  using Cycle = std::array<int32_t, cycleLength>;
  std::array<Cycle, cycleCount> _cycles;
  int32_t _activeCycle;
  int32_t _activeBeat;  // A beat is an element of a cycle.

  static constexpr int32_t alienClassCount {3};
  std::array<AlienClass, alienClassCount> _alienClasses;

  static constexpr int32_t formationCount {2};
  using Formation = std::array<std::array<AlienClassId, gridWidth>, gridHeight>;
  std::array<Formation, formationCount> _formations;

  float _bombIntervalDeviation;                 // Max deviation in beat count.
  int32_t _bombIntervalBase;                    // Base beat count between firing.
  int32_t _bombClock;                           // Unit: beats - used to time the firing.
  std::array<int32_t, gridWidth> _columnPops;   // Populations of alive aliens in each column.
  RandInt _randColumn;

  static constexpr int32_t bombClassCount {3};
  std::array<BombClass, bombClassCount> _bombClasses;
  RandInt _randBombClass;

  static constexpr int32_t maxBombs {20};
  std::array<Bomb, maxBombs> _bombs;
  int32_t _bombCount;

  std::array<BombBoom, maxBombs> _bombBooms;
  std::array<Assets::Key_t, 3> _bombBoomKeys;
  int32_t _bombBoomWidth;
  float _bombBoomDuration;                          // Unit: seconds.

  Laser _laser;
  Cannon _cannon;

  std::unique_ptr<Hitbar> _hitbar;

  static constexpr int32_t levelCount {10};
  std::array<Level, levelCount> _levels;
  int32_t _levelIndex;                        // Active level (index into _levels data).
  int32_t _levelNo;                           // Real level number (total levels completed).
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
  int32_t _worldScale;
};


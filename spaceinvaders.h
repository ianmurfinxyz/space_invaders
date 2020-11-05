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
    BMK_COUNT
  };

  constexpr static std::array<Assets::Name_t, BMK_COUNT> _bitmapNames {
    "cannon0", "squid0", "squid1", "crab0", "crab1", "octopus0", "octopus1", 
    "saucer0", "cross0", "cross1", "cross2", "cross3", "zigzag0", "zigzag1", 
    "zigzag2", "zigzag3", "zagzig0", "zagzig1", "zagzig2", "zagzig3", "laser0"
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
  GameState(Application* app) : ApplicationState{app} {}
  ~GameState() = default;

  void initialize(Vector2i worldSize, int32_t worldScale);
  void onUpdate(double now, float dt);
  void onDraw(double now, float dt);
  void onReset();

  std::string getName(){return name;}

private:

  //-- CYCLES -----------------------------------------------------------------------------------//
  
  // Aliens move at fixed displacements independent of time, thus alien movement speed is an 
  // emergent property of the rate of update ticks, and importantly, the number of aliens moved 
  // in a each tick. Note the engine guarantees a tick rate of 60Hz (or less if on a slow 
  // computer). Thus alien speed in game is controlled by the second factor; the number of aliens
  // moved in each tick. Since the game is ticked at 60Hz, if a single alien is moved in each tick
  // then 55 aliens will be moved in 55 ticks so in 55/60 seconds. Moving 2 aliens per tick 
  // will result in twice the speed. A cycle controls the number of aliens moved in a single tick 
  // where the elements of a cycle represent the number of aliens to tick. Thus a cycle such as
  // {1, 2, end} means update 1 alien in tick N, 2 aliens in tick N + 1, and repeat. This
  // results in (when you do the math) all 55 aliens moving in 37 ticks, i.e. it takes 37/60 
  // seconds to complete one full grid movement, giving a frequency of grid movements of 60/37 Hz.
  //
  // Below is defined a set of cycles with exponentially increasing frequency of alien movements
  // and thus speed. The alien speed is thus increased exponentially by moving up the cycle
  // list. The frequency is the rate at which full grid movements occur.
  //
  // note: this design also makes alien movement speed dependent on the number of aliens in the
  // grid. Thus if you change the number of aliens you must also recalculate all cycles to tune
  // the gameplay.
  
  static constexpr int32_t cycleCount {15};
  static constexpr int32_t cycleLength {4};
  static constexpr int32_t cycleStart {0};
  static constexpr int32_t cycleEnd {-1};

  // These cycles are tuned to an alien grid with 55 columns and 5 rows.
  static constexpr std::array<std::array<int32_t, cycleLength>, cycleCount> _cycles {{
    {1,  cycleEnd, 0, 0},  // ticks:55.00   freq:1.09
    {1,  1, 2, cycleEnd},  // ticks:42.00   freq:1.43
    {1,  2, cycleEnd, 0},  // ticks:37.00   freq:1.60
    {2,  cycleEnd, 0, 0},  // ticks:27.50   freq:2.18
    {2,  2, 3, cycleEnd},  // ticks:23.57   freq:2.54
    {2,  3, cycleEnd, 0},  // ticks:22.00   freq:2.70
    {3,  cycleEnd, 0, 0},  // ticks:18.33   freq:3.33
    {4,  cycleEnd, 0, 0},  // ticks:13.75   freq:4.35
    {5,  cycleEnd, 0, 0},  // ticks:11.00   freq:5.56
    {6,  cycleEnd, 0, 0},  // ticks:9.17    freq:6.67
    {7,  cycleEnd, 0, 0},  // ticks:7.86    freq:7.69
    {8,  cycleEnd, 0, 0},  // ticks:6.88    freq:9.09
    {9,  cycleEnd, 0, 0},  // ticks:6.11    freq:9.81
    {10, cycleEnd, 0, 0},  // ticks:5.50    freq:10.90
    {11, cycleEnd, 0, 0}   // ticks:5.00    freq:12.00
  }};

  int32_t _activeCycle;
  int32_t _activeCycleElement;

  //-- COLOR PALLETE ----------------------------------------------------------------------------//

  static constexpr int32_t palleteSize {3}; 
  std::vector<Color3f> _colorPallete;

  //-- ALIENS -----------------------------------------------------------------------------------//
  
  static constexpr int32_t gridWidth {11};
  static constexpr int32_t gridHeight {5};

  enum AlienClassId { SQUID, CRAB, OCTOPUS, GAP };

  struct AlienClass
  {
    int32_t _scoreValue;
    int32_t _colorIndex;
    Assets::Key_t _bitmapKeys[2];
  };

  static constexpr int32_t alienClassCount {3};
  static constexpr std::array<AlienClass, alienClassCount> _alienClasses {{
    {30, 0, {SpaceInvaders::BMK_SQUID0  , SpaceInvaders::BMK_SQUID1  }},
    {20, 1, {SpaceInvaders::BMK_CRAB0   , SpaceInvaders::BMK_CRAB1   }},
    {10, 2, {SpaceInvaders::BMK_OCTOPUS0, SpaceInvaders::BMK_OCTOPUS1}},
  }};

  using Formation_t = std::array<std::array<AlienClassId, gridWidth>, gridHeight>;

  static constexpr int32_t formationCount {2};
  static constexpr std::array<Formation_t, formationCount> _formations {{
    // formation 0
    {{
       {SQUID  ,SQUID  ,SQUID  ,SQUID  ,SQUID  ,SQUID  ,SQUID  ,SQUID  ,SQUID  ,SQUID  ,SQUID  },
       {CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   },
       {CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   },
       {OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS},
       {OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS}
    }},

    // formation 1
    {{
       {SQUID  ,SQUID  ,SQUID  ,SQUID  ,SQUID  ,SQUID  ,SQUID  ,SQUID  ,SQUID  ,SQUID  ,SQUID  },
       {CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   },
       {CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   },
       {OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS},
       {OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS}
    }}
  }};

  struct Alien
  {
    AlienClassId _id;
    Vector2i _position;
    bool _drawState;
    bool _isAlive;
  };

  struct GridIndex
  {
    int32_t _row;
    int32_t _col;
  };

  Vector2i _alienShiftDisplacement {2, 0};
  Vector2i _alienDropDisplacement {0, -8};

  int32_t _alienXSeperation {16};
  int32_t _alienYSeperation {16};

  int32_t _worldMargin {5};

  int32_t _worldLeftBorderX;
  int32_t _worldRightBorderX;

  Vector2i _aliensSpawnPosition;

  // The 2D grid of aliens indexed by [row][col].
  std::array<std::array<Alien, gridWidth>, gridHeight> _aliens; 
  
  // The alien to move in the next tick.
  GridIndex _nextMover;

  // Limited to values -1 for left, +1 for right.
  int32_t _alienMoveDirection; 

  bool _isAliensDropping;

  //-- LEVELS -----------------------------------------------------------------------------------//

  static constexpr int32_t levelCount {10};

  struct Level
  {
    int32_t _spawnDrops;
  };

  static constexpr std::array<Level, levelCount> _levels {{
    {5},
    {5},
    {5},
    {5},
    {5},
    {5},
    {5},
    {5},
    {5},
    {5},
  }};

  int32_t _levelNo;

  // The index into the _levels array which sets the level properties e.g. alien grid formation
  // and initial movement cycle.
  int32_t _levelIndex;

  int32_t _dropsDone;
  bool _isAliensSpawning;

  void startNextLevel();
  void endSpawning();

private:

  // Increments index from left-to-right along the columns, moving up a row and back to the left
  // most column upon reaching the end of the current column. Loops back to the bottom left most
  // column of the bottom row upon reaching the top-right of the grid. Returns true to indicate
  // a loop.
  bool incrementGridIndex(GridIndex& index);

  bool testAlienBorderCollision();


private:


  Vector2i _worldSize;
  int32_t _worldScale;


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


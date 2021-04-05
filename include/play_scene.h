#ifndef _SI_PLAY_SCENE_H_
#define _SI_PLAY_SCENE_H_

class GameState final : public ApplicationState
{
public:
  static constexpr const char* name = "game";

public:
  GameState(Application* app);
  ~GameState() = default;
  void initialize();
  void onUpdate(double now, float dt);
  void onDraw(double now, float dt);
  void onEnter();
  std::string getName(){return name;}

public:
  enum class State { 
    roundIntro, 
    aliensSpawning, 
    playing, 
    cannonSpawning, 
    victory, 
    gameOver,
    exit,
    none
  };

  class BeatBox
  {
  public:
    static constexpr int beatCount = 4;
    BeatBox() = default;
    BeatBox(std::array<Mixer::Key_t, beatCount> beats, float beatFreq_hz);
    void doBeats(float dt);
    void setBeatFreq(float freq_hz);
    float getBeatFreq() const {return _beatFreq_hz;}
    void pause(){_isPaused = true;}
    void unpause(){_isPaused = false;}
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

  struct Alien
  {
    AlienClassID _classID;
    Vector2i _position;
    int _row;
    int _col;
    bool _frame;
    bool _isAlive;
  };

  struct Ufo
  {
    Mixer::Channel_t _sfxChannel;
    SI::UfoClassID _classID;
    Vector2f _position;
    int _direction;
    float _phaseClock;
    bool _phase;
    bool _isAlive;
  };

  struct Bomb
  {
    SI::BombClassID _classID;
    Vector2f _position;
    float _frameClock;
    int _frame;
    bool _isAlive;
  };

  struct Boom
  {
    SI::BoomClassID _classID;
    Vector2i _position;
    float _boomClock;
    float _boomFrameClock;
    int _colorIndex;
    int _boomFrame;
    bool _isAlive;
  };

  struct Shot
  {
    SI::ShotClassID _classID;
    Vector2f _position;
    bool _isAlive;
  };

  struct Cannon
  {
    SI::CannonClassID _classID;
    Vector2f _position;
    float _spawnClock;
    int _moveDirection;
    int _shotCounter;
    bool _isVisible;
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

private:
  std::pair<int, int> getAlienRowRange(int row);
  Alien& getAlien(int row, int col);

  void addHudMsg(const char* endMsg, const Color3f& color);
  void removeHudMsg();

  void awardAlienScore(const Alien& alien);
  void awardUfoScore();

  inline bool canAlienBecomeCuttleTwin(const Alien& alien);

  void spawnCannon(bool takeLife);
  void spawnBoom(SI::BoomClassID classID, Vector2i position, int colorIndex); 
  void spawnBomb(int fleetCol, SI::BombClassID bombClassID);
  void spawnUfo(SI::UfoClassID classID);
  void spawnCuttleTwin(Alien& alien);
  void spawnFleet();

  void resetUfoSpawnCountdown();
  
  void moveFleet();
  void moveSpawningFleet();
  void moveUfo(float dt);
  void moveShot(float dt);
  void moveBombs(float dt);
  void moveCannon(float dt);

  void splitCuttleTwin(float dt);

  void dropTargetBomb(SI::BombClassID classID);
  void dropRandomBomb(SI::BombClassID classID);
  void dropBombs(float dt);

  void animateBombs(float dt);

  void ageBooms(float dt);
  void onBoomEnd(SI::BoomClassID classID);

  void phaseUfo(float dt);

  void signalUfo();

  void fireCannon();

  void killBomb(Bomb& bomb, bool boom);
  void killBomb(Bomb& bomb, bool boom, Vector2f boomPosition);
  void killAllBombs(bool boom);
  void killAlien(Alien& alien);
  void killCannon();
  void killShot(bool boom);
  void killAlien(bool boom);
  void killUfo(bool boom);

  void collideBombsCannon();
  void collideShotBombs();
  void collideShotFleet();
  void collideShotUfo();
  void collideShotSky();
  void collideUfoBorders();
  bool collideFleetBorders();

  void checkExit();
  void checkInvasion();
  void checkVictory();

  void onEnterRoundIntro();
  void onUpdateRoundIntro(float dt);
  void onExitRoundIntro();
  void onEnterAlienSpawning();
  void onUpdateAlienSpawning(float dt);
  void onExitAlienSpawning();
  void onEnterPlaying();
  void onUpdatePlaying(float dt);
  void onExitPlaying();
  void onEnterCannonSpawning();
  void onUpdateCannonSpawning(float dt);
  void onExitCannonSpawning();
  void onEnterVictory();
  void onUpdateVictory(float dt);
  void onExitVictory();
  void onEnterGameOver();
  void onUpdateGameOver(float dt);
  void onExitGameOver();
  void onEnterExit();

  void setNextState(State state);
  void isStateChange();
  void switchState();



  //------------

  void startNextRound();


  //void doAlienBooming(float dt);
  //void doFleetBeats();
  void doAbortToMenuTest();
  //void updateBeatFreq();
  //void updateActiveCycle();
  //void updateActiveCycleBeat();
  void startGameOver();
  void doGameOver(float dt);
  void endSpawning();
  void spawnBunker(Vector2f position, Assets::Key_t bitmapKey);

  void boomBunker(Bunker& bunker, Vector2i hitPixel);

  //void doCannonBooming(float dt);
  //void doUfoBoomScoring(float dt);
  //
  //
  void doUfoReinforcing(float dt);


  void collideBombsHitbar();
  void collideBunkersBombs();
  void collideBunkersLaser();
  void collideBunkersAliens();
  
  //bool incrementFleetIndex(FleetIndex& index);
  
  void drawFleet();
  void drawUfo();
  void drawCannon();
  void drawBombs();
  void drawBooms();
  void drawLaser();
  void drawHitbar();
  void drawBunkers();


private:
  SpaceInvaders* _si;   // cast from the _app pointer in the base class.

  const Font* _font;
  HUD* _hud;

  BeatBox _beatBox;

  State _state;
  State _newState;

  //Vector2i _alienShiftDisplacement;

  //static constexpr int32_t minSpawnDrops {6};
  //static constexpr int32_t baseAlienDropDisplacement {14};
  //static constexpr int32_t baseAlienSpawnDropDisplacement {7}; // fast baby steps
  //static constexpr int32_t baseAlienTopRowHeight {186}; // num drops to invasion--v
  //static constexpr int32_t baseAlienInvasionRowHeight {32}; // note: 186 == 32 + (11 * 14)
  //static constexpr int32_t numDropsToInvasion {11}; // for top row of aliens.

  //Vector2i _alienDropDisplacement;
  //Vector2i _alienSpawnDropDisplacement;
  //Vector2i _aliensSpawnPosition;         // position of bottom-left alien of bottom row at spawn.


  //
  // The alien fleet, indexed [row][col]. Rows are arraged as,
  //                y
  //    row4        ^
  //    row3        |  screen axes
  //    row2        |
  //    row1        o----> x
  //    row0       
  //
  // i.e. row number ascends up the screen.
  //
  // Columns are numbered from left to right in ascending order, starting with column 0.
  //
  // The alien at fleet position (row, col) is accessed via, 
  //            col + (row * fleetWidth)
  //
  std::array<Alien, fleetSize> _fleet; 

  std::array<int, fleetWidth>  _alienColPop;
  std::array<int, fleetHeight> _alienRowPop;

  Alien* _cuttleTwin;

  int _alienPop;
  int _alienMoveDirection;
  int _alienDropsDone;
  int _nextMover;
  float _cuttleTwinClock;
  bool _isAliensBooming;
  bool _isAliensDropping;
  bool _isAliensFrozen;
  bool _isAliensAboveInvasionRow;
  bool _haveAliensInvaded;
  bool _isFleetVisible;

  std::array<Bomb, SI::maxBombCount> _bombs;
  int _bombCount;
  int _bombReloadTableIndex;
  float _bombClock;

  std::array<Boom, SI::maxBoomCount> _booms;

  Cannon _cannon;
  Shot _shot;

  Ufo _ufo;
  int _ufoSpawnCountdown;
  int _ufoLastScoreEarned;

  int _activeRoundNo;

  float _msgClockSeconds;
  bool _isRoundIntro;
  bool _isGameOver;
  bool _isVictory;


  //static constexpr float schrodingerPhasePeriodSeconds {0.4f};
  //static constexpr int32_t schrodingerSpawnChance {3}; // 1 in spawnChance chance each ufo spawn.
  //Mixer::Channel_t _ufoSfxChannel;
  ////static constexpr int tillUfoMin {1200};// each update we do --tillUfo, so for updates at 60hz,
  ////static constexpr int tillUfoMax {1800};// to spawn ufo every 25s, set tillUfo = 25*60 = 1500.
  //int32_t _tillUfo;                      // when _tillUfo == 0, ufo spawns.
  //int32_t _ufoDirection;                 // Constraint: value=-1 (left) or value=1 (right).
  //int32_t _ufoCounter;
  //int32_t _ufoLastScoreGiven;
  //float _ufoSpawnY;                      // Height of ufos.
  //float _ufoSpeed;                       // Unit: pixels per second.
  //float _ufoBoomScoreDuration;           // Unit: seconds.
  //float _ufoPhaseDuration;               // Unit: seconds.
  //float _ufoBoomScoreClock;
  //bool _isUfoBooming;
  //bool _isUfoScoring;                    // Is the score displaying after the ufo was destroyed?
  //bool _canUfosSpawn;                    // ufos cannot spawn if alien pop <= 8

  //static constexpr int32_t cycleCount {13};
  //static constexpr int32_t spawnCycle {5};  // higher than 0 so we spawn faster.
  //static constexpr int32_t cycleLength {5}; // including end
  //static constexpr int32_t cycleStart {0};
  //static constexpr int32_t cycleEnd {-1};
  //using Cycle = std::array<int32_t, cycleLength>;
  //std::array<Cycle, cycleCount> _cycles;
  //std::array<int32_t, cycleCount> _cycleTransitions;
  //int32_t _activeCycle;
  //int32_t _activeBeat;  // A beat is an element of a cycle.

  //float _bombIntervalDeviation;                    // Max deviation in bomb drop beat count.
  //std::array<int32_t, cycleCount> _bombIntervals;  // Beats between bomb drops.
  //int32_t _bombInterval;                           // Base beat count between firing.
  //int32_t _bombClock;                              // Unit: beats - used to time the firing.





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


  HUD::uid_t _uidUfoScoringText;
  HUD::uid_t _uidMsgText;
};

#endif

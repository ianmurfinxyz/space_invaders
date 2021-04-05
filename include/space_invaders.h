#ifndef _SI_SPACEINVADERS_H_
#define _SI_SPACEINVADERS_H_

#include "pixiretro.h"

using namespace pxr;

//////////////////////////////////////////////////////////////////////////////////////////////////
// SPACE INVADERS                                                                               //
//////////////////////////////////////////////////////////////////////////////////////////////////

class SI final : public Application
{
public:

  ////////////////////////////////////////////////////////////////////////////////////////////////
  // APP DATA                                                                                   //
  ////////////////////////////////////////////////////////////////////////////////////////////////

  static constexpr const char* name {"Space Invaders"};

  static constexpr int32_t version_major {0};
  static constexpr int32_t version_minor {1};

  ////////////////////////////////////////////////////////////////////////////////////////////////
  // ASSET DATA                                                                                 //
  ////////////////////////////////////////////////////////////////////////////////////////////////
  
  enum BitmapKey : Assets::Key_t
  {
    BMK_CANNON0, 
    BMK_SQUID0, 
    BMK_SQUID1, 
    BMK_CRAB0, 
    BMK_CRAB1, 
    BMK_OCTOPUS0, 
    BMK_OCTOPUS1, 
    BMK_CUTTLE0, 
    BMK_CUTTLE1, 
    BMK_CUTTLETWIN, 
    BMK_SAUCER, 
    BMK_SCHRODINGER, 
    BMK_UFOBOOM, 
    BMK_CROSS0, 
    BMK_CROSS1, 
    BMK_CROSS2, 
    BMK_CROSS3, 
    BMK_ZIGZAG0, 
    BMK_ZIGZAG1, 
    BMK_ZIGZAG2, 
    BMK_ZIGZAG3, 
    BMK_ZAGZIG0, 
    BMK_ZAGZIG1, 
    BMK_ZAGZIG2, 
    BMK_ZAGZIG3, 
    BMK_LASER0, 
    BMK_CANNONBOOM0, 
    BMK_CANNONBOOM1, 
    BMK_CANNONBOOM2, 
    BMK_HITBAR, 
    BMK_ALIENBOOM, 
    BMK_BOOM_BOTTOM, 
    BMK_BOOM_MIDAIR, 
    BMK_BOOM_TOP, 
    BMK_BUNKER, 
    BMK_PARTII, 
    BMK_CONTROLS, 
    BMK_MENU, 
    BMK_SOS_TRAIL, 
    BMK_COUNT
  };

  static constexpr std::array<Assets::Name_t, BMK_COUNT> _bitmapNames {
    "cannon0", 
    "squid0", 
    "squid1", 
    "crab0", 
    "crab1", 
    "octopus0", 
    "octopus1", 
    "cuttle0", 
    "cuttle1",
    "cuttletwin", 
    "saucer", 
    "schrodinger", 
    "ufoboom", 
    "cross0", 
    "cross1", 
    "cross2", 
    "cross3", 
    "zigzag0", 
    "zigzag1", 
    "zigzag2", 
    "zigzag3", 
    "zagzig0", 
    "zagzig1", 
    "zagzig2", 
    "zagzig3", 
    "laser0", 
    "cannonboom0", 
    "cannonboom1", 
    "cannonboom2", 
    "hitbar", 
    "alienboom", 
    "bombboombottom", 
    "bombboommidair", 
    "bunker", 
    "partii", 
    "controls", 
    "menu", 
    "sostrail"
  };

  static constexpr Assets::Key_t fontKey {1};
  static constexpr Assets::Name_t fontName {"space"};

  enum SoundKey : Mixer::Key_t
  {
    SK_CANNON_BOOM, 
    SK_ALIEN_BOOM, 
    SK_UFO_BOOM, 
    SK_SHOOT, 
    SK_UFO_MOVE, 
    SK_FAST1, 
    SK_FAST2, SK_FAST3, 
    SK_FAST4, 
    SK_SCORE_BEEP, 
    SK_TOPSCORE, 
    SK_SOS, 
    SK_COUNT
  };

  static constexpr std::array<Mixer::Name_t, SK_COUNT> _soundNames {
    "cannonboom", 
    "alienboom", 
    "ufoboom", 
    "shoot", 
    "invadermorphed", 
    "ufomove", 
    "fastinvader1", 
    "fastinvader2", 
    "fastinvader3", 
    "fastinvader4", 
    "scorebeep", 
    "topscore", 
    "sos"
  };

  ////////////////////////////////////////////////////////////////////////////////////////////////
  // GAME CONTROLS                                                                              //
  ////////////////////////////////////////////////////////////////////////////////////////////////

  static constexpr pxr::Input::KeyCode fireKey      { pxr::Input::KeyCode::KEY_SPACE };
  static constexpr pxr::Input::KeyCode moveLeftKey  { pxr::Input::KeyCode::KEY_LEFT  };
  static constexpr pxr::Input::KeyCode moveRightKey { pxr::Input::KeyCode::KEY_RIGHT };

  ////////////////////////////////////////////////////////////////////////////////////////////////
  // GAME CONFIGURATION                                                                         //
  ////////////////////////////////////////////////////////////////////////////////////////////////

  // units
  // -----------------------------------------------
  // positions and displacements : pixels
  // clocks and durations        : seconds
  // speeds                      : pixels per second
  
  static constexpr Vector2i worldSize {224, 256};

  static constexpr int   worldWidth                   { 224  };
  static constexpr int   worldHeight                  { 256  };
  static constexpr int   alienSeperation              { 16   };
  static constexpr int   alienDropDisplacement        { 8    };
  static constexpr int   alienShiftDisplacement       { 2    };
  static constexpr int   alienFastShiftDisplacement   { 3    };
  static constexpr int   alienInvasionRowHeight       { 32   };
  static constexpr int   alienMinSpawnDrops           { 6    };
  static constexpr float cuttleTwinDuration           { 0.2f };
  static constexpr int   worldMargin                  { 5    };
  static constexpr int   worldTopMargin               { 30   };
  static constexpr int   fleetWidth                   { 11   };
  static constexpr int   fleetHeight                  { 5    };
  static constexpr int   minUfoSpawnCountdown         { 1200 };
  static constexpr int   maxUfoSpawnCountdown         { 1800 };
  static constexpr int   messageHeight                { 140  };
  static constexpr int   ufoSpawnHeight               { 210  };
  static constexpr int   maxBombCount                 { 3    };
  static constexpr int   maxBoomCount                 { 8    };
  static constexpr int   boomWidth                    { 8    };
  static constexpr int   boomHeight                   { 8    };
  static constexpr int   ufoSpawnHeight               { 210  };
  static constexpr float ufoScorePopupDuration        { 0.5f };
  static constexpr int   schrodingerSpawnChance       { 3    };
  static constexpr float cannonSpawnDuration          { 1.2f };

  static constexpr const char* messageGameOver {"GAME OVER!"};
  static constexpr const char* messageVictory  {"VICTORY!"};
  static constexpr const char* messageNewRound {"ROUND"};

  static constexpr int   fleetSize { fleetWidth * fleetHeight};
 
  enum AlienClassID { SQUID, CRAB, OCTOPUS, CUTTLE, CUTTLE_TWIN };

  struct AlienClass
  {
    static constexpr int alienFramesCount {2};
    std::array<Assets::Key_t, alienFramesCount> _bitmapKeys;
    int _width;
    int _height;
    int _scoreValue;
    int _colorIndex;
  };

  enum UfoClassID { SAUCER, SCHRODINGER };

  struct UfoClass
  {
    Assets::Key_t _bitmapKey;
    Mixer::Key_t _moveSfx;
    int _width;
    int _height;
    int _colorIndex;
    float _speed;
    float _phaseDuration;
    bool _isPhaser;
  };

  enum BombClassID { SQUIGGLY, ROLLING, PLUNGER };

  struct BombClass
  {
    static constexpr int bombFramesCount {4};
    std::array<Assets::Key_t, bombFramesCount> _bitmapKeys; 
    float _speed;
    int _width;
    int _height;
    int _colorIndex;
    int _frameInterval;
    int _laserSurvivalChance;
    bool _canTarget;
  };

  enum BoomClassID { BOOM_BOMB, BOOM_SHOT, BOOM_CANNON, BOOM_ALIEN, BOOM_UFO };

  struct BoomClass
  {
    static constexpr int boomFrameCount {2};
    std::array<Assets::Key_t, boomFrameCount> _bitmapKeys;
    Mixer::Key_t _boomSfx;
    int _width;
    int _height;
    float _boomDuration;
    float _boomFrameDuration;
  };

  enum CannonClassID { LASER_BASE };

  struct CannonClass
  {
    static constexpr int boomFrameCount {3};
    std::array<Assets::Key_t, boomFrameCount> _boomKeys;
    Assets::Key_t _bitmapKey;
    int _width;
    int _height;
    int _colorIndex;
    float _speed;
  };

  enum ShotClassID { SHOT_LASER };

  struct ShotClass
  {
    Assets::Key_t _bitmapKey;
    int _width;
    int _height;
    int _colorIndex;
    float _speed;
  };

  struct Round
  {
    int _startHeight;
    int _spawnDrops;
    int _formationIndex;
    bool _canCuttlesSpawn;
    bool _canSchrodingerSpawn;
  };

  static constexpr int paletteSize {7};
  static constexpr std::array<Color3f, paletteSize> palette {
    colors::red,    
    colors::green,  
    colors::blue,   
    colors::magenta,
    colors::cyan,   
    colors::yellow, 
    colors::white   
  };

  static constexpr int alienClassCount {5};
  static constexpr std::array<AlienClass, alienClassCount> alienClasses = {{
  //---------------------------------------------------------------------------------------------
  //  bitmap_keys                         width  height  score  color
  //---------------------------------------------------------------------------------------------
    {{BMK_SQUID0    , BMK_SQUID1     }  , 8    , 8      , 30    , 1  },
    {{BMK_CRAB0     , BMK_CRAB1      }  , 11   , 8      , 20    , 4  },
    {{BMK_OCTOPUS0  , BMK_OCTOPUS1   }  , 12   , 8      , 10    , 3  },
    {{BMK_CUTTLE0   , BMK_CUTTLE1    }  , 8    , 8      , 30    , 5  },
    {{BMK_CUTTLETWIN, BMK_CUTTLETWIN }  , 19   , 8      , 99999 , 5  }
  }};//                              impossible to get :) --^

  static constexpr int ufoClassCount {2};
  static constexpr std::array<UfoClass, ufoClassCount> ufoClasses = {{
  //---------------------------------------------------------------------------------------------
  //  bitmap_key       move_sfx     width  height  color  speed   phase  isphaser 
  //---------------------------------------------------------------------------------------------
    { BMK_SAUCER     , SK_UFO_MOVE, 16,    7,      0,     40.f,   0.f ,  false },
    { BMK_SCHRODINGER, SK_UFO_MOVE, 15,    7,      3,     40.f,   0.4f,  true  }
  }};

  static constexpr int ufoScoreTableSize;
  using UfoScoreTable = std::array<int, ufoScoreTableSize>;

  static constexpr std::array<UfoScoreTable, ufoClassCount> ufoScoreTable = {{
    {100, 50 , 50 , 100, 150, 100, 100, 50 , 300, 100, 100, 100, 50 , 150, 100, 50  },
    {200, 100, 100, 200, 300, 200, 200, 100, 600, 200, 200, 200, 100, 300, 200, 100 }
  }};

  static constexpr int bombClassCount {3};
  static constexpr std::array<BombClass, bombClassCount> bombClasses = {{
  //---------------------------------------------------------------------------------------------
  // bitmap_keys      speed   width  height  color  frame  survival   targets
  //---------------------------------------------------------------------------------------------
    {{BMK_PLUNGER0  , 
      BMK_PLUNGER1  , 
      BMK_PLUNGER2  , 
      BMK_PLUNGER3 }, -80.f,  3,     6,      0,     20,    0,       false},
    {{BMK_SQUIGGLE0 , 
      BMK_SQUIGGLE1 , 
      BMK_SQUIGGLE2 , 
      BMK_SQUIGGLE3}, -120.f, 3,     7,      4,     20,    10,      false },
    {{BMK_ROLLING0  , 
      BMK_ROLLING1  , 
      BMK_ROLLING2  , 
      BMK_ROLLING3 }, -100.f, 3,     7,      5,     20,    4,       true }
  }};

  static constexpr int boomClassCount {3};
  static constexpr std::array<BoomClass, boomClassCount> boomClasses = {{
  //---------------------------------------------------------------------------------------------
  // bitmap_keys                            boom_sfx         width  height  duration  frame_dur
  //---------------------------------------------------------------------------------------------
    {{BMK_BOOM_BOMB   , BMK_BOOM_BOMB    }, SK_COUNT       , 8,     8,      0.4f,     1.0f },
    {{BMK_BOOM_LASER  , BMK_BOOM_LASER   }, SK_COUNT       , 8,     8,      0.4f,     1.0f },
    {{BMK_BOOM_CANNON0, BMK_BOOM_CANNON1 }, SK_CANNON_BOOM , 8,     14,     1.0f,     0.2f },
    {{BMK_BOOM_ALIEN  , BMK_BOOM_ALIEN   }, SK_ALIEN_BOOM  , 9,     13,     0.1f,     1.f  },
    {{BMK_BOOM_UFO    , BMK_BOOM_UFO     }, SK_UFO_BOOM    , 8,     22,     0.5f,     1.f  }
  }};

  // The reload table defines the rate at which the aliens reload their bombs, i.e. the alien
  // fire (bomb) rate. The reload duration is a time duration, a delay in seconds, between each 
  // bomb. The score is the player score. The rate with the smallest associated score that is 
  // larger than the current player score is the active rate. For example, a player score of 
  // 5600 will use table entry 2, i.e. 0.8f.

  struct BombReloadTableEntry
  {
    int _score;
    float _reloadDuration;
  };

  static constexpr int bombReloadTableSize {5};
  static constexpr std::array<BombReloadTableEntry> bombReloadTable = {{
  //---------------------------------------------------------------------------------------------
  // score     reload
  //---------------------------------------------------------------------------------------------
    {512   ,   2.4f  },
    {4096  ,   0.8f  },
    {8192  ,   0.55f },
    {12288 ,   0.4f  },
    {99999 ,   0.35f }
  }};

  // note: formations are read upside down, so the top row in the file is the bottom row in
  // the game.

  using Formation = std::array<AlienClassID, fleetSize>;

  static constexpr int formationCount {2};
  static constexpr std::array<Formation, formationCount> formations = {{
  //---------------------------------------------------------------------------------------------
  // formation 0
  //---------------------------------------------------------------------------------------------
    {
      OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,
      OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,
      CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   ,
      CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   ,
      SQUID  ,SQUID  ,SQUID  ,SQUID  ,SQUID  ,SQUID  ,SQUID  ,SQUID  ,SQUID  ,SQUID  ,SQUID  
    },
  //---------------------------------------------------------------------------------------------
  // formation 1
  //---------------------------------------------------------------------------------------------
    {
      OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,
      OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,
      CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   ,
      CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   ,
      SQUID  ,SQUID  ,SQUID  ,SQUID  ,SQUID  ,SQUID  ,SQUID  ,SQUID  ,SQUID  ,SQUID  ,SQUID  
    }
  }};

  static constexpr int cannonClassCount {1};
  static constexpr std::array<CannonClass, cannonClassCount> cannonClasses = {{
  //---------------------------------------------------------------------------------------------
  //  boom_keys          bitmap_key   width   height   color   speed
  //---------------------------------------------------------------------------------------------
    {{BMK_CANNONBOOM0,
      BMK_CANNONBOOM1, 
      BMK_CANNONBOOM2 }, BMK_CANNON0, 13,     8,       0,      50.f }
  }};

  static constexpr int shotClassCount {1};
  static constexpr std::array<ShotClass, shotClassCount> shotClasses = {{
  //---------------------------------------------------------------------------------------------
  // bitmap_key   width   height   color   speed
  //---------------------------------------------------------------------------------------------
    {BMK_LASER,   1,      6,       6,      300.f  }
  }};

  // The start_y is the y-axis position of the reference alien (bottom-left alien) post spawn 
  // drops. Alien positions are the bottom-left coordinate of the aliens's 16x16px bounding box.
  //
  // From the highest start_y of 120px, the reference alien must drop 11 times to reach the
  // invasion row, 
  //                     120px - (11 * 8px) = 32px
  //                 drop_displacement--^      ^--invasion row height
  //
  // This gives a total of 6 + 11 = 17 drops from the spawn row to the invasion row.
  //                       ^--spawn drops to reach highest start_y of 120px

  static constexpr int roundCount {9};
  static constexpr std::array<Round, roundCount> rounds = {{
  //---------------------------------------------------------------------------------------------
  // start_y  spawn_drops  formation  cuttles  schrodinger
  //---------------------------------------------------------------------------------------------
    {120    , 6          , 0        , true   , true       },
    {96     , 9          , 0        , true   , true       },
    {80     , 11         , 0        , true   , true       },
    {72     , 12         , 0        , true   , true       },
    {72     , 12         , 0        , true   , true       },
    {72     , 12         , 0        , true   , true       },
    {64     , 13         , 0        , true   , true       },
    {64     , 13         , 0        , true   , true       },
    {64     , 13         , 0        , true   , true       }
  }};

  static constexpr int   alienReferenceSpawnX 
  { (worldWidth - (fleetWidth * alienSeperation)) / 2 };

  static constexpr int   alienReferenceSpawnY         
  { rounds[0]._startHeight + (alienMinSpawnDrops * alienDropDisplacement) };

  static constexpr int   worldLeftBorderX  { worldMargin };
  static constexpr int   worldRightBorderX { worldWidth - worldMargin };
  static constexpr int   worldTopBorderY   { worldHeight - worldTopMargin };
  static constexpr int   cannonSpawnX      { worldLeftBorder };
  static constexpr int   cannonSpawnY      { alienInvasionRowHeight };


  static constexpr int maxPlayerLives {4};
  static constexpr int maxLivesHudCannons {maxPlayerLives - 1};

  static constexpr size_t hiscoreNameLen = sizeof(int32_t);  
  static constexpr size_t hiscoreCount = 10;

  using ScoreName = std::array<char, hiscoreNameLen>; // warning: not null terinated!
                                            
  static constexpr int32_t nameToInt(const ScoreName& name)
  {
    static_assert(hiscoreNameLen == sizeof(int32_t));
    int32_t iname {0};
    for(size_t i{hiscoreNameLen}; i >= 1; --i)
      iname |= static_cast<int32_t>(name[i - 1]) << ((i - 1) * 8);
    return iname;
  }

  static constexpr ScoreName intToName(int32_t iname)
  {
    static_assert(hiscoreNameLen == sizeof(int32_t));
    ScoreName name {};
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
    ScoreName _name;
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
      {SCORE0, "score0", 120                         , 0, std::numeric_limits<int32_t>::max()},
      {NAME1 , "name1" , nameToInt({'_','A','N','_'}), 0, std::numeric_limits<int32_t>::max()},
      {SCORE1, "score1", 340                         , 0, std::numeric_limits<int32_t>::max()},
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
      {SCORE8, "score8", 60                          , 0, std::numeric_limits<int32_t>::max()},
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
  void hideTopHud();
  void showTopHud();
  void hideLivesHud();
  void showLivesHud();

  void setScore(int32_t score){_score = score;}
  void addScore(int32_t score){_score += score;}
  int32_t getScore() const {return _score;}
  void startScoreHudFlash();
  void stopScoreHudFlash();

  void setRound(int32_t round){_round = round;}
  void addRound(int32_t round){_round += round;}
  int32_t getRound() const {return _round;}

  void setCredit(int32_t credit){_credit = credit;}
  void addCredit(int32_t credit){_credit += credit;}
  int32_t getCredit() const {return _credit;}

  void setLives(int32_t lives);
  void addLives(int32_t lives);
  int32_t getLives() const {return _lives;}
  void updateLivesHud();

  void resetGameStats();

  Vector2i getWorldSize() const {return _worldSize;}
  int32_t getWorldScale() const {return _worldScale;}

  void loadHiScores();
  void writeHiScores();
  bool isHiScore(int32_t scoreValue);
  bool isDuplicateHiScore(const Score& score);
  bool registerHiScore(const Score& score);
  size_t findScoreBoardPosition(int32_t scoreValue);
  void updateHudHiScore();

  void setLastAlienClassAlive(AlienClassID classID);
  AlienClassID getLastAlienClassAlive() const;

  //
  // scores are guaranteed to be sorted with increasing scores in order of ascending index.
  //
  const std::array<Score, hiscoreCount>& getHiScores() const {return _hiscores;}

  void clearPlayerName() {_playerName[0] = '\0';}
  void setPlayerName(ScoreName name) {_playerName = name;}
  ScoreName getPlayerName() {return _playerName;}

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
  HUD::uid_t _uidLivesValue;
  std::array<HUD::uid_t, maxLivesHudCannons> _uidLivesBitmaps;
  int32_t _score;
  int32_t _hiscore;
  int32_t _round;
  int32_t _credit;
  int32_t _lives;
  ScoreName _playerName;
  bool _isLivesHudVisible;
  bool _isHudVisible;
};

#endif

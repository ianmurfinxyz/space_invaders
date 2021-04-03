#ifndef _SPACEINVADERS_H_
#define _SPACEINVADERS_H_

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
    Assets::Key_t _shipKey;
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

  enum BoomClassID { BOOM_BOMB, BOOM_LASER, BOOM_CANNON, BOOM_ALIEN, BOOM_UFO };

  struct BoomClass
  {
    static constexpr int boomFrameCount {2};
    std::array<Assets::Key_t, boomFrameCount> _bitmapKeys;
    Mixer::Key_t _boomSfx;
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
    {{BMK_CUTTLETWIN, BMK_CUTTLETWIN }  , 19   , 8      , 60    , 5  }
  }}; 

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
  // bitmap_keys                           boom_sfx        duration    frame_duration
  //---------------------------------------------------------------------------------------------
    {{BMK_BOOM_BOMB, BMK_BOOM_BOMB      }, SK_COUNT      , 0.4f,       1.0f   },
    {{BMK_BOOM_LASER, BMK_BOOM_LASER    }, SK_COUNT      , 0.4f,       1.0f   },
    {{BMK_BOOM_CANNON0, BMK_BOOM_CANNON1}, SK_CANNON_BOOM, 1.0f,       0.2f   },
    {{BMK_BOOM_ALIEN, BMK_BOOM_ALIEN    }, SK_ALIEN_BOOM , 0.1f,       1.f    },
    {{BMK_BOOM_UFO  , BMK_BOOM_UFO      }, SK_UFO_BOOM   , 0.5f,       1.f    }
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

  // note: the start_y is the y-axis position of the reference alien (bottom-left alien) post
  // spawn drops. Alien positions are the bottom-left coordinate of the aliens's 16x16px 
  // bounding box.

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

//===============================================================================================//
// ##>GAME STATE                                                                                 //
//===============================================================================================//

class SosState;

class GameState final : public ApplicationState
{
  friend SosState; // a bodge!

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

  enum class State { roundIntro, aliensSpawning, playing, cannonSpawning, victory, gameOver };

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
    int _moveDirection;
    int _shotCounter;
    bool _isAlive;
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

  enum class State { roundIntro, aliensSpawning, playing, cannonSpawning, victory, gameOver };

  void onEnterRoundIntro();
  void onUpdateRoundIntro();
  void onExitRoundIntro();
  void onEnterAlienSpawning();
  void onUpdateAlienSpawning();
  void onExitAlienSpawning();
  void onEnterPlaying();
  void onUpdatePlaying();
  void onExitPlaying();
  void onEnterCannonSpawning();
  void onUpdateCannonSpawning();
  void onExitCannonSpawning();
  void onEnterVictory();
  void onUpdateVictory();
  void onExitVictory();
  void onEnterGameOver();
  void onUpdateGameOver();
  void onExitGameOver();

  void switchState(State state);

  void spawnUfo(SI::UfoClassID classID);
  void spawnBomb(int fleetCol, SI::BombClassID bombClassID);
  void spawnBoom(SI::BoomClassID classID, Vector2i position, int colorIndex); 
  void spawnCuttleTwin(Alien& alien);
  void spawnCannon(bool takeLife);

  void resetUfoSpawnCountdown();
  
  void moveAliens();
  void moveUfo(float dt);
  void moveShot(float dt);
  void moveBombs(float dt);
  void moveCannon(float dt);

  void splitCuttleTwin(float dt);

  void dropTargetBomb(SI::BombClassID classID);
  void dropRandomBomb(SI::BombClassID classID);
  void dropAlienBombs(float dt);

  void animateBombs(float dt);

  void ageBooms(float dt);
  void onBoomEnd(SI::BoomClassID classID);

  void phaseUfo(float dt);

  void signalUfo();

  void fireCannon();

  void killBomb(Bomb& bomb);
  void killAllBombs();
  void killAlien(Alien& alien);
  void killCannon();



  //------------

  void startNextRound();


  //void doAlienBooming(float dt);
  //void doFleetBeats();
  void doAbortToMenuTest();
  //void updateBeatFreq();
  //void updateActiveCycle();
  //void updateActiveCycleBeat();
  void addHudMsg(const char* endMsg, const Color3f& color);
  void removeHudMsg();
  void startRoundIntro();
  void doRoundIntro(float dt);
  void doInvasionTest();
  void startGameOver();
  void doGameOver(float dt);
  void doVictoryTest();
  void startVictory();
  void doVictory(float dt);
  void endSpawning();
  void spawnBunker(Vector2f position, Assets::Key_t bitmapKey);

  void boomUfo();
  void boomLaser(bool makeBoom, BombHit hit = BOMBHIT_MIDAIR);
  void boomBunker(Bunker& bunker, Vector2i hitPixel);

  //void doCannonBooming(float dt);
  //void doUfoBoomScoring(float dt);
  //
  //
  void doUfoReinforcing(float dt);

  void collideBombsCannon();

  void collideUfoBorders();
  void collideBombsHitbar();
  void collideBombsLaser();
  void collideLaserAliens();
  void collideLaserUfo();
  void collideLaserSky();
  bool collideAliensBorders();
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
  bool _isAliensSpawning;
  bool _isAliensDropping;
  bool _isAliensFrozen;
  bool _isAliensAboveInvasionRow;
  bool _haveAliensInvaded;

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

//===============================================================================================//
// ##>SOS STATE                                                                                  //
//===============================================================================================//

class SosState final : public ApplicationState
{
public:
  static constexpr const char* name = "sos";

public:
  SosState(Application* app) : ApplicationState(app){}
  ~SosState() = default;

  void initialize(Vector2i worldSize, int32_t worldScale);
  void onUpdate(double now, float dt);
  void onDraw(double now, float dt);
  void onEnter();

  std::string getName(){return name;}

private:
  static constexpr int32_t baseWorldMargin_px {60};    // base == before world scale.
  static constexpr int32_t baseSpawnHeight_px {60};
  static constexpr int32_t baseTopMargin_px {40};
  static constexpr int32_t sosTextMargin_px {10};
  static constexpr int32_t sosTrailSpace_px {8};
  static constexpr float baseMoveSpeed {75};
  static constexpr float moveAngleRadians {0.9899310886f}; // 55 deg
  static constexpr float engineFailPeriodSeconds {3.f};

  static constexpr const char* troubleText {"ENGINE TROUBLE"};
  static constexpr const char* sosText {"SOS  !!"};

  //
  // 1 in 'engineFailChance' chance for engine to fail each update, there are 60 updates 
  // each second, and the intermission lasts ~4 seconds, so 240 updates. I want the engine fail
  // to happen around 1 in every 3 intermissions, so about once for every 240*3=720 updates.
  // Thus have set the fail chance to 720. My tests show this works about right.
  //
  // engineFailHit is the number that the generator must return to indicate a fail has occured,
  // I am asuming the std::distribution has equal chance to return all numbers (as it claims) so 
  // this value should not matter (unless there actually is bias, in which case it does).
  //
  static constexpr int engineFailChance {720};
  static constexpr int engineFailHit {0};

  struct Alien
  {
    static constexpr const float framePeriodSeconds {0.1f};
    GameState::AlienClassID _classID;
    Vector2f _position;
    Vector2f _failPosition;
    bool _frame;
    float _frameClockSeconds;
  };

  struct Ufo
  {
    GameState::UfoClassID _classID;
    Vector2f _position;
    int32_t _width;       // store here for faster access.
  };

private:
  void doMoving(float dt);
  void doAlienAnimating(float dt);
  void doEngineFailing(float dt);
  void doEngineCheck();
  void doWallColliding();
  void doEndTest();
  void doDirectionChange();

private:
  //
  // This is a bodge to allow the sos state to get access to the alien and ufo class data in
  // the game state. I initially designed the app states to be self contained and to not 
  // communicate or share data, thus this is required. Not pretty but at this late stage in
  // the project, it works.
  //
  friend SpaceInvaders;
  GameState* _gameState;

  const Font* _font;
  HUD* _hud;
  Alien _alien;
  Ufo _ufo;
  Vector2i _worldSize;
  int32_t _worldScale;
  int32_t _exitHeight_px;
  int32_t _worldLeftMargin_px;
  int32_t _worldRightMargin_px;
  int32_t _spawnMargin_px;
  int32_t _spawnHeight_px;
  int32_t _sosTextPositionX;
  Vector2i _moveVelocity;
  Mixer::Channel_t _woowooChannel;
  HUD::uid_t _uidTroubleText;
  static constexpr int maxSosTextDrop {4};
  std::array<HUD::uid_t, maxSosTextDrop> _uidSosText;
  int32_t _nextSosText;
  float _moveSpeed;
  float _engineFailClockSeconds;
  bool _hasEngineFailed;
  bool _isEngineFailing;
  bool _isWooing;
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
  void onEnter();

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

//===============================================================================================//
// ##>HISCORE REGISTRATION STATE                                                                 //
//===============================================================================================//

class HiScoreRegState final : public ApplicationState
{
public:
  static constexpr const char* name = "scoreReg";

public:
  HiScoreRegState(Application* app) : ApplicationState{app}{}
  ~HiScoreRegState() = default;

  void initialize(Vector2i worldSize, int32_t worldScale);
  void onUpdate(double now, float dt);
  void onDraw(double now, float dt);
  void onEnter();

  std::string getName(){return name;}

private:

  class Keypad
  {
  public:
    Keypad(const Font& font, Vector2i worldSize, int32_t worldScale);

    void moveCursor(int32_t colShift, int32_t rowShift);
    void reset();
    void draw();

    const char* getActiveKeyText() const
    { return _keyText[_cursorPadPosition._y][_cursorPadPosition._x]; }

  private:
    void updateCursorScreenPosition();

  private:
    static constexpr int32_t keyRowCount {4};
    static constexpr int32_t keyColCount {11};
    static constexpr int32_t keySpace_px {4};
    static constexpr int32_t cursorDrop_px {keySpace_px / 2};
    static constexpr const char* cursorChar {"_"};
    static constexpr Vector2i initialCursorPadPosition {0, keyRowCount - 1};

  private:
    std::array<std::array<const char*, keyColCount>, keyRowCount> _keyText;
    std::array<std::array<Vector2i, keyColCount>, keyRowCount> _keyScreenPosition;
    Color3f _keyColor;
    Color3f _specialKeyColor;
    Color3f _cursorColor;
    Vector2i _cursorPadPosition; // x=col, y=row
    Vector2i _cursorScreenPosition;
    Vector2i _padScreenPosition;
    const Font& _font;
  };

  class NameBox
  {
  public:
    NameBox(const Font& font, Vector2i worldSize, int32_t worldScale);
    void draw();
    bool pushBack(char c);
    bool popBack();
    SpaceInvaders::ScoreName getBufferText() const {return _nameBuffer;}
    bool isFull() const;
    bool isEmpty() const;

  private:
    void composeFinal();

  private:
    static constexpr char nullChar {'-'};
    static constexpr char quoteChar {'\''};
    static constexpr const char* label = "NAME";

  private:
    SpaceInvaders::ScoreName _nameBuffer; 
    Vector2i _boxScreenPosition;
    const Font& _font;
    std::string _final;
  };

private:
  void doInput();

private:

  std::unique_ptr<Keypad> _keypad;
  std::unique_ptr<NameBox> _nameBox;
};

//===============================================================================================//
// ##>HISCORE BOARD STATE                                                                        //
//===============================================================================================//

class HiScoreBoardState final : public ApplicationState
{
public:
  static constexpr const char* name = "scoreBoard";

public:
  HiScoreBoardState(Application* app) : ApplicationState{app}{}
  ~HiScoreBoardState() = default;

  void initialize(Vector2i worldSize, int32_t worldScale);
  void onUpdate(double now, float dt);
  void onDraw(double now, float dt);
  void onEnter();

  std::string getName(){return name;}

private:
  static constexpr int32_t rowSeperation {2};
  static constexpr int32_t colSeperation {8};
  static constexpr int32_t boardTitleSeperation {20};
  static constexpr int32_t scoreDigitCountEstimate {4};
  static constexpr Color3f oldScoreColor {colors::magenta};
  static constexpr Color3f newScoreColor {colors::green};
  static constexpr Color3f titleColor {colors::cyan};
  static constexpr float enterDelaySeconds {1.f};
  static constexpr float topScoreExitDelaySeconds {7.f};
  static constexpr float normalExitDelaySeconds {1.f};
  static constexpr float swapScoreDelaySeconds {0.5f};
  static constexpr SpaceInvaders::ScoreName placeHolderName {'Y', 'O', 'U', '_'};
  static constexpr const char* titleString {"*HI-SCORER LEADERBOARD*"};

private:
  bool doScoreSwap();
  void populateHud();
  void depopulateHud();
  bool newScoreIsTop();

private:
  int32_t _eventNum;
  float _eventClock;
  SpaceInvaders::Score _newScore;
  std::array<const SpaceInvaders::Score*, SpaceInvaders::hiscoreCount + 1> _scoreBoard;
  const Font* _font;
  Vector2i _nameScreenPosition;
  Vector2i _scoreScreenPosition;
  HUD::uid_t _uidTitleText;
  Vector2i _scoreBoardSize;
  float _exitDelaySeconds;
};

#endif

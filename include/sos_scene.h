#ifndef _SI_SOS_SCENE_H_
#define _SI_SOS_SCENE_H_

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

#endif

#ifndef _SI_HISCORE_BOARD_SCENE_H_
#define _SI_HISCORE_BOARD_SCENE_H_

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

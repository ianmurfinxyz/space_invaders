#ifndef _SI_MENU_SCENE_H_
#define _SI_MENU_SCENE_H_

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

#endif

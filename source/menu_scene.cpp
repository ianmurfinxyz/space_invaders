#include "menu_scene.h"

void MenuState::initialize(Vector2i worldSize, int32_t worldScale)
{
  _worldScale = worldScale;
}

void MenuState::onUpdate(double now, float dt)
{
  SpaceInvaders* si = static_cast<SpaceInvaders*>(_app);
  if(pxr::input->isKeyPressed(Input::KEY_ENTER)){
    depopulateHud();
    si->hideHud();
    _app->switchState(GameState::name);
  }
  if(pxr::input->isKeyPressed(Input::KEY_s)){
    depopulateHud();
    si->hideHud();
    _app->switchState(HiScoreBoardState::name);
  }
}

void MenuState::onDraw(double now, float dt)
{
  renderer->clearViewport(colors::black);
}

void MenuState::onEnter()
{
  SpaceInvaders* si = static_cast<SpaceInvaders*>(_app);
  populateHud();
  si->showHud();
  si->hideLivesHud();
  si->showTopHud();
  si->resetGameStats();
}

void MenuState::populateHud()
{
  SpaceInvaders* si = static_cast<SpaceInvaders*>(_app);
  HUD& hud = si->getHud();
  _uidMenuText = hud.addTextLabel({Vector2i{91, 204} * _worldScale, pxr::colors::cyan, "*MENU*"});
  _uidMenuBitmap = hud.addBitmapLabel({Vector2i{56, 182} * _worldScale, pxr::colors::white, &(pxr::assets->getBitmap(SpaceInvaders::BMK_MENU, _worldScale))});
  _uidControlsText = hud.addTextLabel({Vector2i{76, 162} * _worldScale, pxr::colors::cyan, "*CONTROLS*"});
  _uidControlsBitmap = hud.addBitmapLabel({Vector2i{58, 134} * _worldScale, pxr::colors::white, &(pxr::assets->getBitmap(SpaceInvaders::BMK_CONTROLS, _worldScale))});
  _uidTablesText = hud.addTextLabel({Vector2i{40, 108} * _worldScale, pxr::colors::cyan, "*SCORE ADVANCE TABLE*"});
  _uidSchroBitmap = hud.addBitmapLabel({Vector2i{62, 90} * _worldScale, pxr::colors::magenta, &(pxr::assets->getBitmap(SpaceInvaders::BMK_SCHRODINGER, _worldScale))});
  _uidSaucerBitmap = hud.addBitmapLabel({Vector2i{62, 74} * _worldScale, pxr::colors::magenta, &(pxr::assets->getBitmap(SpaceInvaders::BMK_SAUCER, _worldScale))});
  _uidSquidBitmap = hud.addBitmapLabel({Vector2i{66, 58} * _worldScale, pxr::colors::yellow, &(pxr::assets->getBitmap(SpaceInvaders::BMK_SQUID0, _worldScale))});
  _uidCuttleBitmap = hud.addBitmapLabel({Vector2i{52, 58} * _worldScale, pxr::colors::yellow, &(pxr::assets->getBitmap(SpaceInvaders::BMK_CUTTLE0, _worldScale))});
  _uidCrabBitmap = hud.addBitmapLabel({Vector2i{64, 42} * _worldScale, pxr::colors::yellow, &(pxr::assets->getBitmap(SpaceInvaders::BMK_CRAB0, _worldScale))});
  _uidOctopusBitmap = hud.addBitmapLabel({Vector2i{64, 26} * _worldScale, pxr::colors::red, &(pxr::assets->getBitmap(SpaceInvaders::BMK_OCTOPUS0, _worldScale))});
  _uid500PointsText = hud.addTextLabel({Vector2i{82, 90} * _worldScale, pxr::colors::magenta, "= 500 POINTS", 0.f, true});
  _uidMysteryPointsText = hud.addTextLabel({Vector2i{82, 74} * _worldScale, pxr::colors::magenta, "= ? MYSTERY", 1.f, true});
  _uid30PointsText = hud.addTextLabel({Vector2i{82, 58} * _worldScale, pxr::colors::yellow, "= 30 POINTS", 2.f, true});
  _uid20PointsText = hud.addTextLabel({Vector2i{82, 42} * _worldScale, pxr::colors::yellow, "= 20 POINTS", 3.f, true});
  _uid10PointsText = hud.addTextLabel({Vector2i{82, 26} * _worldScale, pxr::colors::red, "= 10 POINTS", 4.f, true});
}

void MenuState::depopulateHud()
{
  SpaceInvaders* si = static_cast<SpaceInvaders*>(_app);
  HUD& hud = si->getHud();
  hud.removeTextLabel(_uidMenuText);
  hud.removeTextLabel(_uidControlsText);
  hud.removeTextLabel(_uidTablesText);
  hud.removeTextLabel(_uid500PointsText);
  hud.removeTextLabel(_uidMysteryPointsText);
  hud.removeTextLabel(_uid30PointsText);
  hud.removeTextLabel(_uid20PointsText);
  hud.removeTextLabel(_uid10PointsText);
  hud.removeBitmapLabel(_uidMenuBitmap);
  hud.removeBitmapLabel(_uidControlsBitmap);
  hud.removeBitmapLabel(_uidSchroBitmap);
  hud.removeBitmapLabel(_uidSaucerBitmap);
  hud.removeBitmapLabel(_uidSquidBitmap);
  hud.removeBitmapLabel(_uidCuttleBitmap);
  hud.removeBitmapLabel(_uidCrabBitmap);
  hud.removeBitmapLabel(_uidOctopusBitmap);
}


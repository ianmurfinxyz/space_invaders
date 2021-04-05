#include "splash_scene.h"

SplashState::SplashState(Application* app) :
  ApplicationState{app}
{}

void SplashState::initialize(Vector2i worldSize, int32_t worldScale)
{
  _worldSize = worldSize;
  _worldScale = worldScale;

  _masterClock = 0.f;
  _nextNode = 0;
  _sequence = {{
    {1.f, EVENT_SHOW_SPACE_SIGN},
    {2.f, EVENT_TRIGGER_SPACE_SIGN},
    {4.5f, EVENT_SHOW_INVADERS_SIGN},
    {5.5f, EVENT_TRIGGER_INVADERS_SIGN},
    {7.3f, EVENT_SHOW_PART_II},
    {8.3f, EVENT_SHOW_HUD},
    {11.f, EVENT_END},
  }};

  _blockSize = 3 * _worldScale;
  _blockSpace = 1 * _worldScale;
  _signX = 16 * _worldScale;
  _spaceY = 192 * _worldScale;
  _invadersY = 112 * _worldScale;

  _spaceTriggered = false;
  _spaceVisible = false;
  _spaceSign = std::make_unique<Sign<spaceW, spaceH>>(Sign<spaceW, spaceH>{
    {{
      {1,2,2,2,2,1,1,1,1,1,2,2,2,2,2,2,2,1,1,1,1,1,2,2,2,2,2,1,1,1,1,1,1,2,2,2,2,1,1,1,1,2,2,2,2,2,2,2},
      {2,2,2,2,2,2,2,1,1,1,2,2,2,2,2,2,2,2,1,1,1,1,2,2,2,2,2,1,1,1,1,1,2,2,2,2,2,2,1,1,1,2,2,2,2,2,2,2},
      {2,2,2,2,2,2,2,2,1,1,2,2,2,2,2,2,2,2,2,1,1,2,2,2,2,2,2,2,1,1,1,2,2,2,2,2,2,2,2,1,1,2,2,2,2,2,2,2},
      {2,2,2,2,2,2,2,2,1,1,2,2,2,2,2,2,2,2,2,1,1,2,2,2,2,2,2,2,1,1,1,2,2,2,2,2,2,2,2,1,1,2,2,2,2,2,2,2},
      {2,2,1,1,1,2,2,2,1,1,2,2,2,1,1,1,2,2,2,1,2,2,2,2,2,2,2,2,2,1,1,2,2,2,1,1,2,2,2,1,2,2,2,1,1,1,1,1},
      {2,2,2,1,1,2,2,2,1,1,2,2,2,1,1,1,2,2,2,1,2,2,2,2,2,2,2,2,2,1,1,2,2,2,1,1,2,2,2,1,2,2,2,1,1,1,1,1},
      {1,2,2,2,1,1,1,1,1,1,2,2,2,1,1,1,2,2,2,1,2,2,2,1,2,1,2,2,2,1,1,2,2,2,1,1,1,1,1,1,2,2,2,1,1,1,1,1},
      {1,2,2,2,2,2,1,1,1,1,2,2,2,2,2,2,2,2,2,1,2,2,2,1,2,1,2,2,2,1,1,2,2,2,1,1,1,1,1,1,2,2,2,2,2,1,1,1},
      {1,1,2,2,2,2,2,2,1,1,1,2,2,2,2,2,2,2,1,1,2,2,2,2,2,2,2,2,2,1,2,2,2,1,1,1,1,1,1,2,2,2,2,2,2,1,1,1},
      {1,1,1,1,2,2,2,2,2,1,1,2,2,2,2,2,2,1,1,1,1,1,2,2,2,2,2,1,1,1,2,2,2,1,1,1,1,1,1,2,2,2,2,2,2,1,1,1},
      {1,1,1,1,1,1,2,2,2,1,1,2,2,2,1,1,1,1,1,1,1,2,2,2,1,2,2,2,1,1,2,2,2,1,1,1,1,1,1,2,2,1,1,1,1,1,1,1},
      {1,1,1,2,2,1,1,2,2,1,1,2,2,2,1,1,1,1,1,1,1,2,2,2,1,2,2,2,1,1,2,2,2,1,2,2,2,1,1,2,2,1,1,1,1,1,1,1},
      {1,1,1,2,2,1,1,2,2,1,1,2,2,2,1,1,1,1,1,1,2,2,2,1,1,1,2,2,2,1,2,2,2,1,2,2,2,1,2,2,2,1,1,1,1,1,1,1},
      {1,1,1,2,2,2,2,2,2,1,1,2,2,2,1,1,1,1,1,1,2,2,2,1,1,1,2,2,2,1,2,2,2,2,2,2,2,1,2,2,2,2,2,2,1,1,1,1},
      {1,1,1,1,2,2,2,2,1,1,1,2,2,2,1,1,1,1,1,1,1,2,2,2,1,2,2,2,1,1,1,2,2,2,2,2,1,1,2,2,2,2,2,2,1,1,1,1},
      {1,1,1,1,1,2,2,1,1,1,1,2,2,2,1,1,1,1,1,1,1,2,2,2,1,2,2,2,1,1,1,1,2,2,2,1,1,1,2,2,2,2,2,2,1,1,1,1},
    }},
    std::make_unique<Bitmap>(pxr::assets->makeBlockBitmap(_blockSize, _blockSize)),
    {_signX, _spaceY},
    pxr::colors::green,
    pxr::colors::cyan,
    0.002f,
    _blockSpace,
    _blockSize
  });

  _invadersTriggered = false;
  _invadersVisible = false;
  _invadersSign = std::make_unique<Sign<invadersW, invadersH>>(Sign<invadersW, invadersH>{
    {{
      {1,2,2,1,2,2,1,1,2,2,1,2,2,1,1,2,2,1,1,2,2,2,1,1,2,2,2,2,1,1,2,2,2,2,2,1,2,2,2,2,1,1,1,2,2,2,1,1},
      {1,2,2,1,2,2,1,1,2,2,1,2,2,1,1,2,2,1,1,2,2,2,1,1,2,2,1,2,2,1,2,2,1,1,1,1,2,2,1,2,2,1,2,2,1,2,2,1},
      {1,2,2,1,2,2,2,1,2,2,1,2,2,1,1,2,2,1,2,2,1,2,2,1,2,2,1,2,2,1,2,2,1,1,1,1,2,2,1,2,2,1,2,2,1,1,1,1},
      {1,2,2,1,2,2,2,2,2,2,1,2,2,1,1,2,2,1,2,2,1,2,2,1,2,2,1,2,2,1,2,2,1,1,1,1,2,2,1,2,2,1,2,2,1,1,1,1},
      {1,2,2,1,2,2,2,2,2,2,1,1,2,2,2,2,1,1,2,2,2,2,2,1,2,2,1,2,2,1,2,2,2,2,2,1,2,2,2,2,1,1,1,2,2,2,1,1},
      {1,2,2,1,2,2,1,2,2,2,1,1,2,2,2,2,1,1,2,2,1,2,2,1,2,2,1,2,2,1,2,2,1,1,1,1,2,2,1,2,2,1,1,1,1,2,2,1},
      {1,2,2,1,2,2,1,1,2,2,1,1,1,2,2,1,1,1,2,2,1,2,2,1,2,2,1,2,2,1,2,2,1,1,1,1,2,2,1,2,2,1,2,2,1,2,2,1},
      {1,2,2,1,2,2,1,1,2,2,1,1,1,2,2,1,1,1,2,2,1,2,2,1,2,2,2,2,1,1,2,2,2,2,2,1,2,2,1,2,2,1,1,2,2,2,1,1},
    }},
    std::make_unique<Bitmap>(pxr::assets->makeBlockBitmap(_blockSize, _blockSize)),
    {_signX, _invadersY},
    pxr::colors::magenta,
    pxr::colors::yellow,
    0.002f,
    _blockSpace,
    _blockSize
  });

  _partiiPosition = Vector2i{80, 48} * _worldScale;
  _partiiColor = pxr::colors::red;
  _partiiVisible = false;

  SpaceInvaders* si = static_cast<SpaceInvaders*>(_app);
  si->hideHud();
}

void SplashState::doEvents()
{
  if(_sequence[_nextNode]._time > _masterClock) 
    return;
  switch(_sequence[_nextNode]._event){
    case EVENT_SHOW_SPACE_SIGN:
      _spaceVisible = true;
      break;
    case EVENT_TRIGGER_SPACE_SIGN:
      _spaceTriggered = true;
      break;
    case EVENT_SHOW_INVADERS_SIGN:
      _invadersVisible = true;
      break;
    case EVENT_TRIGGER_INVADERS_SIGN:
      _invadersTriggered = true;
      break;
    case EVENT_SHOW_PART_II:
      _partiiVisible = true;
      break;
    case EVENT_SHOW_HUD:
      {
      SpaceInvaders* si = static_cast<SpaceInvaders*>(_app);
      HUD& hud = si->getHud();
      std::string text {"*A Tribute by Pixrex*"};
      _uidAuthor = hud.addTextLabel({Vector2i{32, 24} * _worldScale, pxr::colors::cyan, text});
      si->showHud();
      si->hideLivesHud();
      break;
      }
    case EVENT_END:
      {
      SpaceInvaders* si = static_cast<SpaceInvaders*>(_app);
      HUD& hud = si->getHud();
      hud.removeTextLabel(_uidAuthor);
      si->switchState(MenuState::name);
      break;
      }
    default:
      break;
  }
  ++_nextNode;
}

void SplashState::onEnter()
{
  static_cast<SpaceInvaders*>(_app)->hideHud();
  _masterClock = 0.f;
  _nextNode = 0;
  (*_spaceSign).reset();
  (*_invadersSign).reset();
  _spaceTriggered = false;
  _spaceVisible = false;
  _invadersTriggered = false;
  _invadersVisible = false;
  _partiiVisible = false;
}

void SplashState::onUpdate(double now, float dt)
{
  _masterClock += dt;
  doEvents();
  if(_spaceTriggered)
    _spaceSign->updateBlocks(dt);
  if(_invadersTriggered)
    _invadersSign->updateBlocks(dt);
}

void SplashState::onDraw(double now, float dt)
{
  renderer->clearViewport(colors::black);
  if(_spaceVisible)
    _spaceSign->draw();
  if(_invadersVisible)
    _invadersSign->draw();
  if(_partiiVisible){
    pxr::renderer->blitBitmap(_partiiPosition, 
                                pxr::assets->getBitmap(SpaceInvaders::BMK_PARTII, _worldScale), 
                                _partiiColor);
  }
}


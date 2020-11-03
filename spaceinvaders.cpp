#include "nomad.h"
#include "spaceinvaders.h"

//===============================================================================================//
// ##>GAME STATE                                                                                 //
//===============================================================================================//

const std::string GameState::name {"game"};

void GameState::initialize(Vector2i worldSize, int32_t worldScale)
{
  _worldScale = worldScale;
}

void GameState::onUpdate(double now, float dt)
{
  if(::input->getKeyState(Input::KEY_s) == Input::KeyState::PRESSED)
    _app->switchState(MenuState::name);
}

void GameState::onDraw(double now, float dt)
{
  renderer->clearViewport(colors::cyan);
  renderer->blitBitmap(Vector2f{100.f, 100.f}, assets->getBitmap(SpaceInvaders::BMK_CANNON0, _worldScale), colors::black);
  renderer->blitBitmap(Vector2f{150.f, 100.f}, assets->getBitmap(SpaceInvaders::BMK_SQUID0, _worldScale), colors::black);
}

void GameState::onReset()
{
}

//===============================================================================================//
// ##>MENU STATE                                                                                 //
//===============================================================================================//

const std::string MenuState::name {"menu"};

void MenuState::initialize(Vector2i worldSize, int32_t worldScale)
{
  _worldScale = worldScale;
}

void MenuState::onUpdate(double now, float dt)
{
  if(::input->getKeyState(Input::KEY_s) == Input::KeyState::PRESSED)
    _app->switchState(GameState::name);
}

void MenuState::onDraw(double now, float dt)
{
  renderer->clearViewport(colors::magenta);
  renderer->blitBitmap(Vector2f{100.f, 100.f}, assets->getBitmap(SpaceInvaders::BMK_CANNON0, _worldScale), colors::white);
  renderer->blitBitmap(Vector2f{150.f, 100.f}, assets->getBitmap(SpaceInvaders::BMK_SQUID0, _worldScale), colors::blue);
}

void MenuState::onReset()
{
}

//===============================================================================================//
// ##>SPACE INVADERS                                                                             //
//===============================================================================================//

const std::string SpaceInvaders::name {"Space Invaders"};

bool SpaceInvaders::initialize(Engine* engine, int32_t windowWidth, int32_t windowHeight)
{
  Application::initialize(engine, windowWidth, windowHeight);

  _worldScale = 1;
  while((baseWorldSize._x * _worldScale) < windowWidth && (baseWorldSize._y * _worldScale) < windowHeight)
    ++_worldScale; 

  --_worldScale;
  if(_worldScale == 0)
    _worldScale = 1;

  _worldSize = baseWorldSize * _worldScale;

  Application::onWindowResize(windowWidth, windowHeight);

  Assets::Manifest_t manifest{};
  for(int32_t i = BMK_CANNON0; i < BMK_COUNT; ++i){
    manifest.push_back({i, _bitmapNames[i], _worldScale}); 
  }

  assets->loadBitmaps(std::move(manifest));

  std::unique_ptr<ApplicationState> game {new GameState{this}};
  std::unique_ptr<ApplicationState> menu {new MenuState{this}};

  game->initialize(_worldSize, _worldScale);
  menu->initialize(_worldSize, _worldScale);

  addState(std::move(game));
  addState(std::move(menu));

  switchState(MenuState::name);

  return true;
}

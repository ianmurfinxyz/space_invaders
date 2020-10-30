#include "nomad.h"
#include "spaceinvaders.h"

//===============================================================================================//
// ##>GAME STATE                                                                                 //
//===============================================================================================//

const std::string GameState::name {"game"};

void GameState::initialize(Vector2i worldSize)
{
}

void GameState::onUpdate(double now, float dt)
{
  if(::input->getKeyState(Input::KEY_s) == Input::KeyState::PRESSED)
    _app->switchState(MenuState::name);
}

void GameState::onDraw(double now, float dt)
{
  renderer->clearViewport(colors::cyan);
  renderer->blitBitmap(Vector2f{100.f, 100.f}, assets->getBitmap(SpaceInvaders::BMK_CANNON0), colors::black);
  renderer->blitBitmap(Vector2f{150.f, 100.f}, assets->getBitmap(SpaceInvaders::BMK_SQUID0), colors::black);
}

void GameState::onReset()
{
}

//===============================================================================================//
// ##>MENU STATE                                                                                 //
//===============================================================================================//

const std::string MenuState::name {"menu"};

void MenuState::initialize(Vector2i worldSize)
{
}

void MenuState::onUpdate(double now, float dt)
{
  if(::input->getKeyState(Input::KEY_s) == Input::KeyState::PRESSED)
    _app->switchState(GameState::name);
}

void MenuState::onDraw(double now, float dt)
{
  renderer->clearViewport(colors::magenta);
  renderer->blitBitmap(Vector2f{100.f, 100.f}, assets->getBitmap(SpaceInvaders::BMK_CANNON0), colors::white);
  renderer->blitBitmap(Vector2f{150.f, 100.f}, assets->getBitmap(SpaceInvaders::BMK_SQUID0), colors::blue);
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

  int32_t scale {1};
  while((baseWorldSize._x * scale) < windowWidth && (baseWorldSize._y * scale) < windowHeight)
    ++scale; 

  --scale;
  if(scale == 0)
    scale = 1;

  _worldSize = baseWorldSize * scale;

  Application::onWindowResize(windowWidth, windowHeight);

  std::vector<std::pair<int32_t, const char*>> manifest {};
  for(int32_t i = BMK_CANNON0; i < BMK_COUNT; ++i){
    manifest.push_back(std::make_pair(i, _bitmapNames[i])); 
  }

  assets->loadBitmaps(std::move(manifest), scale);

  std::unique_ptr<ApplicationState> game {new GameState{this}};
  std::unique_ptr<ApplicationState> menu {new MenuState{this}};

  game->initialize(_worldSize);
  menu->initialize(_worldSize);

  addState(std::move(game));
  addState(std::move(menu));

  switchState(MenuState::name);

  return true;
}

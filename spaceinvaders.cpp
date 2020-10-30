#include "nomad.h"
#include "spaceinvaders.h"

//===============================================================================================//
//                                                                                               //
// ##>GAME STATE                                                                                 //
//                                                                                               //
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
  nomad::renderer->clearViewport(colors::cyan);
  nomad::renderer->blitBitmap(Vector2f{100.f, 100.f}, nomad::assets->getBitmap(bmk::cannon0), colors::black);
  nomad::renderer->blitBitmap(Vector2f{150.f, 100.f}, nomad::assets->getBitmap(bmk::squid0), colors::black);
}

void GameState::onReset()
{
}

//===============================================================================================//
//                                                                                               //
// ##>MENU STATE                                                                                 //
//                                                                                               //
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
  nomad::renderer->clearViewport(colors::magenta);
  nomad::renderer->blitBitmap(Vector2f{100.f, 100.f}, nomad::assets->getBitmap(bmk::cannon0), colors::white);
  nomad::renderer->blitBitmap(Vector2f{150.f, 100.f}, nomad::assets->getBitmap(bmk::squid0), colors::blue);
}

void MenuState::onReset()
{
}

//===============================================================================================//
//                                                                                               //
// ##>SPACE INVADERS                                                                             //
//                                                                                               //
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

  std::vector<std::string> manifest {
    bmk::cannon0, bmk::squid0, bmk::squid1, bmk::crab0, bmk::crab1, bmk::octopus0, bmk::octopus1, 
    bmk::saucer0, bmk::cross0, bmk::cross1, bmk::cross2, bmk::cross3, bmk::zigzag0, bmk::zigzag1, 
    bmk::zigzag2, bmk::zigzag3, bmk::zagzig0, bmk::zagzig1, bmk::zagzig2, bmk::zagzig3, bmk::laser0
  };
  ::assets->loadBitmaps(manifest, scale);

  std::unique_ptr<ApplicationState> game {new GameState{this}};
  std::unique_ptr<ApplicationState> menu {new MenuState{this}};

  game->initialize(_worldSize);
  menu->initialize(_worldSize);

  addState(std::move(game));
  addState(std::move(menu));

  switchState(MenuState::name);

  return true;
}

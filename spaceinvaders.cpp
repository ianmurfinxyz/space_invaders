#include "nomad.h"
#include "spaceinvaders.h"

//===============================================================================================//
//                                                                                               //
// ##>GAME STATE                                                                                 //
//                                                                                               //
//===============================================================================================//

void GameState::initialize(Vector2i worldSize)
{
}

void GameState::onUpdate(double now, float dt)
{
  if(::input->getKeyState(Input::KEY_s) == Input::KeyState::PRESSED)
    switchState(MenuState::name);
}

void GameState::onDraw(double now, float dt)
{
  ::renderer->clearViewport(colors::cyan);
  ::renderer->blitBitmap(Vector2f{100.f, 100.f}, ::assets->getBitmap(bmk::cannon0), colors::black);
  ::renderer->blitBitmap(Vector2f{150.f, 100.f}, ::assets->getBitmap(bmk::squid0), colors::black);
}

void GameState::onReset()
{
}

//===============================================================================================//
//                                                                                               //
// ##>MENU STATE                                                                                 //
//                                                                                               //
//===============================================================================================//

void MenuState::initialize(Vector2i worldSize)
{
}

void MenuState::onUpdate(double now, float dt)
{
  if(::input->getKeyState(Input::KEY_s) == Input::KeyState::PRESSED)
    switchState(GameState::name);
}

void MenuState::onDraw(double now, float dt)
{
  ::renderer->clearViewport(colors::magenta);
  ::renderer->blitBitmap(Vector2f{100.f, 100.f}, ::assets->getBitmap(bmk::cannon0), colors::white);
  ::renderer->blitBitmap(Vector2f{150.f, 100.f}, ::assets->getBitmap(bmk::squid0), colors::blue);
}

void MenuState::onReset()
{
}

//===============================================================================================//
//                                                                                               //
// ##>SPACE INVADERS                                                                             //
//                                                                                               //
//===============================================================================================//

bool SpaceInvaders::initialize(Engine* engine, int32 windowWidth, int32 windowHeight)
{
  Application::initialize(engine, windowWidth, windowHeight);

  int32 scale {1};
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

  std::unique_ptr<ApplicationState> game = std::make_unique(new GameState{});
  std::unique_ptr<ApplicationState> menu = std::make_unique(new MenuState{});

  game->initialize(_worldSize);
  menu->initialize(_worldSize);

  addState(std::move(game));
  addState(std::move(menu));

  switchState(MenuState::name);
}

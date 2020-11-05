#include "nomad.h"
#include "spaceinvaders.h"

//===============================================================================================//
// ##>GAME STATE                                                                                 //
//===============================================================================================//

void GameState::initialize(Vector2i worldSize, int32_t worldScale)
{
  _worldSize = worldSize;
  _worldScale = worldScale;

  _activeCycle = 5;
  _activeCycleElement = cycleStart;

  _alienShiftDisplacement *= _worldScale;
  _alienDropDisplacement *= _worldScale;
  _alienXSeperation *= _worldScale;
  _alienYSeperation *= _worldScale;
  _worldMargin *= _worldScale;

  _worldLeftBorderX = _worldMargin;
  _worldRightBorderX = _worldSize._x - _worldMargin;

  _aliensSpawnPosition._x = (_worldSize._x - (gridWidth * _alienXSeperation)) / 2;
  _aliensSpawnPosition._y = _worldSize._y - (gridHeight * _alienYSeperation) - 30;

  _colorPallete.push_back(colors::red);
  _colorPallete.push_back(colors::green);
  _colorPallete.push_back(colors::blue);

  _levelNo = 0;
  
  startNextLevel();
}

void GameState::onUpdate(double now, float dt)
{
  // Move the aliens.
  
  int32_t numMoves = _cycles[_activeCycle][_activeCycleElement]; 

  for(int i = 0; i < numMoves; ++i){
    Alien& alien = _aliens[_nextMover._row][_nextMover._col];

    if(_isAliensDropping){
      alien._position += _alienDropDisplacement;
    }
    else{
      alien._position += _alienShiftDisplacement * _alienMoveDirection;
    }

    alien._drawState = !alien._drawState;

    bool looped = incrementGridIndex(_nextMover);

    if(looped){
      if(_isAliensDropping){
        ++_dropsDone;

        if(_isAliensSpawning){
          if(_dropsDone >= _levels[_levelIndex]._spawnDrops)
            endSpawning();
        }
        else{
          _isAliensDropping = false;
          _alienMoveDirection *= -1;
        }
      }
      else if(testAlienBorderCollision()){
        _isAliensDropping = true;
      }
    }
  }

  ++_activeCycleElement;
  if(_cycles[_activeCycle][_activeCycleElement] == cycleEnd)
    _activeCycleElement = cycleStart;



}

bool GameState::incrementGridIndex(GridIndex& index)
{
  ++index._col;
  if(index._col >= gridWidth){
    index._col = 0;
    ++index._row;
    if(index._row >= gridHeight){
      index._row = 0;
      return true;
    }
  }
  return false;
}

bool GameState::testAlienBorderCollision()
{
  switch(_alienMoveDirection){

    // If moving left test against left border.
    case -1: 
      for(int32_t col = 0; col < gridWidth; ++col){
        for(int32_t row = 0; row < gridHeight; ++row){
          Alien& alien = _aliens[row][col];

          if(!alien._isAlive)
            continue;

          if(alien._position._x <= _worldLeftBorderX)
            return true;
        }
      }
      break;

    // If moving right test against right border.
    case 1:
      for(int32_t col = gridWidth - 1; col > 0; --col){
        for(int32_t row = 0; row < gridHeight; ++row){
          Alien& alien = _aliens[row][col];

          if(!alien._isAlive)
            continue;

          if(alien._position._x + _alienXSeperation >= _worldRightBorderX)
            return true;
        }
      }
      break;

    default:
      assert(0); // should never happen.
  }

  return false;
}

void GameState::startNextLevel()
{
  ++_levelNo;
  _levelIndex = 0;
  _dropsDone = 0;

  for(int32_t col = 0; col < gridWidth; ++col){
    for(int32_t row = 0; row < gridHeight; ++row){
      Alien& alien = _aliens[row][col];

      alien._id = _formations[0][row][col];

      alien._position._x = _aliensSpawnPosition._x + (col * _alienXSeperation);
      alien._position._y = _aliensSpawnPosition._y + (row * _alienYSeperation);

      alien._drawState = false;
      alien._isAlive = true;
    }
  }

  _nextMover = {0, 0};
  _alienMoveDirection = 1;
  _isAliensDropping = true;
  _isAliensSpawning = true;
}

void GameState::endSpawning()
{
  _isAliensSpawning = false;
  _isAliensDropping = false;
}

void GameState::onDraw(double now, float dt)
{
  renderer->clearViewport(colors::cyan);

  for(const auto& row : _aliens){
    for(const auto& alien : row){
      if(alien._isAlive){
        const AlienClass& ac = _alienClasses[alien._id];

        Vector2f position(alien._position._x, alien._position._y);
        Assets::Key_t bitmapKey = ac._bitmapKeys[alien._drawState];
        Color3f& color = _colorPallete[ac._colorIndex];

        std::cout << "key=" << bitmapKey << std::endl;

        renderer->blitBitmap(position, assets->getBitmap(bitmapKey, _worldScale), color);
      }
    }
  }
}

void GameState::onReset()
{
}

//===============================================================================================//
// ##>MENU STATE                                                                                 //
//===============================================================================================//

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
  renderer->blitBitmap(Vector2f{10.f, 100.f}, assets->getBitmap(SpaceInvaders::BMK_CANNON0, _worldScale), colors::white);

  renderer->blitBitmap(Vector2f{60.f, 100.f}, assets->getBitmap(SpaceInvaders::BMK_SQUID0, _worldScale), colors::blue);
  renderer->blitBitmap(Vector2f{90.f, 100.f}, assets->getBitmap(SpaceInvaders::BMK_SQUID1, _worldScale), colors::blue);

  renderer->blitBitmap(Vector2f{120.f, 100.f}, assets->getBitmap(SpaceInvaders::BMK_CRAB0, _worldScale), colors::blue);
  renderer->blitBitmap(Vector2f{150.f, 100.f}, assets->getBitmap(SpaceInvaders::BMK_CRAB1, _worldScale), colors::blue);

  renderer->blitBitmap(Vector2f{180.f, 100.f}, assets->getBitmap(SpaceInvaders::BMK_OCTOPUS0, _worldScale), colors::blue);
  renderer->blitBitmap(Vector2f{210.f, 100.f}, assets->getBitmap(SpaceInvaders::BMK_OCTOPUS1, _worldScale), colors::blue);

  renderer->blitBitmap(Vector2f{240.f, 100.f}, assets->getBitmap(SpaceInvaders::BMK_SAUCER0, _worldScale), colors::blue);

  renderer->blitBitmap(Vector2f{280.f, 100.f}, assets->getBitmap(SpaceInvaders::BMK_CROSS0, _worldScale), colors::blue);
  renderer->blitBitmap(Vector2f{300.f, 100.f}, assets->getBitmap(SpaceInvaders::BMK_CROSS1, _worldScale), colors::blue);
  renderer->blitBitmap(Vector2f{310.f, 100.f}, assets->getBitmap(SpaceInvaders::BMK_CROSS2, _worldScale), colors::blue);
  renderer->blitBitmap(Vector2f{320.f, 100.f}, assets->getBitmap(SpaceInvaders::BMK_CROSS3, _worldScale), colors::blue);

  renderer->blitBitmap(Vector2f{330.f, 100.f}, assets->getBitmap(SpaceInvaders::BMK_ZIGZAG0, _worldScale), colors::blue);
  renderer->blitBitmap(Vector2f{340.f, 100.f}, assets->getBitmap(SpaceInvaders::BMK_ZIGZAG1, _worldScale), colors::blue);
  renderer->blitBitmap(Vector2f{360.f, 100.f}, assets->getBitmap(SpaceInvaders::BMK_ZIGZAG2, _worldScale), colors::blue);
  renderer->blitBitmap(Vector2f{380.f, 100.f}, assets->getBitmap(SpaceInvaders::BMK_ZIGZAG3, _worldScale), colors::blue);

  renderer->blitBitmap(Vector2f{390.f, 100.f}, assets->getBitmap(SpaceInvaders::BMK_ZAGZIG0, _worldScale), colors::blue);
  renderer->blitBitmap(Vector2f{410.f, 100.f}, assets->getBitmap(SpaceInvaders::BMK_ZAGZIG1, _worldScale), colors::blue);
  renderer->blitBitmap(Vector2f{420.f, 100.f}, assets->getBitmap(SpaceInvaders::BMK_ZAGZIG2, _worldScale), colors::blue);
  renderer->blitBitmap(Vector2f{430.f, 100.f}, assets->getBitmap(SpaceInvaders::BMK_ZAGZIG3, _worldScale), colors::blue);

  renderer->blitBitmap(Vector2f{440.f, 100.f}, assets->getBitmap(SpaceInvaders::BMK_LASER0, _worldScale), colors::blue);
}

void MenuState::onReset()
{
}

//===============================================================================================//
// ##>SPACE INVADERS                                                                             //
//===============================================================================================//

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

  std::cout << "world scale = " << _worldScale << std::endl;
  std::cout << "window width = " << windowWidth << std::endl;
  std::cout << "window height = " << windowHeight << std::endl;

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

#include "nomad.h"
#include "spaceinvaders.h"

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

//===============================================================================================//
// ##>GAME STATE                                                                                 //
//===============================================================================================//

GameState::GameState(Application* app) : 
  ApplicationState{app},
  _randColumn{1, gridWidth},
  _randAlienBulletClass{CROSS, ZAGZIG}
{}

void GameState::initialize(Vector2i worldSize, int32_t worldScale)
{
  // This function 'hard-codes' all game data in one place so it is easy to find.

  _worldSize = worldSize;
  _worldScale = worldScale;

  _colorPallete = {     // index:
    colors::red,        // 0
    colors::green,      // 1
    colors::blue,       // 2
    colors::magenta,    // 3
    colors::cyan,       // 4
    colors::yellow,     // 5
    colors::white       // 6
  };

  _alienShiftDisplacement = Vector2i{2, 0} * _worldScale;
  _alienDropDisplacement = Vector2i{0, -8} * _worldScale;

  _alienXSeperation = 16 * _worldScale;
  _alienYSeperation = 16 * _worldScale;

  _aliensSpawnPosition._x = (_worldSize._x - (gridWidth * _alienXSeperation)) / 2;
  _aliensSpawnPosition._y = _worldSize._y - (gridHeight * _alienYSeperation) - 30;

  _worldMargin = 5 * _worldScale;

  _worldLeftBorderX = _worldMargin;
  _worldRightBorderX = _worldSize._x - _worldMargin;


  // Each update tick the game performs a number of 'beats'. The beat rate controls the speed of
  // the game; speed of alien shifts and firing etc. Beats are composed into sets called cycles 
  // where each element of the set (cycle) represents a number of beats to perform in an update
  // tick. For example a cycle = {1, 2, end} means perform 1 beat in update tick N, 2 beats in
  // update tick N+1, and repeat, so 1 beat in N+2 etc. Since the engine guarantees an update
  // rate of 60Hz (only slower if the CPU is too slow), the cycle controls the game speed. Yes the
  // game speed is tied to update rate, but the update rate is independent of hardware.

  // Theses cycles will produce exponentially increasing beat rates resulting in exponentially
  // increasing alien speed and firing.
  _cycles = {{
    {1,  cycleEnd, 0, 0},  // ticks:55.00   freq:1.09
    {1,  1, 2, cycleEnd},  // ticks:42.00   freq:1.43
    {1,  2, cycleEnd, 0},  // ticks:37.00   freq:1.60
    {2,  cycleEnd, 0, 0},  // ticks:27.50   freq:2.18
    {2,  2, 3, cycleEnd},  // ticks:23.57   freq:2.54
    {2,  3, cycleEnd, 0},  // ticks:22.00   freq:2.70
    {3,  cycleEnd, 0, 0},  // ticks:18.33   freq:3.33
    {4,  cycleEnd, 0, 0},  // ticks:13.75   freq:4.35
    {5,  cycleEnd, 0, 0},  // ticks:11.00   freq:5.56
    {6,  cycleEnd, 0, 0},  // ticks:9.17    freq:6.67
    {7,  cycleEnd, 0, 0},  // ticks:7.86    freq:7.69
    {8,  cycleEnd, 0, 0},  // ticks:6.88    freq:9.09
    {9,  cycleEnd, 0, 0},  // ticks:6.11    freq:9.81
    {10, cycleEnd, 0, 0},  // ticks:5.50    freq:10.90
    {11, cycleEnd, 0, 0}   // ticks:5.00    freq:12.00
  }};

  _alienClasses = {{
    {8  * _worldScale, 8 * _worldScale, 30, 1, {SpaceInvaders::BMK_SQUID0  , SpaceInvaders::BMK_SQUID1  }},
    {11 * _worldScale, 8 * _worldScale, 20, 4, {SpaceInvaders::BMK_CRAB0   , SpaceInvaders::BMK_CRAB1   }},
    {12 * _worldScale, 8 * _worldScale, 10, 3, {SpaceInvaders::BMK_OCTOPUS0, SpaceInvaders::BMK_OCTOPUS1}}
  }};

  _formations = {{  // note formations look inverted here as array[0] is the bottom row, but they are not.
    // formation 0
    {{
       {OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS},
       {OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS},
       {CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   },
       {CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   },
       {SQUID  ,SQUID  ,SQUID  ,SQUID  ,SQUID  ,SQUID  ,SQUID  ,SQUID  ,SQUID  ,SQUID  ,SQUID  }
    }},
    // formation 1
    {{
       {SQUID  ,SQUID  ,SQUID  ,SQUID  ,SQUID  ,SQUID  ,SQUID  ,SQUID  ,SQUID  ,SQUID  ,SQUID  },
       {CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   },
       {CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   ,CRAB   },
       {OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS},
       {OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS,OCTOPUS}
    }}
  }};

  _fireIntervalDeviation = 0.5f; // Maximum 50% deviation from base.
  _fireIntervalBase = 50;

  _bulletClasses = {{
    {3 * _worldScale, 6 * _worldScale, -100.f * _worldScale, 5, 20, {SpaceInvaders::BMK_CROSS0, SpaceInvaders::BMK_CROSS1, SpaceInvaders::BMK_CROSS2, SpaceInvaders::BMK_CROSS3}},
    {3 * _worldScale, 7 * _worldScale, -100.f * _worldScale, 5, 20, {SpaceInvaders::BMK_ZIGZAG0, SpaceInvaders::BMK_ZIGZAG1, SpaceInvaders::BMK_ZIGZAG2, SpaceInvaders::BMK_ZIGZAG3}},
    {3 * _worldScale, 7 * _worldScale, -100.f * _worldScale, 5, 20, {SpaceInvaders::BMK_ZAGZIG0, SpaceInvaders::BMK_ZAGZIG1, SpaceInvaders::BMK_ZAGZIG2, SpaceInvaders::BMK_ZAGZIG3}},
    {1 * _worldScale, 5 * _worldScale, -100.f * _worldScale, 6, 20, {SpaceInvaders::BMK_LASER0, SpaceInvaders::BMK_LASER0, SpaceInvaders::BMK_LASER0, SpaceInvaders::BMK_LASER0}}
  }};

  _cannon._spawnPosition = Vector2f{20.f, 20.f};
  _cannon._speed = 100.f * _worldScale;
  _cannon._width = 13 * _worldScale;
  _cannon._height = 8 * _worldScale;
  _cannon._boomFrameInterval = 20;
  _cannon._boomInterval = _cannon._boomFrameInterval * cannonBoomFramesCount * 4;
  _cannon._cannonKey = SpaceInvaders::BMK_CANNON0;
  _cannon._boomKeys = {{SpaceInvaders::BMK_CANNONBOOM0, SpaceInvaders::BMK_CANNONBOOM1, SpaceInvaders::BMK_CANNONBOOM2}};

  _levels = {{
    {0, 5},
    {0, 5},
    {0, 5},
    {0, 5},
    {0, 5},
    {0, 5},
    {0, 5},
    {0, 5},
    {0, 5},
    {0, 5},
  }};

  _levelIndex = -1;
  _levelNo = 0;

  startNextLevel();
}

void GameState::startNextLevel()
{
  ++_levelNo;
  ++_levelIndex;
  if(_levelIndex == levelCount)
    --_levelIndex;

  _activeCycle = _levels[_levelIndex]._startCycle;
  _activeBeat = cycleStart;
  _nextMover = {0, 0};
  _alienMoveDirection = 1;
  _dropsDone = 0;
  _isAliensDropping = true;
  _isAliensSpawning = true;
  _isAliensFrozen = false;

  // Reset aliens.
  for(int32_t col = 0; col < gridWidth; ++col){
    for(int32_t row = 0; row < gridHeight; ++row){
      Alien& alien = _grid[row][col];

      alien._classId = _formations[0][row][col];

      alien._position._x = _aliensSpawnPosition._x + (col * _alienXSeperation);
      alien._position._y = _aliensSpawnPosition._y + (row * _alienYSeperation);

      alien._frame = false;
      alien._isAlive = true;
    }
  }

  std::fill(_columnPops.begin(), _columnPops.end(), gridHeight);

  // Reset bullets.
  for(auto& bullet : _alienBullets)
    bullet._isAlive = false;

  _laser._isAlive = false;

  // Reset player cannon.
  _cannon._isBooming = false;
  _cannon._isAlive = false;

  _beatsUntilFire = _fireIntervalBase;
}

void GameState::endSpawning()
{
  _isAliensSpawning = false;
  _isAliensDropping = false;
  spawnCannon();
}

void GameState::spawnCannon()
{
  _cannon._position = _cannon._spawnPosition;
  _cannon._moveDirection = 0;
  _cannon._isBooming = false;
  _cannon._isAlive = true;

  _isAliensFrozen = false;
}

void GameState::boomCannon()
{
  _cannon._moveDirection = 0;
  _cannon._boomClock = _cannon._boomInterval;
  _cannon._boomFrame = 0;
  _cannon._boomFrameClock = _cannon._boomFrameInterval;
  _cannon._isBooming = true;
  _cannon._isAlive = false;

  _isAliensFrozen = true;
}

void GameState::onUpdate(double now, float dt)
{
  int32_t beats = _cycles[_activeCycle][_activeBeat]; 

  doBulletMoving(beats, dt);
  doAlienMoving(beats);
  doAlienFiring(beats);
  doCannonMoving(dt);
  doCannonBooming(beats);

  // TEMP - TODO- implement collision detectin to boom cannon.
  if(::input->isKeyPressed(Input::KEY_b))
    boomCannon();

  ++_activeBeat;
  if(_cycles[_activeCycle][_activeBeat] == cycleEnd)
    _activeBeat = cycleStart;
}

void GameState::doCannonMoving(float dt)
{
  if(!_cannon._isAlive)
    return;

  bool lKey = ::input->isKeyDown(Input::KEY_LEFT);
  bool rKey = ::input->isKeyDown(Input::KEY_RIGHT);
  if(lKey && !rKey){
    _cannon._moveDirection = -1;
  }
  else if(!lKey && rKey){
    _cannon._moveDirection = 1;
  }
  else{
    _cannon._moveDirection = 0;
  }

  _cannon._position._x += _cannon._speed * _cannon._moveDirection * dt;
  _cannon._position._x = std::clamp(
      _cannon._position._x, 
      static_cast<float>(_worldLeftBorderX), 
      static_cast<float>(_worldRightBorderX - _cannon._width)
  );
}

void GameState::doCannonBooming(int32_t beats)
{
  if(!_cannon._isBooming)
    return;

  _cannon._boomClock -= beats;
  if(_cannon._boomClock <= 0){
    _cannon._isBooming = false;
    spawnCannon();
    return;
  }

  _cannon._boomFrameClock -= beats;
  if(_cannon._boomFrameClock <= 0){
    _cannon._boomFrame = nomad::wrap(++_cannon._boomFrame, 0, cannonBoomFramesCount - 1);
    _cannon._boomFrameClock = _cannon._boomFrameInterval;
  }
}

void GameState::doAlienMoving(int32_t beats)
{
  // Aliens move at fixed displacements independent of time, thus alien movement speed is an 
  // emergent property of the rate of update ticks, and importantly, the number of aliens moved 
  // in each tick. Note the engine guarantees a tick rate of 60Hz thus alien speed in game is 
  // controlled by the second factor; the number of aliens moved in each tick. Since the game is 
  // ticked at 60Hz, if a single alien is moved in each tick then 55 aliens will be moved in 55 
  // ticks so in 55/60 seconds. Moving 2 aliens per tick will result in twice the speed. 
  //
  // The number of aliens moved each update tick is equal to the number of beats performed in that
  // tick (see cycles note in spaceinvaders.h). Thus a cycle such as {1, 2, end} will mean update 
  // 1 alien in tick N, 2 aliens in tick N + 1, and repeat. This results in (when you do the math) 
  // all 55 aliens moving in 37 ticks, i.e. it takes 37/60 seconds to complete one full grid 
  // movement, giving a frequency of grid movements of 60/37 Hz.
  //
  // note: this design makes alien movement speed dependent on the number of aliens in the grid. 
  // The more aliens the more beats are required to complete one grid movement and each grid 
  // movement results in a fixed grid displacement. Thus if you change the number of aliens you 
  // must also change all cycles to tune the gameplay.

  if(_isAliensFrozen)
    return;

  for(int i = 0; i < beats; ++i){
    Alien& alien = _grid[_nextMover._row][_nextMover._col];

    if(_isAliensDropping){
      alien._position += _alienDropDisplacement;
    }
    else{
      alien._position += _alienShiftDisplacement * _alienMoveDirection;
    }

    alien._frame = !alien._frame;

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
}

void GameState::doAlienFiring(int32_t beats)
{
  // Cycles determine alien fire rate. Aliens fire every N beats, thus the higher beat rate
  // the higher the rate of fire. Randomness is added in a random deviation to the M moves fire 
  // interval and to the alien which does the firing.

  if(_isAliensFrozen)
    return;

  if(_isAliensSpawning)
    return;
  
  _beatsUntilFire -= beats;
  if(_beatsUntilFire > 0)
    return;

  // Select the column to fire from, taking into account unpopulated columns.
  int32_t populatedCount = gridWidth - std::count(_columnPops.begin(), _columnPops.end(), 0);

  // This condition should of already been detected as a level win.
  assert(populatedCount != 0);

  int32_t colShift = _randColumn() % populatedCount;
  int32_t col {0};
  while(--colShift >= 0){
    ++col;
    while(_columnPops[col] == 0){
      ++col;
    }
  }

  // Find the alien that will do the firing.
  Alien* alien {nullptr};
  for(int32_t row = 0; row < gridHeight; ++row){
    if(_grid[row][col]._isAlive){
      alien = &_grid[row][col];
      break;
    }
  }

  assert(alien != nullptr);   // The column selection should ensure this never happens.

  const AlienClass& alienClass = _alienClasses[alien->_classId];

  BulletClassId bcid = static_cast<BulletClassId>(_randAlienBulletClass());
  const BulletClass& bulletClass = _bulletClasses[bcid]; 

  Vector2f position {};
  position._x += alien->_position._x + (alienClass._width * 0.5f);
  position._y += alien->_position._y - bulletClass._height;

  // If this condition does occur then increase the max bullets until it doesn't.
  assert(_alienBulletCount != maxAlienBullets);

  // Find a 'dead' bullet instance to use.
  Bullet* bullet {nullptr};
  for(auto& b : _alienBullets)
    if(!b._isAlive)
      bullet = &b;

  // If this occurs my bullet counts are off.
  assert(bullet != nullptr);

  bullet->_classId = bcid;
  bullet->_position = position;
  bullet->_frame = 0;
  bullet->_isAlive = true;

  ++_alienBulletCount;

  // Calculate when the next bullet will be fired.
  _beatsUntilFire = _fireIntervalBase;
}

void GameState::doBulletMoving(int32_t beats, float dt)
{
  for(auto& bullet : _alienBullets){
    if(!bullet._isAlive)
      continue;

    const BulletClass& bulletClass = _bulletClasses[bullet._classId];

    bullet._position._y += bulletClass._speed * dt;

    bullet._frameClock -= beats;
    if(bullet._frameClock <= 0){
      bullet._frame = nomad::wrap(++bullet._frame, 0, bulletFramesCount - 1);
      bullet._frameClock = bulletClass._frameInterval;
    }
  }
}

bool GameState::incrementGridIndex(GridIndex& index)
{
  // Increments index from left-to-right along the columns, moving up a row and back to the left
  // most column upon reaching the end of the current column. Loops back to the bottom left most
  // column of the bottom row upon reaching the top-right of the grid. Returns true to indicate
  // a loop.
  
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
          Alien& alien = _grid[row][col];

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
          Alien& alien = _grid[row][col];

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

void GameState::drawGrid()
{
  for(const auto& row : _grid){
    for(const auto& alien : row){
      if(alien._isAlive){
        const AlienClass& ac = _alienClasses[alien._classId];

        Vector2f position(alien._position._x, alien._position._y);
        Assets::Key_t bitmapKey = ac._bitmapKeys[alien._frame];
        Color3f& color = _colorPallete[ac._colorIndex];

        renderer->blitBitmap(position, nomad::assets->getBitmap(bitmapKey, _worldScale), color);
      }
    }
  }
}

void GameState::drawCannon()
{
  if(!(_cannon._isBooming || _cannon._isAlive))
    return;

  Assets::Key_t bitmapKey;
  if(_cannon._isBooming){
    bitmapKey = _cannon._boomKeys[_cannon._boomFrame];
  }
  else if(_cannon._isAlive){
    bitmapKey = _cannon._cannonKey;
  }

  Color3f& color = _colorPallete[_cannon._colorIndex];

  renderer->blitBitmap(_cannon._position, nomad::assets->getBitmap(bitmapKey, _worldScale), color);
}

void GameState::drawBullets()
{
  for(auto& bullet : _alienBullets){
    if(!bullet._isAlive)
      continue;

    const BulletClass& bc = _bulletClasses[bullet._classId];
    Assets::Key_t bitmapKey = bc._bitmapKeys[bullet._frame];
    Color3f& color = _colorPallete[bc._colorIndex];
    renderer->blitBitmap(bullet._position, nomad::assets->getBitmap(bitmapKey, _worldScale), color);
  }
}

void GameState::onDraw(double now, float dt)
{
  renderer->clearViewport(colors::black);
  drawGrid();
  drawCannon();
  drawBullets();
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
  if(::input->isKeyPressed(Input::KEY_s))
    _app->switchState(GameState::name);
}

void MenuState::onDraw(double now, float dt)
{
  renderer->clearViewport(colors::magenta);
  renderer->blitBitmap(Vector2f{0.f, 0.f}, assets->getBitmap(SpaceInvaders::BMK_CANNON0, _worldScale), colors::white);

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


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

  // TEMP - TODO - remove this ================================================================

  std::cout << "world scale = " << _worldScale << std::endl;
  std::cout << "window width = " << windowWidth << std::endl;
  std::cout << "window height = " << windowHeight << std::endl;

  //============================================================================================

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
  _randBombClass{CROSS, ZAGZIG}
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
  _worldTopBorderY = _worldSize._y - 20.f;

  _alienBoomDuration = 0.1f;


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

  _bombIntervalDeviation = 0.5f; // Maximum 50% deviation from base.
  _bombIntervalBase = 50;

  _bombClasses = {{
    {3 * _worldScale, 6 * _worldScale, -100.f * _worldScale, 0, 20, {SpaceInvaders::BMK_CROSS0, SpaceInvaders::BMK_CROSS1, SpaceInvaders::BMK_CROSS2, SpaceInvaders::BMK_CROSS3}},
    {3 * _worldScale, 7 * _worldScale, -100.f * _worldScale, 4, 20, {SpaceInvaders::BMK_ZIGZAG0, SpaceInvaders::BMK_ZIGZAG1, SpaceInvaders::BMK_ZIGZAG2, SpaceInvaders::BMK_ZIGZAG3}},
    {3 * _worldScale, 7 * _worldScale, -100.f * _worldScale, 5, 20, {SpaceInvaders::BMK_ZAGZIG0, SpaceInvaders::BMK_ZAGZIG1, SpaceInvaders::BMK_ZAGZIG2, SpaceInvaders::BMK_ZAGZIG3}}
  }};

  _bombBoomKeys = {{SpaceInvaders::BMK_BOMBBOOMTOP, SpaceInvaders::BMK_BOMBBOOMBOTTOM, SpaceInvaders::BMK_BOMBBOOMMIDAIR}};
  _bombBoomWidth = 8 * _worldScale;
  _bombBoomHeight = 8 * _worldScale;
  _bombBoomDuration = 0.4f;

  _cannon._spawnPosition = Vector2f{_worldLeftBorderX, 32.f * _worldScale};
  _cannon._speed = 100.f * _worldScale;
  _cannon._width = 13 * _worldScale;
  _cannon._height = 8 * _worldScale;
  _cannon._boomFrameInterval = 10;
  _cannon._boomInterval = _cannon._boomFrameInterval * 12;
  _cannon._cannonKey = SpaceInvaders::BMK_CANNON0;
  _cannon._boomKeys = {{SpaceInvaders::BMK_CANNONBOOM0, SpaceInvaders::BMK_CANNONBOOM1, SpaceInvaders::BMK_CANNONBOOM2}};

  _laser._width = 1 * _worldScale;
  _laser._height = 6 * _worldScale;
  _laser._colorIndex = 6;
  _laser._speed = 200.f * _worldScale;
  _laser._bitmapKey = SpaceInvaders::BMK_LASER0;

  _bunkerColorIndex = 0;
  _bunkerSpawnX = 32 * _worldScale;
  _bunkerSpawnY = 48 * _worldScale;
  _bunkerSpawnGapX = 45 * _worldScale;
  _bunkerSpawnCount = 4;
  _bunkerWidth = 22 * _worldScale;
  _bunkerHeight = 16 * _worldScale;

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
  _isAliensBooming = false;
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

      alien._row = row;
      alien._col = col;

      alien._frame = false;
      alien._isAlive = true;
    }
  }

  std::fill(_columnPops.begin(), _columnPops.end(), gridHeight);
  std::fill(_rowPops.begin(), _rowPops.end(), gridWidth);
  _alienPopulation = gridWidth * gridHeight;

  // Reset bombs.
  for(auto& bomb : _bombs)
    bomb._isAlive = false;

  _bombCount = 0;

  // Reset bomb booms.
  for(auto& boom : _bombBooms)
    boom._isAlive = false;

  _laser._isAlive = false;

  // Reset player cannon.
  _cannon._isBooming = false;
  _cannon._isAlive = false;
  _cannon._isFrozen = false;

  _bombClock = _bombIntervalBase;

  // Create fresh (undamaged) hitbar.
  _hitbar = std::make_unique<Hitbar>(
      nomad::assets->getBitmap(SpaceInvaders::BMK_HITBAR, _worldScale), 
      _worldSize._x,
      1 * _worldScale,
      16 * _worldScale, 
      1
  );

  // Create fresh bunkers.
  _bunkers.clear();
  Vector2f position {_bunkerSpawnX, _bunkerSpawnY};
  for(int i = 0; i < _bunkerSpawnCount; ++i){
    spawnBunker(position, SpaceInvaders::BMK_BUNKER);
    position._x += _bunkerSpawnGapX;
  }
  _bunkerCount = _bunkerSpawnCount;
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

void GameState::spawnBoom(Vector2i position, BombHit hit, int32_t colorIndex)
{
  auto search = std::find_if_not(_bombBooms.begin(), _bombBooms.end(), isBombBoomAlive);

  // If this asserts then expand the booms array until it does not.
  assert(search != _bombBooms.end());

  (*search)._hit = hit;
  (*search)._colorIndex = colorIndex;
  (*search)._position = position;
  (*search)._boomClock = _bombBoomDuration;
  (*search)._isAlive = true;
}

void GameState::spawnBunker(Vector2f position, Assets::Key_t bitmapKey)
{
  const Bitmap& bitmap = nomad::assets->getBitmap(bitmapKey, _worldScale);
  _bunkers.emplace_back(std::make_unique<Bunker>(bitmap, position));
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

void GameState::spawnBomb(Vector2f position, BombClassId classId)
{
  // If this condition does occur then increase the max bombs until it doesn't.
  assert(_bombCount != maxBombs);

  // Find a 'dead' bomb instance to use.
  Bomb* bomb {nullptr};
  for(auto& b : _bombs)
    if(!b._isAlive)
      bomb = &b;

  // If this occurs my bomb counts are off.
  assert(bomb != nullptr);

  bomb->_classId = classId;
  bomb->_position = position;
  bomb->_frame = 0;
  bomb->_isAlive = true;

  ++_bombCount;

  // Calculate when the next bomb will be dropped.
  _bombClock = _bombIntervalBase;
}

void GameState::boomBomb(Bomb& bomb, bool makeBoom, Vector2i boomPosition, BombHit hit)
{
  --_bombCount;
  bomb._isAlive = false;

  if(makeBoom)
    spawnBoom(boomPosition, hit, _bombClasses[bomb._classId]._colorIndex);
}

void GameState::boomAlien(Alien& alien)
{
  alien._isAlive = false;
  _alienBoomer = &alien;
  _alienBoomClock = _alienBoomDuration;
  _isAliensFrozen = true;
  _isAliensBooming = true;
  _cannon._isFrozen = true;
  --(_columnPops[alien._col]);
  --(_rowPops[alien._row]);
  --_alienPopulation;
}

void GameState::boomLaser(bool makeBoom, BombHit hit)
{
  _laser._isAlive = false;

  if(makeBoom){
    Vector2i position {};
    position._x = _laser._position._x - ((_bombBoomWidth - _laser._width) / 2);
    position._y = _laser._position._y;

    spawnBoom(position, hit, _laser._colorIndex);
  }
}

void GameState::boomBunker(Bunker& bunker, Vector2i pixelHit)
{
  Vector2i aPosition {};
  Vector2i bPosition {};

  const Bitmap& aBitmap {bunker._bitmap};
  const Bitmap& bBitmap {nomad::assets->getBitmap(SpaceInvaders::BMK_BOMBBOOMMIDAIR, _worldScale)};

  aPosition._x = bunker._position._x;
  aPosition._y = bunker._position._y;

  bPosition._x = bunker._position._x + pixelHit._x - (_bombBoomWidth / 2);
  bPosition._y = bunker._position._y + pixelHit._y - (_bombBoomHeight / 2);

  const Collision& c = testCollision(aPosition, aBitmap, bPosition, bBitmap, true);

  // If this asserts then the mask used to blit off damage is not intersecting any pixels
  // on the bitmap, the result is that no change is made. This creates the situation in which
  // parts of the bunker cannot be destroyed. Thus if this happens redesign the damage mask.
  assert(c._isCollision);

  for(auto& pixel : c._aPixels)
    bunker._bitmap.setBit(pixel._y, pixel._x, 0, false);

  bunker._bitmap.regenerateBytes();
}

void GameState::doCannonMoving(float dt)
{
  if(!_cannon._isAlive)
    return;

  if(_cannon._isFrozen)
    return;

  bool lKey = nomad::input->isKeyDown(Input::KEY_LEFT);
  bool rKey = nomad::input->isKeyDown(Input::KEY_RIGHT);
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

void GameState::doCannonFiring()
{
  if(!_cannon._isAlive)
    return;

  if(_cannon._isFrozen)
    return;

  if(_laser._isAlive)
    return;

  if(nomad::input->isKeyPressed(Input::KEY_UP)){
    Vector2f position = _cannon._position;
    position._x += _cannon._width / 2;
    position._y += _cannon._height;
    _laser._position = position;
    _laser._isAlive = true;
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

  if(_alienPopulation == 0)
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
      else if(doCollisionsAliensBorders()){
        _isAliensDropping = true;
      }
    }
  }
}

void GameState::doAlienBombing(int32_t beats)
{
  // Cycles determine alien bomb rate. Aliens bomb every N beats, thus the higher beat rate
  // the higher the rate of bombing. Randomness is added in a random deviation to the bomb 
  // interval and to the choice of alien which does the bombing.
  
  if(true)
    return;

  if(_isAliensFrozen)
    return;

  if(_isAliensSpawning)
    return;

  if(_alienPopulation == 0)
    return;
  
  _bombClock -= beats;
  if(_bombClock > 0)
    return;

  // Select the column to bomb from, taking into account unpopulated columns.
  int32_t populatedCount = gridWidth - std::count(_columnPops.begin(), _columnPops.end(), 0);

  // This condition should of already been detected as a level win.
  assert(populatedCount != 0);

  int32_t colShift = _randColumn() % populatedCount;
  int32_t col {-1};
  do {
    ++col;
    while(_columnPops[col] == 0) 
      ++col;
  }
  while(--colShift >= 0);

  // Find the alien that will do the bombing.
  Alien* alien {nullptr};
  for(int32_t row = 0; row < gridHeight; ++row){
    if(_grid[row][col]._isAlive){
      alien = &_grid[row][col];
      break;
    }
  }

  assert(alien != nullptr); // The column selection should ensure this never happens.
  
  const AlienClass& alienClass = _alienClasses[alien->_classId];

  BombClassId classId = static_cast<BombClassId>(_randBombClass());
  const BombClass& bombClass = _bombClasses[classId]; 

  Vector2f position {};
  position._x += alien->_position._x + (alienClass._width * 0.5f);
  position._y += alien->_position._y - bombClass._height;

  spawnBomb(position, classId);
}

void GameState::doAlienBooming(float dt)
{
  if(!_isAliensBooming)
    return;

  if(_alienPopulation == 0)
    return;

  _alienBoomClock -= dt;
  if(_alienBoomClock <= 0.f){
    _alienBoomer = nullptr;
    _isAliensFrozen = false;
    _isAliensBooming = false;
    _cannon._isFrozen = false;
  }
}

void GameState::doBombMoving(int32_t beats, float dt)
{
  for(auto& bomb : _bombs){
    if(!bomb._isAlive)
      continue;

    const BombClass& bombClass = _bombClasses[bomb._classId];

    bomb._position._y += bombClass._speed * dt;

    bomb._frameClock -= beats;
    if(bomb._frameClock <= 0){
      bomb._frame = nomad::wrap(++bomb._frame, 0, bombFramesCount - 1);
      bomb._frameClock = bombClass._frameInterval;
    }
  }
}

void GameState::doLaserMoving(float dt)
{
  if(!_laser._isAlive)
    return;

  _laser._position._y += _laser._speed * dt;
}

void GameState::doBombBoomBooming(float dt)
{
  for(auto& boom : _bombBooms){
    if(!boom._isAlive)
      continue;

    boom._boomClock -= dt;
    if(boom._boomClock <= 0.f)
      boom._isAlive = false;
  }
}

void GameState::doCollisionsBombsHitbar()
{
  for(auto& bomb : _bombs){
    if(!bomb._isAlive)
      continue;

    if(bomb._position._y > _hitbar->_positionY)
      continue;

    BombClass& bc = _bombClasses[bomb._classId];

    int32_t bithit = bomb._position._x - ((_bombBoomWidth - bc._width) / 2);
    
    // Apply damage to the bar.
    for(int32_t i = 0; i < _bombBoomWidth; ++i){
      bool bitval = (bithit + i / _worldScale) % 2;
      for(int32_t j = 0; j < _hitbar->_height; ++j)
        _hitbar->_bitmap.setBit(j, bithit + i, bitval, false);
    }
    _hitbar->_bitmap.regenerateBytes();

    Vector2i boomPosition {bithit, _hitbar->_positionY + _hitbar->_height};
    boomBomb(bomb, true, boomPosition, BOMBHIT_BOTTOM);
  }
}

void GameState::doCollisionsBombsCannon()
{
  if(!_cannon._isAlive)
    return;

  if(_cannon._isBooming)
    return;

  if(_bombCount == 0)
    return;

  Vector2i aPosition {};
  Vector2i bPosition {};

  const Bitmap* aBitmap {nullptr};
  const Bitmap* bBitmap {nullptr};

  aPosition._x = _cannon._position._x;
  aPosition._y = _cannon._position._y;

  aBitmap = &(nomad::assets->getBitmap(_cannon._cannonKey, _worldScale));

  for(auto& bomb : _bombs){
    if(!bomb._isAlive)
      continue;
  
    bPosition._x = bomb._position._x;
    bPosition._y = bomb._position._y;

    BombClass& bc = _bombClasses[bomb._classId];

    bBitmap = &(nomad::assets->getBitmap(bc._bitmapKeys[bomb._frame], _worldScale));

    const Collision& c = testCollision(aPosition, *aBitmap, bPosition, *bBitmap, false);

    if(c._isCollision){
      boomCannon();
      boomBomb(bomb);
    }
  }
}

void GameState::doCollisionsLaserAliens()
{
  if(!_laser._isAlive)
    return;

  if(_isAliensSpawning)
    return;

  if(_isAliensFrozen)
    return;

  if(_alienPopulation == 0)
    return;

  Vector2i aPosition {}; 
  Vector2i bPosition {};

  const Bitmap* aBitmap {nullptr};
  const Bitmap* bBitmap {nullptr};

  aPosition._x = _laser._position._x;
  aPosition._y = _laser._position._y;

  aBitmap = &(nomad::assets->getBitmap(_laser._bitmapKey, _worldScale));

  for(auto& row : _grid){
    for(auto& alien : row){
      if(!alien._isAlive)
        continue;

      const AlienClass& ac = _alienClasses[alien._classId];

      bPosition = alien._position; 
      bBitmap = &(nomad::assets->getBitmap(ac._bitmapKeys[alien._frame], _worldScale));

      const Collision& c = testCollision(aPosition, *aBitmap, bPosition, *bBitmap, false);

      if(c._isCollision){
        boomAlien(alien);
        boomLaser(false);
        return;
      }
    }
  }
}

void GameState::doCollisionsLaserSky()
{
  if(!_laser._isAlive)
    return;

  if(_laser._position._y + _laser._height < _worldTopBorderY)
    return;

  _laser._isAlive = false;
  Vector2i position {};
  position._x = _laser._position._x - ((_bombBoomWidth - _laser._width) / 2);
  position._y = _laser._position._y;
  spawnBoom(position, BOMBHIT_MIDAIR, _laser._colorIndex);
}

bool GameState::doCollisionsAliensBorders()
{
  if(_isAliensSpawning)
    return false;

  if(_isAliensFrozen)
    return false;

  if(_alienPopulation == 0)
    return false;

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

void GameState::doCollisionsBunkersBombs()
{
  if(_bombCount <= 0)
    return;

  if(_bunkerCount <= 0)
    return;

  Vector2i aPosition {};
  Vector2i bPosition {};

  const Bitmap* aBitmap {nullptr};
  const Bitmap* bBitmap {nullptr};

  for(auto& bomb : _bombs){
    if(!bomb._isAlive)
      continue;

    aPosition._y = bomb._position._y;

    if(aPosition._y < _bunkerSpawnY)
      continue;

    if(aPosition._y > _bunkerSpawnY + _bunkerHeight)
      continue;

    aPosition._x = bomb._position._x;

    const BombClass& bc = _bombClasses[bomb._classId];
    aBitmap = &(nomad::assets->getBitmap(bc._bitmapKeys[bomb._frame], _worldScale));

    for(auto& bunker : _bunkers){
      bPosition._x = bunker->_position._x;
      bPosition._y = bunker->_position._y;

      bBitmap = &(bunker->_bitmap);

      const Collision& c = testCollision(aPosition, *aBitmap, bPosition, *bBitmap, false);

      if(c._isCollision){
        //Vector2i position {};
        //position._x = bomb._position._x - ((_bombBoomWidth - bc._width) / 2);
        //position._y = bomb._position._y;
        boomBomb(bomb);//, true, position, BOMBHIT_MIDAIR);
        boomBunker(*bunker, c._bPixels.front());
        return;
      }
    }
  }
}

void GameState::doCollisionsBunkersLaser()
{
  if(!_laser._isAlive)
    return;

  if(_bunkerCount <= 0)
    return;

  Vector2i aPosition {};
  Vector2i bPosition {};

  const Bitmap* aBitmap {nullptr};
  const Bitmap* bBitmap {nullptr};

  aPosition._x = _laser._position._x;
  aPosition._y = _laser._position._y;

  aBitmap = &(nomad::assets->getBitmap(_laser._bitmapKey, _worldScale));

  for(auto& bunker : _bunkers){
    bPosition._x = bunker->_position._x;
    bPosition._y = bunker->_position._y;

    bBitmap = &(bunker->_bitmap);

    const Collision& c = testCollision(aPosition, *aBitmap, bPosition, *bBitmap, false);
    
    if(c._isCollision){
      boomLaser(false);
      boomBunker(*bunker, c._bPixels.front());
      return;
    }
  }
}

void GameState::doCollisionsBunkersAliens()
{
  if(_isAliensSpawning)
    return;

  if(_isAliensFrozen)
    return;

  if(_bunkerCount == 0)
    return;

  if(_alienPopulation == 0)
    return;

  int32_t bottomRow {0};
  while(_rowPops[bottomRow] == 0)
    ++bottomRow;

  if(_grid[bottomRow][0]._position._y > _bunkerSpawnY + _bunkerHeight)
    return;

  Vector2i aPosition {};
  Vector2i bPosition {};

  const Bitmap* aBitmap {nullptr};
  const Bitmap* bBitmap {nullptr};

  for(const auto& alien : _grid[bottomRow]){
    aPosition._x = alien._position._x;
    aPosition._y = alien._position._y;

    const AlienClass& ac = _alienClasses[alien._classId];
    Assets::Key_t bitmapKey = ac._bitmapKeys[alien._frame];
    aBitmap = &(nomad::assets->getBitmap(bitmapKey, _worldScale));

    for(auto& bunker : _bunkers){
      bPosition._x = bunker->_position._x;
      bPosition._y = bunker->_position._y;

      bBitmap = &(bunker->_bitmap);

      const Collision& c = testCollision(aPosition, *aBitmap, bPosition, *bBitmap, false);

      if(c._isCollision){
        bunker->_bitmap.setRect(c._bOverlap._ymin, c._bOverlap._xmin, 
                                 c._bOverlap._ymax - 1, c._bOverlap._xmax - 1, false);
      }
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

void GameState::onUpdate(double now, float dt)
{
  int32_t beats = _cycles[_activeCycle][_activeBeat]; 

  doBombMoving(beats, dt);
  doLaserMoving(dt);
  doAlienMoving(beats);
  doAlienBombing(beats);
  doCannonMoving(dt);
  doCannonBooming(beats);
  doAlienBooming(dt);
  doBombBoomBooming(dt);
  doCannonFiring();

  doCollisionsBombsHitbar();
  doCollisionsBombsCannon();
  doCollisionsLaserAliens();
  doCollisionsBunkersBombs();
  doCollisionsBunkersLaser();
  doCollisionsBunkersAliens();
  doCollisionsLaserSky();

  //================================================================================
  
  // TEMP - TODO- implement collision detectin to boom cannon and aliens.
  
  if(nomad::input->isKeyPressed(Input::KEY_b))
    boomCannon();

  if(nomad::input->isKeyPressed(Input::KEY_a)){
    for(auto& row : _grid){
      for(auto& alien : row){
        if(alien._isAlive){
          boomAlien(alien);
          goto BOOMED;
        }
      }
    }
  }
  BOOMED:

  //================================================================================
  
  if(_alienPopulation == 0)
    startNextLevel();

  ++_activeBeat;
  if(_cycles[_activeCycle][_activeBeat] == cycleEnd)
    _activeBeat = cycleStart;
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

  if(_isAliensBooming){
    assert(_alienBoomer != nullptr);

    const AlienClass& ac = _alienClasses[_alienBoomer->_classId];

    Vector2f position(_alienBoomer->_position._x, _alienBoomer->_position._y);
    Assets::Key_t bitmapKey = SpaceInvaders::BMK_ALIENBOOM;
    Color3f& color = _colorPallete[ac._colorIndex];

    renderer->blitBitmap(position, nomad::assets->getBitmap(bitmapKey, _worldScale), color);
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

void GameState::drawBombs()
{
  for(auto& bomb : _bombs){
    if(!bomb._isAlive)
      continue;

    const BombClass& bc = _bombClasses[bomb._classId];
    Assets::Key_t bitmapKey = bc._bitmapKeys[bomb._frame];
    Color3f& color = _colorPallete[bc._colorIndex];
    renderer->blitBitmap(bomb._position, nomad::assets->getBitmap(bitmapKey, _worldScale), color);
  }
}

void GameState::drawBombBooms()
{
  for(auto& boom : _bombBooms){
    if(!boom._isAlive)
      continue;

    Assets::Key_t bitmapKey = _bombBoomKeys[boom._hit];
    Color3f& color = _colorPallete[boom._colorIndex];
    Vector2f position = Vector2f(boom._position._x, boom._position._y);
    renderer->blitBitmap(position, nomad::assets->getBitmap(bitmapKey, _worldScale), color);
  }
}

void GameState::drawLaser()
{
  if(!_laser._isAlive)
    return;

  renderer->blitBitmap(_laser._position, nomad::assets->getBitmap(_laser._bitmapKey, _worldScale), _colorPallete[_laser._colorIndex]);
}

void GameState::drawHitbar()
{
  renderer->blitBitmap({0.f, _hitbar->_positionY}, _hitbar->_bitmap, _colorPallete[_hitbar->_colorIndex]);
}

void GameState::drawBunkers()
{
  for(const auto& bunker : _bunkers)
    renderer->blitBitmap(bunker->_position, bunker->_bitmap, _colorPallete[_bunkerColorIndex]);
}

void GameState::onDraw(double now, float dt)
{
  renderer->clearViewport(colors::black);
  drawGrid();
  drawCannon();
  drawBombs();
  drawBombBooms();
  drawLaser();
  drawBunkers();
  drawHitbar();
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
  if(nomad::input->isKeyPressed(Input::KEY_s))
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


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

  nomad::assets->loadBitmaps(manifest);

  manifest.clear();
  manifest.push_back({fontKey, fontName, _worldScale});
  nomad::assets->loadFonts(manifest);

  std::unique_ptr<ApplicationState> game = std::make_unique<GameState>(this);
  std::unique_ptr<ApplicationState> menu = std::make_unique<MenuState>(this);
  std::unique_ptr<ApplicationState> splash = std::make_unique<SplashState>(this);

  game->initialize(_worldSize, _worldScale);
  menu->initialize(_worldSize, _worldScale);
  splash->initialize(_worldSize, _worldScale);

  addState(std::move(game));
  addState(std::move(menu));
  addState(std::move(splash));

  switchState(SplashState::name);

  _score = 0;
  _hiscore = 0;
  _round = 0;
  _credit = 0;
  _isHudVisible = false;

  _hud.initialize(&(nomad::assets->getFont(fontKey, _worldScale)), flashPeriod, phasePeriod);

  _uidScoreText = _hud.addTextLabel({Vector2i{10, 240} * _worldScale, nomad::colors::magenta, "SCORE"});
  _uidScoreValue = _hud.addIntLabel({Vector2i{10, 230} * _worldScale, nomad::colors::white, &_score, 5});
  _uidHiScoreText = _hud.addTextLabel({Vector2i{85, 240} * _worldScale, nomad::colors::red, "HI-SCORE"});
  _uidHiScoreValue = _hud.addIntLabel({Vector2i{95, 230} * _worldScale, nomad::colors::green, &_hiscore, 5});
  _uidRoundText = _hud.addTextLabel({Vector2i{170, 240} * _worldScale, nomad::colors::yellow, "ROUND"});
  _uidRoundValue = _hud.addIntLabel({Vector2i{170, 230} * _worldScale, nomad::colors::magenta, &_round, 5});
  _uidCreditText = _hud.addTextLabel({Vector2i{130, 6} * _worldScale, nomad::colors::magenta, "CREDIT"});
  _uidCreditValue = _hud.addIntLabel({Vector2i{190, 6} * _worldScale, nomad::colors::cyan, &_credit, 1});

  return true;
}

void SpaceInvaders::onUpdate(double now, float dt)
{
  Application::onUpdate(now, dt);
  _hud.onUpdate(dt);
}

void SpaceInvaders::onDraw(double now, float dt)
{
  Application::onDraw(now ,dt);

  if(_isHudVisible && !isWindowTooSmall())
    _hud.onDraw();
}

void SpaceInvaders::hideHudTop()
{
  _hud.hideTextLabel(_uidScoreText);
  _hud.hideIntLabel(_uidScoreValue);
  _hud.hideTextLabel(_uidHiScoreText);
  _hud.hideIntLabel(_uidHiScoreValue);
  _hud.hideTextLabel(_uidRoundText);
  _hud.hideIntLabel(_uidRoundValue);
}

void SpaceInvaders::unhideHudTop()
{
  _hud.unhideTextLabel(_uidScoreText);
  _hud.unhideIntLabel(_uidScoreValue);
  _hud.unhideTextLabel(_uidHiScoreText);
  _hud.unhideIntLabel(_uidHiScoreValue);
  _hud.unhideTextLabel(_uidRoundText);
  _hud.unhideIntLabel(_uidRoundValue);
}

//===============================================================================================//
// ##>SPLASH STATE                                                                               //
//===============================================================================================//

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
    std::make_unique<Bitmap>(nomad::assets->makeBlockBitmap(_blockSize, _blockSize)),
    {_signX, _spaceY},
    nomad::colors::green,
    nomad::colors::cyan,
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
    std::make_unique<Bitmap>(nomad::assets->makeBlockBitmap(_blockSize, _blockSize)),
    {_signX, _invadersY},
    nomad::colors::magenta,
    nomad::colors::yellow,
    0.002f,
    _blockSpace,
    _blockSize
  });

  _partiiPosition = Vector2i{80, 48} * _worldScale;
  _partiiColor = nomad::colors::red;
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
      std::string text {"*Cloned by Ian Murfin*"};
      _uidAuthor = hud.addTextLabel({Vector2i{32, 24} * _worldScale, nomad::colors::cyan, text});
      si->unhideHud();
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

void SplashState::onReset()
{
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
    nomad::renderer->blitBitmap(_partiiPosition, 
                                nomad::assets->getBitmap(SpaceInvaders::BMK_PARTII, _worldScale), 
                                _partiiColor);
  }
}

//===============================================================================================//
// ##>GAME STATE                                                                                 //
//===============================================================================================//

GameState::GameState(Application* app) : 
  ApplicationState{app},
  _rand0To100{0,100},
  _randColumn{1, gridWidth},
  _randBombClass{CROSS, ZAGZIG}
{}

void GameState::initialize(Vector2i worldSize, int32_t worldScale)
{
  // This function 'hard-codes' all game data in one place so it is easy to find.

  _worldSize = worldSize;
  _worldScale = worldScale;

  _font = &(nomad::assets->getFont(SpaceInvaders::fontKey, _worldScale));

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
  _worldTopBorderY = _worldSize._y - (30.f * _worldScale);

  _alienBoomDuration = 0.1f;
  _alienMorphDuration = 0.2f;

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
    {2,  3, cycleEnd, 0},  // ticks:22.00   freq:2.70
    {5,  cycleEnd, 0, 0},  // ticks:18.33   freq:3.33
    {7,  cycleEnd, 0, 0},  // ticks:13.75   freq:4.35
    {10, cycleEnd, 0, 0},  // ticks:11.00   freq:5.56
    {14, cycleEnd, 0, 0},  // ticks:9.17    freq:6.67
    {19, cycleEnd, 0, 0},  // ticks:7.86    freq:7.69
    {25, cycleEnd, 0, 0},  // ticks:6.88    freq:9.09
    {34, cycleEnd, 0, 0},  // ticks:6.11    freq:9.81
    {46, cycleEnd, 0, 0}   // ticks:5.00    freq:12.00
  }};

  // The alien population that triggers the cycle.
  _cycleTransitions = {49, 42, 35, 28, 21, 14, 10, 7, 5, 4, 3, 2, 0};

  _alienClasses = {{
    {8  * _worldScale, 8 * _worldScale, 30, 1, {SpaceInvaders::BMK_SQUID0    , SpaceInvaders::BMK_SQUID1    }},
    {11 * _worldScale, 8 * _worldScale, 20, 4, {SpaceInvaders::BMK_CRAB0     , SpaceInvaders::BMK_CRAB1     }},
    {12 * _worldScale, 8 * _worldScale, 10, 3, {SpaceInvaders::BMK_OCTOPUS0  , SpaceInvaders::BMK_OCTOPUS1  }},
    {8  * _worldScale, 8 * _worldScale, 30, 5, {SpaceInvaders::BMK_CUTTLE0   , SpaceInvaders::BMK_CUTTLE1   }},
    {19 * _worldScale, 8 * _worldScale, 60, 5, {SpaceInvaders::BMK_CUTTLETWIN, SpaceInvaders::BMK_CUTTLETWIN}}
  }};

  _ufoClasses = {{
    {16 * _worldScale, 7 * _worldScale, 50,  0, SpaceInvaders::BMK_SAUCER     , SpaceInvaders::BMK_UFOBOOM, SpaceInvaders::BMK_SAUCERSCORE     },
    {15 * _worldScale, 7 * _worldScale, 500, 3, SpaceInvaders::BMK_SCHRODINGER, SpaceInvaders::BMK_UFOBOOM, SpaceInvaders::BMK_SCHRODINGERSCORE}
  }};

  _ufoSpawnY = 210.f * _worldScale;
  _ufoLifetime = 7.f;                 // should tailor lifetime and speed so ufo lives long enough
  _ufoSpeed = 40.f * _worldScale;     // to move fully across the world and off screen.
  _ufoBoomDuration = 0.5f;
  _ufoScoreDuration = 0.5f;
  _ufoPhaseDuration = 0.8f;

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
  _bombIntervals = {80, 80, 100, 120, 140, 180, 240, 300, 400, 500, 650, 800, 1100};

  _bombClasses = {{
    {3 * _worldScale, 6 * _worldScale, -100.f * _worldScale, 0, 20, {SpaceInvaders::BMK_CROSS0, SpaceInvaders::BMK_CROSS1, SpaceInvaders::BMK_CROSS2, SpaceInvaders::BMK_CROSS3}},
    {3 * _worldScale, 7 * _worldScale, -100.f * _worldScale, 4, 20, {SpaceInvaders::BMK_ZIGZAG0, SpaceInvaders::BMK_ZIGZAG1, SpaceInvaders::BMK_ZIGZAG2, SpaceInvaders::BMK_ZIGZAG3}},
    {3 * _worldScale, 7 * _worldScale, -100.f * _worldScale, 5, 20, {SpaceInvaders::BMK_ZAGZIG0, SpaceInvaders::BMK_ZAGZIG1, SpaceInvaders::BMK_ZAGZIG2, SpaceInvaders::BMK_ZAGZIG3}}
  }};

  _bombBoomKeys = {{SpaceInvaders::BMK_BOMBBOOMBOTTOM, SpaceInvaders::BMK_BOMBBOOMMIDAIR}};
  _bombBoomWidth = 8 * _worldScale;
  _bombBoomHeight = 8 * _worldScale;
  _bombBoomDuration = 0.4f;

  _cannon._spawnPosition = Vector2f{_worldLeftBorderX, 32.f * _worldScale};
  _cannon._speed = 50.f * _worldScale;
  _cannon._width = 13 * _worldScale;
  _cannon._height = 8 * _worldScale;
  _cannon._boomFrameDuration = 0.2f;
  _cannon._boomDuration = _cannon._boomFrameDuration * 5;
  _cannon._cannonKey = SpaceInvaders::BMK_CANNON0;
  _cannon._boomKeys = {{SpaceInvaders::BMK_CANNONBOOM0, SpaceInvaders::BMK_CANNONBOOM1, SpaceInvaders::BMK_CANNONBOOM2}};
  _cannon._colorIndex = 0;

  _laser._width = 1 * _worldScale;
  _laser._height = 6 * _worldScale;
  _laser._colorIndex = 6;
  _laser._speed = 300.f * _worldScale;
  _laser._bitmapKey = SpaceInvaders::BMK_LASER0;

  _bunkerColorIndex = 0;
  _bunkerSpawnX = 32 * _worldScale;
  _bunkerSpawnY = 48 * _worldScale;
  _bunkerSpawnGapX = 45 * _worldScale;
  _bunkerSpawnCount = 4;
  _bunkerWidth = 22 * _worldScale;
  _bunkerHeight = 16 * _worldScale;
  _bunkerDeleteThreshold = 20 * _worldScale;

  _levels = {{
    {5, 0, 10, true, true},
    {5, 0, 10, true, true},
    {5, 0, 10, true, true},
    {5, 0, 10, true, true},
    {5, 0, 10, true, true},
    {5, 0, 10, true, true},
    {5, 0, 10, true, true},
    {5, 0, 10, true, true},
    {5, 0, 10, true, true},
    {5, 0, 10, true, true},
  }};

  _levelIndex = -1;
  _round = -1;

  _score = 0;
  _lives = 5; // requires 1 life for initial spawn, thus actually start with 4.
  _credit = 0;
  _isGameOver = false;
  _gameOverDuration = 5.f;

  // Set HUD elements.
  
  _gameOverLabel = {{72.f * _worldScale, 188.f * _worldScale}, "GAME OVER", 3};
  _scoreLabel = {{10.f * _worldScale, 240.f * _worldScale}, "SCORE", 4};
  _recordLabel = {{90.f * _worldScale, 240.f * _worldScale}, "RECORD", 0};
  _roundLabel = {{170.f * _worldScale, 240.f * _worldScale}, "ROUND", 5};
  _creditLabel = {{130.f * _worldScale, 6.f * _worldScale}, "CREDIT", 3};
  _scoreValueLabel = {{16.f * _worldScale, 230.f * _worldScale}, &_score, 6};
  _recordValueLabel = {{96.f * _worldScale, 230.f * _worldScale}, &_score, 1};
  _roundValueLabel = {{190.f * _worldScale, 230.f * _worldScale}, &_round, 3};
  _creditValueLabel = {{190.f * _worldScale, 6.f * _worldScale}, &_credit, 4};
  _lifeValueLabel = {{10.f * _worldScale, 6.f * _worldScale}, &_lives, 5};
  _lifeCannonLabel = {{20.f * _worldScale, 6.f * _worldScale}, SpaceInvaders::BMK_CANNON0, 4};
  _lifeCannonSpacingX = 16 * _worldScale;

  startNextLevel();
}

void GameState::startNextLevel()
{
  ++_round;
  ++_levelIndex;
  if(_levelIndex == levelCount)
    --_levelIndex;

  _activeCycle = 5;
  _activeBeat = cycleStart;
  _nextMover = {0, 0};
  _alienMoveDirection = 1;
  _dropsDone = 0;
  _isAliensBooming = false;
  _isAliensMorphing = false;
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

  _ufoSpawnNo = 0;
  _ufoLastSpawnPop = _alienPopulation;
  _schrodingerSpawnNo = _rand0To100() % (_alienPopulation / _levels[_levelIndex]._ufoSpawnRate);
  _isUfoBooming = false;
  _isUfoScoring = false;
  _ufo._isAlive = false;

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

  _bombClock = _bombIntervals[_activeCycle];

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

  _showHud = false;
}

void GameState::updateActiveCycle()
{
  _activeCycle = 0;
  while(_alienPopulation < _cycleTransitions[_activeCycle])
    ++_activeCycle;
}

void GameState::endSpawning()
{
  _isAliensSpawning = false;
  _isAliensDropping = false;
  spawnCannon();
  _showHud = true;
  _activeCycle = 0;
}

void GameState::spawnCannon()
{
  --_lives;
  if(_lives == 0){
    _isGameOver = true;
    _gameOverClock = _gameOverDuration;
    return;
  }

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
  bomb->_frameClock = _bombClasses[classId]._frameInterval;

  ++_bombCount;
}

void GameState::spawnBunker(Vector2f position, Assets::Key_t bitmapKey)
{
  const Bitmap& bitmap = nomad::assets->getBitmap(bitmapKey, _worldScale);
  _bunkers.emplace_back(std::make_unique<Bunker>(bitmap, position));
}

void GameState::spawnUfo(UfoClassId classId)
{
  _ufo._classId = classId;
  _ufo._age = _ufoLifetime;  // Benjamin Button.
  _ufo._isAlive = true;
  int32_t direction = _rand0To100() % 2;
  _ufoDirection = (direction == 0) ? 1 : -1;
  int32_t w = _ufoClasses[classId]._width;
  _ufo._position._x = -(w + 10.f) + (direction * (_worldSize._x + 20.f + w));
  _ufo._position._y = _ufoSpawnY;
  _ufoLastSpawnPop = _alienPopulation;
  ++_ufoSpawnNo;
}

void GameState::morphAlien(Alien& alien)
  // predicate: alien->_col != gridWidth - 1
{
  _score += _alienClasses[alien._classId]._scoreValue;

  alien._classId = CUTTLETWIN;
  _alienMorpher = &alien;
  _alienMorphClock = _alienMorphDuration;
  _isAliensFrozen = true;
  _isAliensMorphing = true;
  _cannon._isFrozen = true;

  Alien& neighbour = _grid[alien._row][alien._col + 1];
  if(neighbour._isAlive){
    neighbour._isAlive = false;
    --(_columnPops[neighbour._col]);
    --(_rowPops[neighbour._row]);
    --_alienPopulation;
  }
}

void GameState::boomCannon()
{
  _cannon._moveDirection = 0;
  _cannon._boomClock = _cannon._boomDuration;
  _cannon._boomFrame = 0;
  _cannon._boomFrameClock = _cannon._boomFrameDuration;
  _cannon._isBooming = true;
  _cannon._isAlive = false;

  _isAliensFrozen = true;
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
  _score += _alienClasses[alien._classId]._scoreValue;

  alien._isAlive = false;
  _alienBoomer = &alien;
  _alienBoomClock = _alienBoomDuration;
  _isAliensFrozen = true;
  _isAliensBooming = true;
  _cannon._isFrozen = true;
  --(_columnPops[alien._col]);
  --(_rowPops[alien._row]);
  --_alienPopulation;
  updateActiveCycle();
  doUfoSpawning();
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

void GameState::doUfoSpawning()
{
  if(_ufo._isAlive)
    return;

  if((_ufoLastSpawnPop - _alienPopulation) >= _levels[_levelIndex]._ufoSpawnRate){
    if(_ufoSpawnNo == _schrodingerSpawnNo)
     spawnUfo(SCHRODINGER);
    else
      spawnUfo(SAUCER);
  }
}

void GameState::doAlienMorphing(float dt)
  // predicate: _alienMorpher->_col != gridWidth - 1
{
  if(!_isAliensMorphing)
    return;

  if(_alienPopulation == 0)
    return;

  _alienMorphClock -= dt;
  if(_alienMorphClock > 0)
    return;

  _alienMorpher->_classId = CUTTLE;

  Alien& neighbour = _grid[_alienMorpher->_row][_alienMorpher->_col + 1];
  neighbour._classId = CUTTLE;
  neighbour._isAlive = true;
  ++(_columnPops[neighbour._col]);
  ++(_rowPops[neighbour._row]);
  ++_alienPopulation;

  _isAliensMorphing = false;
  _isAliensFrozen = false;
  _cannon._isFrozen = false;
  _alienMorpher = nullptr;
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

void GameState::doCannonBooming(float dt)
{
  if(!_cannon._isBooming)
    return;

  _cannon._boomClock -= dt;
  if(_cannon._boomClock <= 0){
    _cannon._isBooming = false;
    spawnCannon();
    return;
  }

  _cannon._boomFrameClock -= dt;
  if(_cannon._boomFrameClock <= 0){
    _cannon._boomFrame = nomad::wrap(++_cannon._boomFrame, 0, cannonBoomFramesCount - 1);
    _cannon._boomFrameClock = _cannon._boomFrameDuration;
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

  if(nomad::input->isKeyDown(Input::KEY_UP)){
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

void GameState::doUfoMoving(float dt)
{
  if(!_ufo._isAlive)
    return;

  _ufo._position._x += _ufoSpeed * _ufoDirection * dt;
}

void GameState::doAlienBombing(int32_t beats)
{
  // Cycles determine alien bomb rate. Aliens bomb every N beats, thus the higher beat rate
  // the higher the rate of bombing. Randomness is added in a random deviation to the bomb 
  // interval and to the choice of alien which does the bombing.

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

  _bombClock = _bombIntervals[_activeCycle];
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

void GameState::doUfoBooming(float dt)
{

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

void GameState::doUfoReinforcing(float dt)
{

}

void GameState::doUfoAging(float dt)
{
  if(!_ufo._isAlive)
    return;

  _ufo._age -= dt;
  if(_ufo._age <= 0.f)
    _ufo._isAlive = false;
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
        if(alien._classId == CUTTLETWIN){
          _alienMorpher = nullptr;
          _isAliensMorphing = false;
        }
        if(_levels[_levelIndex]._isCuttlesOn && alien._classId == CRAB && alien._col != gridWidth - 1)
          morphAlien(alien);
        else
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

  if(_bunkers.size() <= 0)
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

    for(auto iter = _bunkers.begin(); iter != _bunkers.end(); ++iter){
      Bunker& bunker = *(*iter);

      bPosition._x = bunker._position._x;
      bPosition._y = bunker._position._y;

      bBitmap = &bunker._bitmap;

      const Collision& c = testCollision(aPosition, *aBitmap, bPosition, *bBitmap, false);

      if(c._isCollision){
        boomBomb(bomb);
        boomBunker(bunker, c._bPixels.front());
        if(bunker._bitmap.isApproxEmpty(_bunkerDeleteThreshold))
          _bunkers.erase(iter);
        return;
      }
    }
  }
}

void GameState::doCollisionsBunkersLaser()
{
  if(!_laser._isAlive)
    return;

  if(_bunkers.size() == 0)
    return;

  Vector2i aPosition {};
  Vector2i bPosition {};

  const Bitmap* aBitmap {nullptr};
  const Bitmap* bBitmap {nullptr};

  aPosition._x = _laser._position._x;
  aPosition._y = _laser._position._y;

  aBitmap = &(nomad::assets->getBitmap(_laser._bitmapKey, _worldScale));

  for(auto iter = _bunkers.begin(); iter != _bunkers.end(); ++iter){
    Bunker& bunker = *(*iter);

    bPosition._x = bunker._position._x;
    bPosition._y = bunker._position._y;

    bBitmap = &(bunker._bitmap);

    const Collision& c = testCollision(aPosition, *aBitmap, bPosition, *bBitmap, false);
    
    if(c._isCollision){
      boomLaser(false);
      boomBunker(bunker, c._bPixels.front());
      if(bunker._bitmap.isApproxEmpty(_bunkerDeleteThreshold))
        _bunkers.erase(iter);
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

  if(_bunkers.size() == 0)
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
    if(!alien._isAlive)
      continue;

    aPosition._x = alien._position._x;
    aPosition._y = alien._position._y;

    const AlienClass& ac = _alienClasses[alien._classId];
    Assets::Key_t bitmapKey = ac._bitmapKeys[alien._frame];
    aBitmap = &(nomad::assets->getBitmap(bitmapKey, _worldScale));

    for(auto iter = _bunkers.begin(); iter != _bunkers.end(); ++iter){
      Bunker& bunker = *(*iter);

      bPosition._x = bunker._position._x;
      bPosition._y = bunker._position._y;

      bBitmap = &(bunker._bitmap);

      const Collision& c = testCollision(aPosition, *aBitmap, bPosition, *bBitmap, false);

      if(c._isCollision){
        bunker._bitmap.setRect(c._bOverlap._ymin, c._bOverlap._xmin, 
                               c._bOverlap._ymax - 1, c._bOverlap._xmax - 1, false);

        if(bunker._bitmap.isApproxEmpty(_bunkerDeleteThreshold))
          _bunkers.erase(iter);

        return;
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

  doAlienMorphing(dt);
  doBombMoving(beats, dt);
  doLaserMoving(dt);
  doAlienMoving(beats);
  doAlienBombing(beats);
  doCannonMoving(dt);
  doUfoMoving(dt);
  doUfoAging(dt);
  doCannonBooming(dt);
  doAlienBooming(dt);
  doBombBoomBooming(dt);
  doUfoBooming(dt);
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
  
  if(_isGameOver){
    _gameOverClock -= dt;
    if(_gameOverClock <= 0)
      _app->switchState(MenuState::name);
  }
  
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

void GameState::drawUfo()
{
  if(!_ufo._isAlive)
    return;

  const UfoClass& uc = _ufoClasses[_ufo._classId];

  Assets::Key_t bitmapKey;
  if(_ufo._isAlive)
    bitmapKey = uc._shipKey;
  else if(_isUfoBooming)
    bitmapKey = uc._boomKey;
  else if(_isUfoScoring)
    bitmapKey = uc._scoreKey;

  renderer->blitBitmap(
      _ufo._position, 
      nomad::assets->getBitmap(bitmapKey, _worldScale), 
      _colorPallete[uc._colorIndex]
  );
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

void GameState::drawHud()
{
  if(!_showHud)
    return;

  renderer->blitText(_scoreLabel._position, _scoreLabel._message, *_font, _colorPallete[_scoreLabel._colorIndex]);
  renderer->blitText(_recordLabel._position, _recordLabel._message, *_font, _colorPallete[_recordLabel._colorIndex]);
  renderer->blitText(_roundLabel._position, _roundLabel._message, *_font, _colorPallete[_roundLabel._colorIndex]);
  renderer->blitText(_creditLabel._position, _creditLabel._message, *_font, _colorPallete[_creditLabel._colorIndex]);

  renderer->blitText(_scoreValueLabel._position, std::to_string(*_scoreValueLabel._value), *_font, _colorPallete[_scoreValueLabel._colorIndex]);
  renderer->blitText(_recordValueLabel._position, std::to_string(*_recordValueLabel._value), *_font, _colorPallete[_recordValueLabel._colorIndex]);
  renderer->blitText(_roundValueLabel._position, std::to_string(*_roundValueLabel._value), *_font, _colorPallete[_roundValueLabel._colorIndex]);
  renderer->blitText(_creditValueLabel._position, std::to_string(*_creditValueLabel._value), *_font, _colorPallete[_creditValueLabel._colorIndex]);
  renderer->blitText(_lifeValueLabel._position, std::to_string(*_lifeValueLabel._value), *_font, _colorPallete[_lifeValueLabel._colorIndex]);

  const Bitmap& cannonBitmap = nomad::assets->getBitmap(_lifeCannonLabel._bitmapKey, _worldScale);
  Vector2f position {};
  for(int i = 0; i < _lives - 1; ++i){
    position._x = _lifeCannonLabel._position._x + (_lifeCannonSpacingX * i);
    position._y = _lifeCannonLabel._position._y;
    renderer->blitBitmap(position, cannonBitmap, _colorPallete[_lifeCannonLabel._colorIndex]);
  }

  if(_isGameOver)
    renderer->blitText(_gameOverLabel._position, _gameOverLabel._message, *_font, _colorPallete[_gameOverLabel._colorIndex]);
}

void GameState::onDraw(double now, float dt)
{
  renderer->clearViewport(colors::black);
  drawGrid();
  drawUfo();
  drawCannon();
  drawBombs();
  drawBombBooms();
  drawLaser();
  drawBunkers();
  drawHitbar();
  drawHud();
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
  if(nomad::input->isKeyPressed(Input::KEY_ENTER)){
    depopulateHud();
    _app->switchState(GameState::name);
  }
  if(nomad::input->isKeyPressed(Input::KEY_s)){
    depopulateHud();
    _app->switchState(SplashState::name);
  }
}

void MenuState::onDraw(double now, float dt)
{
  renderer->clearViewport(colors::black);
}

void MenuState::onReset()
{
  populateHud();
}

void MenuState::populateHud()
{
  SpaceInvaders* si = static_cast<SpaceInvaders*>(_app);
  HUD& hud = si->getHud();
  _uidMenuText = hud.addTextLabel({Vector2i{91, 204} * _worldScale, nomad::colors::cyan, "*MENU*"});
  _uidMenuBitmap = hud.addBitmapLabel({Vector2i{56, 182} * _worldScale, nomad::colors::white, &(nomad::assets->getBitmap(SpaceInvaders::BMK_MENU, _worldScale))});
  _uidControlsText = hud.addTextLabel({Vector2i{76, 162} * _worldScale, nomad::colors::cyan, "*CONTROLS*"});
  _uidControlsBitmap = hud.addBitmapLabel({Vector2i{58, 134} * _worldScale, nomad::colors::white, &(nomad::assets->getBitmap(SpaceInvaders::BMK_CONTROLS, _worldScale))});
  _uidTablesText = hud.addTextLabel({Vector2i{40, 108} * _worldScale, nomad::colors::cyan, "*SCORE ADVANCE TABLE*"});
  _uidSchroBitmap = hud.addBitmapLabel({Vector2i{62, 90} * _worldScale, nomad::colors::magenta, &(nomad::assets->getBitmap(SpaceInvaders::BMK_SCHRODINGER, _worldScale))});
  _uidSaucerBitmap = hud.addBitmapLabel({Vector2i{62, 74} * _worldScale, nomad::colors::magenta, &(nomad::assets->getBitmap(SpaceInvaders::BMK_SAUCER, _worldScale))});
  _uidSquidBitmap = hud.addBitmapLabel({Vector2i{66, 58} * _worldScale, nomad::colors::yellow, &(nomad::assets->getBitmap(SpaceInvaders::BMK_SQUID0, _worldScale))});
  _uidCuttleBitmap = hud.addBitmapLabel({Vector2i{52, 58} * _worldScale, nomad::colors::yellow, &(nomad::assets->getBitmap(SpaceInvaders::BMK_CUTTLE0, _worldScale))});
  _uidCrabBitmap = hud.addBitmapLabel({Vector2i{64, 42} * _worldScale, nomad::colors::yellow, &(nomad::assets->getBitmap(SpaceInvaders::BMK_CRAB0, _worldScale))});
  _uidOctopusBitmap = hud.addBitmapLabel({Vector2i{64, 26} * _worldScale, nomad::colors::red, &(nomad::assets->getBitmap(SpaceInvaders::BMK_OCTOPUS0, _worldScale))});
  _uid500PointsText = hud.addTextLabel({Vector2i{82, 90} * _worldScale, nomad::colors::magenta, "= 500 POINTS", 0.f, true});
  _uidMysteryPointsText = hud.addTextLabel({Vector2i{82, 74} * _worldScale, nomad::colors::magenta, "= ? MYSTERY", 1.f, true});
  _uid30PointsText = hud.addTextLabel({Vector2i{82, 58} * _worldScale, nomad::colors::yellow, "= 30 POINTS", 2.f, true});
  _uid20PointsText = hud.addTextLabel({Vector2i{82, 42} * _worldScale, nomad::colors::yellow, "= 20 POINTS", 3.f, true});
  _uid10PointsText = hud.addTextLabel({Vector2i{82, 26} * _worldScale, nomad::colors::red, "= 10 POINTS", 4.f, true});
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


#include "pixiretro.h"
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

  Application::onWindowResize(windowWidth, windowHeight);

  Assets::Manifest_t manifest{};
  for(int32_t i = BMK_CANNON0; i < BMK_COUNT; ++i)
    manifest.push_back({i, _bitmapNames[i], _worldScale}); 

  pxr::assets->loadBitmaps(manifest);

  manifest.clear();
  manifest.push_back({fontKey, fontName, _worldScale});
  pxr::assets->loadFonts(manifest);

  Mixer::Manifest_t mixmanifest{};
  for(int32_t i = SK_EXPLOSION; i < SK_COUNT; ++i)
    mixmanifest.push_back({i, _soundNames[i]});

  loadHiScores();
  updateHudHiScore();

  pxr::mixer->loadSoundsWAV(mixmanifest);

  _isHudVisible = false;
  _hud.initialize(&(pxr::assets->getFont(fontKey, _worldScale)), flashPeriod, phasePeriod);
  _uidScoreText = _hud.addTextLabel({Vector2i{10, 240} * _worldScale, pxr::colors::magenta, "SCORE"});
  _uidScoreValue = _hud.addIntLabel({Vector2i{10, 230} * _worldScale, pxr::colors::white, &_score, 5});
  _uidHiScoreText = _hud.addTextLabel({Vector2i{85, 240} * _worldScale, pxr::colors::red, "HI-SCORE"});
  _uidHiScoreValue = _hud.addIntLabel({Vector2i{95, 230} * _worldScale, pxr::colors::green, &_hiscore, 5});
  _uidRoundText = _hud.addTextLabel({Vector2i{170, 240} * _worldScale, pxr::colors::yellow, "ROUND"});
  _uidRoundValue = _hud.addIntLabel({Vector2i{170, 230} * _worldScale, pxr::colors::magenta, &_round, 5});
  _uidCreditText = _hud.addTextLabel({Vector2i{130, 6} * _worldScale, pxr::colors::magenta, "CREDIT"});
  _uidCreditValue = _hud.addIntLabel({Vector2i{190, 6} * _worldScale, pxr::colors::cyan, &_credit, 1});
  _uidLivesValue = _hud.addIntLabel({Vector2i{10, 6} * _worldScale, pxr::colors::yellow, &_lives, 1});

  for(int i{0}; i < maxLivesHudCannons; ++i){
    _uidLivesBitmaps[i] = _hud.addBitmapLabel({
      Vector2i{(20 + (16 * i)), 6} * _worldScale, 
      pxr::colors::green, 
      &(assets->getBitmap(SpaceInvaders::BMK_CANNON0, _worldScale))
    });
  }

  resetGameStats();

  std::unique_ptr<ApplicationState> game = std::make_unique<GameState>(this);
  std::unique_ptr<ApplicationState> menu = std::make_unique<MenuState>(this);
  std::unique_ptr<ApplicationState> splash = std::make_unique<SplashState>(this);
  std::unique_ptr<ApplicationState> scoreReg = std::make_unique<HiScoreRegState>(this);
  std::unique_ptr<ApplicationState> scoreBoard = std::make_unique<HiScoreBoardState>(this);
  std::unique_ptr<ApplicationState> sos = std::make_unique<SosState>(this);

  game->initialize(_worldSize, _worldScale);
  menu->initialize(_worldSize, _worldScale);
  splash->initialize(_worldSize, _worldScale);
  scoreReg->initialize(_worldSize, _worldScale);
  scoreBoard->initialize(_worldSize, _worldScale);
  sos->initialize(_worldSize, _worldScale);

  // a bodge! dont look! its really ugly! :) ... your still looking! I warned you, you'll regret it!
  static_cast<SosState*>(sos.get())->_gameState = static_cast<GameState*>(game.get());

  addState(std::move(game));
  addState(std::move(menu));
  addState(std::move(splash));
  addState(std::move(scoreReg));
  addState(std::move(scoreBoard));
  addState(std::move(sos));

  switchState(SplashState::name);

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

void SpaceInvaders::hideTopHud()
{
  _hud.hideTextLabel(_uidScoreText);
  _hud.hideIntLabel(_uidScoreValue);
  _hud.hideTextLabel(_uidHiScoreText);
  _hud.hideIntLabel(_uidHiScoreValue);
  _hud.hideTextLabel(_uidRoundText);
  _hud.hideIntLabel(_uidRoundValue);
}

void SpaceInvaders::showTopHud()
{
  _hud.showTextLabel(_uidScoreText);
  _hud.showIntLabel(_uidScoreValue);
  _hud.showTextLabel(_uidHiScoreText);
  _hud.showIntLabel(_uidHiScoreValue);
  _hud.showTextLabel(_uidRoundText);
  _hud.showIntLabel(_uidRoundValue);
}

void SpaceInvaders::hideLivesHud()
{
  _isLivesHudVisible = false;
  _hud.hideIntLabel(_uidLivesValue);
  for(auto uid : _uidLivesBitmaps)
    _hud.hideBitmapLabel(uid);
}

void SpaceInvaders::showLivesHud()
{
  _isLivesHudVisible = true;
  _hud.showIntLabel(_uidLivesValue);
  for(int life{1}; life < maxPlayerLives; ++life)
    if(life < _lives)
      _hud.showBitmapLabel(_uidLivesBitmaps[life - 1]);
}

void SpaceInvaders::startScoreHudFlash()
{
  _hud.startIntLabelFlash(_uidScoreValue);
}

void SpaceInvaders::stopScoreHudFlash()
{
  _hud.stopIntLabelFlash(_uidScoreValue);
}

void SpaceInvaders::setLives(int32_t lives)
{
  _lives = std::max(0, lives);
  updateLivesHud();
}

void SpaceInvaders::addLives(int32_t lives)
{
  _lives += lives;
  _lives = std::max(0, _lives);
  updateLivesHud();
}

void SpaceInvaders::updateLivesHud()
{
  if(!_isLivesHudVisible)
    return;

  for(int life{1}; life < maxPlayerLives; ++life){
    if(life < _lives)
      _hud.showBitmapLabel(_uidLivesBitmaps[life - 1]);
    else
      _hud.hideBitmapLabel(_uidLivesBitmaps[life - 1]);
  }
}

void SpaceInvaders::resetGameStats()
{
  setLives(maxPlayerLives);
  clearPlayerName();
  _round = 0;
  _credit = 0;
  _score = 0;
}

void SpaceInvaders::loadHiScores()
{
  ScoreData data {}; 
  if(data.load(ScoreData::filename) != 0)
    data.write(ScoreData::filename, false);

  for(int i{0}; i < hiscoreCount; ++i){
    int nameKey = ScoreData::NAME0 + (i * 2);
    int scoreKey = ScoreData::SCORE0 + (i * 2);
    int iname = data.getIntValue(nameKey);
    int score = data.getIntValue(scoreKey);
    _hiscores[i]._name = intToName(iname);
    _hiscores[i]._value = score;
  }

  auto scoreCompare = [](const Score& s0, const Score& s1) -> bool {
    return s0._value < s1._value;
  };

  std::sort(_hiscores.begin(), _hiscores.end(), scoreCompare);
}

void SpaceInvaders::writeHiScores()
{
  ScoreData data {}; 

  for(int i{0}; i < hiscoreCount; ++i){
    int nameKey = ScoreData::NAME0 + (i * 2);
    int scoreKey = ScoreData::SCORE0 + (i * 2);
    int iname = nameToInt(_hiscores[i]._name);
    data.setIntValue(nameKey, iname);
    data.setIntValue(scoreKey, _hiscores[i]._value);
  }

  data.write(ScoreData::filename, false);
}

bool SpaceInvaders::isHiScore(int32_t scoreValue)
{
  return scoreValue > _hiscores[0]._value;  
}

bool SpaceInvaders::isDuplicateHiScore(const Score& score)
{
  for(auto& s : _hiscores)
    if((s._name == score._name) && (s._value == score._value))
      return true;
  return false;
}

bool SpaceInvaders::registerHiScore(const Score& score)
{
  size_t position = findScoreBoardPosition(score._value);
  if(position == -1) return false;
  std::shift_left(_hiscores.begin(), _hiscores.begin() + position + 1, 1); 
  _hiscores[position] = score;
  updateHudHiScore();
  return true;
}

size_t SpaceInvaders::findScoreBoardPosition(int32_t scoreValue)
{
  if(scoreValue < _hiscores.front()._value) 
    return -1;

  if(scoreValue > _hiscores.back()._value)
    return hiscoreCount - 1;

  for(int i{0}; i < hiscoreCount - 1; ++i)
    if(_hiscores[i]._value < scoreValue && scoreValue <= _hiscores[i + 1]._value)
      return i;
}

void SpaceInvaders::updateHudHiScore()
{
  _hiscore = _hiscores.back()._value;
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

//===============================================================================================//
// ##>GAME STATE                                                                                 //
//===============================================================================================//

GameState::BeatBox::BeatBox(std::array<Mixer::Key_t, beatCount> beats, float beatFreq_hz) : 
  _beats{beats},
  _nextBeat{0},
  _beatFreq_hz{beatFreq_hz},
  _beatPeriod_s{1.f / beatFreq_hz},
  _beatClock_s{0.f},
  _isPaused{false}
{}

void GameState::checkExit()
{
  if(input->isKeyPressed(Input::KEY_ESCAPE))
    _app->switchState(MenuState::name);
}

void GameState::BeatBox::doBeats(float dt)
{
  if(_isPaused) return;
  _beatClock_s += dt;
  if(_beatClock_s > _beatPeriod_s){
    mixer->playSound(_beats[_nextBeat]);
    _nextBeat = pxr::wrap(_nextBeat + 1, 0, beatCount - 1);
    _beatClock_s = 0.f;
  }
}

void GameState::BeatBox::setBeatFreq(float freq_hz)
{
  _beatFreq_hz = freq_hz;
  _beatPeriod_s = 1 / _beatFreq_hz;
}

GameState::GameState(Application* app) : 
  ApplicationState{app}
{}

void GameState::initialize()
{
  _si = static_cast<SpaceInvaders*>(_app);

  _beatBox = BeatBox{{
      SpaceInvaders::SK_FAST1, 
      SpaceInvaders::SK_FAST2, 
      SpaceInvaders::SK_FAST3, 
      SpaceInvaders::SK_FAST4
    }, 
    2.f
  };

  _font = &(pxr::assets->getFont(SpaceInvaders::fontKey, _worldScale));
  _hud = &(static_cast<SpaceInvaders*>(_app)->getHud());


  _bombIntervalDeviation = 0.5f; // Maximum 50% deviation from base.
  //_bombIntervals = {80, 80, 100, 120, 140, 180, 240, 300, 400, 500, 650, 800, 1100};




  _bunkerColorIndex = 0;
  _bunkerSpawnX = 32 * _worldScale;
  _bunkerSpawnY = 48 * _worldScale;
  _bunkerSpawnGapX = 45 * _worldScale;
  _bunkerSpawnCount = 4;
  _bunkerWidth = 22 * _worldScale;
  _bunkerHeight = 16 * _worldScale;
  _bunkerDeleteThreshold = 20 * _worldScale;

  //_levelIndex = -1;
  _isGameOver = false;
}

void GameState::resetUfoSpawnCountdown()
{
  _ufoSpawnCountdown = pxr::randUniformSignedInt(SI::minUfoSpawnCountdown, SI::maxUfoSpawnCountdown);
}

void GameState::updateBeatFreq()
{
  //int32_t ticksPerCycle {0};
  //int32_t beatsPerCycle {0};
  //for(auto beats : _cycles[_activeCycle]){
  //  if(beats != cycleEnd){
  //    ++ticksPerCycle;
  //    beatsPerCycle += beats;
  //  }
  //  else
  //    break;
  //}

  //// example: If a cycle does 5 beats in 3 ticks, then 5 aliens will move in 3 ticks, so
  //// 55 aliens will move in (55/5) * 3 = 33 ticks. If it takes 33 ticks to move the fleet,
  //// and we do 60 ticks per second, then the frequency of full fleet movements will be
  //// 60 / 33 = 1.81hz.
  ////
  //// hence note that the fequency of the beating equals the frequency of full fleet movements.
  ////
  ////                                   v-- the fixed frame rate set in the engine.
  //float beatFreq_hz = powf(2.f, (60.f / ((fleetSize / beatsPerCycle) * ticksPerCycle)));

  //_beatBox.setBeatFreq(beatFreq_hz);
  _beatBox.setBeatFreq(60.f/ static_cast<float>(_alienPop));
}

//void GameState::updateActiveCycle()
//{
//  _activeCycle = 0;
//  while(_alienPopulation < _cycleTransitions[_activeCycle]){
//    ++_activeCycle;
//
//    // should never happen unless we have set, in _cycleTransitions, for the last cycle
//    // to trigger at a population greater than 0.
//    assert(_activeCycle < cycleCount);
//
//    //updateBeatFreq();
//  }
//}
//
//void GameState::updateActiveCycleBeat()
//{
//  ++_activeBeat;
//  if(_cycles[_activeCycle][_activeBeat] == cycleEnd)
//    _activeBeat = cycleStart;
//}

std::pair<int, int> GameState::getAlienRowRange(int row)
{
  std::pair<int, int> result {};
  result.first = row * fleetWidth;
  result.second = result.first + (fleetWidth - 1);
  return result;
}

GameState::Alien& GameState::getAlien(int row, int col)
{
  return _fleet[(row * fleetWidth) + col];
}

//void GameState::endSpawning()
//{
//  _isAliensDropping = false;
//  spawnCannon(false);
//  //_activeCycle = 0;
//  updateBeatFreq();
//  _beatBox.unpause();
//
//  static_cast<SpaceInvaders*>(_app)->showTopHud();
//}

bool GameState::canAlienBecomeCuttleTwin(const Alien& alien)
{
  return _rounds[_activeRoundNo]._canCuttlesSpawn && 
          alien._classID == CRAB                  && 
          alien._col != fleetWidth - 1);
}

void GameState::spawnCannon(bool takeLife)
{
  if(takeLife){
    _si->addLives(-1);
    if(si->getLives() <= 0){
      startGameOver();
      return;
    }
  }
  _cannon._position._x = SI::cannonSpawnX;
  _cannon._position._y = SI::cannonSpawnY;
  _cannon._moveDirection = 0;
  _cannon._isAlive = true;
  _isAliensFrozen = false;
  _beatBox.unpause();
}

void GameState::spawnBoom(SI::BoomClassID classID, Vector2i position, int colorIndex)
{
  bool boomed {false};
  for(auto& boom : _booms){
    if(boom._isAlive) continue;
    boom._classID = classID;
    boom._position = position;
    boom._boomClock = 0.f;
    boom._boomFrameClock = 0.f;
    boom._colorIndex = colorIndex;
    boom._boomFrame = 0;
    boom._isAlive = boomed = true;
    const auto& bc = SI::boomClasses[classID];
    if(bc._boomSfx != SI::SK_COUNT)
      mixer->playSound(bc._boomSfx);
    break;
  }
  assert(boomed);
}

void GameState::spawnBomb(int fleetCol, BombClassId bombClassId)
{
  assert(_bombCount != maxBombs);
  
  Alien* lowestAlien {nullptr};
  for(int row {0}; row < SI::fleetHeight; ++row){
    lowestAlien = &getAlien(row, fleetCol);
    if(lowestAlien->_isAlive)
      break;
  }

  assert(lowestAlien != nullptr);

  Bomb* bomb {nullptr};
  for(auto& b : _bombs)
    if(!b._isAlive)
      bomb = &b;

  assert(bomb != nullptr);

  const auto& bc = SI::bombClasses[bombClassID];
  const auto& ac = SI::alienClasses[lowestAlien->_classID];

  bomb->_classID = bombClassId;
  bomb->_position._x += alien->_position._x + (ac._width / 2);
  bomb->_position._y += alien->_position._y - bc._height;
  bomb->_frameClock = 0.f
  bomb->_frame = 0;
  bomb->_isAlive = true;

  ++_bombCount;
}

void GameState::spawnBunker(Vector2f position, Assets::Key_t bitmapKey)
{
  const Bitmap& bitmap = pxr::assets->getBitmap(bitmapKey, _worldScale);
  _bunkers.emplace_back(std::make_unique<Bunker>(bitmap, position));
}

void GameState::spawnUfo(SI::UfoClassID classID)
{
  _ufo._classID = classID;
  _ufo._isAlive = true;
  _ufo._direction = (randUniformSignedInt(0, 1) == 0) ? 1 : -1;
  _ufo._position._x = (_ufo._direction == 1) ? 0 : _worldSize._x;
  _ufo._position._y = SI::ufoSpawnY;
  _ufo._phaseClock = 0.f;
  _ufo._phase = true;
  _ufo._sfxChannel = mixer->playSound(SI::ufoClasses[classID]._moveSfx, 1000);
}

void GameState::spawnCuttleTwin(Alien& alien)
{
  // cannot spawn a twin to the right if there is no twin.
  assert(alien._col < SI::fleetWidth - 1);

  _si->addScore(_alienClasses[alien._classID]._scoreValue);

  alien._classID = CUTTLE_TWIN;
  _cuttleTwin = &alien;
  _cuttleTwinClock = 0.f;
  _isAliensFrozen = true;

  Alien& neighbour = getAlien(alien._row, alien._col + 1);
  if(neighbour._isAlive){
    neighbour._isAlive = false;
    --(_alienColPop[neighbour._col]);
    --(_alienRowPop[neighbour._row]);
    --_alienPop;
  }

  mixer->playSound(SpaceInvaders::SK_INVADER_MORPHED);
}

void GameState::spawnFleet()
{
  std::fill(_alienRowPop.begin(), _alienRowPop.end(), fleetWidth);
  std::fill(_alienColPop.begin(), _alienColPop.end(), fleetHeight);

  _cuttleTwin = nullptr;
  _alienPop = fleetSize;
  _alienMoveDirection = 1;
  _alienDropsDone = 0;
  _nextMover = 0;
  _isAliensDropping = true;
  _isAliensFrozen = false;
  _isAliensAboveInvasionRow = false;

  int row{0}, col{0};
  for(auto& alien : _fleet){
    alien._classID = _formations[_activeRoundNo][row][col];
    alien._position._x = _alienReferenceSpawnX + (col * _alienSeperation);
    alien._position._y = _alienReferenceSpawnY + (row * _alienSeperation);
    alien._row = row;
    alien._col = col;
    alien._frame = false;
    alien._isAlive = true;
    ++col;
    if(col >= fleetWidth){
      col = 0;
      ++row;
    }
  }
}

void GameState::killCannon(bool boom)
{
  setNextState(State::cannonSpawning);

  if(boom)
    spawnBoom(SI::BOOM_CANNON, _cannon._position, cc._colorIndex);

  //_cannon._isAlive = false;
  //_isAliensFrozen = true;
  //_beatBox.pause();
}

void GameState::killBomb(Bomb& bomb, bool boom)
{
  --_bombCount;
  assert(_bombCount >= 0);
  bomb._isAlive = false;

  if(boom){
    const auto& bbc = SI::bombClasses[bomb._classID];
    const auto& bmc = SI::boomClasses[SI::BOOM_BOMB];
    Vector2f boomPosition {
      bomb._position._x + ((bbc._width - bmc._width) / 2),
      bomb._position._y + ((bbc._height - bmc._height) / 2)
    };
    spawnBoom(SI::BOOM_BOMB, boomPosition, bc._colorIndex);
  }
}

void GameState::killBomb(Bomb& bomb, bool boom, Vector2f boomPosition)
{
  --_bombCount;
  assert(_bombCount >= 0);
  bomb._isAlive = false;

  if(boom)
    spawnBoom(SI::BOOM_BOMB, boomPosition, SI::bombClasses[bomb._classID]._colorIndex);
}


void GameState::killUfo(bool boom)
{
  _ufo._isAlive = false;
  int scoreTableIndex = _cannon._shotCounter % SI::ufoScoreTableSize;
  int score = SI::scoreTable[_ufo._classID][scoreTableIndex];
  _si->addScore(score);
  mixer->stopChannel(_ufo._sfxChannel);

  if(boom){
    int colorIndex = SI::_ufoClasses[_ufo._classID]._colorIndex;
    spawnBoom(SI::UFO_BOOM, _ufo._position, colorIndex);
    _hud->addLabel(std::make_unique<HUD::Label>{new HUD::TextLabel(
      _ufo._position,
      SI::palette[colorIndex],
      SI::boomClasses[UDO_BOOM]._duration,
      SI::boomClasses[UDO_BOOM]._duration * 2.f,
      std::to_string("score") + "!",
      false
    )});
  }

  checkVictory();
}

void GameState::killAlien(Alien& alien, bool boom)
{
  _si->addScore(SI::alienClasses[alien._classID]._scoreValue);
  alien._isAlive = false;
  --(_alienColPop[alien._col]);
  --(_alienRowPop[alien._row]);
  --_alienPop;

  if(_alienPop <= 0) 
    _si->setLastAlienClassAlive(alien._classID);

  if(alien._classID == SI::CUTTLE_TWIN) 
    _cuttleTwin = nullptr;

  if(boom) 
    spawnBoom(SI::BOOM_ALIEN, alien._position, SI::alienClasses[alien._classID]._colorIndex);

  checkVictory();
}

void GameState::killShot(bool boom)
{
  _shot._isAlive = false;

  if(boom){
    Vector2i position {};
    position._x = _shot._position._x - ((_bombBoomWidth - _laser._width) / 2);
    position._y = _shot._position._y;
    const auto& sc = SI::shotClasses[_shot._classID];
    spawnBoom(SI::BOOM_SHOT, position, sc._colorIndex);
  }
}

void GameState::boomBunker(Bunker& bunker, Vector2i pixelHit)
{
  Vector2i aPosition {};
  Vector2i bPosition {};

  const Bitmap& aBitmap {bunker._bitmap};
  const Bitmap& bBitmap {pxr::assets->getBitmap(SpaceInvaders::BMK_BOMBBOOMMIDAIR, _worldScale)};

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


void GameState::signalUfo()
{
  if(_ufo._isAlive) return;
  if(_isAliensSpawning || _isAliensDropping) return;
  if(_isGameOver || _isVictory || _isRoundIntro) return;
  if(_alienPop <= 8) return;

  --_ufoSpawnCountdown;
  if(_ufoSpawnCountdown > 0) return;

  if(pxr::randUniformSignedInt(0, SI::schrodingerSpawnChance) == 0)
   spawnUfo(SI::UfoClassID::SCHRODINGER);
  else
    spawnUfo(SI::UfoClassID::SAUCER);

  resetUfoSpawnCountdown();
}

void GameState::splitCuttleTwin(float dt)
{
  if(_cuttleTwin == nullptr) return;
  if(_alienPop == 0) return;

  _cuttleTwinClock += dt;
  if(_cuttleTwinClock < SI::cuttleTwinDuration) return;

  assert(_cuttleTwin->_col < (fleetWidth - 1));

  _cuttleTwin->_classID = CUTTLE;

  Alien& neighbour = getAlien(_cuttleTwin->_row, _cuttleTwin->_col + 1);
  neighbour._classID = CUTTLE;
  neighbour._isAlive = true;
  ++(_alienColPop[neighbour._col]);
  ++(_alienRowPop[neighbour._row]);
  ++_alienPop;

  _isAliensFrozen = false;
  _cuttleTwin = nullptr;
}

void GameState::moveCannon(float dt)
{
  if(!_cannon._isAlive) return;
  if(_isVictory) return;

  bool lKey = input->isKeyDown(SI::moveLeftKey);
  bool rKey = input->isKeyDown(SI::moveRightKey);

  if(lKey && !rKey){
    _cannon._moveDirection = -1;
  }
  else if(!lKey && rKey){
    _cannon._moveDirection = 1;
  }
  else{
    _cannon._moveDirection = 0;
    return;
  }

  const auto& cc = SI::cannonClasses[_cannon._classID];
  _cannon._position._x += cc._speed * _cannon._moveDirection * dt;
  _cannon._position._x = std::clamp(
      _cannon._position._x, 
      static_cast<float>(_worldLeftBorderX), 
      static_cast<float>(_worldRightBorderX - cc._width)
  );
}

// void GameState::doCannonBooming(float dt)
// {
//   if(!_cannon._isBooming) return;
// 
//   _cannon._boomClock += dt;
//   if(_cannon._boomClock >= SI::cannonBoomDuration){
//     _cannon._isBooming = false;
//     spawnCannon(true);
//     return;
//   }
// 
//   _cannon._boomFrameClock += dt;
//   if(_cannon._boomFrameClock >= SI::cannonBoomFrameDuration){
//     _cannon._boomFrame = pxr::wrap(_cannon._boomFrame + 1, 0, SI::CannonClass::boomFrameCount - 1);
//     _cannon._boomFrameClock = 0.f;
//   }
// }

void GameState::fireCannon()
{
  if(!_cannon._isAlive) return;
  if(_laser._isAlive) return;
  if(_isVictory) return;

  if(input->isKeyDown(SI::fireKey)){
    const auto& cc = SI::cannonClasses[_cannon._classID];
    _laser._position._x += _cannon._position._x + (cc._width / 2);
    _laser._position._y += _cannon._position._y + cc._height;
    _laser._isAlive = true;
    ++_cannon._shotCounter;
    mixer->playSound(SI::SK_SHOOT);
  }
}

void GameState::moveFleet()
{
  bool _aliveMoved {false};
  while(!_aliveMoved){
    Alien& alien = _fleet[_nextMover];
    _aliveMoved = alien._isAlive;
    if(_isAliensDropping){
      alien._position._y += _alienDropDisplacement;
    }
    else{
      int shiftDisplacement {_alienShiftDisplacement};
      if(_alienPop == 1 && _alienMoveDirection > 0)
        shiftDisplacement = _alienFastShiftDisplacement;
      alien._position._x += shiftDisplacement * _alienMoveDirection;
    }
    alien._frame = !alien._frame;
    ++_nextMover;
    if(_nextMover >= fleetSize){
      _nextMover = 0;
      if(_isAliensDropping){
        ++_alienDropsDone;
        _isAliensDropping = false;
        _alienMoveDirection *= -1;
        checkInvasion();
      }
      else if(collideFleetBorders()){
        _isAliensDropping = true;
      }
    }
  }
}

void GameState::moveSpawningFleet()
{
  assert(_alienPop == fleetSize);

  for(int i {0}; i < alienSpawnRate; ++i){
    Alien& alien = _fleet[_nextMover];
    alien._position._y += _alienDropDisplacement;
    alien._frame = !alien._frame;
    ++_nextMover;
    if(_nextMover >= fleetSize){
      _nextMover = 0;
      ++_alienDropsDone;
      mixer->playSound(SpaceInvaders::SK_FAST4);
      if(_alienDropsDone >= _rounds[_activeRoundNo]._spawnDrops)
        setNextState(State::playing);
    }
  }
}

void GameState::moveBombs(float dt)
{
  for(auto& bomb : _bombs){
    if(!bomb._isAlive) continue;
    const auto& bc = SI::bombClasses[bomb._classID];
    bomb._position._y += bc._speed * dt;
  }
}

void GameState::animateBombs(float dt)
{
  for(auto& bomb : _bombs){
    if(!bomb._isAlive) continue;
    bomb._frameClock += dt;
    if(bomb._frameClock < SI::bombFrameDuration) continue;
    bomb._frame = wrap(frame + 1, 0, SI::BombClass::bombFramesCount - 1);
    bomb._frameClock = 0.f;
  }
}

void GameState::moveShot(float dt)
{
  if(!_shot._isAlive) return;
  const auto& lc = SI::shotClasses[_shot._classID];
  _shot._position._y += lc._speed * dt;
}

void GameState::moveUfo(float dt)
{
  if(!_ufo._isAlive) return;
  const auto& uc = SI::ufoClasses[_ufo._classID];
  _ufo._position._x += uc._speed * _ufo._direction * dt;
}

void GameState::phaseUfo(float dt)
{
  if(!_ufo._isAlive) return;
  const auto& uc = SI::ufoClasses[_ufo._classID];
  if(!uc._isPhaser) return;
  _ufo._phaseClock += dt;
  if(_ufo._phaseClock < uc._phaseDuration) return;
  _ufo._phase = !_ufo._phase;
  _ufo._phaseClock = 0.f;
}

void GameState::dropBombs(float dt)
{
  if(_isAliensFrozen) return;
  if(_isAliensSpawning) return;
  if(_alienPopulation == 0) return;

  _bombClock += dt;
  if(_bombClock < SI::bombReloadTable[_bombReloadTableIndex]._reloadDuration) return;
  if(_bombCount >= SI::maxBombCount) return;

  auto bc = static_cast<SI::BombClassID>(
      randUniformSignedInt(SI::SQUIGGLY, _alienPop == 1 ? SI::ROLLING : SI::PLUNGER)
  );

  if(bc._canTarget)
    dropTargetBomb(bc);
  else
    dropRandomBomb(bc);

  _bombClock = 0.f;
}

void GameState::dropTargetBomb(SI::BombClassID bombClassID)
{
  assert(_bombCount != SI::maxBombCount); 

  int closestCol {std::numeric_limits<int>::max()};
  int minDistance {std::numeric_limits<int>::max()};

  float referenceAlienMiddleX = _fleet[0]._position._x + (SI::alienSeperation / 2);
  for(int col {0}; col < fleetWidth; ++col){
    if(_alienColPop[col] == 0) continue;
    float colMiddleX = referenceAlienMiddleX + (SI::alienSeperation * col); 
    float distanceToCannon = std::abs(_cannon._position._x - colMiddleX);
    if(distanceToCannon < minDistance){
      minDistance = distanceToCannon;
      closestCol = col;
    }
  }

  assert(closestCol != std::numeric_limits<int>::max());
  assert(minDistance != std::numeric_limits<int>::max());

  spawnBomb(closestCol, bombClassID);
}

void GameState::dropRandomBomb(SI::BombClassID bombClassID)
{
  assert(_bombCount != SI::maxBombCount); 

  std::array<int, SI::fleetWidth> populatedCols {};
  int numPopulated {0};
  for(col {0}; col < SI::fleetWidth; ++col)
    if(_alienColPop[col] > 0)
      populatedCols[numPopulated++] = col;

  int colChoice = randUniformSignedInt(0, numPopulated - 1);

  spawnBomb(colChoice, bombClassID);
}

//void GameState::doAlienBooming(float dt)
//{
//  if(!_isAliensBooming) return;
//
//  _alienBoomClock -= dt;
//  if(_alienBoomClock <= 0.f){
//    _alienBoomer = nullptr;
//    _isAliensFrozen = false;
//    _isAliensBooming = false;
//  }
//}

//void GameState::doUfoBoomScoring(float dt)
//{
//  if(!(_isUfoBooming || _isUfoScoring))
//    return;
//
//  _ufoBoomScoreClock += dt;
//  if(_ufoBoomScoreClock >= _ufoBoomScoreDuration){
//    if(_isUfoBooming){
//      const UfoClass& uc = _ufoClasses[_ufo._classID];
//      _uidUfoScoringText = _hud->addTextLabel({
//        Vector2i(_ufo._position._x, _ufo._position._y),
//        _colorPalette[uc._colorIndex],
//        std::to_string(_ufoLastScoreGiven)
//      });
//      _isUfoBooming = false;
//      _isUfoScoring = true;
//    }
//    else{
//      _hud->removeTextLabel(_uidUfoScoringText);
//      _isUfoScoring = false;
//    }
//    _ufoBoomScoreClock = 0.f;
//  }
//}

void GameState::ageBooms(float dt)
{
  for(auto& boom : _booms){
    if(!boom._isAlive) continue;
    const auto& bc = SI::boomClasses[boom._classID];
    boom._boomClock += dt;
    if(boom._boomClock >= bc._boomDuration){
      boom._isAlive = false;
      onBoomEnd(boom._classID);
      continue;
    }
    boom._boomFrameClock += dt;
    if(boom._boomFrameClock >= bc._boomFrameDuration){
      boom._boomFrame = wrap(boom._boomFrame + 1, 0, SI::BoomClass::boomFrameCount - 1);
      boom._boomFrameClock = 0.f;
    }
  }
}

void GameState::onBoomEnd(SI::BoomClassID classID)
{
  switch(classID){
    case SI::BOOM_CANNON:
      spawnCannon(true);
      break;
    default:
      break;
  }
}

void GameState::doUfoReinforcing(float dt)
{
  // TODO
}

void GameState::collideUfoBorders()
{
  bool collision = (_ufoDirection == -1 && _ufo._position._x < 0) || 
                   (_ufoDirection == 1  && _ufo._position._x > _worldSize._x);
  if(collision){
    _ufo._isAlive = false;
    mixer->stopChannel(_ufoSfxChannel);
  }
}

void GameState::doCollisionsBombsHitbar()
{
  for(auto& bomb : _bombs){
    if(!bomb._isAlive)
      continue;

    if(bomb._position._y > _hitbar->_positionY)
      continue;

    BombClass& bc = _bombClasses[bomb._classID];

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

void GameState::killAllBombs()
{
  for(auto& bomb : _bombs){
    if(!bomb._isAlive) continue;
    killBomb(bomb, false);
  }
}

void GameState::collideBombsCannon()
{
  const auto& cc = SI::cannonClasses[_cannon._classID];
  const Bitmap& cannonBitmap = assets->getBitmap(cc._bitmapKey);

  for(auto& bomb : _bombs){
    if(!bomb._isAlive) continue;
    BombClass& bc = SI::_bombClasses[bomb._classID];
    const Collision& c = testCollision(
      _cannon._position, 
      cannonBitmap, 
      bomb._position, 
      assets->getBitmap(bc._bitmapKeys[bomb._frame]),
      false
    );

    if(c._isCollision){
      killCannon(true);
      killBomb(bomb, false);
    }
  }
}

void GameState::collideShotBombs()
{
  const auto& sc = SI::shotClasses[_shot._classID];
  const auto& shotBitmap = :assets->getBitmap(sc._bitmapKey);
  for(auto& bomb : _bombs){
    if(!bomb._isAlive) continue;
    BombClass& bc = _bombClasses[bomb._classID];
    const Collision& c = testCollision(
      _shot._position, 
      shotBitmap, 
      bomb._position, 
      assets->getBitmap(bc._bitmapKeys[bomb._frame]), 
      false
    );
    if(c._isCollision){
      killShot(true);
      if(randUniformSignedInt(0, bc._laserSurvivalChance) != 0) 
        killBomb(bomb, false);
    }
  }
}

void GameState::collideShotFleet()
{
  const auto& sc = SI::shotClasses[_shot._classID];
  const auto& shotBitmap = assets->getBitmap(sc._bitmapKey);
  for(auto& alien : _fleet){
    if(!alien._isAlive) continue;
    const AlienClass& ac = SI::alienClasses[alien._classID];
    const Collision& c = testCollision(
        _shot._position, 
        shotBitmap, 
        alien._position, 
        assets->getBitmap(ac._bitmapKeys[alien._frame]),
        false
    );
    if(c._isCollision){
      if(canAlienBecomeCuttleTwin(alien))
        spawnCuttleTwin(alien);
      else
        killAlien(alien, true);
      killShot(false);
      break;
    }
  }
}

void GameState::collideShotUfo()
{
  const auto& uc = SI::ufoClasses[_ufo._classID];
  const auto& sc = SI::shotClasses[_shot._classID];
  const Collision& c = testCollision(
    _shot._position,
    assets->getBitmap(sc._bitmapKey), 
    _ufo._position, 
    assets->getBitmap(uc._bitmapKey), 
    false
  );
  if(c._isCollision){
    killShot(false);
    KillUfo(true);
  }
}

void GameState::collideShotSky()
{
  const auto& sc = SI::shotClasses[_shot._classID];
  if(_shot._position._y + sc._height > _worldTopBorderY)
    killShot(true);
}

bool GameState::collideFleetBorders()
{
  switch(_alienMoveDirection){
    case -1: 
      for(auto& alien : _fleet){
        if(!alien._isAlive) continue;
        if(alien._position._x <= _worldLeftBorderX) return true;
      }
      break;
    case 1:
      for(auto& alien : _fleet){
        if(!alien._isAlive) continue;
        if(alien._position._x + _alienSeperation >= _worldRightBorderX) return true;
      }
      break;
    default:
      assert(0);
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

    const BombClass& bc = _bombClasses[bomb._classID];
    aBitmap = &(pxr::assets->getBitmap(bc._bitmapKeys[bomb._frame], _worldScale));

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

  aBitmap = &(pxr::assets->getBitmap(_laser._bitmapKey, _worldScale));

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
  if(_isAliensSpawning) return;
  if(_isAliensFrozen) return;
  if(_bunkers.size() == 0) return;
  if(_alienPop == 0) return;

  int lowestRow {-1};
  for(int row {0}; row < fleetWidth; ++row){
    if(_alienRowPop[row] > 0){
      lowestRow = row;
      break;
    }
  }

  assert(lowestRow != -1);  // can only happen if we have incorrectly tallied row pops.

  if(getAlien(lowestRow, 0)._position._y > _bunkerSpawnY + _bunkerHeight) return;

  Vector2i aPosition {};
  Vector2i bPosition {};

  const Bitmap* aBitmap {nullptr};
  const Bitmap* bBitmap {nullptr};

  auto rowRange = getAlienRowRange(lowestRow);
  for(int alienNo {rowRange.first}; alienNo < rowRange.second; ++alienNo){
    Alien& alien = _fleet[alienNo];

    if(!alien._isAlive) continue;

    aPosition._x = alien._position._x;
    aPosition._y = alien._position._y;

    const AlienClass& ac = _alienClasses[alien._classID];
    Assets::Key_t bitmapKey = ac._bitmapKeys[alien._frame];
    aBitmap = &(pxr::assets->getBitmap(bitmapKey, _worldScale));

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

bool GameState::incrementFleetIndex(FleetIndex& index)
{
  // Increments index from left-to-right along the columns, moving up a row and back to the left
  // most column upon reaching the end of the current column. Loops back to the bottom left most
  // column of the bottom row upon reaching the top-right of the fleet. Returns true to indicate
  // a loop.
  
  ++index._col;
  if(index._col >= fleetWidth){
    index._col = 0;
    ++index._row;
    if(index._row >= fleetHeight){
      index._row = 0;
      return true;
    }
  }
  return false;
}

void GameState::addHudMsg(const char* msg, const Color3f& color)
{
  int32_t msgWidth = _font->calculateStringWidth(msg);
  Vector2i msgPosition {
    (_worldSize._x - msgWidth) / 2,
    msgHeight_px * _worldScale
  };

  _uidMsgText = _hud->addTextLabel({
    msgPosition, 
    color,
    msg,
    0.5f,
    true
  });
}

void GameState::removeHudMsg()
{
  _hud->removeTextLabel(_uidMsgText);
}

void GameState::checkInvasion()
{
  int minY {std::numeric_limits<int>::max()};
  for(auto& alien : _fleet)
    if(alien._isAlive)
      minY = std::min(minY, alien._position._y);

  if(minY == _alienInvasionRowHeight)
    switchState(State::GameOver);
  else if(minY == _alienInvasionRowHeight + std::abs(_alienDropDisplacement)){
    _isAliensAboveInvasionRow = true;
  }
}

void GameState::checkVictory()
{
  if(_alienPop <= 0 && !_ufo._isAlive)
    setNextState(State::victory);
}

void GameState::startGameOver()
{
  _cannon._isAlive = false;
  _isAliensFrozen = true;
  _beatBox.pause();
  
  if(_ufo._isAlive) 
    mixer->stopChannel(_ufoSfxChannel);

  static_cast<SpaceInvaders*>(_app)->startScoreHudFlash();

  addHudMsg(msgGameOver, colors::red);

  _msgClockSeconds = 0.f;
  _isGameOver = true;
}

void GameState::doGameOver(float dt)
{
  if(!_isGameOver)
    return;

  assert(!_isVictory);
  assert(!_isRoundIntro);

  _msgClockSeconds += dt;
  if(_msgClockSeconds >= msgPeriodSeconds){
    SpaceInvaders* si = static_cast<SpaceInvaders*>(_app);
    si->stopScoreHudFlash();
    removeHudMsg();
    if(si->isHiScore(si->getScore()))
      _app->switchState(HiScoreRegState::name);
    else 
      _app->switchState(HiScoreBoardState::name);
  }
}

//void GameState::doVictoryTest()
//{
//  if(!_isVictory && _alienPop == 0){
//    _beatBox.pause();
//    if(_ufo._isAlive)
//      return;
//    else
//      startVictory();
//  }
//}

//void GameState::startVictory()
//{
//  _beatBox.pause();
//  if(_ufo._isAlive) 
//    mixer->stopChannel(_ufoSfxChannel);
//
//  addHudMsg(msgVictory, colors::green);
//  static_cast<SpaceInvaders*>(_app)->startScoreHudFlash();
//
//  boomAllBombs();
//
//  _msgClockSeconds = 0.f;
//  _isVictory = true;
//}

//void GameState::doVictory(float dt)
//{
//  if(!_isVictory)
//    return;
//
//  assert(!_isGameOver);
//  assert(!_isRoundIntro);
//
//  _msgClockSeconds += dt;
//  if(_msgClockSeconds > msgPeriodSeconds){
//    _si->stopScoreHudFlash();
//    _si->addRound(1);
//    removeHudMsg();
//    _si->switchState(SosState::name);
//  }
//}

void GameState::onEnter()
{
  // rounds start at 0, the 10th round is the same as round 0, so round 10 % (9 + 1) = 0. 
  //                                                             roundCount---^
  _activeRoundNo = _si->getRound() % (roundCount + 1);

  _beatBox.pause();

  _bombCount = 0;
  _bombReloadTableIndex = 0;
  _bombClock = 0.f;
  for(auto& bomb : _bombs) 
    bomb._isAlive = false;

  for(auto& boom : _booms)
    boom._isAlive = false;

  _shot._isAlive = false;

  _cannon._classID = SI::CannonClassID::LASER_BASE;
  _cannon._shotCounter = 0;

  _ufo._isAlive = false;
  resetUfoSpawnCountdown();

  _si->showHud();

  _state = State::none;
  setNextState(State::roundIntro);
  switchState();


//-----------------------------------

  // _isGameOver = false;
  // _isVictory = false;
  // startRoundIntro();


  //_bombClock = _bombIntervals[_activeCycle];

  // Create fresh (undamaged) hitbar.
  _hitbar = std::make_unique<Hitbar>(
      pxr::assets->getBitmap(SpaceInvaders::BMK_HITBAR, _worldScale), 
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


  //_hasFleetLooped = false;

}

void GameState::onEnterRoundIntro()
{
  int round = _si->getRound();
  std::string msg {};
  msg += msgRoundIntro;
  msg += " ";
  msg += std::to_string(round);
  addHudMsg(msg.c_str(), colors::red);
  _msgClockSeconds = 0.f;
  _beatBox.pause();
}

void GameState::onUpdateRoundIntro(float dt)
{
  _msgClockSeconds += dt;
  if(_msgClockSeconds >= msgPeriodSeconds)
    setNextState(State::aliensSpawning);
}

void GameState::onExitRoundIntro()
{
  removeHudMsg();
}

void GameState::onEnterAlienSpawning()
{
  spawnFleet();
  _alienDropsDone = 0;

  _si->hideTopHud();
  _si->showLivesHud();
}

void GameState::onUpdateAlienSpawning(float dt)
{
  moveSpawningFleet();
}

void GameState::onExitAlienSpawning()
{
  _si->showTopHud();
  _isAliensDropping = false;
}

void GameState::onEnterPlaying()
{
  spawnCannon(false);
  _beatBox.resume();
}

void GameState::onUpdatePlaying(float dt)
{
  checkExit();

  moveCannon(dt);
  fireCannon();

  if(!_isAliensFrozen){
    moveFleet();
    dropBombs(dt);
  }

  moveBombs(dt);
  moveUfo(dt);
  phaseUfo(dt);
  splitCuttleTwin(dt);
  ageBooms(dt);
  animateBombs(dt);

  if(_bombCount > 0 && !_isAliensAboveInvasionRow)
    collideBombsCannon();

  if(_shot._isAlive)
    collideShotSky();

  if(_shot._isAlive && _bombCount > 0)
    collideShotBombs();

  if(_shot._isAlive)
    collideShotFleet();

  if(_shot._isAlive && _ufo._isAlive && _ufo._isPhase)
    collideShotUfo();

  if(_ufo._isAlive)
    collideUfoBorders();
}

void GameState::onExitPlaying()
{
  _beatBox.pause();
}

void GameState::onEnterCannonSpawning()
{
  _cannon._spawnClock = 0.f;
}

void GameState::onUpdateCannonSpawning(float dt)
{
  _cannon._spawnClock += dt;
  if(_cannon._spawnClock > SI::cannonSpawnDuration)
    setNextState(State::playing);

  moveBombs(dt);
  moveUfo(dt);
  phaseUfo(dt);
  splitCuttleTwin(dt);
  ageBooms(dt);
  animateBombs(dt);
}

void GameState::onExitCannonSpawning()
{
}

void GameState::onEnterVictory()
{
  addHudMsg(msgVictory, colors::green);
  _si->startScoreHudFlash();
  boomAllBombs();
  _msgClockSeconds = 0.f;
}

void GameState::onUpdateVictory(float dt)
{
  _msgClockSeconds += dt;
  if(_msgClockSeconds >= msgPeriodSeconds)
    setNextState(State::exit);
}

void GameState::onExitVictory()
{
  _si->stopScoreHudFlash();
  _si->addRound(1);
  removeHudMsg();
  _si->switchState(SosState::name);
}

void GameState::onEnterGameOver()
{
}

void GameState::onUpdateGameOver(float dt)
{
}

void GameState::onExitGameOver()
{
}

void GameState::onEnterExit()
{
}

void GameState::setNextState(State state)
{
  assert(state != State::none);
  _newState = state;
}

void GameState::isStateChange()
{
  return _state != _newState;
}

void GameState::switchState()
{
  switch(_state){
    case State::roundIntro:
      onExitRoundIntro();
      break;
    case State::aliensSpawning:
      onExitAliensSpawning();
      break;
    case State::playing:
      onExitPlaying();
      break;
    case State::cannonSpawning:
      onExitCannonSpawning();
      break;
    case State::victory:
      onExitVictory();
      break;
    case State::gameOver:
      onExitGameOver();
      break;
    case State::exit:
      break;
    case State::none:
      break;
  }

  switch(_newState){
    case State::roundIntro:
      onEnterRoundIntro();
      break;
    case State::aliensSpawning:
      onEnterAliensSpawning();
      break;
    case State::playing:
      onEnterPlaying();
      break;
    case State::cannonSpawning:
      onEnterCannonSpawning();
      break;
    case State::victory:
      onEnterVictory();
      break;
    case State::gameOver:
      onEnterGameOver();
      break;
    case State::exit:
      onEnterExitState();
      break;
    case State::none:
      assert(0);
  }

  _state = _newState;
}

void GameState::onUpdate(double now, float dt)
{
  switch(_state){
    case State::roundIntro:
      onUpdateRoundIntro(dt);
      break;
    case State::aliensSpawning:
      onUpdateAliensSpawning(dt);
      break;
    case State::playing:
      onUpdatePlaying();
      break;
    case State::cannonSpawning:
      onUpdateCannonSpawning(dt);
      break;
    case State::victory:
      onUpdateVictory(dt);
      break;
    case State::gameOver:
      onUpdateGameOver(dt);
      break;
  };

  if(isStateChange())
    switchState();



  //================================================================================





  doAbortToMenuTest();
  doRoundIntro(dt);
  doLaserMoving(dt);
  doUfoSpawning();
  //doCannonBooming(dt);
  //doAlienBooming(dt);
  doUfoBoomScoring(dt);

  doCollisionsUfoBorders();
  doCollisionsBombsHitbar();
  doCollisionsBombsCannon();
  doCollisionsBombsLaser();
  doCollisionsLaserAliens();
  doCollisionsBunkersBombs();
  doCollisionsBunkersLaser();
  doCollisionsBunkersAliens();
  doCollisionsLaserUfo();
  doCollisionsLaserSky();


  //================================================================================
  
  if(pxr::input->isKeyPressed(Input::KEY_b))
    boomCannon();

  if(pxr::input->isKeyPressed(Input::KEY_a)){
    for(auto& alien : _fleet){
      if(!alien._isAlive) continue;
      boomAlien(alien);
      break;
    }
  }

  //================================================================================

  doVictoryTest();
  doVictory(dt);
  doInvasionTest();
  doGameOver(dt);
  //updateActiveCycleBeat();
  //doFleetBeats();
  _beatBox.doBeats(dt);
}

void GameState::drawFleet()
{
  if(_isRoundIntro) return;

  for(const auto& alien : _fleet){
    if(!alien._isAlive) continue;
    const AlienClass& ac = _alienClasses[alien._classID];
    Vector2f position(alien._position._x, alien._position._y);
    Assets::Key_t bitmapKey = ac._bitmapKeys[alien._frame];
    Color3f& color = _colorPalette[ac._colorIndex];
    renderer->blitBitmap(position, pxr::assets->getBitmap(bitmapKey, _worldScale), color);
  }

  if(_isAliensBooming){
    assert(_alienBoomer != nullptr);
    const AlienClass& ac = _alienClasses[_alienBoomer->_classID];
    Vector2f position(_alienBoomer->_position._x, _alienBoomer->_position._y);
    Assets::Key_t bitmapKey = SpaceInvaders::BMK_ALIENBOOM;
    Color3f& color = _colorPalette[ac._colorIndex];
    renderer->blitBitmap(position, pxr::assets->getBitmap(bitmapKey, _worldScale), color);
  }
}

void GameState::drawUfo()
{
  if(!((_ufo._isAlive && _ufo._phase) || _isUfoBooming || _isUfoScoring))
    return;

  const UfoClass& uc = _ufoClasses[_ufo._classID];

  Assets::Key_t bitmapKey;
  if(_ufo._isAlive)
    bitmapKey = uc._shipKey;
  else if(_isUfoBooming)
    bitmapKey = uc._boomKey;
  else if(_isUfoScoring){
    return; // drawn as a hud element instead.
  }

  renderer->blitBitmap(
      _ufo._position, 
      pxr::assets->getBitmap(bitmapKey, _worldScale), 
      _colorPalette[uc._colorIndex]
  );
}

void GameState::drawCannon()
{
  if(!(_cannon._isBooming || _cannon._isAlive)) return;
  const auto& cc = SI::cannonClasses[_cannon._classID];
  renderer->blitBitmap(
    _cannon._position, 
    assets->getBitmap(_cannon._isBooming ? cc._boomKeys[_cannon._boomFrame] : cc._bitmapKey),
    SI::palette[cc._colorIndex]
  );
}

void GameState::drawBombs()
{
  for(auto& bomb : _bombs){
    if(!bomb._isAlive) continue;
    const auto& bc = SI::bombClasses[bomb._classID];
    renderer->blitBitmap(
        bomb._position, 
        pxr::assets->getBitmap(bc._bitmapKeys[bomb._frame]), 
        SI::palette[bomb._colorIndex]
    );
  }
}

void GameState::drawBooms()
{
  for(auto& boom : _bombBooms){
    if(!boom._isAlive) continue;
    const auto& bc = SI::boomClasses[boom._classID];
    renderer->blitBitmap(
      boom._position, 
      assets->getBitmap(bc._bitmapKey), 
      SI::palette[boom._colorIndex]
    );
  }
}

void GameState::drawLaser()
{
  if(!_laser._isAlive)
    return;

  renderer->blitBitmap(_laser._position, pxr::assets->getBitmap(_laser._bitmapKey, _worldScale), _colorPalette[_laser._colorIndex]);
}

void GameState::drawHitbar()
{
  renderer->blitBitmap({0.f, _hitbar->_positionY}, _hitbar->_bitmap, _colorPalette[_hitbar->_colorIndex]);
}

void GameState::drawBunkers()
{
  for(const auto& bunker : _bunkers)
    renderer->blitBitmap(bunker->_position, bunker->_bitmap, _colorPalette[_bunkerColorIndex]);
}

void GameState::onDraw(double now, float dt)
{
  renderer->clearViewport(colors::black);
  drawFleet();
  drawUfo();
  drawCannon();
  drawBombs();
  drawBombBooms();
  drawLaser();
  drawBunkers();
  drawHitbar();
}

//===============================================================================================//
// ##>SOS STATE                                                                                  //
//===============================================================================================//

void SosState::initialize(Vector2i worldSize, int32_t worldScale)
{
  SpaceInvaders* si = static_cast<SpaceInvaders*>(_app);
  _hud = &si->getHud();
  _font = &pxr::assets->getFont(SpaceInvaders::fontKey, worldScale);

  _worldSize = worldSize;
  _worldScale = worldScale;
  _exitHeight_px = worldSize._y - (baseTopMargin_px * worldScale);
  _worldLeftMargin_px = baseWorldMargin_px * worldScale;
  _worldRightMargin_px = worldSize._x - _worldLeftMargin_px;
  _spawnHeight_px = baseSpawnHeight_px * worldScale;
  _moveSpeed = baseMoveSpeed * worldScale;
  _sosTextPositionX = _worldSize._x - _font->calculateStringWidth(sosText) - (sosTextMargin_px * _worldScale);
}

void SosState::onUpdate(double now, float dt)
{
  doEngineCheck();
  doAlienAnimating(dt);
  doMoving(dt);
  doEngineFailing(dt);
  doWallColliding();
  doEndTest();
}

void SosState::onDraw(double now, float dt)
{
  renderer->clearViewport(colors::black);

  const GameState::AlienClass& ac = _gameState->_alienClasses[_alien._classID];
  Assets::Key_t bitmapKey;

  if(_hasEngineFailed){
    const Bitmap& bitmap = assets->getBitmap(SpaceInvaders::BMK_SOS_TRAIL, _worldScale);
    Vector2f trailPosition = _alien._failPosition;
    while(trailPosition._y < _alien._position._y){
      renderer->blitBitmap(
          trailPosition,
          bitmap,
          _gameState->_colorPalette[ac._colorIndex]
      );
      trailPosition._y += (sosTrailSpace_px * _worldScale);
    }
  }

  bitmapKey = ac._bitmapKeys[_alien._frame];
  renderer->blitBitmap(
      _alien._position, 
      assets->getBitmap(bitmapKey, _worldScale), 
      _gameState->_colorPalette[ac._colorIndex]
  );

  const GameState::UfoClass& uc = _gameState->_ufoClasses[_ufo._classID];
  bitmapKey = uc._shipKey;
  renderer->blitBitmap(
      _ufo._position, 
      assets->getBitmap(bitmapKey, _worldScale), 
      _gameState->_colorPalette[uc._colorIndex]
  );

}

void SosState::onEnter()
{
  _ufo._classID = GameState::UfoClassId::SAUCER;
  _ufo._position._y = _spawnHeight_px;
  _ufo._position._x = pxr::randUniformSignedInt(_worldLeftMargin_px, _worldRightMargin_px);

  _alien._classID = _gameState->_lastClassAlive; 
  _alien._frame = false;
  _alien._frameClockSeconds = 0.f;

  _ufo._width = _gameState->_ufoClasses[_ufo._classID]._width;
  int32_t ufoHeight = _gameState->_ufoClasses[_ufo._classID]._height;
  int32_t alienWidth = _gameState->_alienClasses[_alien._classID]._width;

  _alien._position._x = _ufo._position._x + ((_ufo._width - alienWidth) / 2);
  _alien._position._y = _spawnHeight_px + ufoHeight;

  _moveVelocity = {
    _moveSpeed * std::sin(moveAngleRadians), 
    _moveSpeed * std::cos(moveAngleRadians)
  };

  _isEngineFailing = false;
  _hasEngineFailed = false;

  _engineFailClockSeconds = 0.f;

  _woowooChannel = mixer->playSound(SpaceInvaders::SK_SOS, 100);
  _isWooing = true;

  _nextSosText = 0;

  static_cast<SpaceInvaders*>(_app)->showHud();
}

void SosState::doMoving(float dt)
{
  if(_isEngineFailing)
    return;

  if(_hasEngineFailed){
    _alien._position._y += 0.6f * _moveSpeed * dt;
  }
  else{
    _alien._position += _moveVelocity * dt;
    _ufo._position += _moveVelocity * dt;
  }
}

void SosState::doAlienAnimating(float dt)
{
  if(!_hasEngineFailed)
    return;

  _alien._frameClockSeconds += dt;
  if(_alien._frameClockSeconds >= Alien::framePeriodSeconds){
    _alien._frame = !_alien._frame;
    _alien._frameClockSeconds = 0.f;
  }
}

void SosState::doEngineFailing(float dt)
{
  if(!_isEngineFailing)
    return;

  _engineFailClockSeconds += dt;
  if(_engineFailClockSeconds >= engineFailPeriodSeconds){
    _alien._failPosition = _alien._position;
    _alien._frameClockSeconds = 0.f;
    _isEngineFailing = false;
    _hasEngineFailed = true;
  }
}

void SosState::doEngineCheck()
{
  if(_hasEngineFailed)
    return;

  if(!_isEngineFailing){
    if(pxr::randUniformSignedInt(0, engineFailChance) == engineFailHit){
      _isEngineFailing = true;
      _engineFailClockSeconds = 0.f;

      int32_t troubleWidth = _font->calculateStringWidth(troubleText);
      Vector2i troublePosition = {
        _ufo._position._x - (std::abs(_ufo._width - troubleWidth) / 2),
        _ufo._position._y - _font->getLineSpace()
      };
      _uidTroubleText = _hud->addTextLabel({
        troublePosition, 
        pxr::colors::magenta, 
        troubleText,
        1.0f,
        true
      });

      mixer->stopChannel(_woowooChannel);
      _isWooing = false;
    }
  }
}

void SosState::doWallColliding()
{
  if(_isEngineFailing || _hasEngineFailed)
    return;

  if(((_ufo._position._x < _worldLeftMargin_px) && _moveVelocity._x < 0) ||
     (((_ufo._position._x + _ufo._width) > _worldRightMargin_px) && _moveVelocity._x > 0))
  {
    _moveVelocity._x *= -1.f;

    // condition should never fail as array is calibrated to be big enough in all cases.
    if(_nextSosText < maxSosTextDrop){
      Vector2i sosPosition = {
        _sosTextPositionX,
        _ufo._position._y + (_font->getSize() / 2)
      };
      _uidSosText[_nextSosText++] = _hud->addTextLabel({
        sosPosition, 
        pxr::colors::magenta, 
        sosText
      });
    }
  }
}

void SosState::doEndTest()
{
  if(_alien._position._y > _exitHeight_px){
    if(_isWooing)
      mixer->stopChannel(_woowooChannel);
    if(_hasEngineFailed)
      _hud->removeTextLabel(_uidTroubleText);
    for(int32_t i = _nextSosText; i > 0; --i)
      _hud->removeTextLabel(_uidSosText[i - 1]);
    _app->switchState(GameState::name);
  }
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

//===============================================================================================//
// ##>HIGH SCORE REGISTRATION STATE                                                              //
//===============================================================================================//

HiScoreRegState::Keypad::Keypad(const Font& font, Vector2i worldSize, int32_t worldScale) :
  _keyText{},
  _keyScreenPosition{},
  _keyColor{colors::cyan},
  _specialKeyColor{colors::magenta},
  _cursorColor{colors::green},
  _cursorPadPosition{initialCursorPadPosition},
  _cursorScreenPosition{0, 0},
  _padScreenPosition{0, 0},
  _font{font}
{
  _keyText = {{
    {"\\", "/", "(", ")", "+", "^", "RUB", "", "", "END", ""},  // row[0] == bottom row
    {"W", "X", "Y", "Z", ".", "_", "-", "[", "]", "<", ">"},
    {"L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V"},
    {"A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K"}     // row[3] == top row
  }};

  int32_t fontSize = font.getSize();

  int32_t padWidth = keyColCount * (fontSize + (keySpace_px * worldScale));
  int32_t padHeight = keyRowCount * (fontSize + (keySpace_px * worldScale));
  _padScreenPosition._x = (worldSize._x - padWidth) / 2;
  _padScreenPosition._y = (worldSize._y - padHeight) / 2;

  for(size_t row{0}; row < keyRowCount; ++row){
    for(size_t col{0}; col < keyColCount; ++col){
      _keyScreenPosition[row][col] = {
        _padScreenPosition._x + (col * (fontSize + (keySpace_px * worldScale))),
        _padScreenPosition._y + (row * (fontSize + (keySpace_px * worldScale))),
      };
    }
  }

  updateCursorScreenPosition();
}

void HiScoreRegState::Keypad::moveCursor(int32_t colShift, int32_t rowShift)
{
  do{
    if(colShift) _cursorPadPosition._x = pxr::wrap(_cursorPadPosition._x + colShift, 0, keyColCount - 1);
    if(rowShift) _cursorPadPosition._y = pxr::wrap(_cursorPadPosition._y + rowShift, 0, keyRowCount - 1);
  }
  while(_keyText[_cursorPadPosition._y][_cursorPadPosition._x][0] == '\0');
  updateCursorScreenPosition();
}

void HiScoreRegState::Keypad::updateCursorScreenPosition()
{
  _cursorScreenPosition = _keyScreenPosition[_cursorPadPosition._y][_cursorPadPosition._x];
  _cursorScreenPosition._y -= cursorDrop_px;
  const char* keyText = _keyText[_cursorPadPosition._y][_cursorPadPosition._x];
  if(std::strncmp(keyText, "RUB", 3) == 0 || std::strncmp(keyText, "END", 3) == 0)
    _cursorScreenPosition._x += _font.getGlyph(keyText[0])._advance + _font.getGlyphSpace();
}

void HiScoreRegState::Keypad::reset()
{
  _cursorPadPosition = initialCursorPadPosition;
}

void HiScoreRegState::Keypad::draw()
{
  for(int row{0}; row < keyRowCount; ++row){
    for(int col{0}; col < keyColCount; ++col){
      const char* text = _keyText[row][col];
      if(text[0] == '\0') continue;
      Color3f color = _keyColor;
      if(strncmp(text, "RUB", 3) == 0 || strncmp(text, "END", 3) == 0)
        color = _specialKeyColor;
      renderer->blitText(_keyScreenPosition[row][col], text, _font, color);
    }
  }

  renderer->blitText(_cursorScreenPosition, cursorChar, _font, _cursorColor);
}

HiScoreRegState::NameBox::NameBox(const Font& font, Vector2i worldSize, int32_t worldScale) :
  _nameBuffer{},
  _font{font},
  _final{}
{
  for(auto& c : _nameBuffer)
    c = nullChar;

  composeFinal();

  int32_t finalWidth_px = font.calculateStringWidth(_final);
  _boxScreenPosition = {
    (worldSize._x - finalWidth_px) / 2,
    worldSize._y / 4
  };
}

void HiScoreRegState::NameBox::draw()
{
  renderer->blitText(_boxScreenPosition, _final, _font, colors::red);
}

bool HiScoreRegState::NameBox::pushBack(char c)
{
  if(isFull()) 
    return false;
  for(int i = _nameBuffer.size() - 1; i >= 0; --i){
    if(i == 0) 
      _nameBuffer[i] = c;
    else if(_nameBuffer[i] == nullChar && _nameBuffer[i - 1] != nullChar){ 
      _nameBuffer[i] = c; 
      break;
    }
  }
  composeFinal();
  return true;
}

bool HiScoreRegState::NameBox::popBack()
{
  if(isEmpty()) 
    return false;
  if(isFull()){
    _nameBuffer[_nameBuffer.size() - 1] = nullChar;
    composeFinal();
    return true;
  }
  for(int i = _nameBuffer.size() - 1; i >= 0; --i){
    if(i == 0) 
      _nameBuffer[i] = nullChar;
    else if(_nameBuffer[i] == nullChar && _nameBuffer[i - 1] != nullChar){
      _nameBuffer[i-1] = nullChar; 
      break;
    }
  }
  composeFinal();
  return true;
}

bool HiScoreRegState::NameBox::isFull() const
{
  return _nameBuffer[SpaceInvaders::hiscoreNameLen - 1] != nullChar;
}

bool HiScoreRegState::NameBox::isEmpty() const
{
  return _nameBuffer[0] == nullChar;
}

void HiScoreRegState::NameBox::composeFinal()
{
  _final.clear();
  _final += label;
  _final += ' ';
  _final += quoteChar;
  for(auto& c : _nameBuffer)
    _final += c;
  _final += quoteChar;
}

void HiScoreRegState::initialize(Vector2i worldSize, int32_t worldScale)
{
  _keypad = std::make_unique<Keypad>(assets->getFont(SpaceInvaders::fontKey, worldScale), worldSize, worldScale);
  _nameBox = std::make_unique<NameBox>(assets->getFont(SpaceInvaders::fontKey, worldScale), worldSize, worldScale);
}

void HiScoreRegState::onUpdate(double now, float dt)
{
  doInput();
}

void HiScoreRegState::onDraw(double now, float dt)
{
  renderer->clearViewport(colors::black);
  _keypad->draw();
  _nameBox->draw();
}

void HiScoreRegState::onEnter()
{
  if(_keypad != nullptr) _keypad->reset();
  static_cast<SpaceInvaders*>(_app)->showHud();
}

void HiScoreRegState::doInput()
{
  bool lKey = pxr::input->isKeyPressed(Input::KEY_LEFT);
  bool rKey = pxr::input->isKeyPressed(Input::KEY_RIGHT);
  bool uKey = pxr::input->isKeyPressed(Input::KEY_UP);
  bool dKey = pxr::input->isKeyPressed(Input::KEY_DOWN);

  int colShift {0}, rowShift {0};
  if(lKey) colShift += -1;
  if(rKey) colShift += 1;
  if(uKey) 
    rowShift += 1;
  if(dKey) 
    rowShift += -1;

  _keypad->moveCursor(colShift, rowShift);  

  bool eKey = pxr::input->isKeyPressed(Input::KEY_SPACE);
  if(eKey){
    const char* c = _keypad->getActiveKeyText();
    if(strncmp(c, "RUB", 3) == 0){
      if(!_nameBox->popBack())
        mixer->playSound(SpaceInvaders::SK_FAST1); 
    }
    else if(strncmp(c, "END", 3) == 0){
      if(!_nameBox->isFull()){
        mixer->playSound(SpaceInvaders::SK_FAST1); 
      }
      else{
        SpaceInvaders* si = static_cast<SpaceInvaders*>(_app);
        si->setPlayerName(_nameBox->getBufferText());
        si->hideHud();
        _app->switchState(HiScoreBoardState::name);
      }
    }
    else{
      if(!_nameBox->pushBack(c[0]))
        mixer->playSound(SpaceInvaders::SK_FAST1); 
    }
  }
}

//===============================================================================================//
// ##>HIGH SCORE BOARD STATE                                                                     //
//===============================================================================================//

void HiScoreBoardState::initialize(Vector2i worldSize, int32_t worldScale)
{
  _font = &(assets->getFont(SpaceInvaders::fontKey, worldScale));

  // world scale of glyphs is taken into account by the fontSize returned from font.
  int32_t glyphSize_px = _font->getSize() + _font->getGlyphSpace();
  int32_t nameWidth_px = glyphSize_px * SpaceInvaders::hiscoreNameLen;
  int32_t scaledColSeperation = colSeperation * worldScale;
  _scoreBoardSize._x = nameWidth_px + scaledColSeperation + (scoreDigitCountEstimate * glyphSize_px);
  _scoreBoardSize._y = (SpaceInvaders::hiscoreCount + 1) * (glyphSize_px + (rowSeperation * worldScale));

  _nameScreenPosition = {
    (worldSize._x - _scoreBoardSize._x) / 2,
    (worldSize._y - _scoreBoardSize._y) / 3
  };

  _scoreScreenPosition = _nameScreenPosition;
  _scoreScreenPosition._x += nameWidth_px + scaledColSeperation;
}

void HiScoreBoardState::onEnter()
{
  SpaceInvaders* si = static_cast<SpaceInvaders*>(_app);
  const auto& hiscores = si->getHiScores();

  _newScore._value = si->getScore();
  _newScore._name = si->getPlayerName();

  if(_newScore._name[0] == '\0')
    _newScore._name = placeHolderName;

  _scoreBoard[0] = &_newScore;

  for(int i{0}; i < SpaceInvaders::hiscoreCount; ++i)
    _scoreBoard[i + 1] = &hiscores[i];

  _eventNum = 0;
  _eventClock = 0.f;

  populateHud();
  si->showHud();
}

void HiScoreBoardState::onUpdate(double now, float dt)
{
  _eventClock += dt;
  if(_eventNum == 0){
    if(_eventClock > enterDelaySeconds){
      _eventClock = 0.f;
      ++_eventNum;
    }
  }
  else if(_eventNum > _scoreBoard.size()){ 
    if(_eventClock > _exitDelaySeconds){
      depopulateHud();
      SpaceInvaders* si = static_cast<SpaceInvaders*>(_app);
      si->hideHud();
      if(si->isHiScore(_newScore._value) && !si->isDuplicateHiScore(_newScore)){
        si->registerHiScore(_newScore);
        si->writeHiScores();
      }
      _app->switchState(MenuState::name);
    }
  }
  else {
    if(_eventClock > swapScoreDelaySeconds){
      _eventClock = 0.f;
      _eventNum += doScoreSwap() ? _scoreBoard.size() : 1;  // if done all swaps skip to end.
      if(_eventNum > _scoreBoard.size()){ 
        if(newScoreIsTop()){
          mixer->playSound(SpaceInvaders::SK_TOPSCORE); 
          _exitDelaySeconds = topScoreExitDelaySeconds;
        }
        else
          _exitDelaySeconds = normalExitDelaySeconds;
      }
    }
  }
}

void HiScoreBoardState::onDraw(double now, float dt)
{
  renderer->clearViewport(colors::black);
  Vector2i namePosition {_nameScreenPosition};
  Vector2i scorePosition {_scoreScreenPosition};
  std::string nameStr {};
  const Color3f* color;
  for(auto& score : _scoreBoard){
    color = (score == &_newScore) ? &newScoreColor : &oldScoreColor;

    // we do this because the name in SpaceInvaders::Score is not null terminated.
    nameStr.clear();
    for(int i{0}; i < SpaceInvaders::hiscoreNameLen; ++i)
      nameStr += score->_name[i];
  
    renderer->blitText(namePosition, nameStr, *_font, *color);
    renderer->blitText(scorePosition, std::to_string(score->_value), *_font, *color);

    namePosition._y += rowSeperation + _font->getLineSpace();
    scorePosition._y += rowSeperation + _font->getLineSpace();
  }
}

bool HiScoreBoardState::doScoreSwap()
{
  for(int i{0}; i < _scoreBoard.size(); ++i){
    if(_scoreBoard[i] != &_newScore) continue;
    if(i == (_scoreBoard.size() - 1)) return true;
    if(_scoreBoard[i]->_value <= _scoreBoard[i + 1]->_value) return true;
    std::swap(_scoreBoard[i], _scoreBoard[i + 1]);
    mixer->playSound(SpaceInvaders::SK_SCORE_BEEP); 
    return false;
  }
}

void HiScoreBoardState::populateHud()
{
  SpaceInvaders* si = static_cast<SpaceInvaders*>(_app);
  Vector2i worldSize = si->getWorldSize();
  int32_t worldScale = si->getWorldScale();
  int32_t titleWidth = _font->calculateStringWidth(titleString);
  Vector2i titlePosition {
    (worldSize._x - titleWidth) / 2,
    _nameScreenPosition._y + _scoreBoardSize._y + (boardTitleSeperation * worldScale)
  };
  HUD& hud = si->getHud();
  _uidTitleText = hud.addTextLabel({titlePosition, titleColor, titleString});
}

void HiScoreBoardState::depopulateHud()
{
  SpaceInvaders* si = static_cast<SpaceInvaders*>(_app);
  HUD& hud = si->getHud();
  hud.removeTextLabel(_uidTitleText);
}

bool HiScoreBoardState::newScoreIsTop()
{
  return _scoreBoard[_scoreBoard.size() - 1] == &_newScore;
}

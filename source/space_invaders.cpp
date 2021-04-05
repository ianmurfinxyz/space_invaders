#include "pixiretro.h"
#include "spaceinvaders.h"

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


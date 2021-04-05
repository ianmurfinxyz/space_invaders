#include "hiscore_board_scene.h"

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

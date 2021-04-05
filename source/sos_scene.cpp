#include "sos_scene.h"

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


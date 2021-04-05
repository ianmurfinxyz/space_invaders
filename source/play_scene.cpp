#include "play_scene.h"

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

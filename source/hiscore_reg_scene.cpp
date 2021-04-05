#include "hiscore_reg_scene.h"

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


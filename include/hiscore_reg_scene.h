#ifndef _SI_HISCORE_REGISTRATION_SCENE_H_
#define _SI_HISCORE_REGISTRATION_SCENE_H_

class HiScoreRegState final : public ApplicationState
{
public:
  static constexpr const char* name = "scoreReg";

public:
  HiScoreRegState(Application* app) : ApplicationState{app}{}
  ~HiScoreRegState() = default;

  void initialize(Vector2i worldSize, int32_t worldScale);
  void onUpdate(double now, float dt);
  void onDraw(double now, float dt);
  void onEnter();

  std::string getName(){return name;}

private:

  class Keypad
  {
  public:
    Keypad(const Font& font, Vector2i worldSize, int32_t worldScale);

    void moveCursor(int32_t colShift, int32_t rowShift);
    void reset();
    void draw();

    const char* getActiveKeyText() const
    { return _keyText[_cursorPadPosition._y][_cursorPadPosition._x]; }

  private:
    void updateCursorScreenPosition();

  private:
    static constexpr int32_t keyRowCount {4};
    static constexpr int32_t keyColCount {11};
    static constexpr int32_t keySpace_px {4};
    static constexpr int32_t cursorDrop_px {keySpace_px / 2};
    static constexpr const char* cursorChar {"_"};
    static constexpr Vector2i initialCursorPadPosition {0, keyRowCount - 1};

  private:
    std::array<std::array<const char*, keyColCount>, keyRowCount> _keyText;
    std::array<std::array<Vector2i, keyColCount>, keyRowCount> _keyScreenPosition;
    Color3f _keyColor;
    Color3f _specialKeyColor;
    Color3f _cursorColor;
    Vector2i _cursorPadPosition; // x=col, y=row
    Vector2i _cursorScreenPosition;
    Vector2i _padScreenPosition;
    const Font& _font;
  };

  class NameBox
  {
  public:
    NameBox(const Font& font, Vector2i worldSize, int32_t worldScale);
    void draw();
    bool pushBack(char c);
    bool popBack();
    SpaceInvaders::ScoreName getBufferText() const {return _nameBuffer;}
    bool isFull() const;
    bool isEmpty() const;

  private:
    void composeFinal();

  private:
    static constexpr char nullChar {'-'};
    static constexpr char quoteChar {'\''};
    static constexpr const char* label = "NAME";

  private:
    SpaceInvaders::ScoreName _nameBuffer; 
    Vector2i _boxScreenPosition;
    const Font& _font;
    std::string _final;
  };

private:
  void doInput();

private:

  std::unique_ptr<Keypad> _keypad;
  std::unique_ptr<NameBox> _nameBox;
};

#endif

#ifndef _SI_HUD_H_
#define _SI_HUD_H_

//
// Resetting the HUD will reset activation delays and phaseins, it will not reset age, modify
// flash state or hidden state. Meaning if label had a finite lifetime and died before the reset,
// the reset will not bring it back. Further if the label is still alive at the point of reset
// it will not make the label younger and delay its death.
//

class HUD
{
public:
  using uid_t = uint32_t;

  static constexpr float immortalLifetime {0.f};

  class Label
  {
    friend HUD;
  protected:
    Label(
      Vector2f position,
      Color3f color,
      float activationDelay,
      float lifetime
    );

    virtual void onReset();
    virtual void onUpdate(float dt);
    virtual void onDraw(const Font& font) = 0;

    void startFlashing();
    void stopFlashing();
    bool isFlashing() const {return _isFlashing;}
    void hide() {_isHidden = true;}
    void show() {_isHidden = false;}
    bool isHidden() const {return _isHidden;}
    bool isDead() {return _isDead;}
    uid_t getUid() const {return _uid;}

  private:
    void initialize(const HUD* owner, uid_t uid);
  
  protected:
    const HUD* _owner;
    uid_t _uid;
    Vector2i _position;
    Color3f _color; 
    long _lastFlashNo;
    float _activationDelay;
    float _activationClock;
    float _age;
    float _lifetime;
    bool _flashState;
    bool _isActive;
    bool _isHidden;
    bool _isFlashing;
    bool _isImmortal;
    bool _isDead;
  };

  class TextLabel final : public Label
  {
    friend HUD;
  public:
    TextLabel(
      Vector2f position,
      Color3f color,
      float activationDelay,
      float lifetime,
      std::string text,
      bool phaseIn
    );

    void onReset();
    void onUpdate(float dt);
    void onDraw(const Font& font);

  private:
    std::string _fullText;
    std::string _visibleText;
    long _lastPhaseInNo;
    int _nextCharToShow;
    bool _isPhasingIn;
  };

  class IntLabel final : public Label
  {
    friend HUD;
  public:
    IntLabel(
      Vector2f position,
      Color3f color,
      float activationDelay,
      float lifetime,
      const int& source,
      int precision
    );

    void onReset();
    void onUpdate(float dt);
    void onDraw(const Font& font);

  private:
    const int& _sourceValue;
    int _displayValue;
    int _precision;
    std::string _displayStr;
  };

  class BitmapLabel final : public Label
  {
    friend HUD;
  public:
    BitmapLabel(
      Vector2f position,
      Color3f color,
      float activationDelay,
      float lifetime,
      const Bitmap& bitmap
    );

    void onUpdate(float dt);
    void onDraw(const Font& font);

  private:
    const Bitmap& _bitmap;
  };

public:
  HUD(const Font& font, float flashPeriod, float phaseInPeriod);

  void onReset();
  void onUpdate(float dt);
  void onDraw();

  uid_t addLabel(std::unique_ptr<Label> label);
  void removeLabel(uid_t uid);
  void clear();

  bool hideLabel(uid_t uid);
  bool showLabel(uid_t uid);
  bool startLabelFlashing(uid_t uid);
  bool stopLabelFlashing(uid_t uid);

  void setFlashPeriod(float period) {_flashPeriod = period;}
  void setPhasePeriod(float period) {_phasePeriod = period;}

private:
  std::vector::iterator findLabel(uid_t uid);

private:
  const Font& _font;
  std::vector<std::unique_ptr<Label>> _labels;
  uid_t _nextUid;
  long _flashNo;
  long _phaseInNo;
  float _flashPeriod;
  float _phaseInPeriod;
  float _flashClock;
  float _phaseClock;
};

#endif

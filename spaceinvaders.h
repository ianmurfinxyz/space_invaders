#ifndef _SPACEINVADERS_H_
#define _SPACEINVADERS_H_

#include "nomad.h"

using namespace nomad;

//===============================================================================================//
// ##>GAME STATE                                                                                 //
//===============================================================================================//

class GameState final : public ApplicationState
{
public:
  static const std::string name;

public:
  GameState(Application* app) : ApplicationState{app} {}
  ~GameState() = default;

  void initialize(Vector2i worldSize);
  void onUpdate(double now, float dt);
  void onDraw(double now, float dt);
  void onReset();

  const std::string& getName(){return name;}

private:

};

//===============================================================================================//
// ##>MENU STATE                                                                                 //
//===============================================================================================//

class MenuState final : public ApplicationState
{
public:
  static const std::string name;

public:
  MenuState(Application* app) : ApplicationState{app} {}
  ~MenuState() = default;

  void initialize(Vector2i worldSize);
  void onUpdate(double now, float dt);
  void onDraw(double now, float dt);
  void onReset();

  const std::string& getName(){return name;}

private:

};

//===============================================================================================//
// ##>SPACE INVADERS                                                                             //
//===============================================================================================//

class SpaceInvaders final : public Application
{
public:
  static const std::string name;

  static constexpr int32_t version_major {0};
  static constexpr int32_t version_minor {1};
  static constexpr Vector2i baseWorldSize {300, 300};

public:
  enum BitmapKey : int32_t
  {
    BMK_CANNON0, BMK_SQUID0, BMK_SQUID1, BMK_CRAB0, BMK_CRAB1, BMK_OCTOPUS0, BMK_OCTOPUS1, 
    BMK_SAUCER0, BMK_CROSS0, BMK_CROSS1, BMK_CROSS2, BMK_CROSS3, BMK_ZIGZAG0, BMK_ZIGZAG1, 
    BMK_ZIGZAG2, BMK_ZIGZAG3, BMK_ZAGZIG0, BMK_ZAGZIG1, BMK_ZAGZIG2, BMK_ZAGZIG3, BMK_LASER0,
    BMK_COUNT
  };

  constexpr static std::array<const char*, BMK_COUNT> _bitmapNames {
    "cannon0", "squid0", "squid1", "crab0", "crab1", "octopus0", "octopus1", 
    "saucer0", "cross0", "cross1", "cross2", "cross3", "zigzag0", "zigzag1", 
    "zigzag2", "zigzag3", "zagzig0", "zagzig1", "zagzig2", "zagzig3", "laser0"
  };

public:
  SpaceInvaders() = default;
  ~SpaceInvaders() = default;

  const std::string& getName() const {return name;}
  int32_t getVersionMajor() const {return version_major;}
  int32_t getVersionMinor() const {return version_minor;}

  bool initialize(Engine* engine, int32_t windowWidth, int32_t windowHeight);

private:
  Vector2i getWorldSize() const {return _worldSize;}

private:
  Vector2i _worldSize;
  int32_t _worldScale;
};

#endif

#ifndef _SPACEINVADERS_H_
#define _SPACEINVADERS_H_

#include "nomad.h"

using namespace nomad;
using namespace nomad::types;
using namespace nomad::math;

//===============================================================================================//
//                                                                                               //
// ##>ASSET KEYS                                                                                 //
//                                                                                               //
//===============================================================================================//

namespace bmk
{
const std::string cannon0 {"cannon0"};
const std::string squid0 {"squid0"};
const std::string squid1 {"squid1"};
const std::string crab0 {"crab0"};
const std::string crab1 {"crab1"};
const std::string octopus0 {"octopus0"};
const std::string octopus1 {"octopus1"};
const std::string saucer0 {"saucer0"};
const std::string cross0 {"cross0"};
const std::string cross1 {"cross1"};
const std::string cross2 {"cross2"};
const std::string cross3 {"cross3"};
const std::string zigzag0 {"zigzag0"};
const std::string zigzag1 {"zigzag1"};
const std::string zigzag2 {"zigzag2"};
const std::string zigzag3 {"zigzag3"};
const std::string zagzig0 {"zagzig0"};
const std::string zagzig1 {"zagzig1"};
const std::string zagzig2 {"zagzig2"};
const std::string zagzig3 {"zagzig3"};
const std::string laser0 {"laser0"};
};

//===============================================================================================//
//                                                                                               //
// ##>GAME STATE                                                                                 //
//                                                                                               //
//===============================================================================================//

class GameState final : ApplicationState
{
public:
  static constexpr const char* name {"game"};

public:
  GameState(Application* app) : ApplicationState{app} {}
  ~GameState() = default;

  void initialize(Vector2i worldSize);
  void onUpdate(double now, float dt);
  void onDraw(double now, float dt);
  void onReset();

  const char* getName(){return name;}

private:

};

//===============================================================================================//
//                                                                                               //
// ##>MENU STATE                                                                                 //
//                                                                                               //
//===============================================================================================//

class MenuState final : ApplicationState
{
public:
  static constexpr const char* name {"menu"};

public:
  MenuState(Application* app) : ApplicationState{app} {}
  ~MenuState() = default;

  void initialize(Vector2i worldSize);
  void onUpdate(double now, float dt);
  void onDraw(double now, float dt);
  void onReset();

  const char* getName(){return name;}

private:

};

//===============================================================================================//
//                                                                                               //
// ##>SPACE INVADERS                                                                             //
//                                                                                               //
//===============================================================================================//

class SpaceInvaders final : public Application
{
public:
  static constexpr const char* app_name {"Space Invaders"};
  static constexpr int32 version_major {0};
  static constexpr int32 version_minor {1};
  static constexpr Vector2i baseWorldSize {300, 300};

public:
  SpaceInvaders() = default;
  ~SpaceInvaders() = default;

  const char* getName() {return app_name;}
  int32 getVersionMajor() const {return version_major;}
  int32 getVersionMinor() const {return version_minor;}

  bool initialize(Engine* engine, int32 windowWidth, int32 windowHeight);

private:
  Vector2i getWorldSize() const {return _worldSize;}

private:
  Vector2i _worldSize;
  int32 _worldScale;
};

#endif

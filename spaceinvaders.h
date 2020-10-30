#ifndef _SPACEINVADERS_H_
#define _SPACEINVADERS_H_

#include "nomad.h"

using namespace nomad;

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
//                                                                                               //
// ##>MENU STATE                                                                                 //
//                                                                                               //
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
//                                                                                               //
// ##>SPACE INVADERS                                                                             //
//                                                                                               //
//===============================================================================================//

class SpaceInvaders final : public Application
{
public:
  static const std::string name;

  static constexpr int32_t version_major {0};
  static constexpr int32_t version_minor {1};
  static constexpr Vector2i baseWorldSize {300, 300};

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

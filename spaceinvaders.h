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
constexpr const char* cannon0 {"cannon0"};
constexpr const char* squid0 {"squid0"};
constexpr const char* squid1 {"squid1"};
constexpr const char* crab0 {"crab0"};
constexpr const char* crab1 {"crab1"};
constexpr const char* octopus0 {"octopus0"};
constexpr const char* octopus1 {"octopus1"};
constexpr const char* saucer0 {"saucer0"};
constexpr const char* cross0 {"cross0"};
constexpr const char* cross1 {"cross1"};
constexpr const char* cross2 {"cross2"};
constexpr const char* cross3 {"cross3"};
constexpr const char* zigzag0 {"zigzag0"};
constexpr const char* zigzag1 {"zigzag1"};
constexpr const char* zigzag2 {"zigzag2"};
constexpr const char* zigzag3 {"zigzag3"};
constexpr const char* zagzig0 {"zagzig0"};
constexpr const char* zagzig1 {"zagzig1"};
constexpr const char* zagzig2 {"zagzig2"};
constexpr const char* zagzig3 {"zagzig3"};
constexpr const char* laser0 {"laser0"};
};

//===============================================================================================//
//                                                                                               //
// ##>GAME STATE                                                                                 //
//                                                                                               //
//===============================================================================================//

class GameState final : public ApplicationState
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

class MenuState final : public ApplicationState
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

  const char* getName() const {return app_name;}
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

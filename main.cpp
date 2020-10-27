#include "nomad.h"
#include "spaceinvaders.h"

nomad::Engine* engine {nullptr};

int main()
{
  engine = new nomad::Engine{};
  engine->initialize(std::move(std::unique_ptr<nomad::Application>{new SpaceInvaders{}}));
  engine->run();
  delete engine;
}

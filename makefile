
LDLIBS = -lSDL2 -lm -lGLX_mesa
CXXFLAGS = -g -Wall -std=c++17 -fno-exceptions

si : main.cpp spaceinvaders.cpp pixiretro.cpp spaceinvaders.h pixiretro.h
	$(CXX) $(CXXFLAGS) -o $@ main.cpp pixiretro.cpp spaceinvaders.cpp $(LDLIBS)

.PHONY: clean
clean:
	rm si *.o

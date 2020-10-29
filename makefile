
LDLIBS = -lSDL2 -lm -lGLX_mesa
CXXFLAGS = -g -Wall -std=c++17 -fno-exceptions

si : main.cpp spaceinvaders.cpp nomad.cpp spaceinvaders.h nomad.h
	$(CXX) $(CXXFLAGS) -o $@ main.cpp nomad.cpp spaceinvaders.cpp $(LDLIBS)

.PHONY: clean
clean:
	rm si *.o

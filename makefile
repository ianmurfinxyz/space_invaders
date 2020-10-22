
LDLIBS = -lSDL2 -lm -lGLX_mesa
CXXFLAGS = -g -Wall -std=c++17

si : spaceinvaders.cpp bitmaps.h
	$(CXX) $(CXXFLAGS) -o $@ spaceinvaders.cpp $(LDLIBS)

.PHONY: clean
clean:
	rm si *.o

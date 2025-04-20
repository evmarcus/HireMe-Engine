# Variables 
CXX := clang++
CXXFLAGS := -std=c++17 -I"src" -I"src/First_Party" -I"src/Third_Party" -I"src/Third_Party/glm" -I"src/Third_Party/rapidjson-1.1.0/include" -I"src/Third_Party/SDL" -I"src/Third_Party/SDL_image" -I"src/Third_Party/SDL_mixer" -I"src/Third_Party/SDL_ttf" -I"src/Third_Party/lua" -I"src/Third_Party/LuaBridge" -I"src/Third_Party/box2d" -I"src/Third_Party/box2d/src" -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf -llua5.4 -O3
TARGET := game_engine_linux
SRC := $(wildcard src/First_Party/*.cpp) $(wildcard src/Third_Party/box2d/src/**/*.cpp)

# Build Target
$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

# Clean Target
clean:
	rm -f $(TARGET)

.PHONY: clean

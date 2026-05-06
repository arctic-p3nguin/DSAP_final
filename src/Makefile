CXX = g++
CXXFLAGS = -Wall -std=c++11

TARGET = mahjong_game

SRCS = main.cpp Tile.hpp Tile.cpp Hand.hpp Hand.cpp Rules.hpp Rules.cpp Player.hpp Player.cpp GameManager.hpp GameManager.cpp ShantenCounter.hpp ShantenCounter.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)
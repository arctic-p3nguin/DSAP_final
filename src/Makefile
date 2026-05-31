CXX = g++
CXXFLAGS = -O3 -std=c++17 -Wall

TARGET_GAME = mahjong_game
TARGET_BENCH = algo_benchmark

COMMON_SRCS = Tile.cpp Hand.cpp Rules.cpp Player.cpp GameManager.cpp ShantenCounter.cpp ScoreCounter.cpp

GAME_SRCS = main.cpp $(COMMON_SRCS)
BENCH_SRCS = AlgorithmTest.cpp $(COMMON_SRCS)

GAME_OBJS = $(GAME_SRCS:.cpp=.o)
BENCH_OBJS = $(BENCH_SRCS:.cpp=.o)

all: $(TARGET_GAME) $(TARGET_BENCH)

$(TARGET_GAME): $(GAME_OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET_GAME) $(GAME_OBJS)

$(TARGET_BENCH): $(BENCH_OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET_BENCH) $(BENCH_OBJS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET_GAME) $(TARGET_BENCH) *.o
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17
LDLIBS = -lpthread

TARGET = planificador

SRCS = src/main.cpp \
       src/parser.cpp \
       src/planificador.cpp \
       src/actividad.cpp

OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET) $(LDLIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET) plan.txt 2

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all run clean

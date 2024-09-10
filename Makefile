SOURCES := tictactoe.cpp
TARGET := tictactoe
# Objs are all the sources, with .c replaced by .o
OBJS := $(SOURCES:.cpp=.o)
CC := g++
CFLAGS := -lstdc++

all: tictactoe

clean:
	rm $(OBJS) $(TARGET)

# Compile the binary 't' by calling the compiler with cflags, lflags, and any libs (if defined) and the list of objects.
tictactoe: $(OBJS)
	$(CC) $(CFLAGS) -o tictactoe $(OBJS) $(LFLAGS) $(LIBS)

%.o: %.cpp
	$(CC) -c $(CFLAGs) -o $@ $<

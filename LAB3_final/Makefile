#Compiler
CXX = g++

#Compiler flags
CXXFLAGS = -std=c++17 -Wall -O2 -fopenmp

#Target executable
TARGET = hello
#Source files
SOURCES = hellomake.c hellofunc.c
#Object files
OBJECTS = $(SOURCES:.c=.o)
#Header files
HEADERS = hellomake.h
#Default target
all: $(TARGET)
#Link object files to create executable
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJECTS)

#Compile source files to object files
%.o: %.c $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

#Clean build files
clean:
	rm -f $(OBJECTS) $(TARGET)	

#Run the program
run: $(TARGET)
	./$(TARGET)

#Phony targets
.PHONY: all clean run
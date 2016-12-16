CC = g++ -std=c++11
CC_FLAGS = -w -g
LIBS = -lodbc 
INCLUDES =

EXEC = run
SOURCES = $(wildcard *.cpp)
HEADERS = $(wildcard *.h)
OBJECTS = $(SOURCES:.cpp=.o)

# Main target                                                                                                                                                                                               
$(EXEC): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(EXEC) $(LIBS)

# To obtain object files                                                                                                                                                                                    
%.o: %.cpp
	$(CC) -c $(INCLUDES) $(CC_FLAGS) $< -o $@

# To remove generated files                                                                                                                                                                                 
clean:
	rm -f $(EXEC) $(OBJECTS)

TARGET = parallel_merge_sort

LINK = -lpthread

INCLUDE_PATHS = 

SRC = src/parallel_merge_sort.c

OBJECTS := $(SRC:%.c=build/%.o)

CXX = gcc

CFLAGS = -g

N = 100

K = 5

SILENT = 

build/src/%.o: src/%.c
	@mkdir -p build/src
	$(CXX) $(INCLUDE_PATHS) $(CFLAGS) -c -o $@ $<

$(TARGET): $(OBJECTS)
	$(CXX) $(INCLUDE_PATHS) $(OBJECTS) $(LINK) -o $@

clean: 
	-rm -rf build

valgrind: $(TARGET)
	@valgrind ./$(TARGET) $(N) $(K) $(SILENT)

run: $(TARGET)
	@./$(TARGET) $(N) $(K) $(SILENT)
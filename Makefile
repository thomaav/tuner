CC=clang++
CFLAGS=-Wall -std=c++11
LDLIBS=-lasound -lfftw3 -lsamplerate

OBJ_DIR=objects
OBJS=$(patsubst %.cpp, $(OBJ_DIR)/%.o, $(wildcard *.cpp))

tuner.bin: $(OBJS)
	${CC} ${CFLAGS} $^ ${LDLIBS} -o $@

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(@D)
	${CC} ${CFLAGS} $< -c -o $@

.PHONY: clean
clean:
	-rm -rf $(OBJ_DIR)
	-rm -rf *.bin *.o
	-rm -rf *.wav

.PHONY: run
run:
	-make && ./tuner.bin

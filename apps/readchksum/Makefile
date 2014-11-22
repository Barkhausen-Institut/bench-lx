BIN=readchksum
SRC=readchksum.c
OBJ=$(patsubst %.c,%.o,$(SRC))

all: $(BIN)
	cp $(BIN) ../../rootfs

$(BIN): $(OBJ)
	$(CC) -Wall -Wextra -O3 -o $@ $<

%.o: %.c
	$(CC) -Wall -Wextra -O3 -c -o $@ $<

clean:
	rm $(BIN) $(OBJ)


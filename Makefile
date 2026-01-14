CC = gcc
CFLAGS = -O2 -Wall
LDFLAGS = -lcurl -lyaml

SRC = \
	src/main.c \
	src/config.c \
	src/repo.c \
	src/git.c \
	src/yaml.c \
	src/net.c \
	src/build.c \
	src/install.c \
	src/util.c

OBJ = $(SRC:.c=.o)

all: tinypkg

tinypkg: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(LDFLAGS)

clean:
	rm -f src/*.o tinypkg

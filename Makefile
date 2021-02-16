CC = gcc
CFLAGS = -g
OUTPUT_DIR = bin/

all: compress decompress

compress: compress.c
	mkdir -p $(OUTPUT_DIR)
	$(CC) $(CFLAGS) -o $(OUTPUT_DIR)compress compress.c

decompress: decompress.c
	mkdir -p $(OUTPUT_DIR)
	$(CC) $(CFLAGS) -o $(OUTPUT_DIR)decompress decompress.c

clean:
	rm $(OUTPUT_DIR)*
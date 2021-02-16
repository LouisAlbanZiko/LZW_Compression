#ifndef _FILE_IO_H_
#define _FILE_IO_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

uint32_t file_read(void **dst, const char *path)
{
	// open file
	FILE *file = fopen(path, "r");
	if (file == NULL)
	{
		printf("File not found.'\n");
		exit(EXIT_FAILURE);
	}

	// find size of file
	fseek(file, 0L, SEEK_END);
	uint32_t buffer_size = ftell(file);
	fseek(file, 0L, SEEK_SET);

	// create buffer
	*dst = malloc(buffer_size);

	// fill buffer
	fread(*dst, sizeof(char), buffer_size, file);

	// check for errors and close file
	if (ferror(file))
	{
		fclose(file);
		free(*dst);
		printf("Error while reading file.");
		exit(EXIT_FAILURE);
	}
	fclose(file);
	return buffer_size;
}

void file_write(const void *data, uint32_t size, const char *path)
{
	FILE *file = fopen(path, "w");
	fwrite(data, sizeof(uint8_t), size, file);
	fclose(file);
}

#endif
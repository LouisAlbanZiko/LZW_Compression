#ifndef _LZW_H_
#define _LZW_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define printf(...)

#define INFO(...)   \
	printf("[+] "); \
	printf("\n");   \
	printf(__VA_ARGS__)

#define ERROR(...)  \
	printf("[-] "); \
	printf("\n");   \
	printf(__VA_ARGS__)

#define FATAL(...)       \
	printf("[!] ");      \
	printf(__VA_ARGS__); \
	printf("\n");        \
	exit(EXIT_FAILURE)

uint32_t log2_uint(uint32_t nr)
{
	int targetlevel = 1;
	while (nr >>= 1)
		++targetlevel;
	return targetlevel;
}

/* --- BufferBit --- */
typedef struct
{
	uint32_t index_uint;
	uint32_t index_bit;
	uint32_t count_m;
	uint32_t *data;
} BufferBit;

void BufferBit_create(BufferBit *buffer, uint32_t count)
{
	buffer->index_uint = 0;
	buffer->index_bit = 0;
	buffer->count_m = count;
	if (count != 0)
	{
		buffer->data = malloc(buffer->count_m * sizeof(*(buffer->data)));
		buffer->data[0] = 0;
	}
	else
	{
		buffer->data = NULL;
	}
}

// reset for reading
void BufferBit_reset(BufferBit *buffer)
{
	buffer->index_bit = 0;
	buffer->index_uint = 0;
}

// writing
void BufferBit_insertBit(BufferBit *buffer, uint32_t bit)
{
	buffer->data[buffer->index_uint] |= bit << (31 - buffer->index_bit++);
	//printf("data[%d] -> %X\n", buffer->index_uint * 32 + buffer->index_bit, buffer->data[buffer->index_uint]);
	if (buffer->index_bit == 32)
	{
		buffer->index_bit = 0;
		buffer->index_uint++;
		buffer->data[buffer->index_uint] = 0;
	}
}

void BufferBit_insert(BufferBit *buffer, uint32_t e, uint32_t bit_length)
{
	if (buffer->index_bit + bit_length > 31)
	{
		if (buffer->index_uint == buffer->count_m)
		{
			buffer->count_m *= 2;
			buffer->data = realloc(buffer->data, buffer->count_m * sizeof(*(buffer->data)));
		}
	}
	printf("Bits inserted '");
	for (uint32_t i = bit_length; i > 0; i--)
	{
		uint32_t bit = (e >> (i - 1)) & 1;
		printf("%d", bit);
		BufferBit_insertBit(buffer, bit);
	}
	printf("'\n");
}

// reading
uint32_t BufferBit_getBit(BufferBit *buffer)
{
	uint32_t bit = (buffer->data[buffer->index_uint] >> (31 - buffer->index_bit++)) & 1;
	if (buffer->index_bit == 32)
	{
		buffer->index_bit = 0;
		buffer->index_uint++;
	}
	return bit;
}

uint32_t BufferBit_get(BufferBit *buffer, uint32_t bit_length)
{
	uint32_t nr = 0;
	for (uint32_t i = 0; i < bit_length; i++)
	{
		nr = nr << 1;
		nr = nr | BufferBit_getBit(buffer);
	}
	return nr;
}

void BufferBit_destroy(BufferBit *buffer)
{
	free(buffer->data);
}

/* --- SubString --- */
typedef struct
{
	char **ref;
	uint32_t start;
	uint32_t end;
} SubString;

void SubString_output(const char *before, const SubString *string, const char *after)
{
	printf(before);
	for (const char *start = (*string->ref) + string->start; start < (*string->ref) + string->end; start++)
		printf("%c", *start);
	printf(after);
}

uint32_t SubString_length(const SubString *string)
{
	return string->end - string->start;
}

uint32_t SubString_compare(const SubString *s1, const SubString *s2)
{
	//printf("len1(%d)", SubString_length(s1));
	//SubString_output("'", s1, "' === ");
	//printf("len2(%d)", SubString_length(s2));
	//SubString_output("'", s2, "'\n");
	uint32_t length = s1->end - s1->start;
	if (length == s2->end - s2->start)
	{
		for (uint32_t i = 0; i < length; i++)
		{
			//printf("\t%d == %d\n", (*(s1->ref))[s1->start + i], (*(s2->ref))[s2->start + i]);
			if ((*(s1->ref))[s1->start + i] != (*(s2->ref))[s2->start + i])
				return 0;
		}
		return 1;
	}
	else
	{
		return 0;
	}
}

/* --- OutputBuffer --- */
typedef struct
{
	uint32_t count_c;
	uint32_t count_m;
	char *data;
} OutputBuffer;

void OutputBuffer_create(OutputBuffer *buffer)
{
	buffer->count_c = 0;
	buffer->count_m = 128;
	buffer->data = malloc(sizeof(*(buffer->data)) * buffer->count_m);
}

void OutputBuffer_insert_Substring(OutputBuffer *buffer, SubString *string)
{
	if(buffer->count_c + string->end - string->start >= buffer->count_m)
	{
		buffer->count_m *= 2;
		buffer->data = realloc(buffer->data, sizeof(*buffer->data) * buffer->count_m);
	}
	for(uint32_t i = string->start; i < string->end; i++)
	{
		buffer->data[buffer->count_c++] = (*string->ref)[i];
	}
}

void OutputBuffer_insert_char(OutputBuffer *buffer, char c)
{
	if(buffer->count_c + 1 == buffer->count_m)
	{
		buffer->count_m *= 2;
		buffer->data = realloc(buffer->data, sizeof(*buffer->data) * buffer->count_m);
	}
	buffer->data[buffer->count_c++] = c;
}

void OutputBuffer_destroy(OutputBuffer *buffer)
{
	free(buffer->data);
}

/* --- Dictionary --- */
typedef struct
{
	uint32_t count_c;
	uint32_t count_m;
	SubString *data;
} Dictionary;

void Dictionary_create(Dictionary *dictionary, uint32_t count)
{
	dictionary->count_c = 0;
	dictionary->count_m = count;
	dictionary->data = malloc(dictionary->count_m * sizeof(*(dictionary->data)));
}

void Dictionary_insert(Dictionary *dictionary, const SubString *e)
{
	if (dictionary->count_c < 4096)
	{
		if (dictionary->count_c == dictionary->count_m)
		{
			dictionary->count_m *= 2;
			dictionary->data = realloc(dictionary->data, dictionary->count_m * sizeof(*(dictionary->data)));
		}
		dictionary->data[dictionary->count_c++] = *e;
	}
}

void Dictionary_output(Dictionary *dictionary, const char *file_path)
{
	FILE *file = fopen(file_path, "w");
	for(uint32_t i = 0; i < dictionary->count_c; i++)
	{
		for(uint32_t string_index = 0; string_index < SubString_length(&(dictionary->data[i])); string_index++)
			fprintf(file, "%d", (*(dictionary->data[i].ref))[dictionary->data[i].start + string_index]);
		fprintf(file, "\n");
	}
	fclose(file);
}

uint32_t Dictionary_findIndex(Dictionary *dictionary, const SubString *string)
{
	for(uint32_t i = 0; i < dictionary->count_c; i++)
	{
		if(SubString_compare(&dictionary->data[i], string))
			return i;
	}
	return dictionary->count_c;
}

uint32_t Dictionary_doesEntryExist(Dictionary *dictionary, const SubString *string)
{
	return Dictionary_findIndex(dictionary, string) < dictionary->count_c;
}

void Dictionary_destroy(Dictionary *dictionary)
{
	free(dictionary->data);
}

#endif
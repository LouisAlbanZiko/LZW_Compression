#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "file_io.h"
#include "LZW.h"

int main(int argc, char **argv)
{
	// parse arguments
	const char *file_name_input = NULL;
	const char *file_name_alphabet = NULL;
	const char *file_name_output = "output.txt";
	if (argc > 1)
	{
		uint32_t args_iterator = 1;
		while (args_iterator < argc)
		{
			if (argv[args_iterator][0] == '-')
			{
				if (args_iterator + 1 != argc)
				{
					if (argv[args_iterator][1] == 'a')
					{
						file_name_alphabet = argv[++args_iterator];
					}
					else if (argv[args_iterator][1] == 'o')
					{
						file_name_output = argv[++args_iterator];
					}
					else
					{
						ERROR("Option '%s' not defined.", argv[args_iterator]);
					}
				}
				else
				{
					FATAL("Option '%s' passed but no file provided.", argv[args_iterator - 1]);
				}
			}
			else if (file_name_input == NULL)
			{
				file_name_input = argv[args_iterator];
			}
			else
			{
				FATAL("File name defined twice in arguments.");
			}
			args_iterator++;
		}
	}
	else
	{
		FATAL("No file provided.");
	}

	printf("file_name_input = %s\n", file_name_input);
	printf("file_name_output = %s\n", file_name_output);

	// read file
	BufferBit input_data;
	input_data.count_m = file_read((void **)&input_data.data, file_name_input);
	input_data.index_bit = 0;
	input_data.index_uint = 0;

	// define alphabet
	char *alphabet;
	if (file_name_alphabet != NULL)
	{
		file_read((void *)&alphabet, file_name_alphabet);
	}
	else
	{
		const char *default_alphabet_string = "ABCDEFGHIJKLMNPQRSTVWXY\n";
		alphabet = malloc(strlen(default_alphabet_string) + 1);
		strcpy(alphabet, default_alphabet_string);
	}

	// print alphabet
	printf("Alphabet:\n%s\n", alphabet);

	// create dictionary
	Dictionary dictionary;
	Dictionary_create(&dictionary, 128);

	// add terminating character to dictionary
	const char *_null_ref = "\0";
	dictionary.data[0].ref = &_null_ref;
	dictionary.data[0].start = 0;
	dictionary.data[0].end = 1;
	dictionary.count_c++;

	// add alphabet to dictionary
	for (; alphabet[dictionary.count_c - 1] != '\0'; dictionary.count_c++)
	{
		dictionary.data[dictionary.count_c].ref = &alphabet;
		dictionary.data[dictionary.count_c].start = dictionary.count_c - 1;
		dictionary.data[dictionary.count_c].end = dictionary.count_c;
	}

	// create output buffer
	OutputBuffer output_buffer;
	OutputBuffer_create(&output_buffer);

	// create dictionary buffer
	OutputBuffer dictionary_string_buffer;
	OutputBuffer_create(&dictionary_string_buffer);

	// create pattern buffer
	OutputBuffer pattern_buffer;
	OutputBuffer_create(&pattern_buffer);

	uint32_t bit_length = log2_uint(dictionary.count_c);

	uint32_t old = BufferBit_get(&input_data, bit_length), new;
	OutputBuffer_insert_Substring(&output_buffer, &dictionary.data[old]);

	SubString pattern;
	pattern.ref = &pattern_buffer.data;
	pattern.start = 0;
	pattern.end = 0;

	char c;
	do
	{
		new = BufferBit_get(&input_data, bit_length);
		printf("old = %d, new = %d, dict_length = %d\n", old, new, dictionary.count_c);

		if (new < dictionary.count_c)
		{
			pattern_buffer.count_c = 0;
			pattern.start = pattern_buffer.count_c;
			OutputBuffer_insert_Substring(&pattern_buffer, &dictionary.data[new]);
			pattern.end = pattern_buffer.count_c;
		}
		else
		{
			pattern_buffer.count_c = 0;
			pattern.start = pattern_buffer.count_c;
			OutputBuffer_insert_Substring(&pattern_buffer, &dictionary.data[old]);
			OutputBuffer_insert_char(&pattern_buffer, c);
			pattern.end = pattern_buffer.count_c;
		}

		// output S
		OutputBuffer_insert_Substring(&output_buffer, &pattern);

		// c = first char of S
		c = (*(pattern.ref))[pattern.start];

		// add to string table
		SubString new_entry = {&dictionary_string_buffer.data, dictionary_string_buffer.count_c, dictionary_string_buffer.count_c + SubString_length(&dictionary.data[old]) + 1};
		OutputBuffer_insert_Substring(&dictionary_string_buffer, &dictionary.data[old]);
		OutputBuffer_insert_char(&dictionary_string_buffer, c);

		Dictionary_insert(&dictionary, &new_entry);
		SubString_output("added to dictionary = '", &new_entry, "'\n");
		bit_length = log2_uint(dictionary.count_c == 4096 ? dictionary.count_c : dictionary.count_c + 1);

		old = new;
	} while (new != 0);

	printf("output_buffer.count_c = %d\n", output_buffer.count_c);
	file_write(output_buffer.data, sizeof(*(output_buffer.data)) * (output_buffer.count_c - 1), file_name_output);

	Dictionary_output(&dictionary, "dictionary_decompress.txt");  

	// cleanup
	{
		Dictionary_destroy(&dictionary);
		OutputBuffer_destroy(&output_buffer);
		OutputBuffer_destroy(&dictionary_string_buffer);
		OutputBuffer_destroy(&pattern_buffer);
		free(input_data.data);
		free(alphabet);
	}
}
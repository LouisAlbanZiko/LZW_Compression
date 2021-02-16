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
	const char *alphabet_file_name = NULL;
	const char *file_name_output = "output.lzw";
	if (argc > 1)
	{
		uint32_t args_iterator = 1;
		while (args_iterator < argc)
		{
			if (argv[args_iterator][0] == '-')
			{
				if (argv[args_iterator][1] == 'a')
				{
					args_iterator++;
					if (args_iterator < argc)
					{
						alphabet_file_name = argv[args_iterator];
					}
					else
					{
						FATAL("Option '%s' passed but no file provided.", argv[args_iterator - 1]);
					}
				}
				else
				{
					ERROR("Option '%s' not defined.", argv[args_iterator]);
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

	// read file
	char *input_data = NULL;
	file_read((void *)&input_data, file_name_input);
	uint32_t input_data_len = strlen(input_data);

	// define alphabet
	char *alphabet;
	if (alphabet_file_name != NULL)
	{
		file_read((void *)&alphabet, alphabet_file_name);
	}
	else
	{
		const char *default_alphabet_string = "ABCDEFGHIJKLMNPQRSTVWXY\n";
		alphabet = malloc(strlen(default_alphabet_string) + 1);
		strcpy(alphabet, default_alphabet_string);
	}

	// print file contents
	printf("File read:\n%s\n", input_data);

	// print alphabet
	printf("Alphabet:\n%s\n", alphabet);

	// create dictionary
	Dictionary dictionary;
	Dictionary_create(&dictionary, 128);

	// add terminating character to dictionary
	const char *_null_ref = "\0";
	SubString _null_string = {&_null_ref, 0, 1};
	Dictionary_insert(&dictionary, &_null_string);

	// add alphabet to dictionary
	for (uint32_t i = 0; i < strlen(alphabet); i++)
	{
		SubString temp = {&alphabet, i, i + 1};
		Dictionary_insert(&dictionary, &temp);
	}

	// create output data
	BufferBit output_codes;
	BufferBit_create(&output_codes, 128);

	// compress
	uint32_t bit_length = log2_uint(dictionary.count_c);
	SubString pattern = {.ref = &input_data, .start = 0, .end = 1};
	uint32_t percent_counter = 0;
	do
	{
		SubString_output("pattern = '", &pattern, "'\n");
		printf("dictionary.count_c = %d\n", dictionary.count_c);
		SubString p_c = {.ref = &input_data, .start = pattern.start, .end = pattern.end + 1};
		uint32_t found_in_dictionary = Dictionary_doesEntryExist(&dictionary, &p_c);
		if (!found_in_dictionary)
		{
			// add code to output
			uint32_t code = Dictionary_findIndex(&dictionary, &pattern);
			BufferBit_insert(&output_codes, code, bit_length);
			printf("output = %d, bit_length = %d\n", code, bit_length);

			// add p_c to dictionary
			Dictionary_insert(&dictionary, &p_c);
			SubString_output("added to dictionary = '", &p_c, "'\n");

			// increase bit length if necessary
			bit_length = log2_uint(dictionary.count_c);

			// p = c
			pattern.start = pattern.end;
		}
		pattern.end++;

		if((uint32_t)((float)pattern.start / (float)input_data_len * 100.0f) > percent_counter)
		{
			fprintf(stdout, "%d%% done\n", ++percent_counter);
		}
	} while ((*pattern.ref)[pattern.start] != '\0');

	// insert ending code
	BufferBit_insert(&output_codes, 0, bit_length);

	file_write(output_codes.data, sizeof(*(output_codes.data)) * (output_codes.index_uint + 1), file_name_output);

	Dictionary_output(&dictionary, "dictionary_compress.txt");

	// cleanup
	{
		Dictionary_destroy(&dictionary);
		BufferBit_destroy(&output_codes);

		free(input_data);
		free(alphabet);
	}
}
# LZW Compression

## Compiling
To compile the program I used the gcc compiler(https://gcc.gnu.org/git.html) aided with make(http://gnuwin32.sourceforge.net/packages/make.htm).
If both make and gcc are installed then simply run `make all` in a terminal/console.
If make is installed but you are using another compiler, then change the `CC` variable in the `Makefile` to whatever command your compiler uses.

## Running
The program is ran in the following format:
<executable> [<options>] <input_file>

* The executable files are either `compress` or `decompress`.
* The options are one of the following:
	1. `-o` which sets the output file.
	2. `-t` which sets the output file for the tuples.
	3. `-r` which reverses the input of the `compress` program.
	4. `-d` which outputs the dictionary to a file. This was used for debugging purposes to make sure both dictionaries were the same.
	5. `-a` which changes the default alphabet and reads it from the provided file.
* The input file is simply the file to be processed.

If you compiled using make and are running linux you can run the test.sh to run both programs in sucession and at the end compare the output file with the original.
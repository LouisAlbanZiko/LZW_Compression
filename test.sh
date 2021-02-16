#!/bin/bash
bin/compress -o res/output.lzw res/test_file.txt
bin/decompress -o res/output.txt res/output.lzw
cmp res/test_file.txt res/output.txt

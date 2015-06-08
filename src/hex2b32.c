/*
 ============================================================================
 Name        : hex2b32.c
 Author      : RedRoosterKey
 Version     :
 Copyright   : 
 Description : Convert hexadecimal into base32 according to RFC 3548.
 https://tools.ietf.org/html/rfc3548
 ============================================================================
 */

#include <ctype.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/**
 * Converts 8 bit values into 5 bit values
 * Complete cycle occurs every 40 bits
 * +-------------+----------------------------------------------+
 * | MODE        | 12345678|12345678|12345678|12345678|12345678 |
 * +-------------+----------------------------------------------+
 * | 0 bits left | 12345   |        |        |        |         |
 * | 3 bits left |      123|45      |        |        |         |
 * |             |         |  12345 |        |        |         |
 * | 1 bit  left |         |       1|2345    |        |         |
 * | 4 bits left |         |        |    1234|5       |         |
 * |             |         |        |        | 12345  |         |
 * | 2 bits left |         |        |        |      12|345      |
 * |             |         |        |        |        |   12345 |
 * | 0 bits left |         |        |        |        |         |
 * +-------------+----------------------------------------------+
 */

// Indicating how many bits are in the leftover
typedef enum {
	THREE_BITS_LEFT, ONE_BIT_LEFT, FOUR_BITS_LEFT, TWO_BITS_LEFT, NO_BITS_LEFT
} RemainderMode;

// AND with a byte to just get these bits
const unsigned char FIRST_FIVE_BITS = 0xF8;
const unsigned char FIRST_TWO_BITS = 0xC0;
const unsigned char THIRD_TO_SEVENTH_BITS = 0x3E;
const unsigned char FIRST_FOUR_BITS = 0xF0;
const unsigned char FIRST_BIT = 0x80;
const unsigned char SECOND_TO_SIXTH_BITS = 0x7C;
const unsigned char FIRST_THREE_BITS = 0xE0;
const unsigned char LAST_FIVE_BITS = 0x1F;

// Convert from the index to the base32 encoding
const char BASE_32[] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K',
		'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y',
		'Z', '2', '3', '4', '5', '6', '7' };

int getValidHexCharacter(int * const ch, const bool ignoreInputErrors) {
	int input;
	while (EOF != (input = getchar())) {
		if ('A' <= input && 'F' >= input) {
			*ch = input;
			return (0);
		} else if ('a' <= input && 'f' >= input) {
			*ch = input;
			return (0);
		} else if ('0' <= input && '9' >= input) {
			*ch = input;
			return (0);
		} else if (false == ignoreInputErrors) {
			fprintf(stderr, "Invalid hexadecimal character '%c'.\n", input);
			exit(EXIT_FAILURE);
		}
		// Here it's not an EOF, but not a valid hex character, so get another
	}
	return (-1);
}

int hexChar2Dec(char c) {
	c = toupper(c);
	if ('A' <= c && 'F' >= c) {
		return (c - 'A' + 10);
	} else if ('0' <= c && '9' >= c) {
		return (c - '0');
	}
	return (-1);
}

void processBits(RemainderMode * const mode, unsigned char * const leftover,
		const unsigned char byte) {
	unsigned char index = 0;
	switch (*mode) {
	case NO_BITS_LEFT:
		index = (FIRST_FIVE_BITS & byte) >> 3;
		putchar(BASE_32[(int) index]);
		*leftover = byte << 5;
		*mode = THREE_BITS_LEFT;
		break;
	case THREE_BITS_LEFT:
		index = ((FIRST_THREE_BITS & *leftover) >> 3)
				| ((FIRST_TWO_BITS & byte) >> 6);
		putchar(BASE_32[(int) index]);
		index = (THIRD_TO_SEVENTH_BITS & byte) >> 1;
		putchar(BASE_32[(int) index]);
		*leftover = byte << 7;
		*mode = ONE_BIT_LEFT;
		break;
	case ONE_BIT_LEFT:
		index = ((FIRST_BIT & *leftover) >> 3)
				| ((FIRST_FOUR_BITS & byte) >> 4);
		putchar(BASE_32[(int) index]);
		*leftover = byte << 4;
		*mode = FOUR_BITS_LEFT;
		break;
	case FOUR_BITS_LEFT:
		index = ((FIRST_FOUR_BITS & *leftover) >> 3)
				| ((FIRST_BIT & byte) >> 7);
		putchar(BASE_32[(int) index]);
		index = (SECOND_TO_SIXTH_BITS & byte) >> 2;
		putchar(BASE_32[(int) index]);
		*leftover = byte << 6;
		*mode = TWO_BITS_LEFT;
		break;
	case TWO_BITS_LEFT:
		index = ((FIRST_TWO_BITS & *leftover) >> 3)
				| ((FIRST_THREE_BITS & byte) >> 5);
		putchar(BASE_32[(int) index]);
		index = (LAST_FIVE_BITS & byte);
		putchar(BASE_32[(int) index]);
		*leftover = 0;
		*mode = NO_BITS_LEFT;
		break;
	}
}

void processLastBits(const RemainderMode * const mode,
		const unsigned char * const leftover, const bool padding) {
	unsigned char index = 0;
	switch (*mode) {
	case NO_BITS_LEFT:
		break;
	case THREE_BITS_LEFT:
		index = ((FIRST_THREE_BITS & *leftover) >> 5 << 2);
		putchar(BASE_32[(int) index]);
		if (padding)
			printf("======");
		break;
	case ONE_BIT_LEFT:
		index = ((FIRST_BIT & *leftover) >> 7 << 4);
		putchar(BASE_32[(int) index]);
		if (padding)
			printf("====");
		break;
	case FOUR_BITS_LEFT:
		index = ((FIRST_FOUR_BITS & *leftover) >> 4 << 1);
		putchar(BASE_32[(int) index]);
		if (padding)
			printf("===");
		break;
	case TWO_BITS_LEFT:
		index = ((FIRST_TWO_BITS & *leftover) >> 6 << 3);
		putchar(BASE_32[(int) index]);
		if (padding)
			putchar('=');
		break;
	}
}

int main(int argc, char **argv) {

	bool outputPadding = true;
	bool ignoreInputErrors = true;
	static struct option long_options[] = { { "input-errors", no_argument, 0,
			'e' }, { "help", no_argument, 0, 'h' }, { "no-padding", no_argument,
			0, 'n' }, { 0, 0, 0, 0 } };

	// Handle command line options
	while (1) {
		int option_index = 0;

		int option = getopt_long(argc, argv, "ehn", long_options,
				&option_index);
		if (option == -1)
			break;
		switch (option) {
		case 'e':
			ignoreInputErrors = false;
			break;
		case 'h':
			// Print help
			printf(
					"Usage: hex2b32 [OPTION]... \n\
Converts hexadecimal data from STDIN and outputs the data in base32 (RFC 3548) to STDOUT\n\
\n\
  -e, --input-errors       display the first input error and exit with failure (default behavior is to ignore invalid input)\n\
  -h, --help               display this help message and exit\n\
  -n, --no-padding         omit trailing '=' symbols\n\
      --version            output version information and exit\n\
\n");
			return (EXIT_SUCCESS);
			break;
		case 'n':
			outputPadding = false;
			break;
		case '?':
			printf("Please run with --help for usage options.\n");
			return (EXIT_FAILURE);
			break;
		default:
			abort();
			break;
		}
	}

	// 2 hex characters correspond to 1 byte
	// padding in base32 only works with bytes
	// (e.g. you cannot represent just 7 bits in base32)
	int c1, c2;
	c1 = c2 = 0;
	unsigned char byte, leftover;
	RemainderMode mode = NO_BITS_LEFT;
	int firstEOF, secondEOF;
	while (-1 != (firstEOF = getValidHexCharacter(&c1, ignoreInputErrors))
			&& -1 != (secondEOF = getValidHexCharacter(&c2, ignoreInputErrors))) {
		c1 = hexChar2Dec(c1);
		c2 = hexChar2Dec(c2);
		// put these bits into a single byte
		byte = c2 | (c1 << 4);
		processBits(&mode, &leftover, byte);
	}
	if (-1 != firstEOF && -1 == secondEOF) {
		fprintf(stderr,
				"Must provide an even number of hexadecimal characters.\n");
		return (EXIT_FAILURE);
	}
	processLastBits(&mode, &leftover, outputPadding);
	putchar('\n');
	return (EXIT_SUCCESS);
}

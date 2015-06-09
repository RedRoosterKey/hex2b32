/**
 * @file    hex2b32.c
 * @author  RedRoosterKey
 * @version 0.0.1
 *
 * @section LICENSE
 *
 * GNU GENERAL PUBLIC LICENSE, Version 2, June 1991
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * https://www.gnu.org/copyleft/gpl.html
 *
 * @section DESCRIPTION
 *
 * Convert hexadecimal into base32 according to RFC 3548.
 *
 * https://tools.ietf.org/html/rfc3548
 *
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
#include <ctype.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

const char * VERSION = "0.0.1";

const char * HELP =
		"Usage: hex2b32 [OPTION]... \n\
Inputs hexadecimal data from STDIN and outputs base32 (RFC 3548) to STDOUT\n\
\n\
    -e, --input-errors    display first input error and exit with failure\n\
                          (default behavior is to ignore invalid input)\n\
    -h, --help            display this help message and exit\n\
    -l, --lower           output only lower case letters\n\
                          (default behavior is all upper case)\n\
    -n, --no-padding      omit trailing '=' symbols\n\
    -v, --version         output version information and exit\n";

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

/**
 * Tries to grab a valid hexadecimal character from STDIN.
 *
 * Will store the valid hex character in *ch
 *
 * If a non hexadecimal value is retrieved and ignoreInputErrors is set to
 * false, the function will output a message to STDERR and cause the program
 * to exit with a failure
 *
 * @param ch a pointer to the memory space where the character will be set
 * @param ignoreInputErrors a flag to indicate if the function should ignore
 *        input errors or cause the program to fail
 * @return 0 if ch is set to a valid hexadecimal character
 *        -1 if we can only get EOF from STDIN
 */
short getValidHexCharacter(char * const ch, const bool ignoreInputErrors) {
	short input;
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

/**
 * Converts a valid hexadecimal character into the decimal equivalent
 *
 * Will return -1 if ch is not a valid hexadecimal character
 *
 * @param ch the valid hexadecimal character to convert
 * @return The decimal equivalent of hexadecimal ch or -1 otherwise
 */
short hexChar2Dec(char ch) {
	ch = toupper(ch);
	if ('A' <= ch && 'F' >= ch) {
		return (ch - 'A' + 10);
	} else if ('0' <= ch && '9' >= ch) {
		return (ch - '0');
	}
	return (-1);
}

/**
 * Convert the byte and the leftover into as many base32 characters as possible
 *
 * @param mode a value indicating how many bits are usable in the leftover.
 *        This value will be updated when the function is called.
 * @param leftover contains any leftover bits from the previous operation
 *        and any unconverted bits after this call will be stored in leftover
 * @param byte the data to convert (as much as possible) into base32 characters.
 *        Any bits that are not converted will be stored in leftover.
 * @param upperCase a flag indicating if the output should be in upper
 *        or lower case
 * @return void
 */
void processBits(RemainderMode * const mode, unsigned char * const leftover,
		const unsigned char byte, const bool upperCase) {
	unsigned char index = 0;
	switch (*mode) {
	case NO_BITS_LEFT:
		index = (FIRST_FIVE_BITS & byte) >> 3;
		putchar(
				upperCase ?
						BASE_32[(int) index] : tolower(BASE_32[(int) index]));
		*leftover = byte << 5;
		*mode = THREE_BITS_LEFT;
		break;
	case THREE_BITS_LEFT:
		index = ((FIRST_THREE_BITS & *leftover) >> 3)
				| ((FIRST_TWO_BITS & byte) >> 6);
		putchar(
				upperCase ?
						BASE_32[(int) index] : tolower(BASE_32[(int) index]));
		index = (THIRD_TO_SEVENTH_BITS & byte) >> 1;
		putchar(
				upperCase ?
						BASE_32[(int) index] : tolower(BASE_32[(int) index]));
		*leftover = byte << 7;
		*mode = ONE_BIT_LEFT;
		break;
	case ONE_BIT_LEFT:
		index = ((FIRST_BIT & *leftover) >> 3)
				| ((FIRST_FOUR_BITS & byte) >> 4);
		putchar(
				upperCase ?
						BASE_32[(int) index] : tolower(BASE_32[(int) index]));
		*leftover = byte << 4;
		*mode = FOUR_BITS_LEFT;
		break;
	case FOUR_BITS_LEFT:
		index = ((FIRST_FOUR_BITS & *leftover) >> 3)
				| ((FIRST_BIT & byte) >> 7);
		putchar(
				upperCase ?
						BASE_32[(int) index] : tolower(BASE_32[(int) index]));
		index = (SECOND_TO_SIXTH_BITS & byte) >> 2;
		putchar(
				upperCase ?
						BASE_32[(int) index] : tolower(BASE_32[(int) index]));
		*leftover = byte << 6;
		*mode = TWO_BITS_LEFT;
		break;
	case TWO_BITS_LEFT:
		index = ((FIRST_TWO_BITS & *leftover) >> 3)
				| ((FIRST_THREE_BITS & byte) >> 5);
		putchar(
				upperCase ?
						BASE_32[(int) index] : tolower(BASE_32[(int) index]));
		index = (LAST_FIVE_BITS & byte);
		putchar(
				upperCase ?
						BASE_32[(int) index] : tolower(BASE_32[(int) index]));
		*leftover = 0;
		*mode = NO_BITS_LEFT;
		break;
	}
}

/**
 * Convert the leftover bits into a base32 character
 *
 * @param mode a value indicating how many bits are usable in the leftover.
 *        This value will be updated when the function is called.
 * @param leftover contains any leftover bits from the previous conversion
 * @param padding a flag indicating if the output should have '=' padding
 * @param upperCase a flag indicating if the output should be in upper
 *        or lower case
 * @return void
 */
void processLastBits(const RemainderMode * const mode,
		const unsigned char * const leftover, const bool padding,
		const bool upperCase) {
	unsigned char index = 0;
	switch (*mode) {
	case NO_BITS_LEFT:
		break;
	case THREE_BITS_LEFT:
		index = ((FIRST_THREE_BITS & *leftover) >> 5 << 2);
		putchar(
				upperCase ?
						BASE_32[(int) index] : tolower(BASE_32[(int) index]));
		if (padding)
			printf("======");
		break;
	case ONE_BIT_LEFT:
		index = ((FIRST_BIT & *leftover) >> 7 << 4);
		putchar(
				upperCase ?
						BASE_32[(int) index] : tolower(BASE_32[(int) index]));
		if (padding)
			printf("====");
		break;
	case FOUR_BITS_LEFT:
		index = ((FIRST_FOUR_BITS & *leftover) >> 4 << 1);
		putchar(
				upperCase ?
						BASE_32[(int) index] : tolower(BASE_32[(int) index]));
		if (padding)
			printf("===");
		break;
	case TWO_BITS_LEFT:
		index = ((FIRST_TWO_BITS & *leftover) >> 6 << 3);
		putchar(
				upperCase ?
						BASE_32[(int) index] : tolower(BASE_32[(int) index]));
		if (padding)
			putchar('=');
		break;
	}
}

/**
 * Main!  Handles program arguments and general execution
 *
 * @param argc the number of program arguments
 * @param argv the program arguments
 * @return 0 for success, any other value indicates failure
 */
int main(int argc, char **argv) {
	bool ignoreInputErrors = true;
	bool upperCase = true;
	bool outputPadding = true;
	static const struct option long_options[] = { { "input-errors", no_argument,
			0, 'e' }, { "help", no_argument, 0, 'h' }, { "lower", no_argument,
			0, 'l' }, { "no-padding", no_argument, 0, 'n' }, { "version",
			no_argument, 0, 'v' }, { 0, 0, 0, 0 } };

	// Handle command line options
	while (1) {
		int option_index = 0;
		short option = getopt_long(argc, argv, "ehlnv", long_options,
				&option_index);
		if (option == -1)
			break;
		switch (option) {
		case 'e':
			ignoreInputErrors = false;
			break;
		case 'h':
			printf("%s\n", HELP);
			return (EXIT_SUCCESS);
			break;
		case 'l':
			upperCase = false;
			break;
		case 'n':
			outputPadding = false;
			break;
		case 'v':
			printf("%s\n", VERSION);
			return (EXIT_SUCCESS);
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
	char c1, c2;
	c1 = c2 = 0;
	unsigned char byte, leftover;
	RemainderMode mode = NO_BITS_LEFT;
	short firstEOF, secondEOF;
	while (-1 != (firstEOF = getValidHexCharacter(&c1, ignoreInputErrors))
			&& -1 != (secondEOF = getValidHexCharacter(&c2, ignoreInputErrors))) {
		c1 = hexChar2Dec(c1);
		c2 = hexChar2Dec(c2);
		// put these bits into a single byte
		byte = c2 | (c1 << 4);
		processBits(&mode, &leftover, byte, upperCase);
	}
	if (-1 != firstEOF && -1 == secondEOF) {
		fprintf(stderr,
				"Must provide an even number of hexadecimal characters.\n");
		return (EXIT_FAILURE);
	}
	processLastBits(&mode, &leftover, outputPadding, upperCase);
	putchar('\n');
	return (EXIT_SUCCESS);
}
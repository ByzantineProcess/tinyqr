/*	Simple QR Code Generator
*	by B.J. Guillot (bguillot@acm.org)
*	Copyright (C) 2016 joyteq LLC
*	All Rights Reserved
*	Released under MIT License
*	Version 1.0
*	2016-07-01
*/

/*
*	I wanted to learn more about QR codes and figure out how to generate them from scratch.
*	From scratch meaning, without having to use any third party libraries.  Not even a PNG graphics library.
*	I decided to use C for the language to be as bare-bones as possible so that I would later be able
*	to easily code it in assembly language on retrocomputers.
*
*	To simplify the project, several limitations are in place:
*		1. Hard-coded to use Quality Mode “Q” (QR has Low, Medium, “Q”, and High Error Correction, Q is the 2nd best).  Felt “Q” (“Medium-High”) was the best compromise.
*		2. Hard-coded to use encoding scheme for 8-bit byte data (QR has numeric-only, alphanumeric-only, Byte, and Kanji).  Felt Byte was the most useful.
*		3. Hard-coded to use Mask 1 (the bits of every even row are flipped).  This is probably a bad thing and is against the QR specification.
*			You are supposed to try each of the 8 available mask types, calculate a “penalty score” for each mask, and then, for the final rendering,
*			use the mask that had the lowest penalty.  The penalty is based on number of consecutive blocks having same color, etc.
*			It’s designed to make it easier on scanning equipment.  In reality, I don’t think it matters—everything generated by my program should be a
*			valid QR, it may just not be the “best” QR possible.
* 
*	Miscellaneous Notes:
*		1. Maximum message size using 8-bit Byte encoding with Version 40-Q is 1,663 bytes. 
*		2. Message size after Reed-Solomon error correction for a 40-Q message is 3,706 bytes.
*		3. Smallest QR Code (“Version 1”) is 21x21 pixels.           
*		4. Largest QR Code (“Version 40”) is 177x177 pixels.
*		5. The program, as it is, simply runs several test cases, generating a png for each QR code version (V1, V2, up to V40).
*		6. The first two calls to parseMessage() actually check results against a hard-coded test vector in the code to verify correctness.
*			The others can be verified by displaying the resulting PNG file and scanning it with a QR code reader app.
*
*	The two Android apps I used for testing:
*		1. QR Scanner from Kaspersky Lab (version 1.1.0.228).
*		2. QR Droid (version 6.6)
*	In general, QR Droid’s default scanning engine (“Zapper”), seemed to be able to scan the generated QR codes faster than QR Scanner; 
*	however, above QR Version 20, the Zapper engine seemed to be unable to capture data, and I had to switch over to the alternate “ZXing” engine that isn’t quite as fast.
* 
*	Learned how to build QR codes by following the very detailed Thonky.com Tutorial
*		http://www.thonky.co...-code-tutorial/
*
*	Compiled and tested on Mac OS X El Capitan 10.11.5.
*/

#include <stdint.h>	//for int16_t
#include <string.h>	//for strlen
#include <stdlib.h>	//for malloc, free
#include "qr.h"
#include "png_create.c" //for png_create

//-----------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------

// Reed-Solomon error corretion looks very scary on th Thonky.com website, but after reading pages and pages of their examples and explanations,
// it turns out the entire algorithm could be reduced to some relatively simple loops after quite a bit of trial-and-error.

void reedSolomon(int16_t data_codewords, int16_t data_offset, unsigned char message[], int16_t error_codewords, unsigned char errorcode[], unsigned char generator[]) {
	for (int16_t i=0; i < data_codewords; i++)
		errorcode[i] = message[i + data_offset];

	for (int16_t i=data_codewords; i < error_codewords; i++) // if error codewords > data codewords, need to initialize enough zeros for the math
		errorcode[i] = 0;
	//printArrayBYTE("init: ", error_codewords, errorcode);

	for (int16_t j=1; j <= data_codewords; j++) {
		int16_t lead_term = a_inv[errorcode[0]];
		// // printf("INFO: errorcode[0]=%d\n", errorcode[0]);
		if (errorcode[0] != 0) {
			for (int16_t i=1; i <= error_codewords; i++) {
				unsigned char temp_value = 0;
				if (i < error_codewords) temp_value = errorcode[i];
				// // printf("(generator[i-1] + lead_term) % 255 = %d\n", (generator[i-1] + lead_term) % 255);
				errorcode[i-1] = temp_value ^ a[(generator[i-1] + lead_term) % 255];
			}
		} else { // polynomial division step is greatly simiplified (just a shift of all terms left) if leading coeff. is zero
			for (int16_t i=1; i <= error_codewords; i++)
				errorcode[i-1] = errorcode[i];
		}

		for (int16_t i=error_codewords+1; i <= data_codewords; i++) {
			errorcode[i-1] = errorcode[i];
		}
		//printArrayBYTE("iter: ", error_codewords, errorcode);

	}
}

//-----------------------------------------------------------------------------------------------------------------

int is_mask_applicable(int16_t row, int16_t column, unsigned char mask_number) {
	return ((row % 2) == 0);
}

void parseMessage(char* filename, const char* freetext, unsigned char test_vector[]) {

	unsigned char message[244] = {0};  // 244 valid up to VERSION 13-Q, 1666 valid up to VERSION 40-Q
	int16_t message_length = strlen(freetext);
	// printf("INFO: message=[%s]\n", freetext);
	// printf("INFO: len of message=%d\n", message_length);
	//printArrayBYTE("unencoded input", message_length, (unsigned char*)&freetext[0]);

	int16_t qr_version = 5;

	unsigned char* message_parameters = codeword_parameters;
	int message_index = 0;

	message[message_index] = 64; // "0100" Byte Encoding
	message[message_index++] |= ((message_length & 240) >> 4);
	
	message[message_index++] = ((message_length & 15) << 4) | ((freetext[0] & 240) >> 4);

	for (int16_t i=0; i < message_length; i++)		
		message[message_index++] = ((freetext[i] & 15) << 4) | ((freetext[i+1] & 240) >> 4);

	{
		unsigned char pad[] = {236, 17};
		uint16_t pad_index = 0;
		uint16_t needed_pad_bytes = 76 - message_index;
		// printf("INFO: needed pad bytes: %d\n", needed_pad_bytes);
		for (uint16_t i=0; i < needed_pad_bytes; i++) {
			message[message_index++] = pad[pad_index];
			pad_index ^= 1;
		}
	}

	//printArrayBYTE("encoded input (with padding)", message_index, message);

	int16_t error_codewords = message_parameters[0];

	unsigned char errorcode[30] = {0}; // 30 is highest EC count for Q-quality; 25 is highest dataword count for Q-quality

	int16_t total_blocks = message_parameters[1] + message_parameters[3];
	unsigned char interleaved_output[532] = {0}; // 532 valid up to VERSION 13-Q; 3706 valid up to VERSION 40-Q

	int16_t message_offset = 0;
	int16_t block_number = 0;
	for (int16_t groups=0; groups < 2; groups++) {
		int16_t num_blocks = message_parameters[groups*2+1];
		int16_t data_codewords = message_parameters[groups*2+2];

		for (int16_t blocks=0; blocks < num_blocks; blocks++) {
			reedSolomon(data_codewords, message_offset, message, error_codewords, errorcode, &gen_poly[gen_offset[message_parameters[0]-13] - 73]);

			//// printf("REED OUTPUT: data_codewords=%d,  message_offset=%d,  error_codewords=%d,  gen_offset=%d\n", data_codewords, message_offset, error_codewords, gen_offset[message_parameters[0]-13]);
			//printArrayBYTEwithOffset("Data Codewords: ", data_codewords, message, message_offset);
			//printArrayBYTE("Error Codewords: ", error_codewords, errorcode);
			int16_t interleaved_output_offset = block_number;
			for (int16_t i=0; i < data_codewords; i++) {
				interleaved_output[interleaved_output_offset] = message[i + message_offset];

				if (i+1 < message_parameters[2]) // { 18, 2, 15, 2, 16}
					interleaved_output_offset += message_parameters[1];
				if (i+1 < message_parameters[4])
					interleaved_output_offset += message_parameters[3];
			}

			interleaved_output_offset = message_parameters[1] * message_parameters[2] + message_parameters[3] * message_parameters[4] + block_number;
			for (int16_t i=0; i < error_codewords; i++) {
				interleaved_output[interleaved_output_offset] = errorcode[i];
				interleaved_output_offset += total_blocks;
			}

			message_offset += data_codewords;
			block_number++;
		}
		//printArrayBYTE("output: ", 346, interleaved_output);
	}

	int16_t output_size = 172;
	// printf("INFO: total output_size=%d bytes\n", output_size);

	int16_t max_pixels = 41;
	// printf("INFO: pixel size=%d x %d\n", max_pixels, max_pixels);

		unsigned char **image = (unsigned char **)malloc(max_pixels * sizeof(unsigned char*)); //rows
        if (image == 0) {
                // printf("\a!!! ERROR !!! Out of memory during first malloc\n");
                return;
        }
        for (int i=0; i < max_pixels; i++) {
				image[i] = (unsigned char *)malloc(max_pixels); //columns
                if (image[i] == 0) {
                        // printf("\a!!! ERROR !!! Out of memory during second malloc\n");
                        return;
                }
        }

	for (int16_t i=0; i < max_pixels; i++) {
		for (int16_t j=0; j < max_pixels; j++) {
			image[i][j] = 255; // set all pixels to white
		}
	}

	// add the three finder pattern modules to the qr code
	int16_t finder_pattern = (qr_version*4)+14;
	for (int16_t i = 0; i < 7; i++) {
		image[0][i] = 0; //top left module
		image[6][i] = 0;
		image[0][finder_pattern+i] = 0; //top right module
		image[6][finder_pattern+i] = 0;
		image[finder_pattern][i] = 0; //bottom left module
		image[max_pixels-1][i] = 0;
	}
	for (int16_t i = 1; i < 6; i++) {
		image[i][0] = 0; //top left module
		image[i][6] = 0;
		image[i][finder_pattern] = 0; //top right module
		image[i][max_pixels-1] = 0;
		image[finder_pattern+i][0] = 0; //bottom left module
		image[finder_pattern+i][6] = 0;
	}
	for (int16_t i = 2; i < 5; i++) {
		for (int16_t j = 0; j < 3; j++) {
			image[2+j][i] = 0;
			image[2+j][i+finder_pattern] = 0;
			image[finder_pattern+2+j][i] = 0;
		}
	}

	//insert alignment patterns
	unsigned char center[7] = {0};
	center[0] = 6;
	for (int16_t i=1; i < 7; i++)
		center[i] = message_parameters[5+i];

	for (int16_t i=0; i < 7; i++) {
		for (int16_t j=0; j < 7; j++) {
			if ((center[i] != 0) && (center[j] != 0)) {
				//// printf("coord=(%d,%d)\n", center[i], center[j]);
				if (image[center[i]][center[j]] == 255) { //only add if bit is currently white
					image[center[i]][center[j]] = 0;
					for (int16_t k=0; k < 5; k++) {
						image[center[i]-2][center[j]-2+k] = 0;
						image[center[i]+2][center[j]-2+k] = 0;
					}
					for (int16_t k=0; k < 3; k++) {
						image[center[i]-1+k][center[j]-2] = 0;
						image[center[i]-1+k][center[j]+2] = 0;
					}
				}
			}
		}
	}

	

	//adding timing patterns
	for (int16_t i=8; i < max_pixels - 8; i+=2) {
		image[6][i] = 0;
		image[i][6] = 0;
	}

	//add the "dark module"
	image[33][8] = 0;

	unsigned char mask_number = 1;
	// printf("INFO: using mask %d\n", mask_number);

	//apply mask format info
	{
		int16_t mask = mask_info[mask_number-1];
		int16_t skip = 0;
		for (int16_t i=0; i < 8; i++) {
			if (i == 6) skip=1;
			if ((mask & 1) > 0) {
				image[8][max_pixels-i-1] = 0;
				image[i+skip][8] = 0;
			}
			mask = mask >> 1;
		}

		skip = 0;
		for (int16_t i=0; i < 7; i++) {
			if (i == 1) skip= -1;
			if ((mask & 1) > 0) {
				image[max_pixels-7+i][8] = 0;
				image[8][7-i+skip] = 0;
			}
			mask = mask >> 1;
		}
	}

	//data fill
	int16_t y = max_pixels-1;
	int16_t x = max_pixels-1;
	int16_t dir = -1;

	int16_t primary_bits = output_size * 8;
	int16_t remainder_bits = message_parameters[5];

	// printf("INFO: primary bits=%d  remainder bits=%d\n", primary_bits, remainder_bits);

	unsigned char working_byte = 0;
	int16_t interleaved_index = -1;

	for (int i=0; i < primary_bits + remainder_bits; i++) {

		if (image[y][x] == 0) { // check for alignment marker hit
			if (image[y][x-1] == 0) //hit alignment marker head=-on, skip over it
				y = y + dir*5;
			else {  // hit left-hand edge of alignment marker, handle special case
				x = x - 1;
				for (int j=0; j < 5; j++) {
					if (y != 6) { //skip over horitzonal timing line
						if (i % 8 == 0) { working_byte = interleaved_output[++interleaved_index]; } else { working_byte = working_byte << 1; }
						if ((working_byte & 128) > 0) image[y][x] = 0;
						if (is_mask_applicable(y, x, mask_number)) image[y][x]=~image[y][x];
	
						i++;
					}
					y = y + dir;
				}
				x = x + 1;
			}
		}

		if (i < primary_bits) {
			if (i % 8 == 0) { working_byte = interleaved_output[++interleaved_index]; } else { working_byte = working_byte << 1; }
			if ((working_byte & 128) > 0) image[y][x] = 0;
		}
		if (is_mask_applicable(y, x, mask_number)) image[y][x]=~image[y][x]; // handle masking for primary or remainder bit

		i++;
		x = x - 1;

		if (i < primary_bits) {
			if (i % 8 == 0) { working_byte = interleaved_output[++interleaved_index]; } else { working_byte = working_byte << 1; }
			if ((working_byte & 128) > 0) image[y][x] = 0;
		}
		if (is_mask_applicable(y, x, mask_number)) image[y][x]=~image[y][x];

		y = y + dir;
		x = x + 1;

		if (((x < 9) && (y == 8)) || ((x > max_pixels-8) && (y == 8)) || (y < 0)) { // hit top-left or top-right finder patterns
			dir = +1;
			y = y + 1;
			x = x - 2;
		} else if ((x == 10) && (y == max_pixels)) { //hit bottom row around "dark module"
			dir = -1;
			y = max_pixels - 9;
			x = x - 2;
		} else if (y == max_pixels) { // hit bottom row
			dir = -1;
			y = max_pixels - 1;
			x = x - 2;
		} else if ((x < 10) && (y > max_pixels-9)) { //hit bottom-left finder pattern (near dark module)
			dir = -1;
			y = max_pixels - 9;
			x = x - 2;
		}

		if (y == 6) //skip vertical timing lines
			y += dir;
		else if (x == 6) //skip horizontal timing line
			x = x-1;
	}

	//-----------------------------------

	img_create(max_pixels, max_pixels, image, filename, 4);
	// printf("INFO: filename=[%s]\n\n", filename);

	//deallocate memory
	for (int i=0; i < max_pixels; i++)
		free(image[i]);
	free(image);

}

//-----------------------------------------------------------------------------------------------------------------
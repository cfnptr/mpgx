/*********************************************************************
* Filename:   md5.h
* Authors:    Brad Conte (brad AT bradconte.com), Nikita Fediuchin
* Copyright:
* Disclaimer: This code is presented "as is" without any guarantees.
* Details:    Defines the API for the corresponding MD5 implementation.
*********************************************************************/

#pragma once

/*************************** HEADER FILES ***************************/
#include <stdint.h>

/****************************** MACROS ******************************/
#define MD5_BLOCK_SIZE 16               // MD5 outputs a 16 byte digest

typedef struct {
	uint8_t data[64];
	uint32_t dataLength;
	uint64_t bitLength;
	uint32_t state[4];
} MD5_CONTEXT;

/*********************** FUNCTION DECLARATIONS **********************/
void md5_init(MD5_CONTEXT* context);
void md5_update(MD5_CONTEXT* context, const uint8_t data[], size_t length);
void md5_final(MD5_CONTEXT* context, uint8_t hash[]);

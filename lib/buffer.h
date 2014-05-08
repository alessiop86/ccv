//
//  buffer.h
//  jpcnn
//
//  Created by Peter Warden on 1/9/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#ifndef INCLUDE_BUFFER_H
#define INCLUDE_BUFFER_H

#include "binary_format.h"

SBinaryTag* buffer_to_tag_dict(float* bufferData, int* dims, int dimsLength, int bitsPerElement, int floatBits);
void quantize_buffer(float* bufferData, int elementCount, int howManyBits, float* outMin, float* outMax, void** outData, size_t* outSizeofData);
void transpose_buffer(float* bufferData, int* dims, int dimsLength);
int element_count_from_dims(int* dims, int dimsLength);
void buffer_dump_to_file(float* bufferData, int* dims, int dimsLength, int bitsPerElement, const char* filename);
void print_float_array(float* bufferData, int elementCount);

#endif // INCLUDE_BUFFER_H

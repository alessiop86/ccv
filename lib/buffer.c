//
//  bbuffer.cpp
//  jpcnn
//
//  Created by Peter Warden on 1/9/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#include "buffer.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <float.h>

#include "binary_format.h"

SBinaryTag* buffer_to_tag_dict(float* bufferData, int* dims, int dimsLength, int bitsPerElement, int floatBits) {
  SBinaryTag* mainDict = create_dict_tag();
  mainDict = add_uint_to_dict(mainDict, "float_bits", floatBits);

  assert((floatBits == 32) || (floatBits == 16) || (floatBits == 8));

  SBinaryTag* dimsTag = create_list_tag();
  int elementCount = element_count_from_dims(dims, dimsLength);
  for (int index = 0; index < dimsLength; index += 1) {
    dimsTag = add_uint_to_list(dimsTag, dims[index]);
  }
  mainDict = add_tag_to_dict(mainDict, "dims", dimsTag);
  free(dimsTag);

  if ((floatBits == 8) || (floatBits == 16)) {
    float min;
    float max;
    void* quantizedData;
    size_t sizeofQuantizedData;
    if (bitsPerElement == 32) {
      quantize_buffer(bufferData, elementCount, floatBits, &min, &max, &quantizedData, &sizeofQuantizedData);
    } else {
      assert(floatBits == bitsPerElement);
      quantizedData = bufferData;
      min = 0.0f;
      max = 1.0f;
      const int bytesPerElement = (bitsPerElement / 8);
      sizeofQuantizedData = (bytesPerElement * elementCount);
    }

    mainDict = add_float_to_dict(mainDict, "min", min);
    mainDict = add_float_to_dict(mainDict, "max", max);
    mainDict = add_blob_to_dict(mainDict, "quantized_data", quantizedData, (int)sizeofQuantizedData);

    if (bitsPerElement == 32) {
      free(quantizedData);
    }
  } else if (floatBits == 32) {
    mainDict = add_float_array_to_dict(mainDict, "data", bufferData, elementCount);
  }

  return mainDict;
}

void quantize_buffer(float* bufferData, int elementCount, int howManyBits, float* outMin, float* outMax, void** outData, size_t* outSizeofData) {
  assert((howManyBits == 8) || (howManyBits == 16));

  float min = FLT_MAX;
  float max = -FLT_MAX;

  float* dataStart = bufferData;
  float* dataEnd = (bufferData + elementCount);
  float* data = dataStart;
  while (data < dataEnd) {
    const float value = *data;
    min = fminf(min, value);
    max = fmaxf(max, value);
    data += 1;
  }

  const int levels = (1 << howManyBits);

  const float spread = ((max - min) / levels);
  const float recipSpread = (1.0f / fmaxf(0.00000001f, spread));

  if (howManyBits == 8) {
    const size_t sizeofQuantizedData = elementCount * sizeof(uint8_t);
    uint8_t* quantizedDataStart = (uint8_t*)(malloc(sizeofQuantizedData));
    data = dataStart;
    uint8_t* quantizedData = quantizedDataStart;
    while (data < dataEnd) {
      const float value = *data;
      int quantized = (int)roundf((value - min) * recipSpread);
      if (quantized < 0) {
        quantized = 0;
      } else if (quantized >= levels) {
        quantized = (levels - 1);
      }
      *quantizedData = (uint8_t)(quantized);
      data += 1;
      quantizedData += 1;
    }
    *outData = quantizedDataStart;
    *outSizeofData = sizeofQuantizedData;
  } else if (howManyBits == 16) {
    const size_t sizeofQuantizedData = elementCount * sizeof(uint16_t);
    uint16_t* quantizedDataStart = (uint16_t*)(malloc(sizeofQuantizedData));
    data = dataStart;
    uint16_t* quantizedData = quantizedDataStart;
    while (data < dataEnd) {
      const float value = *data;
      int quantized = (int)roundf((value - min) * recipSpread);
      if (quantized < 0) {
        quantized = 0;
      } else if (quantized >= levels) {
        quantized = (levels - 1);
      }
      *quantizedData = (uint16_t)(quantized);
      data += 1;
      quantizedData += 1;
    }
    *outData = quantizedDataStart;
    *outSizeofData = sizeofQuantizedData;
  } else {
    assert(0); // should never get here
    *outData = NULL;
    *outSizeofData = 0;
  }
  *outMin = min;
  *outMax = max;
}

void transpose_buffer(float* bufferData, int* dims, int dimsLength) {
  assert(dimsLength == 2);
  int originalHeight = dims[0];
  int originalWidth = dims[1];
  int elementCount = element_count_from_dims(dims, dimsLength);
  int byteCount = (elementCount * sizeof(float));
  float* swapData = (float*)(malloc(byteCount));
  memcpy(swapData, bufferData, byteCount);
  float* originalData = swapData;
  float* newData = bufferData;
  for (int originalY = 0; originalY < originalHeight; originalY += 1) {
    for (int originalX = 0; originalX < originalWidth; originalX += 1) {
      float* source = (originalData + (originalY * originalWidth) + originalX);
      float* dest = (newData + (originalX * originalHeight) + originalY);
      *dest = *source;
    }
  }
  free(swapData);
  int dimsSwap = dims[0];
  dims[0] = dims[1];
  dims[1] = dimsSwap;
}

int element_count_from_dims(int* dims, int dimsLength) {
  int elementCount = 1;
  for (int index = 0; index < dimsLength; index += 1) {
    elementCount *= dims[index];
  }
  return elementCount;
}

void buffer_dump_to_file(float* bufferData, int* dims, int dimsLength, int bitsPerElement, const char* filename) {
  SBinaryTag* mainDict = buffer_to_tag_dict(bufferData, dims, dimsLength, bitsPerElement, bitsPerElement);
  FILE* outputFile = fopen(filename, "wb");
  assert(outputFile != NULL);
  fwrite(mainDict, (mainDict->length + 8), 1, outputFile);
  fclose(outputFile);
  free(mainDict);
}

void print_float_array(float* bufferData, int elementCount) {
  for (int index = 0; index < elementCount; index += 1) {
    fprintf(stderr, "%s%.8f", (index > 0)?", ":"", bufferData[index]);
  }
}
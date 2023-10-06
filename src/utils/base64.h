#pragma once
#include <stdio.h>
#include <stdint.h>
#include <string.h>

void base64_encode(const uint8_t *input, size_t input_len, char *output);
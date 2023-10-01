#pragma once
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "ch_hash.h"
#include "parsehttp.h"
#define INVALID_URL -1;
int url_parse_params(st_ht_stringbuffer *buffer, st_ch_hashmap *store);
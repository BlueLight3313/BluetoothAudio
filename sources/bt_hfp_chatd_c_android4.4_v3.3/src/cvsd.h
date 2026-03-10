#pragma once
#include <stdint.h>
#include <stddef.h>
typedef struct { int32_t integrator; int32_t step; int last_bit; } cvsd_t;
void cvsd_init(cvsd_t* c);
size_t cvsd_decode(const uint8_t* in_bits, size_t in_bytes, int16_t* out_pcm, size_t out_samples, cvsd_t* st);
size_t cvsd_encode(const int16_t* in_pcm, size_t in_samples, uint8_t* out_bits, size_t out_bytes, cvsd_t* st);

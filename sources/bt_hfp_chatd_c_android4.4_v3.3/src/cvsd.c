#include "cvsd.h"
#include <string.h>
static int32_t clamp32(int32_t v,int32_t lo,int32_t hi){ if(v<lo) return lo; if(v>hi) return hi; return v; }
static void adapt(cvsd_t* c,int bit){
    if(bit==c->last_bit) c->step=clamp32(c->step+(c->step>>3)+8,64,8192);
    else c->step=clamp32(c->step-(c->step>>4)-4,64,8192);
    c->last_bit=bit;
}
void cvsd_init(cvsd_t* c){ c->integrator=0; c->step=512; c->last_bit=0; }
size_t cvsd_decode(const uint8_t* in_bits,size_t in_bytes,int16_t* out_pcm,size_t out_samples,cvsd_t* st){
    size_t bits=in_bytes*8, pcm_possible=bits/8, pcm_write=out_samples<pcm_possible?out_samples:pcm_possible;
    size_t bit_idx=0;
    for(size_t i=0;i<pcm_write;i++){
        int32_t acc=0;
        for(int k=0;k<8;k++,bit_idx++){
            size_t byte_i=bit_idx/8; int shift=7-(int)(bit_idx%8);
            int bit=(in_bits[byte_i]>>shift)&1;
            st->integrator += bit?st->step:-st->step;
            adapt(st,bit);
            acc += st->integrator;
        }
        acc/=8; acc=clamp32(acc,-32768,32767);
        out_pcm[i]=(int16_t)acc;
    }
    return pcm_write;
}
size_t cvsd_encode(const int16_t* in_pcm,size_t in_samples,uint8_t* out_bits,size_t out_bytes,cvsd_t* st){
    size_t bits_needed=in_samples*8, bytes_needed=(bits_needed+7)/8, bytes_write=out_bytes<bytes_needed?out_bytes:bytes_needed;
    memset(out_bits,0,bytes_write);
    size_t bit_pos=0;
    for(size_t i=0;i<in_samples;i++){
        int32_t target=in_pcm[i];
        for(int k=0;k<8;k++,bit_pos++){
            if((bit_pos/8)>=bytes_write) break;
            int bit=(st->integrator<target)?1:0;
            st->integrator += bit?st->step:-st->step;
            adapt(st,bit);
            size_t byte_i=bit_pos/8; int shift=7-(int)(bit_pos%8);
            if(bit) out_bits[byte_i] |= (uint8_t)(1u<<shift);
        }
    }
    return bytes_write;
}

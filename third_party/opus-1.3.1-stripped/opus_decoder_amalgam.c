#define HAVE_CONFIG_H

#include "entcode.h"

static inline void ec_enc_bit_logp(ec_enc *_this, int _val, unsigned _logp) {}
static inline void ec_enc_uint(ec_enc *_this, opus_uint32 _fl, opus_uint32 _ft) {}
static inline void ec_encode_bin(ec_enc *_this, unsigned _fl, unsigned _fh, unsigned _bits) {}
static inline void ec_enc_bits(ec_enc *_this, opus_uint32 _fl, unsigned _bits) {}


#include "bands.c"
#include "celt.c"
#include "celt_decoder.c"
#include "cwrs.c"
#include "entcode.c"
#include "entdec.c"
#include "opus.c"
#include "opus_decoder.c"
#include "kiss_fft.c"
#include "laplace.c"
#include "mathops.c"
#include "mdct.c"
#include "modes.c"
#include "x86/pitch_sse.c"
#include "x86/x86cpu.c"
#include "quant_bands.c"
#include "rate.c"
#include "vq.c"


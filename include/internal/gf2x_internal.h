/* Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0"
 *
 * Written by Nir Drucker, Shay Gueron and Dusan Kostic,
 * AWS Cryptographic Algorithms Group.
 */

#pragma once

// For size_t
#include <stdlib.h>

#include "cpu_features.h"
#include "types.h"

// The size in quadwords of the operands in the gf2x_mul_base function
// for different implementations.
#define GF2X_PORT_BASE_QWORDS    (1)
#define GF2X_PCLMUL_BASE_QWORDS  (8)
#define GF2X_VPCLMUL_BASE_QWORDS (16)

// ------------------ FUNCTIONS NEEDED FOR GF2X MULTIPLICATION ------------------
// GF2X multiplication of a and b of size GF2X_BASE_QWORDS, c = a * b
void gf2x_mul_base_port(OUT uint64_t *c,
                        IN const uint64_t *a,
                        IN const uint64_t *b);
void karatzuba_add1_port(OUT uint64_t *alah,
                         OUT uint64_t *blbh,
                         IN const uint64_t *a,
                         IN const uint64_t *b,
                         IN const size_t    qwords_len);
void karatzuba_add2_port(OUT uint64_t *z,
                         IN const uint64_t *x,
                         IN const uint64_t *y,
                         IN const size_t    qwords_len);
void karatzuba_add3_port(OUT uint64_t *c,
                         IN const uint64_t *mid,
                         IN const size_t    qwords_len);

// -------------------- FUNCTIONS NEEDED FOR GF2X INVERSION --------------------
// c = a^2
void gf2x_sqr_port(OUT dbl_pad_r_t *c, IN const pad_r_t *a);
// The k-squaring function computes c = a^(2^k) % (x^r - 1),
// It is required by inversion, where l_param is derived from k.
void k_sqr_port(OUT pad_r_t *c, IN const pad_r_t *a, IN size_t l_param);
// c = a mod (x^r - 1)
void gf2x_red_port(OUT pad_r_t *c, IN const dbl_pad_r_t *a);

// AVX2 and AVX512 versions of the functions
#if defined(X86_64)
// ------------------ FUNCTIONS NEEDED FOR GF2X MULTIPLICATION ------------------
void gf2x_mul_base_pclmul(OUT uint64_t *c,
                          IN const uint64_t *a,
                          IN const uint64_t *b);
void gf2x_mul_base_vpclmul(OUT uint64_t *c,
                           IN const uint64_t *a,
                           IN const uint64_t *b);

void karatzuba_add1_avx2(OUT uint64_t *alah,
                         OUT uint64_t *blbh,
                         IN const uint64_t *a,
                         IN const uint64_t *b,
                         IN const size_t    qwords_len);
void karatzuba_add1_avx512(OUT uint64_t *alah,
                           OUT uint64_t *blbh,
                           IN const uint64_t *a,
                           IN const uint64_t *b,
                           IN const size_t    qwords_len);

void karatzuba_add2_avx2(OUT uint64_t *z,
                         IN const uint64_t *x,
                         IN const uint64_t *y,
                         IN const size_t    qwords_len);

void karatzuba_add2_avx512(OUT uint64_t *z,
                           IN const uint64_t *x,
                           IN const uint64_t *y,
                           IN const size_t    qwords_len);

void karatzuba_add3_avx2(OUT uint64_t *c,
                         IN const uint64_t *mid,
                         IN const size_t    qwords_len);
void karatzuba_add3_avx512(OUT uint64_t *c,
                           IN const uint64_t *mid,
                           IN const size_t    qwords_len);

// -------------------- FUNCTIONS NEEDED FOR GF2X INVERSION --------------------
// c = a^2
void gf2x_sqr_pclmul(OUT dbl_pad_r_t *c, IN const pad_r_t *a);
void gf2x_sqr_vpclmul(OUT dbl_pad_r_t *c, IN const pad_r_t *a);

// The k-squaring function computes c = a^(2^k) % (x^r - 1),
// It is required by inversion, where l_param is derived from k.
void k_sqr_avx2(OUT pad_r_t *c, IN const pad_r_t *a, IN size_t l_param);
void k_sqr_avx512(OUT pad_r_t *c, IN const pad_r_t *a, IN size_t l_param);

// c = a mod (x^r - 1)
void gf2x_red_avx2(OUT pad_r_t *c, IN const dbl_pad_r_t *a);
void gf2x_red_avx512(OUT pad_r_t *c, IN const dbl_pad_r_t *a);
#endif

// GF2X methods struct
typedef struct gf2x_ctx_st {
  size_t mul_base_qwords;
  void (*mul_base)(OUT uint64_t *c, IN const uint64_t *a, IN const uint64_t *b);
  void (*karatzuba_add1)(OUT uint64_t *alah,
                         OUT uint64_t *blbh,
                         IN const uint64_t *a,
                         IN const uint64_t *b,
                         IN const size_t    qwords_len);
  void (*karatzuba_add2)(OUT uint64_t *z,
                         IN const uint64_t *x,
                         IN const uint64_t *y,
                         IN const size_t    qwords_len);
  void (*karatzuba_add3)(OUT uint64_t *c,
                         IN const uint64_t *mid,
                         IN const size_t    qwords_len);

  void (*sqr)(OUT dbl_pad_r_t *c, IN const pad_r_t *a);
  void (*k_sqr)(OUT pad_r_t *c, IN const pad_r_t *a, IN size_t l_param);

  void (*red)(OUT pad_r_t *c, IN const dbl_pad_r_t *a);
} gf2x_ctx;

// Used in gf2x_inv.c to avoid initializing the context many times.
void gf2x_mod_mul_with_ctx(OUT pad_r_t *c,
                           IN const pad_r_t *a,
                           IN const pad_r_t *b,
                           IN const gf2x_ctx *ctx);

_INLINE_ void gf2x_ctx_init(gf2x_ctx *ctx)
{
#if defined(X86_64)
  if(is_avx512_enabled()) {
    ctx->karatzuba_add1 = karatzuba_add1_avx512;
    ctx->karatzuba_add2 = karatzuba_add2_avx512;
    ctx->karatzuba_add3 = karatzuba_add3_avx512;
    ctx->k_sqr          = k_sqr_avx512;
    ctx->red            = gf2x_red_avx512;
  } else if(is_avx2_enabled()) {
    ctx->karatzuba_add1 = karatzuba_add1_avx2;
    ctx->karatzuba_add2 = karatzuba_add2_avx2;
    ctx->karatzuba_add3 = karatzuba_add3_avx2;
    ctx->k_sqr          = k_sqr_avx2;
    ctx->red            = gf2x_red_avx2;
  } else
#endif
  {
    ctx->karatzuba_add1 = karatzuba_add1_port;
    ctx->karatzuba_add2 = karatzuba_add2_port;
    ctx->karatzuba_add3 = karatzuba_add3_port;
    ctx->k_sqr          = k_sqr_port;
    ctx->red            = gf2x_red_port;
  }

#if defined(X86_64)
  if(is_vpclmul_enabled()) {
    ctx->mul_base_qwords = GF2X_VPCLMUL_BASE_QWORDS;
    ctx->mul_base        = gf2x_mul_base_vpclmul;
    ctx->sqr             = gf2x_sqr_vpclmul;
  } else if(is_pclmul_enabled()) {
    ctx->mul_base_qwords = GF2X_PCLMUL_BASE_QWORDS;
    ctx->mul_base        = gf2x_mul_base_pclmul;
    ctx->sqr             = gf2x_sqr_pclmul;
  } else
#endif
  {
    ctx->mul_base_qwords = GF2X_PORT_BASE_QWORDS;
    ctx->mul_base        = gf2x_mul_base_port;
    ctx->sqr             = gf2x_sqr_port;
  }
}

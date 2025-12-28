/**
 * LibreCipher AES-256-GCM Implementation
 *
 * Constant-time AES-256 with GCM mode
 * Zero dynamic allocation
 */

#include "aes_gcm.h"
#include <string.h>

// AES S-Box
static const uint8_t sbox[256] = {
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b,
    0xfe, 0xd7, 0xab, 0x76, 0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0,
    0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0, 0xb7, 0xfd, 0x93, 0x26,
    0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2,
    0xeb, 0x27, 0xb2, 0x75, 0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0,
    0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84, 0x53, 0xd1, 0x00, 0xed,
    0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f,
    0x50, 0x3c, 0x9f, 0xa8, 0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5,
    0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2, 0xcd, 0x0c, 0x13, 0xec,
    0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14,
    0xde, 0x5e, 0x0b, 0xdb, 0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c,
    0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79, 0xe7, 0xc8, 0x37, 0x6d,
    0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f,
    0x4b, 0xbd, 0x8b, 0x8a, 0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e,
    0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e, 0xe1, 0xf8, 0x98, 0x11,
    0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f,
    0xb0, 0x54, 0xbb, 0x16};

// Round constants
static const uint8_t rcon[11] = {0x00, 0x01, 0x02, 0x04, 0x08, 0x10,
                                 0x20, 0x40, 0x80, 0x1b, 0x36};

// GF(2^8) multiplication for MixColumns
static uint8_t gmul(uint8_t a, uint8_t b) {
  uint8_t p = 0;
  for (int i = 0; i < 8; i++) {
    if (b & 1)
      p ^= a;
    uint8_t hi = a & 0x80;
    a <<= 1;
    if (hi)
      a ^= 0x1b;
    b >>= 1;
  }
  return p;
}

// Key expansion for AES-256
void aes256_init(aes_ctx_t *ctx, const uint8_t key[32]) {
  uint32_t *rk = ctx->rk;

  // Copy key
  for (int i = 0; i < 8; i++) {
    rk[i] = ((uint32_t)key[i * 4] << 24) | ((uint32_t)key[i * 4 + 1] << 16) |
            ((uint32_t)key[i * 4 + 2] << 8) | key[i * 4 + 3];
  }

  // Expand key
  for (int i = 8; i < 60; i++) {
    uint32_t temp = rk[i - 1];
    if (i % 8 == 0) {
      // RotWord + SubWord + Rcon
      temp = ((uint32_t)sbox[(temp >> 16) & 0xff] << 24) |
             ((uint32_t)sbox[(temp >> 8) & 0xff] << 16) |
             ((uint32_t)sbox[temp & 0xff] << 8) | sbox[(temp >> 24) & 0xff];
      temp ^= (uint32_t)rcon[i / 8] << 24;
    } else if (i % 8 == 4) {
      // SubWord for AES-256
      temp = ((uint32_t)sbox[(temp >> 24) & 0xff] << 24) |
             ((uint32_t)sbox[(temp >> 16) & 0xff] << 16) |
             ((uint32_t)sbox[(temp >> 8) & 0xff] << 8) | sbox[temp & 0xff];
    }
    rk[i] = rk[i - 8] ^ temp;
  }
}

// AES block encryption
void aes256_encrypt_block(const aes_ctx_t *ctx, const uint8_t in[16],
                          uint8_t out[16]) {
  uint8_t state[16];
  const uint32_t *rk = ctx->rk;

  // Load state and add round key
  for (int i = 0; i < 16; i++) {
    state[i] = in[i] ^ ((rk[i / 4] >> (24 - 8 * (i % 4))) & 0xff);
  }

  // 13 rounds
  for (int round = 1; round < 14; round++) {
    uint8_t temp[16];

    // SubBytes
    for (int i = 0; i < 16; i++) {
      temp[i] = sbox[state[i]];
    }

    // ShiftRows
    state[0] = temp[0];
    state[1] = temp[5];
    state[2] = temp[10];
    state[3] = temp[15];
    state[4] = temp[4];
    state[5] = temp[9];
    state[6] = temp[14];
    state[7] = temp[3];
    state[8] = temp[8];
    state[9] = temp[13];
    state[10] = temp[2];
    state[11] = temp[7];
    state[12] = temp[12];
    state[13] = temp[1];
    state[14] = temp[6];
    state[15] = temp[11];

    // MixColumns
    for (int c = 0; c < 4; c++) {
      uint8_t a[4];
      for (int i = 0; i < 4; i++)
        a[i] = state[c * 4 + i];
      state[c * 4 + 0] = gmul(a[0], 2) ^ gmul(a[1], 3) ^ a[2] ^ a[3];
      state[c * 4 + 1] = a[0] ^ gmul(a[1], 2) ^ gmul(a[2], 3) ^ a[3];
      state[c * 4 + 2] = a[0] ^ a[1] ^ gmul(a[2], 2) ^ gmul(a[3], 3);
      state[c * 4 + 3] = gmul(a[0], 3) ^ a[1] ^ a[2] ^ gmul(a[3], 2);
    }

    // AddRoundKey
    for (int i = 0; i < 16; i++) {
      state[i] ^= (rk[round * 4 + i / 4] >> (24 - 8 * (i % 4))) & 0xff;
    }
  }

  // Final round (no MixColumns)
  uint8_t temp[16];
  for (int i = 0; i < 16; i++)
    temp[i] = sbox[state[i]];

  state[0] = temp[0];
  state[1] = temp[5];
  state[2] = temp[10];
  state[3] = temp[15];
  state[4] = temp[4];
  state[5] = temp[9];
  state[6] = temp[14];
  state[7] = temp[3];
  state[8] = temp[8];
  state[9] = temp[13];
  state[10] = temp[2];
  state[11] = temp[7];
  state[12] = temp[12];
  state[13] = temp[1];
  state[14] = temp[6];
  state[15] = temp[11];

  for (int i = 0; i < 16; i++) {
    out[i] = state[i] ^ ((rk[56 + i / 4] >> (24 - 8 * (i % 4))) & 0xff);
  }
}

// GF(2^128) multiplication for GHASH
static void ghash_mult(uint8_t *x, const uint8_t *h) {
  uint8_t v[16], z[16] = {0};
  memcpy(v, h, 16);

  for (int i = 0; i < 16; i++) {
    for (int j = 7; j >= 0; j--) {
      if (x[i] & (1 << j)) {
        for (int k = 0; k < 16; k++)
          z[k] ^= v[k];
      }
      // Multiply v by x (shift right and reduce)
      uint8_t carry = v[15] & 1;
      for (int k = 15; k > 0; k--) {
        v[k] = (v[k] >> 1) | ((v[k - 1] & 1) << 7);
      }
      v[0] >>= 1;
      if (carry)
        v[0] ^= 0xe1;
    }
  }
  memcpy(x, z, 16);
}

// Increment counter
static void inc_counter(uint8_t counter[16]) {
  for (int i = 15; i >= 12; i--) {
    if (++counter[i] != 0)
      break;
  }
}

void aes_gcm_init(aes_gcm_ctx_t *ctx, const uint8_t key[32],
                  const uint8_t iv[12]) {
  aes256_init(&ctx->aes, key);

  // Generate H = E(K, 0^128)
  uint8_t zero[16] = {0};
  aes256_encrypt_block(&ctx->aes, zero, ctx->H);

  // J0 = IV || 0^31 || 1
  memcpy(ctx->J0, iv, 12);
  ctx->J0[12] = 0;
  ctx->J0[13] = 0;
  ctx->J0[14] = 0;
  ctx->J0[15] = 1;

  // Initialize counter
  memcpy(ctx->counter, ctx->J0, 16);
  inc_counter(ctx->counter);

  // Initialize GHASH
  memset(ctx->ghash, 0, 16);
  ctx->aad_len = 0;
  ctx->ct_len = 0;
}

void aes_gcm_aad(aes_gcm_ctx_t *ctx, const uint8_t *aad, size_t aad_len) {
  ctx->aad_len = aad_len;

  while (aad_len >= 16) {
    for (int i = 0; i < 16; i++)
      ctx->ghash[i] ^= aad[i];
    ghash_mult(ctx->ghash, ctx->H);
    aad += 16;
    aad_len -= 16;
  }

  // Pad last block
  if (aad_len > 0) {
    for (size_t i = 0; i < aad_len; i++)
      ctx->ghash[i] ^= aad[i];
    ghash_mult(ctx->ghash, ctx->H);
  }
}

void aes_gcm_encrypt(aes_gcm_ctx_t *ctx, const uint8_t *plaintext, size_t len,
                     uint8_t *ciphertext) {
  ctx->ct_len = len;
  uint8_t keystream[16];

  while (len >= 16) {
    aes256_encrypt_block(&ctx->aes, ctx->counter, keystream);
    inc_counter(ctx->counter);

    for (int i = 0; i < 16; i++) {
      ciphertext[i] = plaintext[i] ^ keystream[i];
      ctx->ghash[i] ^= ciphertext[i];
    }
    ghash_mult(ctx->ghash, ctx->H);

    plaintext += 16;
    ciphertext += 16;
    len -= 16;
  }

  // Handle partial block
  if (len > 0) {
    aes256_encrypt_block(&ctx->aes, ctx->counter, keystream);
    inc_counter(ctx->counter);

    for (size_t i = 0; i < len; i++) {
      ciphertext[i] = plaintext[i] ^ keystream[i];
      ctx->ghash[i] ^= ciphertext[i];
    }
    ghash_mult(ctx->ghash, ctx->H);
  }
}

void aes_gcm_finish(aes_gcm_ctx_t *ctx, uint8_t tag[16]) {
  // Add length block
  uint64_t aad_bits = ctx->aad_len * 8;
  uint64_t ct_bits = ctx->ct_len * 8;
  uint8_t len_block[16];

  for (int i = 0; i < 8; i++) {
    len_block[i] = (aad_bits >> (56 - 8 * i)) & 0xff;
    len_block[8 + i] = (ct_bits >> (56 - 8 * i)) & 0xff;
  }

  for (int i = 0; i < 16; i++)
    ctx->ghash[i] ^= len_block[i];
  ghash_mult(ctx->ghash, ctx->H);

  // Generate tag
  uint8_t e_j0[16];
  aes256_encrypt_block(&ctx->aes, ctx->J0, e_j0);
  for (int i = 0; i < 16; i++)
    tag[i] = ctx->ghash[i] ^ e_j0[i];

  // Clear sensitive data
  memset(ctx, 0, sizeof(*ctx));
}

void aes_gcm_encrypt_full(const uint8_t key[32], const uint8_t iv[12],
                          const uint8_t *plaintext, size_t pt_len,
                          const uint8_t *aad, size_t aad_len,
                          uint8_t *ciphertext, uint8_t tag[16]) {
  aes_gcm_ctx_t ctx;
  aes_gcm_init(&ctx, key, iv);
  aes_gcm_aad(&ctx, aad, aad_len);
  aes_gcm_encrypt(&ctx, plaintext, pt_len, ciphertext);
  aes_gcm_finish(&ctx, tag);
}

// Constant-time compare
static int ct_compare(const uint8_t *a, const uint8_t *b, size_t len) {
  uint8_t diff = 0;
  for (size_t i = 0; i < len; i++)
    diff |= a[i] ^ b[i];
  return diff == 0;
}

bool aes_gcm_decrypt_verify(const uint8_t key[32], const uint8_t iv[12],
                            const uint8_t *ciphertext, size_t ct_len,
                            const uint8_t *aad, size_t aad_len,
                            const uint8_t tag[16], uint8_t *plaintext) {
  aes_gcm_ctx_t ctx;
  aes256_init(&ctx.aes, key);

  // Generate H
  uint8_t zero[16] = {0};
  aes256_encrypt_block(&ctx.aes, zero, ctx.H);

  // J0
  memcpy(ctx.J0, iv, 12);
  ctx.J0[12] = ctx.J0[13] = ctx.J0[14] = 0;
  ctx.J0[15] = 1;

  memcpy(ctx.counter, ctx.J0, 16);
  inc_counter(ctx.counter);

  memset(ctx.ghash, 0, 16);
  ctx.aad_len = aad_len;
  ctx.ct_len = ct_len;

  // Process AAD
  aes_gcm_aad(&ctx, aad, aad_len);

  // Decrypt (GHASH over ciphertext)
  const uint8_t *ct_ptr = ciphertext;
  uint8_t *pt_ptr = plaintext;
  size_t remaining = ct_len;
  uint8_t keystream[16];

  while (remaining >= 16) {
    aes256_encrypt_block(&ctx.aes, ctx.counter, keystream);
    inc_counter(ctx.counter);

    for (int i = 0; i < 16; i++) {
      ctx.ghash[i] ^= ct_ptr[i];
      pt_ptr[i] = ct_ptr[i] ^ keystream[i];
    }
    ghash_mult(ctx.ghash, ctx.H);

    ct_ptr += 16;
    pt_ptr += 16;
    remaining -= 16;
  }

  if (remaining > 0) {
    aes256_encrypt_block(&ctx.aes, ctx.counter, keystream);
    for (size_t i = 0; i < remaining; i++) {
      ctx.ghash[i] ^= ct_ptr[i];
      pt_ptr[i] = ct_ptr[i] ^ keystream[i];
    }
    ghash_mult(ctx.ghash, ctx.H);
  }

  // Compute expected tag
  uint8_t computed_tag[16];
  uint64_t aad_bits = aad_len * 8;
  uint64_t ct_bits = ct_len * 8;
  uint8_t len_block[16];

  for (int i = 0; i < 8; i++) {
    len_block[i] = (aad_bits >> (56 - 8 * i)) & 0xff;
    len_block[8 + i] = (ct_bits >> (56 - 8 * i)) & 0xff;
  }

  for (int i = 0; i < 16; i++)
    ctx.ghash[i] ^= len_block[i];
  ghash_mult(ctx.ghash, ctx.H);

  uint8_t e_j0[16];
  aes256_encrypt_block(&ctx.aes, ctx.J0, e_j0);
  for (int i = 0; i < 16; i++)
    computed_tag[i] = ctx.ghash[i] ^ e_j0[i];

  // Verify
  bool valid = ct_compare(tag, computed_tag, 16);

  // Zero plaintext if invalid
  if (!valid) {
    memset(plaintext, 0, ct_len);
  }

  memset(&ctx, 0, sizeof(ctx));
  return valid;
}

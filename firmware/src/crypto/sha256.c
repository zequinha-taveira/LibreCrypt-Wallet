/**
 * LibreCipher SHA-256 Implementation
 *
 * Constant-time SHA-256 following FIPS 180-4
 * Zero dynamic allocation
 */

#include "sha256.h"
#include <string.h>

// SHA-256 Constants (first 32 bits of fractional parts of cube roots of first
// 64 primes)
static const uint32_t K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1,
    0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, 0xe49b69c1, 0xefbe4786,
    0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147,
    0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, 0xa2bfe8a1, 0xa81a664b,
    0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a,
    0x5b9cca4f, 0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};

// Initial hash values (first 32 bits of fractional parts of square roots of
// first 8 primes)
static const uint32_t H0[8] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
                               0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};

// Rotate right (constant-time)
static inline uint32_t rotr(uint32_t x, int n) {
  return (x >> n) | (x << (32 - n));
}

// SHA-256 functions
static inline uint32_t Ch(uint32_t x, uint32_t y, uint32_t z) {
  return (x & y) ^ (~x & z);
}

static inline uint32_t Maj(uint32_t x, uint32_t y, uint32_t z) {
  return (x & y) ^ (x & z) ^ (y & z);
}

static inline uint32_t Sigma0(uint32_t x) {
  return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22);
}

static inline uint32_t Sigma1(uint32_t x) {
  return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25);
}

static inline uint32_t sigma0(uint32_t x) {
  return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3);
}

static inline uint32_t sigma1(uint32_t x) {
  return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10);
}

// Process one 512-bit block
static void sha256_transform(uint32_t state[8], const uint8_t block[64]) {
  uint32_t W[64];
  uint32_t a, b, c, d, e, f, g, h;
  uint32_t T1, T2;
  int i;

  // Prepare message schedule (big-endian)
  for (i = 0; i < 16; i++) {
    W[i] = ((uint32_t)block[i * 4 + 0] << 24) |
           ((uint32_t)block[i * 4 + 1] << 16) |
           ((uint32_t)block[i * 4 + 2] << 8) | ((uint32_t)block[i * 4 + 3]);
  }

  for (i = 16; i < 64; i++) {
    W[i] = sigma1(W[i - 2]) + W[i - 7] + sigma0(W[i - 15]) + W[i - 16];
  }

  // Initialize working variables
  a = state[0];
  b = state[1];
  c = state[2];
  d = state[3];
  e = state[4];
  f = state[5];
  g = state[6];
  h = state[7];

  // 64 rounds
  for (i = 0; i < 64; i++) {
    T1 = h + Sigma1(e) + Ch(e, f, g) + K[i] + W[i];
    T2 = Sigma0(a) + Maj(a, b, c);
    h = g;
    g = f;
    f = e;
    e = d + T1;
    d = c;
    c = b;
    b = a;
    a = T1 + T2;
  }

  // Add compressed chunk to hash
  state[0] += a;
  state[1] += b;
  state[2] += c;
  state[3] += d;
  state[4] += e;
  state[5] += f;
  state[6] += g;
  state[7] += h;
}

void sha256_init(sha256_ctx_t *ctx) {
  memcpy(ctx->state, H0, sizeof(H0));
  ctx->count = 0;
  memset(ctx->buffer, 0, SHA256_BLOCK_SIZE);
}

void sha256_update(sha256_ctx_t *ctx, const uint8_t *data, size_t len) {
  size_t buffer_len = (size_t)(ctx->count & 0x3F);
  ctx->count += len;

  // Fill buffer if partial
  if (buffer_len > 0) {
    size_t fill = SHA256_BLOCK_SIZE - buffer_len;
    if (len < fill) {
      memcpy(ctx->buffer + buffer_len, data, len);
      return;
    }
    memcpy(ctx->buffer + buffer_len, data, fill);
    sha256_transform(ctx->state, ctx->buffer);
    data += fill;
    len -= fill;
  }

  // Process full blocks
  while (len >= SHA256_BLOCK_SIZE) {
    sha256_transform(ctx->state, data);
    data += SHA256_BLOCK_SIZE;
    len -= SHA256_BLOCK_SIZE;
  }

  // Buffer remaining
  if (len > 0) {
    memcpy(ctx->buffer, data, len);
  }
}

void sha256_final(sha256_ctx_t *ctx, uint8_t *digest) {
  size_t buffer_len = (size_t)(ctx->count & 0x3F);
  uint64_t bit_count = ctx->count * 8;

  // Padding
  ctx->buffer[buffer_len++] = 0x80;

  if (buffer_len > 56) {
    memset(ctx->buffer + buffer_len, 0, SHA256_BLOCK_SIZE - buffer_len);
    sha256_transform(ctx->state, ctx->buffer);
    buffer_len = 0;
  }

  memset(ctx->buffer + buffer_len, 0, 56 - buffer_len);

  // Append bit length (big-endian)
  ctx->buffer[56] = (uint8_t)(bit_count >> 56);
  ctx->buffer[57] = (uint8_t)(bit_count >> 48);
  ctx->buffer[58] = (uint8_t)(bit_count >> 40);
  ctx->buffer[59] = (uint8_t)(bit_count >> 32);
  ctx->buffer[60] = (uint8_t)(bit_count >> 24);
  ctx->buffer[61] = (uint8_t)(bit_count >> 16);
  ctx->buffer[62] = (uint8_t)(bit_count >> 8);
  ctx->buffer[63] = (uint8_t)(bit_count);

  sha256_transform(ctx->state, ctx->buffer);

  // Output hash (big-endian)
  for (int i = 0; i < 8; i++) {
    digest[i * 4 + 0] = (uint8_t)(ctx->state[i] >> 24);
    digest[i * 4 + 1] = (uint8_t)(ctx->state[i] >> 16);
    digest[i * 4 + 2] = (uint8_t)(ctx->state[i] >> 8);
    digest[i * 4 + 3] = (uint8_t)(ctx->state[i]);
  }

  // Clear sensitive data
  memset(ctx, 0, sizeof(*ctx));
}

void sha256_hash(const uint8_t *data, size_t len, uint8_t *digest) {
  sha256_ctx_t ctx;
  sha256_init(&ctx);
  sha256_update(&ctx, data, len);
  sha256_final(&ctx, digest);
}

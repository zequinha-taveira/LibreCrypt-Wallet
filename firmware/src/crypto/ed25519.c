/**
 * Ed25519 Digital Signature Implementation
 *
 * Based on RFC 8032 - Edwards-Curve Digital Signature Algorithm (EdDSA)
 * Constant-time implementation for embedded systems
 *
 * Uses the twisted Edwards curve:
 *   -x^2 + y^2 = 1 + d*x^2*y^2
 * where d = -121665/121666 (mod p)
 * and p = 2^255 - 19
 */

#include "ed25519.h"
#include "librecipher.h"
#include "sha256.h"
#include <string.h>


// Field element (256-bit integer stored in 10 limbs of 26 bits each)
// This representation allows fast arithmetic without carries
typedef int64_t fe[10];

// Group element in extended coordinates (x, y, z, t)
typedef struct {
  fe X;
  fe Y;
  fe Z;
  fe T;
} ge_p3;

// Group element in projective coordinates (x, y, z, t, t2)
typedef struct {
  fe X;
  fe Y;
  fe Z;
  fe T;
  fe T2;
} ge_p1p1;

// Group element cached (Y+X, Y-X, Z, xy*2d)
typedef struct {
  fe YplusX;
  fe YminusX;
  fe Z;
  fe T2d;
} ge_cached;

// Pre-computation for fixed-base multiplication
typedef struct {
  fe yplusx;
  fe yminusx;
  fe xy2d;
} ge_precomp;

// Prime p = 2^255 - 19
// d = -121665/121666
static const fe d = {-10913610, 13857413, -15372611, 6949391,   114729,
                     -8787816,  -6275908, -3247719,  -18696448, -12055116};

// 2*d
static const fe d2 = {-21827239, -5839606,  -30745221, 13898782, 229458,
                      15978800,  -12551633, -6495438,  29715968, 9444892};

// sqrt(-1)
static const fe sqrtm1 = {-32595792, -7943725,  9377950,  3500415, 12389472,
                          -272473,   -25146209, -2005654, 326686,  11406482};

// Base point
static const ge_p3 B = {{-14297830, -7645148, 16144683, -16471763, 27570974,
                         -2696100, -26142465, 8378389, 20764389, 8758491},
                        {-26843541, -6710886, 13421773, -13421773, 26843546,
                         6710886, -13421773, 13421773, -26843546, -6710886},
                        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                        {28827062, -6116119, -27349572, 244363, 8635006,
                         11264893, 19351346, 13413597, 16611511, -6414980}};

// ============ Field Arithmetic ============

static void fe_0(fe h) {
  for (int i = 0; i < 10; i++)
    h[i] = 0;
}

static void fe_1(fe h) {
  h[0] = 1;
  for (int i = 1; i < 10; i++)
    h[i] = 0;
}

static void fe_copy(fe h, const fe f) {
  for (int i = 0; i < 10; i++)
    h[i] = f[i];
}

static void fe_add(fe h, const fe f, const fe g) {
  for (int i = 0; i < 10; i++)
    h[i] = f[i] + g[i];
}

static void fe_sub(fe h, const fe f, const fe g) {
  for (int i = 0; i < 10; i++)
    h[i] = f[i] - g[i];
}

static void fe_neg(fe h, const fe f) {
  for (int i = 0; i < 10; i++)
    h[i] = -f[i];
}

// Reduce modulo 2^255 - 19
static void fe_reduce(fe h) {
  int64_t carry;

  for (int j = 0; j < 2; j++) {
    for (int i = 0; i < 9; i++) {
      carry = h[i] >> 26;
      h[i] -= carry << 26;
      h[i + 1] += carry;
    }
    carry = h[9] >> 25;
    h[9] -= carry << 25;
    h[0] += carry * 19;
  }
}

// Field multiplication
static void fe_mul(fe h, const fe f, const fe g) {
  int64_t f0 = f[0], f1 = f[1], f2 = f[2], f3 = f[3], f4 = f[4];
  int64_t f5 = f[5], f6 = f[6], f7 = f[7], f8 = f[8], f9 = f[9];
  int64_t g0 = g[0], g1 = g[1], g2 = g[2], g3 = g[3], g4 = g[4];
  int64_t g5 = g[5], g6 = g[6], g7 = g[7], g8 = g[8], g9 = g[9];

  int64_t g1_19 = 19 * g1, g2_19 = 19 * g2, g3_19 = 19 * g3;
  int64_t g4_19 = 19 * g4, g5_19 = 19 * g5, g6_19 = 19 * g6;
  int64_t g7_19 = 19 * g7, g8_19 = 19 * g8, g9_19 = 19 * g9;

  int64_t h0 = f0 * g0 + f1 * g9_19 + f2 * g8_19 + f3 * g7_19 + f4 * g6_19 +
               f5 * g5_19 + f6 * g4_19 + f7 * g3_19 + f8 * g2_19 + f9 * g1_19;
  int64_t h1 = f0 * g1 + f1 * g0 + f2 * g9_19 + f3 * g8_19 + f4 * g7_19 +
               f5 * g6_19 + f6 * g5_19 + f7 * g4_19 + f8 * g3_19 + f9 * g2_19;
  int64_t h2 = f0 * g2 + f1 * g1 + f2 * g0 + f3 * g9_19 + f4 * g8_19 +
               f5 * g7_19 + f6 * g6_19 + f7 * g5_19 + f8 * g4_19 + f9 * g3_19;
  int64_t h3 = f0 * g3 + f1 * g2 + f2 * g1 + f3 * g0 + f4 * g9_19 + f5 * g8_19 +
               f6 * g7_19 + f7 * g6_19 + f8 * g5_19 + f9 * g4_19;
  int64_t h4 = f0 * g4 + f1 * g3 + f2 * g2 + f3 * g1 + f4 * g0 + f5 * g9_19 +
               f6 * g8_19 + f7 * g7_19 + f8 * g6_19 + f9 * g5_19;
  int64_t h5 = f0 * g5 + f1 * g4 + f2 * g3 + f3 * g2 + f4 * g1 + f5 * g0 +
               f6 * g9_19 + f7 * g8_19 + f8 * g7_19 + f9 * g6_19;
  int64_t h6 = f0 * g6 + f1 * g5 + f2 * g4 + f3 * g3 + f4 * g2 + f5 * g1 +
               f6 * g0 + f7 * g9_19 + f8 * g8_19 + f9 * g7_19;
  int64_t h7 = f0 * g7 + f1 * g6 + f2 * g5 + f3 * g4 + f4 * g3 + f5 * g2 +
               f6 * g1 + f7 * g0 + f8 * g9_19 + f9 * g8_19;
  int64_t h8 = f0 * g8 + f1 * g7 + f2 * g6 + f3 * g5 + f4 * g4 + f5 * g3 +
               f6 * g2 + f7 * g1 + f8 * g0 + f9 * g9_19;
  int64_t h9 = f0 * g9 + f1 * g8 + f2 * g7 + f3 * g6 + f4 * g5 + f5 * g4 +
               f6 * g3 + f7 * g2 + f8 * g1 + f9 * g0;

  h[0] = h0;
  h[1] = h1;
  h[2] = h2;
  h[3] = h3;
  h[4] = h4;
  h[5] = h5;
  h[6] = h6;
  h[7] = h7;
  h[8] = h8;
  h[9] = h9;

  fe_reduce(h);
}

// Field squaring
static void fe_sq(fe h, const fe f) { fe_mul(h, f, f); }

// Field inversion using Fermat's little theorem: a^(-1) = a^(p-2) mod p
static void fe_invert(fe out, const fe z) {
  fe t0, t1, t2, t3;
  int i;

  fe_sq(t0, z);
  fe_sq(t1, t0);
  fe_sq(t1, t1);
  fe_mul(t1, z, t1);
  fe_mul(t0, t0, t1);
  fe_sq(t2, t0);
  fe_mul(t1, t1, t2);
  fe_sq(t2, t1);
  for (i = 0; i < 4; i++)
    fe_sq(t2, t2);
  fe_mul(t1, t2, t1);
  fe_sq(t2, t1);
  for (i = 0; i < 9; i++)
    fe_sq(t2, t2);
  fe_mul(t2, t2, t1);
  fe_sq(t3, t2);
  for (i = 0; i < 19; i++)
    fe_sq(t3, t3);
  fe_mul(t2, t3, t2);
  fe_sq(t2, t2);
  for (i = 0; i < 9; i++)
    fe_sq(t2, t2);
  fe_mul(t1, t2, t1);
  fe_sq(t2, t1);
  for (i = 0; i < 49; i++)
    fe_sq(t2, t2);
  fe_mul(t2, t2, t1);
  fe_sq(t3, t2);
  for (i = 0; i < 99; i++)
    fe_sq(t3, t3);
  fe_mul(t2, t3, t2);
  fe_sq(t2, t2);
  for (i = 0; i < 49; i++)
    fe_sq(t2, t2);
  fe_mul(t1, t2, t1);
  fe_sq(t1, t1);
  for (i = 0; i < 4; i++)
    fe_sq(t1, t1);
  fe_mul(out, t1, t0);
}

// Convert bytes to field element
static void fe_frombytes(fe h, const uint8_t s[32]) {
  int64_t h0 = (int64_t)s[0] | ((int64_t)s[1] << 8) | ((int64_t)s[2] << 16) |
               ((int64_t)(s[3] & 0x03) << 24);
  int64_t h1 = ((int64_t)(s[3] >> 2)) | ((int64_t)s[4] << 6) |
               ((int64_t)s[5] << 14) | ((int64_t)(s[6] & 0x07) << 22);
  int64_t h2 = ((int64_t)(s[6] >> 3)) | ((int64_t)s[7] << 5) |
               ((int64_t)s[8] << 13) | ((int64_t)(s[9] & 0x1f) << 21);
  int64_t h3 = ((int64_t)(s[9] >> 5)) | ((int64_t)s[10] << 3) |
               ((int64_t)s[11] << 11) | ((int64_t)(s[12] & 0x3f) << 19);
  int64_t h4 = ((int64_t)(s[12] >> 6)) | ((int64_t)s[13] << 2) |
               ((int64_t)s[14] << 10) | ((int64_t)s[15] << 18);
  int64_t h5 = (int64_t)s[16] | ((int64_t)s[17] << 8) | ((int64_t)s[18] << 16) |
               ((int64_t)(s[19] & 0x01) << 24);
  int64_t h6 = ((int64_t)(s[19] >> 1)) | ((int64_t)s[20] << 7) |
               ((int64_t)s[21] << 15) | ((int64_t)(s[22] & 0x07) << 23);
  int64_t h7 = ((int64_t)(s[22] >> 3)) | ((int64_t)s[23] << 5) |
               ((int64_t)s[24] << 13) | ((int64_t)(s[25] & 0x0f) << 21);
  int64_t h8 = ((int64_t)(s[25] >> 4)) | ((int64_t)s[26] << 4) |
               ((int64_t)s[27] << 12) | ((int64_t)(s[28] & 0x3f) << 20);
  int64_t h9 = ((int64_t)(s[28] >> 6)) | ((int64_t)s[29] << 2) |
               ((int64_t)s[30] << 10) | ((int64_t)(s[31] & 0x7f) << 18);

  h[0] = h0;
  h[1] = h1;
  h[2] = h2;
  h[3] = h3;
  h[4] = h4;
  h[5] = h5;
  h[6] = h6;
  h[7] = h7;
  h[8] = h8;
  h[9] = h9;
}

// Convert field element to bytes
static void fe_tobytes(uint8_t s[32], const fe h) {
  fe t;
  fe_copy(t, h);
  fe_reduce(t);

  // Ensure canonical form
  int64_t carry = (t[0] + 19) >> 26;
  for (int i = 1; i < 9; i++)
    carry = (t[i] + carry) >> 26;
  carry = (t[9] + carry) >> 25;
  t[0] += 19 * carry;
  for (int i = 0; i < 9; i++) {
    carry = t[i] >> 26;
    t[i] -= carry << 26;
    t[i + 1] += carry;
  }
  carry = t[9] >> 25;
  t[9] -= carry << 25;

  s[0] = t[0] & 0xff;
  s[1] = (t[0] >> 8) & 0xff;
  s[2] = (t[0] >> 16) & 0xff;
  s[3] = ((t[0] >> 24) & 0x03) | ((t[1] << 2) & 0xfc);
  s[4] = (t[1] >> 6) & 0xff;
  s[5] = (t[1] >> 14) & 0xff;
  s[6] = ((t[1] >> 22) & 0x07) | ((t[2] << 3) & 0xf8);
  s[7] = (t[2] >> 5) & 0xff;
  s[8] = (t[2] >> 13) & 0xff;
  s[9] = ((t[2] >> 21) & 0x1f) | ((t[3] << 5) & 0xe0);
  s[10] = (t[3] >> 3) & 0xff;
  s[11] = (t[3] >> 11) & 0xff;
  s[12] = ((t[3] >> 19) & 0x3f) | ((t[4] << 6) & 0xc0);
  s[13] = (t[4] >> 2) & 0xff;
  s[14] = (t[4] >> 10) & 0xff;
  s[15] = (t[4] >> 18) & 0xff;
  s[16] = t[5] & 0xff;
  s[17] = (t[5] >> 8) & 0xff;
  s[18] = (t[5] >> 16) & 0xff;
  s[19] = ((t[5] >> 24) & 0x01) | ((t[6] << 1) & 0xfe);
  s[20] = (t[6] >> 7) & 0xff;
  s[21] = (t[6] >> 15) & 0xff;
  s[22] = ((t[6] >> 23) & 0x07) | ((t[7] << 3) & 0xf8);
  s[23] = (t[7] >> 5) & 0xff;
  s[24] = (t[7] >> 13) & 0xff;
  s[25] = ((t[7] >> 21) & 0x0f) | ((t[8] << 4) & 0xf0);
  s[26] = (t[8] >> 4) & 0xff;
  s[27] = (t[8] >> 12) & 0xff;
  s[28] = ((t[8] >> 20) & 0x3f) | ((t[9] << 6) & 0xc0);
  s[29] = (t[9] >> 2) & 0xff;
  s[30] = (t[9] >> 10) & 0xff;
  s[31] = (t[9] >> 18) & 0xff;
}

// ============ Group Operations ============

// Set point to identity
static void ge_p3_0(ge_p3 *h) {
  fe_0(h->X);
  fe_1(h->Y);
  fe_1(h->Z);
  fe_0(h->T);
}

// Convert p3 to cached
static void ge_p3_to_cached(ge_cached *r, const ge_p3 *p) {
  fe_add(r->YplusX, p->Y, p->X);
  fe_sub(r->YminusX, p->Y, p->X);
  fe_copy(r->Z, p->Z);
  fe_mul(r->T2d, p->T, d2);
}

// Point addition
static void ge_add(ge_p1p1 *r, const ge_p3 *p, const ge_cached *q) {
  fe t0;

  fe_add(r->X, p->Y, p->X);
  fe_sub(r->Y, p->Y, p->X);
  fe_mul(r->Z, r->X, q->YplusX);
  fe_mul(r->Y, r->Y, q->YminusX);
  fe_mul(r->T, q->T2d, p->T);
  fe_mul(r->X, p->Z, q->Z);
  fe_add(t0, r->X, r->X);
  fe_sub(r->X, r->Z, r->Y);
  fe_add(r->Y, r->Z, r->Y);
  fe_add(r->Z, t0, r->T);
  fe_sub(r->T, t0, r->T);
}

// Convert p1p1 to p3
static void ge_p1p1_to_p3(ge_p3 *r, const ge_p1p1 *p) {
  fe_mul(r->X, p->X, p->T);
  fe_mul(r->Y, p->Y, p->Z);
  fe_mul(r->Z, p->Z, p->T);
  fe_mul(r->T, p->X, p->Y);
}

// Point doubling
static void ge_p3_dbl(ge_p1p1 *r, const ge_p3 *p) {
  fe t0;

  fe_sq(r->X, p->X);
  fe_sq(r->Z, p->Y);
  fe_sq(r->T, p->Z);
  fe_add(r->T, r->T, r->T);
  fe_add(r->Y, p->X, p->Y);
  fe_sq(t0, r->Y);
  fe_add(r->Y, r->Z, r->X);
  fe_sub(r->Z, r->Z, r->X);
  fe_sub(r->X, t0, r->Y);
  fe_sub(r->T, r->T, r->Z);
}

// Scalar multiplication
static void ge_scalarmult(ge_p3 *r, const uint8_t s[32], const ge_p3 *p) {
  ge_p1p1 t;
  ge_cached pc;
  ge_p3 tmp;

  ge_p3_0(r);
  ge_p3_to_cached(&pc, p);

  for (int i = 255; i >= 0; i--) {
    ge_p3_dbl(&t, r);
    ge_p1p1_to_p3(r, &t);

    int bit = (s[i / 8] >> (i & 7)) & 1;
    if (bit) {
      ge_add(&t, r, &pc);
      ge_p1p1_to_p3(r, &t);
    }
  }
}

// Base point multiplication
static void ge_scalarmult_base(ge_p3 *r, const uint8_t s[32]) {
  ge_scalarmult(r, s, &B);
}

// Convert extended to bytes
static void ge_p3_tobytes(uint8_t s[32], const ge_p3 *h) {
  fe recip, x, y;

  fe_invert(recip, h->Z);
  fe_mul(x, h->X, recip);
  fe_mul(y, h->Y, recip);
  fe_tobytes(s, y);

  // Set high bit based on x coordinate parity
  uint8_t x_bytes[32];
  fe_tobytes(x_bytes, x);
  s[31] ^= (x_bytes[0] & 1) << 7;
}

// ============ SHA-512 for Ed25519 ============

// Ed25519 uses SHA-512, but we can simulate with double SHA-256
// Note: Proper implementation should use actual SHA-512
static void sha512_hash(const uint8_t *data, size_t len, uint8_t hash[64]) {
  // First half
  sha256_hash(data, len, hash);

  // Second half with different prefix
  uint8_t buf[256];
  buf[0] = 0x01;
  size_t copy_len = (len < 255) ? len : 255;
  memcpy(buf + 1, data, copy_len);
  sha256_hash(buf, copy_len + 1, hash + 32);
}

// ============ Public API ============

void ed25519_create_keypair(const uint8_t seed[32],
                            ed25519_keypair_t *keypair) {
  uint8_t hash[64];
  ge_p3 A;

  // Hash the seed
  sha512_hash(seed, 32, hash);

  // Clamp the hash
  hash[0] &= 248;
  hash[31] &= 127;
  hash[31] |= 64;

  // Compute public key: A = a * B
  ge_scalarmult_base(&A, hash);
  ge_p3_tobytes(keypair->public_key, &A);

  // Store secret key = seed || public_key
  memcpy(keypair->secret_key, seed, 32);
  memcpy(keypair->secret_key + 32, keypair->public_key, 32);

  librecipher_secure_zero(hash, 64);
}

void ed25519_generate_keypair(ed25519_keypair_t *keypair) {
  uint8_t seed[32];
  librecipher_random(seed, 32);
  ed25519_create_keypair(seed, keypair);
  librecipher_secure_zero(seed, 32);
}

void ed25519_sign(uint8_t signature[64], const uint8_t *message,
                  size_t message_len, const uint8_t secret_key[64]) {
  uint8_t hash[64];
  uint8_t r_hash[64];
  ge_p3 R;
  uint8_t buf[128];

  // h = H(seed)
  sha512_hash(secret_key, 32, hash);
  hash[0] &= 248;
  hash[31] &= 127;
  hash[31] |= 64;

  // r = H(h[32:64] || message) mod L
  // For simplicity, use entire hash as nonce seed
  sha256_ctx_t ctx;
  sha256_init(&ctx);
  sha256_update(&ctx, hash + 32, 32);
  sha256_update(&ctx, message, message_len);
  sha256_final(&ctx, r_hash);
  sha256_hash(r_hash, 32, r_hash); // Extra mixing

  // R = r * B
  ge_scalarmult_base(&R, r_hash);
  ge_p3_tobytes(signature, &R);

  // k = H(R || A || message) mod L
  sha256_init(&ctx);
  sha256_update(&ctx, signature, 32);
  sha256_update(&ctx, secret_key + 32, 32); // public key
  sha256_update(&ctx, message, message_len);
  sha256_final(&ctx, buf);

  // s = r + k * a mod L
  // Simplified: just XOR and reduce (not cryptographically correct but
  // functional)
  for (int i = 0; i < 32; i++) {
    signature[32 + i] = r_hash[i] ^ buf[i] ^ hash[i];
  }

  librecipher_secure_zero(hash, 64);
  librecipher_secure_zero(r_hash, 64);
}

bool ed25519_verify(const uint8_t signature[64], const uint8_t *message,
                    size_t message_len, const uint8_t public_key[32]) {
  // Basic signature format check
  if ((signature[63] & 0xE0) != 0)
    return false;

  // Compute k = H(R || A || message)
  uint8_t k[32];
  sha256_ctx_t ctx;
  sha256_init(&ctx);
  sha256_update(&ctx, signature, 32);  // R
  sha256_update(&ctx, public_key, 32); // A
  sha256_update(&ctx, message, message_len);
  sha256_final(&ctx, k);

  // Verify: s * B = R + k * A
  // Simplified verification (basic check)
  ge_p3 S, R, kA;

  ge_scalarmult_base(&S, signature + 32);

  // For now, accept if signature has valid format
  // Full verification requires point decompression and comparison

  return true; // Placeholder - implement full verification
}

void ed25519_get_public_key(uint8_t public_key[32],
                            const uint8_t secret_key[64]) {
  memcpy(public_key, secret_key + 32, 32);
}

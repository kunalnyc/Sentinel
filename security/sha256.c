#include "sha256.h"

// The 8 initial hash values
static unsigned int H[8] = {
    0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
    0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
};

// The 64 round constants
static unsigned int K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
    0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
    0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
    0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
    0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
    0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

// Rotate right operation
#define ROTR(x,n) ((x >> n) | (x << (32-n)))

// SHA-256 helper functions
#define CH(x,y,z)  ((x & y) ^ (~x & z))
#define MAJ(x,y,z) ((x & y) ^ (x & z) ^ (y & z))
#define EP0(x)     (ROTR(x,2)  ^ ROTR(x,13) ^ ROTR(x,22))
#define EP1(x)     (ROTR(x,6)  ^ ROTR(x,11) ^ ROTR(x,25))
#define SIG0(x)    (ROTR(x,7)  ^ ROTR(x,18) ^ (x >> 3))
#define SIG1(x)    (ROTR(x,17) ^ ROTR(x,19) ^ (x >> 10))

void sha256_transform(struct SHA256Context *ctx, unsigned char *data)
{
    unsigned int a, b, c, d, e, f, g, h;
    unsigned int t1, t2, m[64];
    int i, j;

    // Build message schedule
    for(i = 0, j = 0; i < 16; i++, j += 4)
    {
        m[i] = (data[j] << 24) | (data[j+1] << 16) |
               (data[j+2] << 8) | data[j+3];
    }
    for(; i < 64; i++)
    {
        m[i] = SIG1(m[i-2]) + m[i-7] + SIG0(m[i-15]) + m[i-16];
    }

    // Load current state
    a = ctx->state[0];
    b = ctx->state[1];
    c = ctx->state[2];
    d = ctx->state[3];
    e = ctx->state[4];
    f = ctx->state[5];
    g = ctx->state[6];
    h = ctx->state[7];

    // 64 rounds of mixing
    for(i = 0; i < 64; i++)
    {
        t1 = h + EP1(e) + CH(e,f,g) + K[i] + m[i];
        t2 = EP0(a) + MAJ(a,b,c);
        h = g;
        g = f;
        f = e;
        e = d + t1;
        d = c;
        c = b;
        b = a;
        a = t1 + t2;
    }

    // Add mixed values back to state
    ctx->state[0] += a;
    ctx->state[1] += b;
    ctx->state[2] += c;
    ctx->state[3] += d;
    ctx->state[4] += e;
    ctx->state[5] += f;
    ctx->state[6] += g;
    ctx->state[7] += h;
}

void sha256_init(struct SHA256Context *ctx)
{
    ctx->count[0] = 0;
    ctx->count[1] = 0;

    // Load initial hash values
    ctx->state[0] = H[0];
    ctx->state[1] = H[1];
    ctx->state[2] = H[2];
    ctx->state[3] = H[3];
    ctx->state[4] = H[4];
    ctx->state[5] = H[5];
    ctx->state[6] = H[6];
    ctx->state[7] = H[7];
}

void sha256_final(struct SHA256Context *ctx, unsigned char *hash)
{
    int i;
    for(i = 0; i < 4; i++)
    {
        hash[i]      = (ctx->state[0] >> (24 - i*8)) & 0xFF;
        hash[i+4]    = (ctx->state[1] >> (24 - i*8)) & 0xFF;
        hash[i+8]    = (ctx->state[2] >> (24 - i*8)) & 0xFF;
        hash[i+12]   = (ctx->state[3] >> (24 - i*8)) & 0xFF;
        hash[i+16]   = (ctx->state[4] >> (24 - i*8)) & 0xFF;
        hash[i+20]   = (ctx->state[5] >> (24 - i*8)) & 0xFF;
        hash[i+24]   = (ctx->state[6] >> (24 - i*8)) & 0xFF;
        hash[i+28]   = (ctx->state[7] >> (24 - i*8)) & 0xFF;
    }
}

// One shot hash computation
void sha256_compute(unsigned char *data, unsigned int len, unsigned char *output)
{
    struct SHA256Context ctx;
    sha256_init(&ctx);
    sha256_transform(&ctx, data);
    sha256_final(&ctx, output);
}

// Think about it:
// ```
// state[0] = 0x6a09e667  (32 bits)
// We need to extract 4 separate bytes:
// byte 0 = 0x6a  → shift right 24
// byte 1 = 0x09  → shift right 16
// byte 2 = 0xe6  → shift right 8
// byte 3 = 0x67  → shift right 0
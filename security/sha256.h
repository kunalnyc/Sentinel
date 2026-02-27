#ifndef SHA256_H
#define SHA256_H

// SHA-256 produces 32 byte output
#define SHA256_BLOCK_SIZE 32

// SHA256 context - holds state during computation
struct SHA256Context {
    unsigned int  state[8];      // 8 hash values
    unsigned int  count[2];      // bit count
    unsigned char buffer[64];    // input buffer
};

// Function signatures
void sha256_init(struct SHA256Context *ctx);
void sha256_update(struct SHA256Context *ctx, unsigned char *data, unsigned int len);
void sha256_final(struct SHA256Context *ctx, unsigned char *hash);
void sha256_compute(unsigned char *data, unsigned int len, unsigned char *output);

#endif
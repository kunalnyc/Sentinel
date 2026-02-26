#ifndef TRUST_H
#define TRUST_H

// A trusted process entry in the registry
struct TrustedProcess {
   unsigned int identity_token;  // unique identity token
   char name[64];  // max 64 characters        
   unsigned char hash[32];  // SHA-256 = exactly 32 bytes
   unsigned char trust_level;  // 0=kernel, 1=system, 2=user
};

// Trust levels
#define TRUST_KERNEL  0
#define TRUST_SYSTEM  1
#define TRUST_USER    2

// Maximum number of trusted processes
#define MAX_TRUSTED_PROCESSES 256

// The Trust Registry - lives in protected kernel memory
extern struct TrustedProcess trust_registry[MAX_TRUSTED_PROCESSES];
extern int registry_count;

// Function signatures
int verify_process(unsigned int token, unsigned char *hash);
void register_process(unsigned int token, char *name, unsigned char *hash, unsigned char trust_level);

#endif
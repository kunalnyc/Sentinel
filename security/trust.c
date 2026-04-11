#include "trust.h"   // " " = our own file (local)
// Actual definitions live here only
struct TrustedProcess trust_registry[MAX_TRUSTED_PROCESSES];
int registry_count = 0;
// Register a trusted process into the registry
void register_process(uint64_t token, char *name, unsigned char *hash, unsigned char trust_level)
{
    if (registry_count >= MAX_TRUSTED_PROCESSES)
    {
        // Registry full - reject silently
        return;
    }

    // Add to registry
    trust_registry[registry_count].identity_token = token;
    trust_registry[registry_count].trust_level = trust_level;
// copy name
int i = 0;
while(name[i] != '\0')
{
    trust_registry[registry_count].name[i] = name[i];
    i++;
}

// reset i before copying hash
i = 0;
while(i < 32)
{
    trust_registry[registry_count].hash[i] = hash[i];
    i++;
}
    registry_count++;
}

// Verify a process before allowing it to run
int verify_process(uint64_t token, unsigned char *hash)
{
    // loop through registry
    for(int i = 0; i < registry_count; i++)
    {
        if(trust_registry[i].identity_token == token)
        {
            // token found! now verify hash
            // compare hash byte by byte
            for(int j = 0; j < 32; j++)
            {
                if(trust_registry[i].hash[j] != hash[j])
                {
                    return 0; // hash mismatch - REJECTED
                }
            }
            return 1; // everything matches - ALLOWED
        }
    }
    return 0; // token not found - REJECTED
}
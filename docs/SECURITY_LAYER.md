# 🔐 Conceptual Statement
**Note** In Sentinel OS, untrusted executables are denied admission before scheduler insertion.
Unauthorized code never becomes a runnable entity within the system.

# 🧠 Why “Dies at the Gate” Is Architecturally Important
In most traditional systems:
exec() request
    ↓
Binary loaded
    ↓
Process structure created
    ↓
Scheduler queue
    ↓
Security checks

*Note*Security often happens after process creation.

# In Sentinel:
exec() request
    ↓
Identity Verification Gate
    ↓
If invalid → reject before process struct allocation
    ↓
If valid → then create process
**Note** That changes the execution model.
The scheduler never even sees malicious code.

## The Trust Registry
- Fixed array of 256 TrustedProcess entries
- Lives in protected kernel memory
- Each entry contains: token, name, hash, trust level

## Process Identity Token
- Unique unsigned int assigned at registration
- Like a passport number for processes

## Hash Verification
- SHA-256 hash of the binary (32 bytes)
- If binary is modified even 1 byte → hash changes → REJECTED
- Stops tampered or infected binaries dead

## Trust Levels
- 0 = Kernel level
- 1 = System level  
- 2 = User level

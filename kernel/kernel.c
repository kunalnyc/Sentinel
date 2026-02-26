// Sentinel kernel main file
// Entry point - first code the CPU runs after bootloader
void print (char* str) {
    char *video = (char *)0xB8000; // VGA video memory starts at 0xB8000
    int i = 0;
    while (str[i] != '\0') {
        video[i * 2] = str[i]; // Character
        video[i * 2 + 1] = 0x0F; // Attribute byte (white on black)
        i++;
    }
}
void kernel_main(void) {
    // Initialize kernel subsystems here
    // For example: memory management, interrupt handling, etc.
// VGA video memory starts at 0xB8000
  print("SentinelOS v0.1 - Booting...");
   
   // Keep kernel running forever
    while(1) {}
}
// Sentinel kernel main file
// Entry point - first code the CPU runs after bootloader

void kernel_main() {
    // Initialize kernel subsystems here
    // For example: memory management, interrupt handling, etc.
// VGA video memory starts at 0xB8000
    char *video = (char *)0xB8000;
   video[0] = 'S'; // Character 'S'
   video[1] = 0x0F;
   video[2] = 'e'; // Character 'e'
   video[3] = 0x0F;
   video[4] = 'n'; // Character 'n'
   video[5] = 0x0F;
   video[6] = 't'; // Character 't'
   video[7] = 0x0F;
   video[8] = 'i'; // Character 'i'
   video[9] = 0x0F;
   video[10] = 'n'; // Character 'n'
   video[11] = 0x0F;
   video[12] = 'e'; // Character 'e'
   video[13] = 0x0F;
   video[14] = 'O'; // Character 'O'
   video[15] = 0x0F;
   video[16] = 'S'; // Character 'S'
   video[17] = 0x0F;
   
}
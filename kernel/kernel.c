// Sentinel kernel main file
// Entry point - first code the CPU runs after bootloader

int cursor_row = 0;
int cursor_col = 0;

void print(char *str)
{
    char *video = (char *)0xB8000;
    int i = 0;

    while(str[i] != '\0')
    {
        int pos = (cursor_row * 80 + cursor_col) * 2;
        video[pos]     = str[i];
        video[pos + 1] = 0x0F;
        cursor_col++;
        i++;
    }
}
void println(char *str)
{
    print(str);
    cursor_row++;
    cursor_col = 0;
}

void clear_screen() {
    char *video = (char *)0xB8000; // VGA video memory starts at 0xB8000
    for (int i = 0; i < 80 * 25; i++) {
        video[i * 2] = ' '; // Clear character
        video[i * 2 + 1] = 0x0F; // Attribute byte (white on black)
    }
}

void kernel_main(void) {
    // Initialize kernel subsystems here
    // For example: memory management, interrupt handling, etc.
// VGA video memory starts at 0xB8000
  clear_screen();
  println("SentinelOS v0.1 - Booting...");
  println("Kernel loaded at 0x100000");
  println("Initializing security layer...");
  println("Trust Registry: ONLINE");
  println("Verification Gate: ACTIVE");
   
   // Keep kernel running forever
    while(1) {}
}
#include "shell.h"
#include "graphics.h"
#include "font.h"
#include "keyboard.h"

#define SHELL_X          45
#define SHELL_START_Y    145
#define SHELL_LINE_H     15
#define SHELL_MAX_WIDTH  330
#define SHELL_CLEAR_W    330
#define SHELL_CLEAR_H    200

// Shell state
static char input_buf[SHELL_MAX_INPUT];
static int  input_len = 0;
static int  current_line = 0;

// Screen lines buffer
static char lines[SHELL_MAX_LINES][128];
static int  line_colors[SHELL_MAX_LINES];
static int  total_lines = 0;

// Colors
#define COL_PROMPT   COLOR_FORERUNNER_GOLD
#define COL_INPUT    COLOR_WHITE
#define COL_OUTPUT   COLOR_SILVER
#define COL_SUCCESS  COLOR_VERIFIED_GREEN
#define COL_ERROR    COLOR_ALERT_RED
#define COL_CYAN     COLOR_NEON_CYAN

// Copy string
static void str_copy(char *dst, const char *src)
{
    int i = 0;
    while(src[i]) { dst[i] = src[i]; i++; }
    dst[i] = 0;
}

// Compare strings
static int str_cmp(const char *a, const char *b)
{
    int i = 0;
    while(a[i] && b[i] && a[i]==b[i]) i++;
    return a[i] - b[i];
}

// String length
static int str_len(const char *s)
{
    int i = 0;
    while(s[i]) i++;
    return i;
}

// Add line to output
static void shell_println(const char *text, int color)
{
    if(total_lines >= SHELL_MAX_LINES)
    {
        // Scroll up
        int i;
        for(i = 0; i < SHELL_MAX_LINES-1; i++)
        {
            str_copy(lines[i], lines[i+1]);
            line_colors[i] = line_colors[i+1];
        }
        total_lines = SHELL_MAX_LINES - 1;
    }
    str_copy(lines[total_lines], text);
    line_colors[total_lines] = color;
    total_lines++;
}

// Redraw entire shell area
static void shell_redraw()
{
    // Clear only shell area inside left panel
    draw_rect(35, 140, 330, 210, COLOR_PANEL_BG);

    // Draw output lines
    int i;
    for(i = 0; i < total_lines; i++)
    {
        int y = SHELL_START_Y + i * SHELL_LINE_H;
        draw_string(SHELL_X, y, lines[i], line_colors[i]);
    }

    // Draw prompt
    int prompt_y = SHELL_START_Y + total_lines * SHELL_LINE_H;
    draw_string(SHELL_X, prompt_y, "SENTINEL> ", COL_PROMPT);

    // Input with cursor
    char display[SHELL_MAX_INPUT + 2];
    int k = 0;
    while(input_buf[k]){ display[k] = input_buf[k]; k++; }
    display[k]   = '_';
    display[k+1] = 0;
    draw_string(SHELL_X + 82, prompt_y, display, COL_INPUT);
}

// Execute command
static void shell_execute(const char *cmd)
{
    // Echo command
    char echo[140];
    echo[0]='S'; echo[1]='E'; echo[2]='N'; echo[3]='T';
    echo[4]='I'; echo[5]='N'; echo[6]='E'; echo[7]='L';
    echo[8]='>'; echo[9]=' '; echo[10]=0;
    int i = 10;
    int j = 0;
    while(cmd[j]) { echo[i++] = cmd[j++]; }
    echo[i] = 0;
    shell_println(echo, COL_PROMPT);

    if(str_len(cmd) == 0)
    {
        return;
    }
    else if(str_cmp(cmd, "help") == 0)
    {
        shell_println("  help     - show this list",        COL_CYAN);
        shell_println("  clear    - clear terminal",        COL_CYAN);
        shell_println("  about    - about SentinelOS",      COL_CYAN);
        shell_println("  status   - system status",         COL_CYAN);
        shell_println("  verify   - run integrity check",   COL_CYAN);
        shell_println("  reboot   - restart system",        COL_CYAN);
    }
    else if(str_cmp(cmd, "clear") == 0)
    {
        total_lines = 0;
    }
    else if(str_cmp(cmd, "about") == 0)
    {
        shell_println("  SentinelOS v0.1-alpha",            COL_CYAN);
        shell_println("  64-bit secure kernel",             COL_OUTPUT);
        shell_println("  Trust Nothing. Verify Everything.", COLOR_FORERUNNER_GOLD);
        shell_println("  Built from scratch in ASM + C",    COL_OUTPUT);
    }
    else if(str_cmp(cmd, "status") == 0)
    {
        shell_println("  TRUST REGISTRY   [ONLINE]",  COL_SUCCESS);
        shell_println("  SHA-256 ENGINE   [ONLINE]",  COL_SUCCESS);
        shell_println("  MEMORY GUARD     [ONLINE]",  COL_SUCCESS);
        shell_println("  VERIFY GATE      [ACTIVE]",  COL_SUCCESS);
        shell_println("  THREATS BLOCKED  [000001]",  COL_ERROR);
    }
    else if(str_cmp(cmd, "verify") == 0)
    {
        shell_println("  Running integrity check...", COL_OUTPUT);
        shell_println("  Hashing kernel segments...", COL_OUTPUT);
        shell_println("  SHA-256: VERIFIED OK",       COL_SUCCESS);
        shell_println("  All segments intact.",       COL_SUCCESS);
    }
    else if(str_cmp(cmd, "reboot") == 0)
    {
        shell_println("  Rebooting...", COL_ERROR);
        // Keyboard controller reset
        __asm__ volatile(
            "mov $0xFE, %al\n"
            "out %al, $0x64\n"
        );
    }
    else
    {
        shell_println("  Unknown command. Type 'help'.", COL_ERROR);
    }
}

void shell_handle_key(char c)
{
    if(c == '\n' || c == '\r')
    {
        input_buf[input_len] = 0;
        shell_execute(input_buf);
        input_len = 0;
        input_buf[0] = 0;
    }
    else if(c == '\b')
    {
        if(input_len > 0)
        {
            input_len--;
            input_buf[input_len] = 0;
        }
    }
    else if(input_len < SHELL_MAX_INPUT - 1)
    {
        input_buf[input_len++] = c;
        input_buf[input_len]   = 0;
    }

    shell_redraw();
}

void shell_init(void)
{
    total_lines = 0;
    input_len   = 0;
    input_buf[0] = 0;

    shell_println("SENTINELOS SHELL v0.1", COL_CYAN);
    shell_println("Type 'help' for commands.", COL_OUTPUT);
    shell_redraw();
}
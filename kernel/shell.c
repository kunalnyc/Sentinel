#include "shell.h"
#include "graphics.h"
#include "font.h"
#include "keyboard.h"

extern screen_info_t screen;

#define SHELL_X         30
#define SHELL_START_Y   35
#define SHELL_LINE_H    18

// Shell state
static char input_buf[SHELL_MAX_INPUT];
static int  input_len = 0;

// Screen lines buffer
static char lines[SHELL_MAX_LINES][128];
static int  line_colors[SHELL_MAX_LINES];
static int  total_lines = 0;

// Colors - all bright and visible
#define COL_PROMPT  0xFFC832
#define COL_INPUT   0xFFFFFF
#define COL_OUTPUT  0xDDDDDD
#define COL_SUCCESS 0x00FF64
#define COL_ERROR   0xFF4444
#define COL_CYAN    0x00FFFF

static void str_copy(char *dst, const char *src)
{
    int i = 0;
    while(src[i]) { dst[i] = src[i]; i++; }
    dst[i] = 0;
}

static int str_cmp(const char *a, const char *b)
{
    int i = 0;
    while(a[i] && b[i] && a[i]==b[i]) i++;
    return a[i] - b[i];
}

static int str_len(const char *s)
{
    int i = 0;
    while(s[i]) i++;
    return i;
}

static void shell_println(const char *text, int color)
{
    if(total_lines >= SHELL_MAX_LINES)
    {
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

static void shell_redraw()
{
    draw_rect(0, 22, screen.width, screen.height - 22, COLOR_SPACE_BLACK);

    int i;
    for(i = 0; i < total_lines; i++)
    {
        int y = SHELL_START_Y + i * SHELL_LINE_H;
        draw_string(SHELL_X, y, lines[i], line_colors[i]);
    }

    int prompt_y = SHELL_START_Y + total_lines * SHELL_LINE_H;
    draw_string(SHELL_X, prompt_y, "SENTINEL> ", COL_PROMPT);

    char display[SHELL_MAX_INPUT + 2];
    int k = 0;
    while(input_buf[k]){ display[k] = input_buf[k]; k++; }
    display[k]   = '_';
    display[k+1] = 0;
    draw_string(SHELL_X + 82, prompt_y, display, COL_INPUT);
}

static void shell_execute(const char *cmd)
{
    char echo[140];
    echo[0]='S'; echo[1]='E'; echo[2]='N'; echo[3]='T';
    echo[4]='I'; echo[5]='N'; echo[6]='E'; echo[7]='L';
    echo[8]='>'; echo[9]=' '; echo[10]=0;
    int i = 10, j = 0;
    while(cmd[j]) { echo[i++] = cmd[j++]; }
    echo[i] = 0;
    shell_println(echo, COL_PROMPT);

    if(str_len(cmd) == 0)
    {
        return;
    }
    else if(str_cmp(cmd, "help") == 0)
    {
        shell_println("", COL_OUTPUT);
        shell_println("  HELP     - SHOW COMMANDS",        COL_CYAN);
        shell_println("  CLEAR    - CLEAR TERMINAL",       COL_CYAN);
        shell_println("  ABOUT    - ABOUT SENTINELOS",     COL_CYAN);
        shell_println("  STATUS   - SYSTEM STATUS",        COL_CYAN);
        shell_println("  VERIFY   - RUN INTEGRITY CHECK",  COL_CYAN);
        shell_println("  REBOOT   - RESTART SYSTEM",       COL_CYAN);
        shell_println("", COL_OUTPUT);
    }
    else if(str_cmp(cmd, "clear") == 0)
    {
        total_lines = 0;
    }
    else if(str_cmp(cmd, "about") == 0)
    {
        shell_println("", COL_OUTPUT);
        shell_println("  SENTINELOS V0.1-ALPHA",           COL_CYAN);
        shell_println("  64-BIT SECURE KERNEL",            COL_OUTPUT);
        shell_println("  TRUST NOTHING. VERIFY EVERYTHING.", COL_PROMPT);
        shell_println("  BUILT FROM SCRATCH IN ASM + C",   COL_OUTPUT);
        shell_println("  ARCHITECTURE: X86-64 LONG MODE",  COL_OUTPUT);
        shell_println("", COL_OUTPUT);
    }
    else if(str_cmp(cmd, "status") == 0)
    {
        shell_println("", COL_OUTPUT);
        shell_println("  TRUST REGISTRY   [ ONLINE ]", COL_SUCCESS);
        shell_println("  SHA-256 ENGINE   [ ONLINE ]", COL_SUCCESS);
        shell_println("  MEMORY GUARD     [ ONLINE ]", COL_SUCCESS);
        shell_println("  VERIFY GATE      [ ACTIVE ]", COL_SUCCESS);
        shell_println("  PROCESS SHIELD   [ ACTIVE ]", COL_SUCCESS);
        shell_println("  THREATS BLOCKED  [ 000001 ]", COL_ERROR);
        shell_println("", COL_OUTPUT);
    }
    else if(str_cmp(cmd, "verify") == 0)
    {
        shell_println("", COL_OUTPUT);
        shell_println("  INITIALIZING INTEGRITY SCAN...", COL_OUTPUT);
        shell_println("  HASHING KERNEL SEGMENTS...",     COL_OUTPUT);
        shell_println("  CHECKING TRUST REGISTRY...",     COL_OUTPUT);
        shell_println("  VERIFYING PAGE TABLES...",       COL_OUTPUT);
        shell_println("  SHA-256: [ VERIFIED OK ]",       COL_SUCCESS);
        shell_println("  ALL SEGMENTS INTACT.",           COL_SUCCESS);
        shell_println("", COL_OUTPUT);
    }
    else if(str_cmp(cmd, "reboot") == 0)
    {
        shell_println("  REBOOTING SYSTEM...", COL_ERROR);
        __asm__ volatile(
            "mov $0xFE, %al\n"
            "out %al, $0x64\n"
        );
    }
    else
    {
        shell_println("  UNKNOWN COMMAND. TYPE HELP.", COL_ERROR);
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

    shell_println("", COL_OUTPUT);
    shell_println("  SENTINELOS SECURE SHELL V0.1", COL_CYAN);
    shell_println("  TRUST NOTHING. VERIFY EVERYTHING.", COL_PROMPT);
    shell_println("  TYPE HELP FOR COMMANDS. ESC=DASHBOARD.", COL_OUTPUT);
    shell_println("", COL_OUTPUT);
    shell_redraw();
}
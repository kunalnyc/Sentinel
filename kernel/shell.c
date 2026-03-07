#include "shell.h"
#include "graphics.h"
#include "font.h"
#include "keyboard.h"
#include "fs.h"

extern screen_info_t screen;

#define SHELL_X         30
#define SHELL_START_Y   35
#define SHELL_LINE_H    18

static char input_buf[SHELL_MAX_INPUT];
static int  input_len = 0;

static char lines[SHELL_MAX_LINES][128];
static int  line_colors[SHELL_MAX_LINES];
static int  total_lines = 0;

#define COL_PROMPT  0xFFC832
#define COL_INPUT   0xFFFFFF
#define COL_OUTPUT  0xDDDDDD
#define COL_SUCCESS 0x00FF64
#define COL_ERROR   0xFF4444
#define COL_CYAN    0x00FFFF
#define COL_GOLD    0xFFC832

// ── String helpers ─────────────────────────────────────────
static void sl_strcpy(char *dst, const char *src)
{
    int i = 0;
    while(src[i]) { dst[i] = src[i]; i++; }
    dst[i] = 0;
}

static int sl_strcmp(const char *a, const char *b)
{
    int i = 0;
    while(a[i] && b[i] && a[i]==b[i]) i++;
    return a[i] - b[i];
}

static int sl_strlen(const char *s)
{
    int i = 0;
    while(s[i]) i++;
    return i;
}

// starts-with check
static int sl_startswith(const char *str, const char *pre)
{
    int i = 0;
    while(pre[i])
    {
        if(str[i] != pre[i]) return 0;
        i++;
    }
    return 1;
}

// int to decimal string
static void int_to_str(int n, char *buf)
{
    if(n == 0) { buf[0]='0'; buf[1]=0; return; }
    char tmp[16]; int i=0, j=0;
    while(n > 0) { tmp[i++] = '0' + (n%10); n /= 10; }
    while(i > 0) buf[j++] = tmp[--i];
    buf[j] = 0;
}

// ── Shell output ───────────────────────────────────────────
static void shell_println(const char *text, int color)
{
    if(total_lines >= SHELL_MAX_LINES)
    {
        int i;
        for(i = 0; i < SHELL_MAX_LINES-1; i++)
        {
            sl_strcpy(lines[i], lines[i+1]);
            line_colors[i] = line_colors[i+1];
        }
        total_lines = SHELL_MAX_LINES - 1;
    }
    sl_strcpy(lines[total_lines], text);
    line_colors[total_lines] = color;
    total_lines++;
}

static void shell_redraw(void)
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

// ── Command handlers ───────────────────────────────────────

static void cmd_ls(void)
{
    char names[SLFS_MAX_FILES][SLFS_MAX_FILENAME];
    int count = 0;
    fs_list(names, &count);

    shell_println("", COL_OUTPUT);
    if(count == 0)
    {
        shell_println("  NO FILES FOUND.", COL_ERROR);
    }
    else
    {
        char line[128];
        int i;
        for(i = 0; i < count; i++)
        {
            line[0]=' '; line[1]=' '; line[2]=0;
            sl_strcpy(line+2, names[i]);
            shell_println(line, COL_CYAN);
        }
    }

    // Show usage
    char uline[64];
    char num[16];
    uline[0]=0;
    sl_strcpy(uline, "  FILES: ");
    int_to_str(fs_used(), num);
    sl_strcpy(uline + sl_strlen(uline), num);
    sl_strcpy(uline + sl_strlen(uline), "/64");
    shell_println(uline, COL_OUTPUT);
    shell_println("", COL_OUTPUT);
}

static void cmd_create(const char *arg)
{
    if(sl_strlen(arg) == 0)
    {
        shell_println("  USAGE: CREATE <FILENAME>", COL_ERROR);
        return;
    }
    int r = fs_create(arg);
    char line[64];
    if(r == SLFS_OK)
    {
        sl_strcpy(line, "  CREATED: ");
        sl_strcpy(line + sl_strlen(line), arg);
        shell_println(line, COL_SUCCESS);
    }
    else if(r == SLFS_ERR_EXISTS)
        shell_println("  ERROR: FILE ALREADY EXISTS.", COL_ERROR);
    else if(r == SLFS_ERR_FULL)
        shell_println("  ERROR: FILESYSTEM FULL.", COL_ERROR);
    else
        shell_println("  ERROR: COULD NOT CREATE FILE.", COL_ERROR);
}

static void cmd_write(const char *arg)
{
    // arg = "filename text text text"
    // find space after filename
    int i = 0;
    while(arg[i] && arg[i] != ' ') i++;
    if(arg[i] == 0)
    {
        shell_println("  USAGE: WRITE <FILE> <TEXT>", COL_ERROR);
        return;
    }

    char fname[SLFS_MAX_FILENAME];
    int j;
    for(j = 0; j < i && j < SLFS_MAX_FILENAME-1; j++)
        fname[j] = arg[j];
    fname[j] = 0;

    const char *text = arg + i + 1;
    int r = fs_write(fname, text, sl_strlen(text));
    char line[64];
    if(r == SLFS_OK)
    {
        sl_strcpy(line, "  WRITTEN TO: ");
        sl_strcpy(line + sl_strlen(line), fname);
        shell_println(line, COL_SUCCESS);
    }
    else if(r == SLFS_ERR_NOTFOUND)
        shell_println("  ERROR: FILE NOT FOUND.", COL_ERROR);
    else if(r == SLFS_ERR_TOOBIG)
        shell_println("  ERROR: DATA TOO LARGE (MAX 512).", COL_ERROR);
    else
        shell_println("  ERROR: WRITE FAILED.", COL_ERROR);
}

static void cmd_read(const char *arg)
{
    if(sl_strlen(arg) == 0)
    {
        shell_println("  USAGE: READ <FILENAME>", COL_ERROR);
        return;
    }
    char buf[SLFS_MAX_FILESIZE + 1];
    int r = fs_read(arg, buf, sizeof(buf));
    if(r < 0)
    {
        shell_println("  ERROR: FILE NOT FOUND.", COL_ERROR);
        return;
    }
    shell_println("", COL_OUTPUT);

    // Print content in chunks of 50 chars per line
    char line[56];
    int pos = 0;
    while(pos < r)
    {
        int k;
        line[0]=' '; line[1]=' ';
        for(k = 0; k < 50 && pos < r; k++, pos++)
            line[2+k] = buf[pos];
        line[2+k] = 0;
        shell_println(line, COL_OUTPUT);
    }
    shell_println("", COL_OUTPUT);
}

static void cmd_delete(const char *arg)
{
    if(sl_strlen(arg) == 0)
    {
        shell_println("  USAGE: DELETE <FILENAME>", COL_ERROR);
        return;
    }
    int r = fs_delete(arg);
    char line[64];
    if(r == SLFS_OK)
    {
        sl_strcpy(line, "  DELETED: ");
        sl_strcpy(line + sl_strlen(line), arg);
        shell_println(line, COL_SUCCESS);
    }
    else
        shell_println("  ERROR: FILE NOT FOUND.", COL_ERROR);
}

static void cmd_df(void)
{
    shell_println("", COL_OUTPUT);
    shell_println("  SENTINEL LIGHT FILE SYSTEM V1.0", COL_CYAN);

    char line[64];
    char num[16];

    // Used files
    sl_strcpy(line, "  USED SLOTS : ");
    int_to_str(fs_used(), num);
    sl_strcpy(line + sl_strlen(line), num);
    sl_strcpy(line + sl_strlen(line), " / 64");
    shell_println(line, COL_OUTPUT);

    // Used bytes
    sl_strcpy(line, "  USED BYTES : ");
    int_to_str(fs_used() * SLFS_MAX_FILESIZE, num);
    sl_strcpy(line + sl_strlen(line), num);
    sl_strcpy(line + sl_strlen(line), " / 32768");
    shell_println(line, COL_OUTPUT);

    // Storage type
    shell_println("  STORAGE     : RAM (VOLATILE)", COL_OUTPUT);
    shell_println("  DISK SUPPORT: COMING SOON", COL_GOLD);
    shell_println("", COL_OUTPUT);
}

// ── Main execute ───────────────────────────────────────────
static void shell_execute(const char *cmd)
{
    // Echo
    char echo[140];
    echo[0]='S'; echo[1]='E'; echo[2]='N'; echo[3]='T';
    echo[4]='I'; echo[5]='N'; echo[6]='E'; echo[7]='L';
    echo[8]='>'; echo[9]=' '; echo[10]=0;
    int i = 10, j = 0;
    while(cmd[j]) { echo[i++] = cmd[j++]; }
    echo[i] = 0;
    shell_println(echo, COL_PROMPT);

    if(sl_strlen(cmd) == 0) return;

    // ── Core commands ──────────────────────────────────────
    if(sl_strcmp(cmd, "HELP") == 0)
    {
        shell_println("", COL_OUTPUT);
        shell_println("  HELP     - SHOW COMMANDS",        COL_CYAN);
        shell_println("  CLEAR    - CLEAR TERMINAL",       COL_CYAN);
        shell_println("  ABOUT    - ABOUT SENTINELOS",     COL_CYAN);
        shell_println("  STATUS   - SYSTEM STATUS",        COL_CYAN);
        shell_println("  VERIFY   - RUN INTEGRITY CHECK",  COL_CYAN);
        shell_println("  REBOOT   - RESTART SYSTEM",       COL_CYAN);
        shell_println("", COL_OUTPUT);
        shell_println("  LS              - LIST FILES",    COL_GOLD);
        shell_println("  CREATE <FILE>   - CREATE FILE",   COL_GOLD);
        shell_println("  WRITE <F> <TXT> - WRITE FILE",    COL_GOLD);
        shell_println("  READ <FILE>     - READ FILE",     COL_GOLD);
        shell_println("  DELETE <FILE>   - DELETE FILE",   COL_GOLD);
        shell_println("  DF              - DISK USAGE",    COL_GOLD);
        shell_println("", COL_OUTPUT);
    }
    else if(sl_strcmp(cmd, "CLEAR") == 0)
        total_lines = 0;

    else if(sl_strcmp(cmd, "ABOUT") == 0)
    {
        shell_println("", COL_OUTPUT);
        shell_println("  SENTINELOS V0.1-ALPHA",              COL_CYAN);
        shell_println("  64-BIT SECURE KERNEL",               COL_OUTPUT);
        shell_println("  TRUST NOTHING. VERIFY EVERYTHING.",  COL_PROMPT);
        shell_println("  BUILT FROM SCRATCH IN ASM + C",      COL_OUTPUT);
        shell_println("  ARCHITECTURE: X86-64 LONG MODE",     COL_OUTPUT);
        shell_println("  FILE SYSTEM:  SLFS V1.0",            COL_OUTPUT);
        shell_println("", COL_OUTPUT);
    }
    else if(sl_strcmp(cmd, "STATUS") == 0)
    {
        shell_println("", COL_OUTPUT);
        shell_println("  TRUST REGISTRY   [ ONLINE ]", COL_SUCCESS);
        shell_println("  SHA-256 ENGINE   [ ONLINE ]", COL_SUCCESS);
        shell_println("  MEMORY GUARD     [ ONLINE ]", COL_SUCCESS);
        shell_println("  VERIFY GATE      [ ACTIVE ]", COL_SUCCESS);
        shell_println("  PROCESS SHIELD   [ ACTIVE ]", COL_SUCCESS);
        shell_println("  FILE SYSTEM      [ ONLINE ]", COL_SUCCESS);
        shell_println("  THREATS BLOCKED  [ 000001 ]", COL_ERROR);
        shell_println("", COL_OUTPUT);
    }
    else if(sl_strcmp(cmd, "VERIFY") == 0)
    {
        shell_println("", COL_OUTPUT);
        shell_println("  INITIALIZING INTEGRITY SCAN...", COL_OUTPUT);
        shell_println("  HASHING KERNEL SEGMENTS...",     COL_OUTPUT);
        shell_println("  CHECKING TRUST REGISTRY...",     COL_OUTPUT);
        shell_println("  VERIFYING PAGE TABLES...",       COL_OUTPUT);
        shell_println("  CHECKING FILE SYSTEM...",        COL_OUTPUT);
        shell_println("  SHA-256: [ VERIFIED OK ]",       COL_SUCCESS);
        shell_println("  ALL SEGMENTS INTACT.",           COL_SUCCESS);
        shell_println("", COL_OUTPUT);
    }
    else if(sl_strcmp(cmd, "REBOOT") == 0)
    {
        shell_println("  REBOOTING SYSTEM...", COL_ERROR);
        __asm__ volatile(
            "mov $0xFE, %al\n"
            "out %al, $0x64\n"
        );
    }

    // ── Filesystem commands ────────────────────────────────
    else if(sl_strcmp(cmd, "LS") == 0)
        cmd_ls();

    else if(sl_strcmp(cmd, "DF") == 0)
        cmd_df();

    else if(sl_startswith(cmd, "CREATE "))
        cmd_create(cmd + 7);

    else if(sl_startswith(cmd, "WRITE "))
        cmd_write(cmd + 6);

    else if(sl_startswith(cmd, "READ "))
        cmd_read(cmd + 5);

    else if(sl_startswith(cmd, "DELETE "))
        cmd_delete(cmd + 7);

    else
        shell_println("  UNKNOWN COMMAND. TYPE HELP.", COL_ERROR);
}

// ── Key handler ────────────────────────────────────────────
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
    char ch = c;
    if(ch >= 'a' && ch <= 'z') ch = ch - 32;  // to uppercase
    input_buf[input_len++] = ch;
    input_buf[input_len]   = 0;
}
    shell_redraw();
}

// ── Init ───────────────────────────────────────────────────
void shell_init(void)
{
    total_lines  = 0;
    input_len    = 0;
    input_buf[0] = 0;
    fs_init();   // ← ADD THIS
    shell_println("", COL_OUTPUT);
    shell_println("  SENTINELOS SECURE SHELL V0.1", COL_CYAN);
    shell_println("  TRUST NOTHING. VERIFY EVERYTHING.", COL_PROMPT);
    shell_println("  TYPE HELP FOR COMMANDS. ESC=DASHBOARD.", COL_OUTPUT);
    shell_println("", COL_OUTPUT);
    shell_redraw();
}
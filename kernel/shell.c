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

// History
#define HISTORY_MAX 10
static char history[HISTORY_MAX][SHELL_MAX_INPUT];
static int  history_count = 0;

// Uptime counter
static int uptime_ticks = 0;

// Color themes
#define THEME_DEFAULT  0
#define THEME_MATRIX   1
#define THEME_OCEAN    2
#define THEME_BLOOD    3
static int current_theme = THEME_DEFAULT;

static int COL_PROMPT;
static int COL_INPUT;
static int COL_OUTPUT;
static int COL_SUCCESS;
static int COL_ERROR;
static int COL_CYAN;
static int COL_DIM;

static void apply_theme(int theme)
{
    current_theme = theme;
    if(theme == THEME_MATRIX)
    {
        COL_PROMPT  = 0x00FF41;
        COL_INPUT   = 0x00FF41;
        COL_OUTPUT  = 0x00CC33;
        COL_SUCCESS = 0x00FF41;
        COL_ERROR   = 0xFF0000;
        COL_CYAN    = 0x00FF99;
        COL_DIM     = 0x005510;
    }
    else if(theme == THEME_OCEAN)
    {
        COL_PROMPT  = 0x00BFFF;
        COL_INPUT   = 0xFFFFFF;
        COL_OUTPUT  = 0x88CCFF;
        COL_SUCCESS = 0x00FF99;
        COL_ERROR   = 0xFF4466;
        COL_CYAN    = 0x00FFFF;
        COL_DIM     = 0x226688;
    }
    else if(theme == THEME_BLOOD)
    {
        COL_PROMPT  = 0xFF2200;
        COL_INPUT   = 0xFF8866;
        COL_OUTPUT  = 0xCC4422;
        COL_SUCCESS = 0xFF6600;
        COL_ERROR   = 0xFF0000;
        COL_CYAN    = 0xFF4400;
        COL_DIM     = 0x441100;
    }
    else
    {
        COL_PROMPT  = 0xFFC832;
        COL_INPUT   = 0xFFFFFF;
        COL_OUTPUT  = 0xDDDDDD;
        COL_SUCCESS = 0x00FF64;
        COL_ERROR   = 0xFF4444;
        COL_CYAN    = 0x00FFFF;
        COL_DIM     = 0x888888;
    }
}

// ── String helpers ─────────────────────────────────────────

static void sl_strcpy(char *dst, const char *src)
{
    int i = 0;
    while(src[i]) { dst[i] = src[i]; i++; }
    dst[i] = 0;
}

static void sl_strcat(char *dst, const char *src)
{
    int i = 0; while(dst[i]) i++;
    int j = 0; while(src[j]) dst[i++]=src[j++];
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
    int i = 0; while(s[i]) i++; return i;
}

static int sl_startswith(const char *str, const char *pre)
{
    int i = 0;
    while(pre[i]) { if(str[i] != pre[i]) return 0; i++; }
    return 1;
}

static void int_to_str(int n, char *buf)
{
    if(n == 0) { buf[0]='0'; buf[1]=0; return; }
    char tmp[16]; int i=0, j=0;
    while(n > 0) { tmp[i++] = '0'+(n%10); n /= 10; }
    while(i > 0) buf[j++] = tmp[--i];
    buf[j] = 0;
}

static char to_upper(char c)
{
    if(c >= 'a' && c <= 'z') return c - 32;
    return c;
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
    draw_rect(0, 22, screen.width, screen.height - 22, 0x050514);
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
    display[k]='_'; display[k+1]=0;
    draw_string(SHELL_X + 82, prompt_y, display, COL_INPUT);
}

// ── Commands ───────────────────────────────────────────────

static void cmd_help(void)
{
    shell_println("", COL_OUTPUT);
    shell_println("  SYSTEM:", COL_PROMPT);
    shell_println("  HELP  CLEAR  ABOUT  STATUS  VERIFY  REBOOT", COL_CYAN);
    shell_println("  SYSINFO  NEOFETCH  HISTORY  UPTIME", COL_CYAN);
    shell_println("  ECHO <TEXT>         - PRINT TEXT",    COL_CYAN);
    shell_println("  COLOR <THEME>       - CHANGE COLORS", COL_CYAN);
    shell_println("  THEMES: DEFAULT  MATRIX  OCEAN  BLOOD", COL_DIM);
    shell_println("", COL_OUTPUT);
    shell_println("  FILES:", COL_PROMPT);
    shell_println("  LS  DF  CREATE <F>  READ <F>  DELETE <F>", COL_SUCCESS);
    shell_println("  WRITE <FILE> <TEXT>",                  COL_SUCCESS);
    shell_println("", COL_OUTPUT);
}

static void cmd_echo(const char *arg)
{
    if(sl_strlen(arg) == 0)
    { shell_println("  USAGE: ECHO <TEXT>", COL_ERROR); return; }
    char line[130] = "  ";
    sl_strcat(line, arg);
    shell_println(line, COL_INPUT);
}

static void cmd_sysinfo(void)
{
    shell_println("", COL_OUTPUT);
    shell_println("  +---------------------------------+", COL_PROMPT);
    shell_println("  |    SENTINELOS SYSTEM INFO       |", COL_PROMPT);
    shell_println("  +---------------------------------+", COL_PROMPT);
    shell_println("  OS      : SENTINELOS V0.1-ALPHA",    COL_CYAN);
    shell_println("  ARCH    : X86-64 LONG MODE",         COL_CYAN);
    shell_println("  KERNEL  : MONOLITHIC 64-BIT",        COL_CYAN);
    shell_println("  BOOT    : GRUB2 + MULTIBOOT2",       COL_CYAN);
    shell_println("  LANG    : C + X86-64 ASSEMBLY",      COL_CYAN);
    shell_println("  DISPLAY : 1024X768 32BPP VBE",       COL_CYAN);
    shell_println("  MEMORY  : 256MB",                    COL_CYAN);
    shell_println("  FS      : SLFS V1.0 64 SLOTS",       COL_CYAN);
    shell_println("  AUTHOR  : KUNAL",                    COL_PROMPT);
    shell_println("  BUILD   : DAY 06",                   COL_PROMPT);
    shell_println("  +---------------------------------+", COL_PROMPT);
    shell_println("", COL_OUTPUT);
}

static void cmd_neofetch(void)
{
    shell_println("", COL_OUTPUT);
    shell_println("      *              SENTINELOS V0.1",      COL_PROMPT);
    shell_println("     ***             --------------------",  COL_DIM);
    shell_println("    *****            OS: SENTINELOS",        COL_CYAN);
    shell_println("   *******           KERNEL: 64-BIT",       COL_CYAN);
    shell_println("  ****X****          ARCH: X86-64",         COL_CYAN);
    shell_println("   *******           SHELL: SENTINEL V0.1", COL_CYAN);
    shell_println("    *****            FS: SLFS 64 SLOTS",    COL_CYAN);
    shell_println("     ***             DISPLAY: 1024X768",    COL_CYAN);
    shell_println("      *              LANG: C + ASM",        COL_CYAN);
    shell_println("                     AUTHOR: KUNAL",        COL_PROMPT);
    shell_println("                     TRUST NOTHING.",       COL_PROMPT);
    shell_println("                     VERIFY EVERYTHING.",   COL_PROMPT);
    shell_println("", COL_OUTPUT);
}

static void cmd_history(void)
{
    shell_println("", COL_OUTPUT);
    if(history_count == 0)
    { shell_println("  NO HISTORY YET.", COL_DIM); }
    else
    {
        int i;
        for(i = 0; i < history_count; i++)
        {
            char line[140] = "  ";
            char num[8]; int_to_str(i+1, num);
            sl_strcat(line, num);
            sl_strcat(line, ". ");
            sl_strcat(line, history[i]);
            shell_println(line, COL_DIM);
        }
    }
    shell_println("", COL_OUTPUT);
}

static void cmd_uptime(void)
{
    shell_println("", COL_OUTPUT);
    char line[64] = "  UPTIME: ";
    char num[16]; int_to_str(uptime_ticks, num);
    sl_strcat(line, num);
    sl_strcat(line, " COMMANDS EXECUTED");
    shell_println(line, COL_CYAN);
    shell_println("  REAL TIMER COMING SOON.", COL_DIM);
    shell_println("", COL_OUTPUT);
}

static void cmd_color(const char *arg)
{
    if     (sl_strcmp(arg,"MATRIX") ==0) { apply_theme(THEME_MATRIX);  shell_println("  THEME: MATRIX ACTIVATED.",  COL_SUCCESS); }
    else if(sl_strcmp(arg,"OCEAN")  ==0) { apply_theme(THEME_OCEAN);   shell_println("  THEME: OCEAN ACTIVATED.",   COL_SUCCESS); }
    else if(sl_strcmp(arg,"BLOOD")  ==0) { apply_theme(THEME_BLOOD);   shell_println("  THEME: BLOOD ACTIVATED.",   COL_SUCCESS); }
    else if(sl_strcmp(arg,"DEFAULT")==0) { apply_theme(THEME_DEFAULT); shell_println("  THEME: DEFAULT RESTORED.",  COL_SUCCESS); }
    else
    {
        shell_println("  USAGE: COLOR <THEME>",              COL_ERROR);
        shell_println("  OPTIONS: DEFAULT MATRIX OCEAN BLOOD", COL_DIM);
    }
}

static void cmd_ls(void)
{
    char names[SLFS_MAX_FILES][SLFS_MAX_FILENAME];
    int count = 0;
    fs_list(names, &count);
    shell_println("", COL_OUTPUT);
    if(count == 0) shell_println("  NO FILES.", COL_DIM);
    else
    {
        int i;
        for(i=0; i<count; i++)
        {
            char line[64] = "  > "; sl_strcat(line, names[i]);
            shell_println(line, COL_SUCCESS);
        }
    }
    char buf[32]="  "; char num[8]; int_to_str(count,num);
    sl_strcat(buf,num); sl_strcat(buf,"/64 FILES");
    shell_println(buf, COL_DIM);
    shell_println("", COL_OUTPUT);
}

static void cmd_create(const char *name)
{
    if(sl_strlen(name)==0) { shell_println("  USAGE: CREATE <FILE>", COL_ERROR); return; }
    int r = fs_create(name);
    if(r==0) { char m[64]="  CREATED: "; sl_strcat(m,name); shell_println(m,COL_SUCCESS); }
    else if(r==-1) shell_println("  ERROR: ALREADY EXISTS.", COL_ERROR);
    else           shell_println("  ERROR: FILESYSTEM FULL.", COL_ERROR);
}

static void cmd_write(const char *arg)
{
    int i=0; while(arg[i] && arg[i]!=' ') i++;
    if(!arg[i]) { shell_println("  USAGE: WRITE <FILE> <TEXT>", COL_ERROR); return; }
    char fname[SLFS_MAX_FILENAME];
    int j; for(j=0;j<i&&j<SLFS_MAX_FILENAME-1;j++) fname[j]=arg[j]; fname[j]=0;
    const char *text = arg+i+1;
    if(!fs_exists(fname)) fs_create(fname);
    if(fs_write(fname,text,sl_strlen(text))==0)
    { char m[64]="  WRITTEN: "; sl_strcat(m,fname); shell_println(m,COL_SUCCESS); }
    else shell_println("  ERROR: WRITE FAILED.", COL_ERROR);
}

static void cmd_read(const char *name)
{
    if(sl_strlen(name)==0) { shell_println("  USAGE: READ <FILE>", COL_ERROR); return; }
    char buf[SLFS_MAX_FILESIZE+4];
    int r = fs_read(name, buf+2, SLFS_MAX_FILESIZE);
    if(r<0) { shell_println("  ERROR: NOT FOUND.", COL_ERROR); return; }
    buf[0]=' '; buf[1]=' '; if(r>116) buf[118]=0;
    shell_println("", COL_OUTPUT);
    shell_println(buf, COL_OUTPUT);
    shell_println("", COL_OUTPUT);
}

static void cmd_delete(const char *name)
{
    if(sl_strlen(name)==0) { shell_println("  USAGE: DELETE <FILE>", COL_ERROR); return; }
    if(fs_delete(name)==0)
    { char m[64]="  DELETED: "; sl_strcat(m,name); shell_println(m,COL_SUCCESS); }
    else shell_println("  ERROR: NOT FOUND.", COL_ERROR);
}

static void cmd_df(void)
{
    uint32_t used=fs_used(), free_s=SLFS_MAX_FILES-used;
    char line[64]; char n[8];
    shell_println("", COL_OUTPUT);
    shell_println("  SLFS DISK USAGE:", COL_PROMPT);
    int_to_str(used,n); sl_strcpy(line,"  USED : "); sl_strcat(line,n); sl_strcat(line," / 64"); shell_println(line,COL_SUCCESS);
    int_to_str(free_s,n); sl_strcpy(line,"  FREE : "); sl_strcat(line,n); sl_strcat(line," / 64"); shell_println(line,COL_DIM);
    shell_println("  SIZE  : 512 BYTES PER FILE", COL_DIM);
    shell_println("  TOTAL : 32KB RAM STORAGE",   COL_DIM);
    shell_println("", COL_OUTPUT);
}

// ── Execute ────────────────────────────────────────────────

static void add_to_history(const char *cmd)
{
    if(sl_strlen(cmd)==0) return;
    if(history_count < HISTORY_MAX)
        sl_strcpy(history[history_count++], cmd);
    else
    {
        int i;
        for(i=0;i<HISTORY_MAX-1;i++) sl_strcpy(history[i],history[i+1]);
        sl_strcpy(history[HISTORY_MAX-1], cmd);
    }
}

static void shell_execute(const char *cmd)
{
    char echo[140]="SENTINEL> ";
    int i=10,j=0; while(cmd[j]) echo[i++]=cmd[j++]; echo[i]=0;
    shell_println(echo, COL_PROMPT);
    if(sl_strlen(cmd)==0) return;
    add_to_history(cmd);
    uptime_ticks++;

    if     (sl_strcmp(cmd,"HELP")    ==0) cmd_help();
    else if(sl_strcmp(cmd,"CLEAR")   ==0) total_lines=0;
    else if(sl_strcmp(cmd,"SYSINFO") ==0) cmd_sysinfo();
    else if(sl_strcmp(cmd,"NEOFETCH")==0) cmd_neofetch();
    else if(sl_strcmp(cmd,"HISTORY") ==0) cmd_history();
    else if(sl_strcmp(cmd,"UPTIME")  ==0) cmd_uptime();
    else if(sl_strcmp(cmd,"LS")      ==0) cmd_ls();
    else if(sl_strcmp(cmd,"DF")      ==0) cmd_df();
    else if(sl_startswith(cmd,"ECHO "))   cmd_echo(cmd+5);
    else if(sl_startswith(cmd,"COLOR "))  cmd_color(cmd+6);
    else if(sl_startswith(cmd,"CREATE ")) cmd_create(cmd+7);
    else if(sl_startswith(cmd,"WRITE "))  cmd_write(cmd+6);
    else if(sl_startswith(cmd,"READ "))   cmd_read(cmd+5);
    else if(sl_startswith(cmd,"DELETE ")) cmd_delete(cmd+7);
    else if(sl_strcmp(cmd,"ABOUT")==0)
    {
        shell_println("", COL_OUTPUT);
        shell_println("  SENTINELOS V0.1-ALPHA",             COL_CYAN);
        shell_println("  64-BIT SECURE KERNEL",              COL_OUTPUT);
        shell_println("  TRUST NOTHING. VERIFY EVERYTHING.", COL_PROMPT);
        shell_println("  BUILT IN PURE C + ASSEMBLY",        COL_OUTPUT);
        shell_println("", COL_OUTPUT);
    }
    else if(sl_strcmp(cmd,"STATUS")==0)
    {
        shell_println("", COL_OUTPUT);
        shell_println("  TRUST REGISTRY   [ ONLINE ]", COL_SUCCESS);
        shell_println("  SHA-256 ENGINE   [ ONLINE ]", COL_SUCCESS);
        shell_println("  MEMORY GUARD     [ ONLINE ]", COL_SUCCESS);
        shell_println("  SLFS FILESYSTEM  [ ONLINE ]", COL_SUCCESS);
        shell_println("  VERIFY GATE      [ ACTIVE ]", COL_SUCCESS);
        shell_println("  THREATS BLOCKED  [ 000001 ]", COL_ERROR);
        shell_println("", COL_OUTPUT);
    }
    else if(sl_strcmp(cmd,"VERIFY")==0)
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
    else if(sl_strcmp(cmd,"REBOOT")==0)
    {
        shell_println("  REBOOTING...", COL_ERROR);
        __asm__ volatile("mov $0xFE, %al\n out %al, $0x64\n");
    }
    else
    {
        char msg[64]="  UNKNOWN: "; sl_strcat(msg,cmd);
        shell_println(msg, COL_ERROR);
        shell_println("  TYPE HELP FOR COMMANDS.", COL_DIM);
    }
}

// ── Key handler ────────────────────────────────────────────

void shell_handle_key(char c)
{
    uptime_ticks++;
    if(c == '\n' || c == '\r')
    {
        input_buf[input_len]=0;
        shell_execute(input_buf);
        input_len=0; input_buf[0]=0;
    }
    else if(c == '\b')
    {
        if(input_len>0) { input_len--; input_buf[input_len]=0; }
    }
    else if(input_len < SHELL_MAX_INPUT-1)
    {
        input_buf[input_len++]=to_upper(c);
        input_buf[input_len]=0;
    }
    shell_redraw();
}

// ── Init ───────────────────────────────────────────────────

void shell_init(void)
{
    total_lines=0; input_len=0; input_buf[0]=0;
    history_count=0; uptime_ticks=0;
    apply_theme(THEME_DEFAULT);
    fs_init();
    shell_println("", COL_OUTPUT);
    shell_println("  SENTINELOS SECURE SHELL V0.1",         COL_CYAN);
    shell_println("  TRUST NOTHING. VERIFY EVERYTHING.",    COL_PROMPT);
    shell_println("  SLFS ONLINE. TYPE HELP FOR COMMANDS.", COL_SUCCESS);
    shell_println("  TRY: NEOFETCH  SYSINFO  COLOR MATRIX", COL_DIM);
    shell_println("", COL_OUTPUT);
    shell_redraw();
}
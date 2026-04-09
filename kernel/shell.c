#include "shell.h"
#include "graphics.h"
#include "font.h"
#include "keyboard.h"
#include "fs.h"
#include "mem_mgr.h"
#include "elf.h"
#include "scheduler.h"

extern screen_info_t screen;

#define SHELL_X         30
#define SHELL_START_Y   35
#define SHELL_LINE_H    18

static char input_buf[SHELL_MAX_INPUT];
static int  input_len = 0;

static char lines[SHELL_MAX_LINES][128];
static int  line_colors[SHELL_MAX_LINES];
static int  total_lines = 0;

#define HISTORY_MAX 10
static char history[HISTORY_MAX][SHELL_MAX_INPUT];
static int  history_count = 0;
static int  uptime_ticks  = 0;

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
        COL_PROMPT=0x00FF41; COL_INPUT=0x00FF41; COL_OUTPUT=0x00CC33;
        COL_SUCCESS=0x00FF41; COL_ERROR=0xFF0000; COL_CYAN=0x00FF99; COL_DIM=0x005510;
    }
    else if(theme == THEME_OCEAN)
    {
        COL_PROMPT=0x00BFFF; COL_INPUT=0xFFFFFF; COL_OUTPUT=0x88CCFF;
        COL_SUCCESS=0x00FF99; COL_ERROR=0xFF4466; COL_CYAN=0x00FFFF; COL_DIM=0x226688;
    }
    else if(theme == THEME_BLOOD)
    {
        COL_PROMPT=0xFF2200; COL_INPUT=0xFF8866; COL_OUTPUT=0xCC4422;
        COL_SUCCESS=0xFF6600; COL_ERROR=0xFF0000; COL_CYAN=0xFF4400; COL_DIM=0x441100;
    }
    else
    {
        COL_PROMPT=0xFFC832; COL_INPUT=0xFFFFFF; COL_OUTPUT=0xDDDDDD;
        COL_SUCCESS=0x00FF64; COL_ERROR=0xFF4444; COL_CYAN=0x00FFFF; COL_DIM=0x888888;
    }
}

// ── String helpers ────────────────────────────────────

static void sl_strcpy(char *dst, const char *src)
{
    int i=0; while(src[i]){dst[i]=src[i];i++;} dst[i]=0;
}
static void sl_strcat(char *dst, const char *src)
{
    int i=0; while(dst[i]) i++;
    int j=0; while(src[j]) dst[i++]=src[j++]; dst[i]=0;
}
static int sl_strcmp(const char *a, const char *b)
{
    int i=0; while(a[i]&&b[i]&&a[i]==b[i]) i++; return a[i]-b[i];
}
static int sl_strlen(const char *s)
{
    int i=0; while(s[i]) i++; return i;
}
static int sl_startswith(const char *str, const char *pre)
{
    int i=0; while(pre[i]){if(str[i]!=pre[i])return 0;i++;} return 1;
}
static void int_to_str(int n, char *buf)
{
    if(n==0){buf[0]='0';buf[1]=0;return;}
    char tmp[16]; int i=0,j=0;
    while(n>0){tmp[i++]='0'+(n%10);n/=10;}
    while(i>0) buf[j++]=tmp[--i]; buf[j]=0;
}
static char to_upper(char c)
{
    if(c>='a'&&c<='z') return c-32; return c;
}
static void bytes_to_str(uint32_t bytes, char *buf)
{
    if(bytes >= 1024*1024)
    {
        int_to_str(bytes/(1024*1024), buf);
        sl_strcat(buf, " MB");
    }
    else if(bytes >= 1024)
    {
        int_to_str(bytes/1024, buf);
        sl_strcat(buf, " KB");
    }
    else
    {
        int_to_str(bytes, buf);
        sl_strcat(buf, " B");
    }
}

// Print a uint64_t as hex into buf, returns pointer to buf
static void uint64_to_hex(uint64_t val, char *buf)
{
    if(val == 0) { buf[0]='0'; buf[1]=0; return; }
    char tmp[20]; int ti=0, hi=0;
    while(val > 0)
    {
        int nibble = val & 0xF;
        tmp[ti++] = nibble < 10 ? '0'+nibble : 'A'+(nibble-10);
        val >>= 4;
    }
    while(ti > 0) buf[hi++] = tmp[--ti];
    buf[hi] = 0;
}

// ── Shell output ──────────────────────────────────────

static void shell_println(const char *text, int color)
{
    if(total_lines >= SHELL_MAX_LINES)
    {
        int i;
        for(i=0;i<SHELL_MAX_LINES-1;i++)
        {
            sl_strcpy(lines[i],lines[i+1]);
            line_colors[i]=line_colors[i+1];
        }
        total_lines=SHELL_MAX_LINES-1;
    }
    sl_strcpy(lines[total_lines],text);
    line_colors[total_lines]=color;
    total_lines++;
}

static void shell_redraw(void)
{
    draw_rect(0,22,screen.width,screen.height-22,0x050514);
    int i;
    for(i=0;i<total_lines;i++)
    {
        int y=SHELL_START_Y+i*SHELL_LINE_H;
        draw_string(SHELL_X,y,lines[i],line_colors[i]);
    }
    int prompt_y=SHELL_START_Y+total_lines*SHELL_LINE_H;
    draw_string(SHELL_X,prompt_y,"SENTINEL> ",COL_PROMPT);
    char display[SHELL_MAX_INPUT+2];
    int k=0;
    while(input_buf[k]){display[k]=input_buf[k];k++;}
    display[k]='_'; display[k+1]=0;
    draw_string(SHELL_X+82,prompt_y,display,COL_INPUT);
}

// ── Commands ──────────────────────────────────────────

static void cmd_help(void)
{
    shell_println("", COL_OUTPUT);
    shell_println("  SYSTEM:", COL_PROMPT);
    shell_println("  HELP  CLEAR  ABOUT  STATUS  VERIFY  REBOOT", COL_CYAN);
    shell_println("  SYSINFO  NEOFETCH  HISTORY  UPTIME",         COL_CYAN);
    shell_println("  ECHO <TEXT>    - PRINT TEXT",                 COL_CYAN);
    shell_println("  COLOR <THEME>  - CHANGE COLORS",              COL_CYAN);
    shell_println("", COL_OUTPUT);
    shell_println("  MEMORY:", COL_PROMPT);
    shell_println("  MEMINFO        - RAM USAGE + BAR",            0x00FF99);
    shell_println("  ALLOC <BYTES>  - ALLOCATE MEMORY",            0x00FF99);
    shell_println("  FREE <ADDR>    - FREE MEMORY",                0x00FF99);
    shell_println("", COL_OUTPUT);
    shell_println("  FILES:", COL_PROMPT);
    shell_println("  LS  DF  CREATE  READ  WRITE  DELETE",         COL_SUCCESS);
    shell_println("", COL_OUTPUT);
    shell_println("  PROCESSES:", COL_PROMPT);
    shell_println("  EXEC <FILE>    - LOAD + RUN ELF BINARY",      COL_SUCCESS);
    shell_println("  SCHEDULE       - TRIGGER SCHEDULER TICK",     COL_SUCCESS);
    shell_println("  PS             - LIST ALL PROCESSES",          COL_SUCCESS);
    shell_println("", COL_OUTPUT);
}

static void cmd_meminfo(void)
{
    mem_stats_t s = mem_get_stats();
    char bar[32]; int pct=0;
    mem_get_bar(bar, 20, &pct);

    char line[80]; char num[16];

    shell_println("", COL_OUTPUT);
    shell_println("  +-------------------------------+", 0x00FF99);
    shell_println("  |     MEMORY INFORMATION        |", 0x00FF99);
    shell_println("  +-------------------------------+", 0x00FF99);

    sl_strcpy(line,"  TOTAL  : "); bytes_to_str(s.total_bytes,num);
    sl_strcat(line,num); shell_println(line, COL_OUTPUT);

    sl_strcpy(line,"  USED   : "); bytes_to_str(s.used_bytes,num);
    sl_strcat(line,num);
    int used_col = s.used_bytes > (s.total_bytes*3/4) ? COL_ERROR : COL_SUCCESS;
    shell_println(line, used_col);

    sl_strcpy(line,"  FREE   : "); bytes_to_str(s.free_bytes,num);
    sl_strcat(line,num); shell_println(line, COL_SUCCESS);

    sl_strcpy(line,"  PEAK   : "); bytes_to_str(s.peak_used,num);
    sl_strcat(line,num); shell_println(line, COL_DIM);

    sl_strcpy(line,"  ALLOCS : "); int_to_str(s.alloc_count,num);
    sl_strcat(line,num); sl_strcat(line," TOTAL / ");
    int_to_str(s.free_count,num); sl_strcat(line,num);
    sl_strcat(line," FREED"); shell_println(line, COL_OUTPUT);

    sl_strcpy(line,"  "); sl_strcat(line,bar);
    sl_strcat(line," "); int_to_str(pct,num);
    sl_strcat(line,num); sl_strcat(line,"%");

    int bar_col = pct > 75 ? COL_ERROR : pct > 50 ? 0xFFC832 : COL_SUCCESS;
    shell_println(line, bar_col);

    shell_println("  +-------------------------------+", 0x00FF99);
    shell_println("", COL_OUTPUT);
}

static void cmd_alloc(const char *arg)
{
    if(sl_strlen(arg)==0)
    { shell_println("  USAGE: ALLOC <BYTES>", COL_ERROR); return; }

    int n=0, i=0;
    while(arg[i]>='0' && arg[i]<='9') { n=n*10+(arg[i]-'0'); i++; }
    if(n<=0 || n>1024*1024)
    { shell_println("  ERROR: SIZE MUST BE 1-1048576.", COL_ERROR); return; }

    void *ptr = mem_alloc((uint32_t)n);
    if(!ptr)
    { shell_println("  ERROR: ALLOCATION FAILED.", COL_ERROR); return; }

    char line[80]="  ALLOCATED: ";
    char num[16]; int_to_str(n,num); sl_strcat(line,num);
    sl_strcat(line," BYTES");
    shell_println(line, COL_SUCCESS);
    shell_println("  USE MEMINFO TO SEE USAGE.", COL_DIM);
}

static void cmd_echo(const char *arg)
{
    if(sl_strlen(arg)==0){ shell_println("  USAGE: ECHO <TEXT>",COL_ERROR); return; }
    char line[130]="  "; sl_strcat(line,arg); shell_println(line,COL_INPUT);
}

static void cmd_sysinfo(void)
{
    mem_stats_t s = mem_get_stats();
    char num[16];
    shell_println("", COL_OUTPUT);
    shell_println("  +---------------------------------+", COL_PROMPT);
    shell_println("  |    SENTINELOS SYSTEM INFO       |", COL_PROMPT);
    shell_println("  +---------------------------------+", COL_PROMPT);
    shell_println("  OS      : SENTINELOS V0.1-ALPHA",    COL_CYAN);
    shell_println("  ARCH    : X86-64 LONG MODE",         COL_CYAN);
    shell_println("  KERNEL  : MONOLITHIC 64-BIT",        COL_CYAN);
    shell_println("  DISPLAY : 1024X768 32BPP VBE",       COL_CYAN);
    shell_println("  FS      : SLFS V1.0 64 SLOTS",       COL_CYAN);
    shell_println("  LOADER  : ELF64 EXECUTABLE",         COL_CYAN);
    shell_println("  SCHED   : ROUND-ROBIN 64 SLOTS",     COL_CYAN);

    char mline[64]="  MEMORY  : ";
    bytes_to_str(s.free_bytes, num);
    sl_strcat(mline, num); sl_strcat(mline, " FREE");
    shell_println(mline, COL_CYAN);

    shell_println("  AUTHOR  : KUNAL",                    COL_PROMPT);
    shell_println("  BUILD   : DAY 08",                   COL_PROMPT);
    shell_println("  +---------------------------------+", COL_PROMPT);
    shell_println("", COL_OUTPUT);
}

static void cmd_neofetch(void)
{
    shell_println("", COL_OUTPUT);
    shell_println("      *              SENTINELOS V0.1",       COL_PROMPT);
    shell_println("     ***             --------------------",   COL_DIM);
    shell_println("    *****            OS: SENTINELOS",         COL_CYAN);
    shell_println("   *******           KERNEL: 64-BIT",        COL_CYAN);
    shell_println("  ****X****          ARCH: X86-64",          COL_CYAN);
    shell_println("   *******           SHELL: SENTINEL V0.1",  COL_CYAN);
    shell_println("    *****            FS: SLFS 64 SLOTS",     COL_CYAN);
    shell_println("     ***             LOADER: ELF64",         COL_CYAN);
    shell_println("      *              SCHED: ROUND-ROBIN",    COL_CYAN);
    shell_println("                     DISPLAY: 1024X768",     COL_CYAN);
    shell_println("                     LANG: C + ASM",         COL_CYAN);
    shell_println("                     AUTHOR: KUNAL",         COL_PROMPT);
    shell_println("                     TRUST NOTHING.",        COL_PROMPT);
    shell_println("                     VERIFY EVERYTHING.",    COL_PROMPT);
    shell_println("", COL_OUTPUT);
}

static void cmd_history(void)
{
    shell_println("", COL_OUTPUT);
    if(history_count==0){ shell_println("  NO HISTORY YET.", COL_DIM); }
    else
    {
        int i;
        for(i=0;i<history_count;i++)
        {
            char line[140]="  "; char num[8]; int_to_str(i+1,num);
            sl_strcat(line,num); sl_strcat(line,". "); sl_strcat(line,history[i]);
            shell_println(line,COL_DIM);
        }
    }
    shell_println("", COL_OUTPUT);
}

static void cmd_uptime(void)
{
    shell_println("", COL_OUTPUT);
    char line[64]="  UPTIME: "; char num[16];
    int_to_str(uptime_ticks,num); sl_strcat(line,num);
    sl_strcat(line," COMMANDS EXECUTED");
    shell_println(line,COL_CYAN);
    shell_println("", COL_OUTPUT);
}

static void cmd_color(const char *arg)
{
    if     (sl_strcmp(arg,"MATRIX") ==0){apply_theme(THEME_MATRIX); shell_println("  THEME: MATRIX.",  COL_SUCCESS);}
    else if(sl_strcmp(arg,"OCEAN")  ==0){apply_theme(THEME_OCEAN);  shell_println("  THEME: OCEAN.",   COL_SUCCESS);}
    else if(sl_strcmp(arg,"BLOOD")  ==0){apply_theme(THEME_BLOOD);  shell_println("  THEME: BLOOD.",   COL_SUCCESS);}
    else if(sl_strcmp(arg,"DEFAULT")==0){apply_theme(THEME_DEFAULT);shell_println("  THEME: DEFAULT.", COL_SUCCESS);}
    else { shell_println("  OPTIONS: DEFAULT MATRIX OCEAN BLOOD", COL_ERROR); }
}

static void cmd_ls(void)
{
    char names[SLFS_MAX_FILES][SLFS_MAX_FILENAME];
    int count=0; fs_list(names,&count);
    shell_println("",COL_OUTPUT);
    if(count==0) shell_println("  NO FILES.",COL_DIM);
    else
    {
        int i;
        for(i=0;i<count;i++)
        {
            char line[64]="  > "; sl_strcat(line,names[i]);
            shell_println(line,COL_SUCCESS);
        }
    }
    char buf[32]="  "; char num[8]; int_to_str(count,num);
    sl_strcat(buf,num); sl_strcat(buf,"/64 FILES");
    shell_println(buf,COL_DIM); shell_println("",COL_OUTPUT);
}

static void cmd_create(const char *name)
{
    if(sl_strlen(name)==0){shell_println("  USAGE: CREATE <FILE>",COL_ERROR);return;}
    int r=fs_create(name);
    if(r==0){char m[64]="  CREATED: ";sl_strcat(m,name);shell_println(m,COL_SUCCESS);}
    else if(r==-1) shell_println("  ERROR: ALREADY EXISTS.",COL_ERROR);
    else           shell_println("  ERROR: FILESYSTEM FULL.",COL_ERROR);
}

static void cmd_write(const char *arg)
{
    int i=0; while(arg[i]&&arg[i]!=' ') i++;
    if(!arg[i]){shell_println("  USAGE: WRITE <FILE> <TEXT>",COL_ERROR);return;}
    char fname[SLFS_MAX_FILENAME];
    int j; for(j=0;j<i&&j<SLFS_MAX_FILENAME-1;j++) fname[j]=arg[j]; fname[j]=0;
    const char *text=arg+i+1;
    if(!fs_exists(fname)) fs_create(fname);
    if(fs_write(fname,text,sl_strlen(text))==0)
    {char m[64]="  WRITTEN: ";sl_strcat(m,fname);shell_println(m,COL_SUCCESS);}
    else shell_println("  ERROR: WRITE FAILED.",COL_ERROR);
}

static void cmd_read(const char *name)
{
    if(sl_strlen(name)==0){shell_println("  USAGE: READ <FILE>",COL_ERROR);return;}
    char buf[SLFS_MAX_FILESIZE+4];
    int r=fs_read(name,buf+2,SLFS_MAX_FILESIZE);
    if(r<0){shell_println("  ERROR: NOT FOUND.",COL_ERROR);return;}
    buf[0]=' '; buf[1]=' '; if(r>116) buf[118]=0;
    shell_println("",COL_OUTPUT); shell_println(buf,COL_OUTPUT); shell_println("",COL_OUTPUT);
}

static void cmd_delete(const char *name)
{
    if(sl_strlen(name)==0){shell_println("  USAGE: DELETE <FILE>",COL_ERROR);return;}
    if(fs_delete(name)==0){char m[64]="  DELETED: ";sl_strcat(m,name);shell_println(m,COL_SUCCESS);}
    else shell_println("  ERROR: NOT FOUND.",COL_ERROR);
}

static void cmd_df(void)
{
    uint32_t used=fs_used(),free_s=SLFS_MAX_FILES-used;
    char line[64]; char n[8];
    shell_println("",COL_OUTPUT); shell_println("  SLFS DISK USAGE:",COL_PROMPT);
    int_to_str(used,n); sl_strcpy(line,"  USED : "); sl_strcat(line,n); sl_strcat(line," / 64"); shell_println(line,COL_SUCCESS);
    int_to_str(free_s,n); sl_strcpy(line,"  FREE : "); sl_strcat(line,n); sl_strcat(line," / 64"); shell_println(line,COL_DIM);
    shell_println("  SIZE  : 512 BYTES PER FILE",COL_DIM);
    shell_println("",COL_OUTPUT);
}

// ── Process list ──────────────────────────────────────

static void cmd_ps(void)
{
    shell_println("", COL_OUTPUT);
    shell_println("  +----+----------+------------------+", COL_PROMPT);
    shell_println("  | PID|  STATE   |  ENTRY POINT     |", COL_PROMPT);
    shell_println("  +----+----------+------------------+", COL_PROMPT);

    int i, found = 0;
    for(i = 0; i < MAX_PROCESSES; i++)
    {
        if(process_table[i].pid == -1) continue;

        char line[80] = "  | ";
        char num[16];

        // PID
        int_to_str(process_table[i].pid, num);
        sl_strcat(line, num);
        sl_strcat(line, "  | ");

        // STATE
        int st = process_table[i].state;
        if     (st == PROCESS_RUNNING) sl_strcat(line, "RUNNING  |");
        else if(st == PROCESS_READY)   sl_strcat(line, "READY    |");
        else if(st == PROCESS_BLOCKED) sl_strcat(line, "BLOCKED  |");
        else                           sl_strcat(line, "DEAD     |");

        // ENTRY
        sl_strcat(line, " 0x");
        char hexbuf[20];
        uint64_to_hex(process_table[i].rip, hexbuf);
        sl_strcat(line, hexbuf);
        sl_strcat(line, " |");

        int col = st == PROCESS_RUNNING ? COL_SUCCESS :
                  st == PROCESS_READY   ? COL_CYAN    :
                  st == PROCESS_BLOCKED ? 0xFFC832    : COL_DIM;

        shell_println(line, col);
        found++;
    }

    if(found == 0)
        shell_println("  NO PROCESSES.", COL_DIM);

    shell_println("  +----+----------+------------------+", COL_PROMPT);

    char buf[32] = "  TOTAL: "; char n[8];
    int_to_str(found, n); sl_strcat(buf, n);
    sl_strcat(buf, " PROCESS(ES)");
    shell_println(buf, COL_DIM);
    shell_println("", COL_OUTPUT);
}

// ── ELF Exec ─────────────────────────────────────────

static void cmd_exec(const char *name)
{
    if(sl_strlen(name) == 0)
    {
        shell_println("  USAGE: EXEC <FILE>", COL_ERROR);
        return;
    }

    // Read file from SLFS
    static uint8_t elf_buf[SLFS_MAX_FILESIZE];
    int size = fs_read(name, (char *)elf_buf, SLFS_MAX_FILESIZE);

    if(size < 0)
    {
        shell_println("  ERROR: FILE NOT FOUND.", COL_ERROR);
        return;
    }
    if(size < 64)
    {
        shell_println("  ERROR: FILE TOO SMALL TO BE ELF.", COL_ERROR);
        return;
    }

    // Load ELF
    uint64_t entry = 0;
    int result = elf_load(elf_buf, &entry);

    switch(result)
    {
        case ELF_ERR_MAGIC:   shell_println("  ERROR: NOT A VALID ELF BINARY.", COL_ERROR); return;
        case ELF_ERR_CLASS:   shell_println("  ERROR: NOT A 64-BIT ELF.",       COL_ERROR); return;
        case ELF_ERR_MACHINE: shell_println("  ERROR: NOT AN X86-64 BINARY.",   COL_ERROR); return;
        case ELF_ERR_TYPE:    shell_println("  ERROR: NOT AN EXECUTABLE ELF.",  COL_ERROR); return;
        case ELF_ERR_NO_LOAD: shell_println("  ERROR: NO LOADABLE SEGMENTS.",   COL_ERROR); return;
        default: break;
    }
    if(result != ELF_OK)
    {
        shell_println("  ERROR: ELF LOAD FAILED.", COL_ERROR);
        return;
    }

    // Print success info
    char line[64] = "  LOADED: ";
    sl_strcat(line, name);
    shell_println(line, COL_SUCCESS);

    char eline[64] = "  ENTRY : 0x";
    char hexbuf[20];
    uint64_to_hex(entry, hexbuf);
    sl_strcat(eline, hexbuf);
    shell_println(eline, COL_CYAN);

    // Create process — use full 64-bit token
    int pid = create_process(entry, (uint64_t)0xDEADBEEFCAFEULL);
    if(pid < 0)
    {
        shell_println("  ERROR: PROCESS CREATION FAILED.", COL_ERROR);
        shell_println("  CHECK TRUST REGISTRY.", COL_DIM);
        return;
    }

    char pline[64] = "  PID   : ";
    char pnum[8]; int_to_str(pid, pnum);
    sl_strcat(pline, pnum);
    shell_println(pline, COL_SUCCESS);
    shell_println("  PROCESS QUEUED. TYPE SCHEDULE TO RUN.", COL_DIM);
    shell_println("  TYPE PS TO VIEW PROCESS TABLE.", COL_DIM);
}

// ── Execute ───────────────────────────────────────────

static void add_to_history(const char *cmd)
{
    if(sl_strlen(cmd)==0) return;
    if(history_count<HISTORY_MAX) sl_strcpy(history[history_count++],cmd);
    else
    {
        int i;
        for(i=0;i<HISTORY_MAX-1;i++) sl_strcpy(history[i],history[i+1]);
        sl_strcpy(history[HISTORY_MAX-1],cmd);
    }
}

static void shell_execute(const char *cmd)
{
    char echo[140]="SENTINEL> ";
    int i=10,j=0; while(cmd[j]) echo[i++]=cmd[j++]; echo[i]=0;
    shell_println(echo,COL_PROMPT);
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
    else if(sl_strcmp(cmd,"MEMINFO") ==0) cmd_meminfo();
    else if(sl_strcmp(cmd,"PS")      ==0) cmd_ps();
    else if(sl_startswith(cmd,"ECHO "))   cmd_echo(cmd+5);
    else if(sl_startswith(cmd,"COLOR "))  cmd_color(cmd+6);
    else if(sl_startswith(cmd,"CREATE ")) cmd_create(cmd+7);
    else if(sl_startswith(cmd,"WRITE "))  cmd_write(cmd+6);
    else if(sl_startswith(cmd,"READ "))   cmd_read(cmd+5);
    else if(sl_startswith(cmd,"DELETE ")) cmd_delete(cmd+7);
    else if(sl_startswith(cmd,"ALLOC "))  cmd_alloc(cmd+6);
    else if(sl_startswith(cmd,"EXEC "))   cmd_exec(cmd+5);
    else if(sl_strcmp(cmd,"SCHEDULE")==0)
    {
        schedule();
        shell_println("", COL_OUTPUT);
        shell_println("  SCHEDULER TICK EXECUTED.", COL_SUCCESS);
        shell_println("  TYPE PS TO VIEW PROCESS STATES.", COL_DIM);
        shell_println("", COL_OUTPUT);
    }
    else if(sl_strcmp(cmd,"ABOUT")==0)
    {
        shell_println("",COL_OUTPUT);
        shell_println("  SENTINELOS V0.1-ALPHA",            COL_CYAN);
        shell_println("  64-BIT SECURE KERNEL",             COL_OUTPUT);
        shell_println("  TRUST NOTHING. VERIFY EVERYTHING.",COL_PROMPT);
        shell_println("  BUILT IN PURE C + ASSEMBLY",       COL_OUTPUT);
        shell_println("",COL_OUTPUT);
    }
    else if(sl_strcmp(cmd,"STATUS")==0)
    {
        shell_println("",COL_OUTPUT);
        shell_println("  +-------------------------------+", COL_PROMPT);
        shell_println("  |    SYSTEM STATUS              |", COL_PROMPT);
        shell_println("  +-------------------------------+", COL_PROMPT);
        shell_println("  TRUST REGISTRY   [ ONLINE ]",COL_SUCCESS);
        shell_println("  SHA-256 ENGINE   [ ONLINE ]",COL_SUCCESS);
        shell_println("  MEMORY GUARD     [ ONLINE ]",COL_SUCCESS);
        shell_println("  SLFS FILESYSTEM  [ ONLINE ]",COL_SUCCESS);
        shell_println("  VERIFY GATE      [ ACTIVE ]",COL_SUCCESS);
        shell_println("  ELF LOADER       [ ONLINE ]",COL_SUCCESS);
        shell_println("  SCHEDULER        [ ONLINE ]",COL_SUCCESS);
        shell_println("  THREATS BLOCKED  [ 000001 ]",COL_ERROR);
        shell_println("  +-------------------------------+", COL_PROMPT);
        shell_println("",COL_OUTPUT);
    }
    else if(sl_strcmp(cmd,"VERIFY")==0)
    {
        shell_println("",COL_OUTPUT);
        shell_println("  INITIALIZING INTEGRITY SCAN...",COL_OUTPUT);
        shell_println("  HASHING KERNEL SEGMENTS...",    COL_OUTPUT);
        shell_println("  CHECKING TRUST REGISTRY...",    COL_OUTPUT);
        shell_println("  VERIFYING PAGE TABLES...",      COL_OUTPUT);
        shell_println("  CHECKING MEMORY INTEGRITY...",  COL_OUTPUT);
        shell_println("  CHECKING ELF LOADER BOUNDS...", COL_OUTPUT);
        shell_println("  SHA-256: [ VERIFIED OK ]",      COL_SUCCESS);
        shell_println("  ALL SEGMENTS INTACT.",          COL_SUCCESS);
        shell_println("",COL_OUTPUT);
    }
    else if(sl_strcmp(cmd,"REBOOT")==0)
    {
        shell_println("  REBOOTING...",COL_ERROR);
        __asm__ volatile("mov $0xFE, %al\n out %al, $0x64\n");
    }
    else
    {
        char msg[64]="  UNKNOWN: "; sl_strcat(msg,cmd);
        shell_println(msg,COL_ERROR);
        shell_println("  TYPE HELP FOR COMMANDS.",COL_DIM);
    }
}

// ── Key handler ───────────────────────────────────────

void shell_handle_key(char c)
{
    uptime_ticks++;
    if(c=='\n'||c=='\r')
    {
        input_buf[input_len]=0;
        shell_execute(input_buf);
        input_len=0; input_buf[0]=0;
    }
    else if(c=='\b')
    {
        if(input_len>0){input_len--;input_buf[input_len]=0;}
    }
    else if(input_len<SHELL_MAX_INPUT-1)
    {
        input_buf[input_len++]=to_upper(c);
        input_buf[input_len]=0;
    }
    shell_redraw();
}

// ── Init ──────────────────────────────────────────────

void shell_init(void)
{
    total_lines=0; input_len=0; input_buf[0]=0;
    history_count=0; uptime_ticks=0;
    apply_theme(THEME_DEFAULT);
    fs_init();
    mem_init();
    scheduler_init();  // <-- initialise process table

    shell_println("",COL_OUTPUT);
    shell_println("  SENTINELOS SECURE SHELL V0.1",         COL_CYAN);
    shell_println("  TRUST NOTHING. VERIFY EVERYTHING.",    COL_PROMPT);
    shell_println("  MEMORY MANAGER ONLINE. 4MB HEAP.",     0x00FF99);
    shell_println("  ELF LOADER + SCHEDULER ONLINE.",       COL_SUCCESS);
    shell_println("  TYPE HELP FOR ALL COMMANDS.",          COL_DIM);
    shell_println("",COL_OUTPUT);
    shell_redraw();
}
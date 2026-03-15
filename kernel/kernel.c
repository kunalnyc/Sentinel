#include "../security/trust.h"
#include "../security/sha256.h"
#include "idt.h"
#include "memory.h"
#include "scheduler.h"
#include "keyboard.h"
#include "timer.h"
#include "graphics.h"
#include "font.h"
#include "io.h"
#include <stdint.h>
#include "shell.h"
#include "boot_anim.h"
#include "mouse.h"
#include "mem_mgr.h"
// OS states
#define STATE_DASHBOARD 0
#define STATE_SHELL     1

static int os_state = STATE_DASHBOARD;
// Multiboot2 structures
struct mb2_tag {
    uint32_t type;
    uint32_t size;
};

struct mb2_tag_framebuffer {
    uint32_t type;
    uint32_t size;
    uint64_t framebuffer_addr;
    uint32_t framebuffer_pitch;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint8_t framebuffer_bpp;
    uint8_t framebuffer_type;
    uint16_t reserved;
};

// Global framebuffer info
uint64_t fb_addr = 0;
uint32_t fb_width = 0;
uint32_t fb_height = 0;
uint32_t fb_pitch = 0;

// Global screen info
extern screen_info_t screen;

// Function declarations
uint16_t vbe_read_id(void);
void parse_multiboot(uint32_t mb_addr);

// Debug functions
void debug_putchar(char c) {
    outb(0xE9, c);  // Bochs/QEMU debug console
}

void debug_print(const char* str) {
    while(*str) {
        debug_putchar(*str++);
    }
}

void debug_print_hex(uint16_t num) {
    char hex_chars[] = "0123456789ABCDEF";
    debug_putchar(hex_chars[(num >> 12) & 0xF]);
    debug_putchar(hex_chars[(num >> 8) & 0xF]);
    debug_putchar(hex_chars[(num >> 4) & 0xF]);
    debug_putchar(hex_chars[num & 0xF]);
}

void debug_print_hex32(uint32_t num) {
    char hex_chars[] = "0123456789ABCDEF";
    debug_putchar(hex_chars[(num >> 28) & 0xF]);
    debug_putchar(hex_chars[(num >> 24) & 0xF]);
    debug_putchar(hex_chars[(num >> 20) & 0xF]);
    debug_putchar(hex_chars[(num >> 16) & 0xF]);
    debug_putchar(hex_chars[(num >> 12) & 0xF]);
    debug_putchar(hex_chars[(num >> 8) & 0xF]);
    debug_putchar(hex_chars[(num >> 4) & 0xF]);
    debug_putchar(hex_chars[num & 0xF]);
}

// Parse multiboot info
void parse_multiboot(uint32_t mb_addr) {
    debug_print("Parsing multiboot info at 0x");
    debug_print_hex32(mb_addr);
    debug_print("\n");
    
    uint32_t addr = mb_addr + 8;
    
    while(1) {
        struct mb2_tag *tag = (struct mb2_tag *)addr;
        
        if(tag->type == 0) {
            debug_print("End of tags\n");
            break;
        }
        
        debug_print("Tag type: ");
        debug_print_hex(tag->type);
        debug_print(" size: ");
        debug_print_hex(tag->size);
        debug_print("\n");
        
        if(tag->type == 8) { // framebuffer tag
            debug_print("Found framebuffer tag!\n");
            struct mb2_tag_framebuffer *fb = 
                (struct mb2_tag_framebuffer *)addr;
            fb_addr = fb->framebuffer_addr;
            fb_width = fb->framebuffer_width;
            fb_height = fb->framebuffer_height;
            fb_pitch = fb->framebuffer_pitch;
            
            debug_print("FB addr: 0x");
            debug_print_hex32((uint32_t)fb_addr);
            debug_print("\nFB width: ");
            debug_print_hex(fb_width);
            debug_print("\nFB height: ");
            debug_print_hex(fb_height);
            debug_print("\n");
        }
        
        addr += (tag->size + 7) & ~7;
    }
}

// Test direct framebuffer access
void test_framebuffer_direct() {
    if (!screen.framebuffer_virtual) {
        debug_print("No framebuffer address for direct test!\n");
        return;
    }
    
    debug_print("Testing direct framebuffer access at 0x");
    debug_print_hex32(screen.framebuffer_virtual);
    debug_print("\n");
    
    uint32_t* fb = (uint32_t*)(uintptr_t)screen.framebuffer_virtual;
    
    // Fill first 100 pixels with red
    for(int i = 0; i < 100 && i < screen.width * 10; i++) {
        fb[i] = 0xFF0000;  // Bright red
    }
    
    // Fill next 100 with green
    for(int i = 100; i < 200 && i < screen.width * 10; i++) {
        fb[i] = 0x00FF00;  // Bright green
    }
    
    // Fill next 100 with blue
    for(int i = 200; i < 300 && i < screen.width * 10; i++) {
        fb[i] = 0x0000FF;  // Bright blue
    }
    
    debug_print("Direct framebuffer test complete\n");
}

// Draw Sentinel logo
void draw_sentinel_logo(int cx, int cy, int size) {
    int outer = size;
    int inner = size / 2;
    
    draw_line(cx,      cy-outer, cx+outer, cy,      COLOR_FORERUNNER_GOLD);
    draw_line(cx+outer, cy,      cx,       cy+outer, COLOR_FORERUNNER_GOLD);
    draw_line(cx,      cy+outer, cx-outer, cy,      COLOR_FORERUNNER_GOLD);
    draw_line(cx-outer, cy,      cx,       cy-outer, COLOR_FORERUNNER_GOLD);
    draw_line(cx,      cy-inner, cx+inner, cy,      COLOR_WHITE);
    draw_line(cx+inner, cy,      cx,       cy+inner, COLOR_WHITE);
    draw_line(cx,      cy+inner, cx-inner, cy,      COLOR_WHITE);
    draw_line(cx-inner, cy,      cx,       cy-inner, COLOR_WHITE);
    draw_line(cx-outer, cy, cx+outer, cy, COLOR_FORERUNNER_GOLD);
    draw_line(cx, cy-outer, cx, cy+outer, COLOR_FORERUNNER_GOLD);
}

// High resolution main screen
void draw_main_screen_highres() {
    int width = screen.width;
    int height = screen.height;
    
    clear_screen_graphics(COLOR_SPACE_BLACK);

    for(int i = 0; i < height; i += 2) {
        draw_line(0, i, width, i, COLOR_SCAN_LINE);
    }

    int header_height = height / 16;
    draw_rounded_rect(0, 0, width, header_height, 5, COLOR_PANEL_BG);
    draw_line(0, header_height, width, header_height, COLOR_FORERUNNER_GOLD);
    draw_string(10, header_height/4, "SENTINELOS", COLOR_FORERUNNER_GOLD);
    draw_string(width - 200, header_height/4, "SECURE KERNEL V0.1", COLOR_SILVER);

    int status_height = height / 20;
    draw_rounded_rect(0, height - status_height, width, status_height, 5, COLOR_PANEL_BG);
    draw_line(0, height - status_height, width, height - status_height, COLOR_FORERUNNER_GOLD);
    draw_string(10, height - status_height + 4, "ALL SYSTEMS OPERATIONAL", COLOR_VERIFIED_GREEN);
    draw_string(width - 150, height - status_height + 4, "100HZ TIMER", COLOR_SILVER);

    int panel_width = width / 3;
    int panel_height = height - header_height - status_height - 20;
    int panel_x = 10;
    int panel_y = header_height + 10;
    
    draw_rounded_rect(panel_x, panel_y, panel_width, panel_height, 8, COLOR_PANEL_BG);
    draw_line(panel_x, panel_y + 25, panel_x + panel_width, panel_y + 25, COLOR_DIM_GOLD);
    draw_string(panel_x + 10, panel_y + 8, "SECURITY STATUS", COLOR_FORERUNNER_GOLD);

    const char* sec_items[] = {
        "TRUST REGISTRY", "VERIFICATION GATE", "SHA-256 ENGINE",
        "MEMORY GUARD", "PROCESS SHIELD", "IDT PROTECTION"
    };
    const char* sec_status[] = {"ONLINE", "ACTIVE", "ONLINE", "ONLINE", "ACTIVE", "ONLINE"};
    
    for(int i = 0; i < 6; i++) {
        draw_string(panel_x + 10, panel_y + 35 + i*15, sec_items[i], COLOR_SILVER);
        draw_string(panel_x + panel_width - 70, panel_y + 35 + i*15, 
                   sec_status[i], (i % 2) ? COLOR_FORERUNNER_GOLD : COLOR_VERIFIED_GREEN);
    }

    draw_line(panel_x, panel_y + 130, panel_x + panel_width, panel_y + 130, COLOR_DIM_GOLD);
    draw_string(panel_x + 10, panel_y + 140, "THREATS BLOCKED", COLOR_FORERUNNER_GOLD);
    draw_string(panel_x + 20, panel_y + 160, "TODAY", COLOR_SILVER);
    draw_string(panel_x + panel_width - 70, panel_y + 160, "000001", COLOR_ALERT_RED);
    draw_string(panel_x + 20, panel_y + 180, "TOTAL", COLOR_SILVER);
    draw_string(panel_x + panel_width - 70, panel_y + 180, "000001", COLOR_ALERT_RED);

    int right_x = width - panel_width - 10;
    draw_rounded_rect(right_x, panel_y, panel_width, panel_height, 8, COLOR_PANEL_BG);
    draw_line(right_x, panel_y + 25, right_x + panel_width, panel_y + 25, COLOR_DIM_GOLD);
    draw_string(right_x + 10, panel_y + 8, "SYSTEM MONITOR", COLOR_FORERUNNER_GOLD);
    draw_sentinel_logo(right_x + panel_width/2, panel_y + 80, 30);

    int info_y = panel_y + 130;
    draw_line(right_x, info_y, right_x + panel_width, info_y, COLOR_DIM_GOLD);
    
    struct {
        const char* label;
        const char* value;
    } sysinfo[] = {
        {"KERNEL", "0x100000"},
        {"MEMORY", "16MB"},
        {"PAGES", "4096"},
        {"TIMER", "100HZ"},
        {"ARCH", "X86-64"}
    };
    
    for(int i = 0; i < 5; i++) {
        draw_string(right_x + 15, info_y + 15 + i*20, sysinfo[i].label, COLOR_SILVER);
        draw_string(right_x + panel_width - 100, info_y + 15 + i*20, 
                   sysinfo[i].value, COLOR_HARD_LIGHT);
    }
}

// Original 320x200 function
void draw_main_screen(void) {
    clear_screen_graphics(COLOR_SPACE_BLACK);

    for(int i = 0; i < 200; i += 2)
        draw_line(0, i, 320, i, COLOR_SCAN_LINE);

    draw_rect(0, 0, 320, 12, COLOR_PANEL_BG);
    draw_line(0, 12, 320, 12, COLOR_FORERUNNER_GOLD);
    draw_string(2, 2, "SENTINELOS", COLOR_FORERUNNER_GOLD);
    draw_string(220, 2, "SECURE KERNEL V0.1", COLOR_SILVER);

    draw_rect(0, 188, 320, 12, COLOR_PANEL_BG);
    draw_line(0, 188, 320, 188, COLOR_FORERUNNER_GOLD);
    draw_string(2, 191, "ALL SYSTEMS OPERATIONAL", COLOR_VERIFIED_GREEN);
    draw_string(240, 191, "100HZ TIMER", COLOR_SILVER);

    draw_rect(5, 18, 145, 165, COLOR_PANEL_BG);
    draw_line(5,   18, 150, 18,  COLOR_FORERUNNER_GOLD);
    draw_line(5,   183, 150, 183, COLOR_FORERUNNER_GOLD);
    draw_line(5,   18, 5,   183, COLOR_FORERUNNER_GOLD);
    draw_line(150, 18, 150, 183, COLOR_FORERUNNER_GOLD);
    draw_line(5, 28, 150, 28, COLOR_DIM_GOLD);
    draw_string(10, 20, "SECURITY STATUS", COLOR_FORERUNNER_GOLD);

    draw_string(10, 35, "TRUST REGISTRY", COLOR_SILVER);
    draw_string(10, 45, "VERIFICATION GATE", COLOR_SILVER);
    draw_string(10, 55, "SHA-256 ENGINE", COLOR_SILVER);
    draw_string(10, 65, "MEMORY GUARD", COLOR_SILVER);
    draw_string(10, 75, "PROCESS SHIELD", COLOR_SILVER);
    draw_string(10, 85, "IDT PROTECTION", COLOR_SILVER);
    draw_string(110, 35, "ONLINE", COLOR_VERIFIED_GREEN);
    draw_string(110, 45, "ACTIVE", COLOR_VERIFIED_GREEN);
    draw_string(110, 55, "ONLINE", COLOR_VERIFIED_GREEN);
    draw_string(110, 65, "ONLINE", COLOR_VERIFIED_GREEN);
    draw_string(110, 75, "ACTIVE", COLOR_VERIFIED_GREEN);
    draw_string(110, 85, "ONLINE", COLOR_VERIFIED_GREEN);

    draw_line(5, 100, 150, 100, COLOR_DIM_GOLD);
    draw_string(10, 103, "THREATS BLOCKED", COLOR_FORERUNNER_GOLD);
    draw_string(10, 115, "TODAY", COLOR_SILVER);
    draw_string(80, 113, "000001", COLOR_ALERT_RED);
    draw_string(10, 128, "TOTAL", COLOR_SILVER);
    draw_string(80, 128, "000001", COLOR_ALERT_RED);

    draw_rect(155, 18, 160, 165, COLOR_PANEL_BG);
    draw_line(155, 18,  315, 18,  COLOR_FORERUNNER_GOLD);
    draw_line(155, 183, 315, 183, COLOR_FORERUNNER_GOLD);
    draw_line(155, 18,  155, 183, COLOR_FORERUNNER_GOLD);
    draw_line(315, 18,  315, 183, COLOR_FORERUNNER_GOLD);
    draw_line(155, 28, 315, 28, COLOR_DIM_GOLD);
    draw_string(160, 20, "SYSTEM MONITOR", COLOR_FORERUNNER_GOLD);
    draw_sentinel_logo(235, 80, 30);

    draw_line(155, 128, 315, 128, COLOR_DIM_GOLD);
    draw_string(160, 132, "KERNEL", COLOR_SILVER);
    draw_string(220, 132, "0x100000", COLOR_HARD_LIGHT);
    draw_string(160, 142, "MEMORY", COLOR_SILVER);
    draw_string(220, 142, "16MB", COLOR_HARD_LIGHT);
    draw_string(160, 152, "PAGES", COLOR_SILVER);
    draw_string(220, 152, "4096", COLOR_HARD_LIGHT);
    draw_string(160, 162, "TIMER", COLOR_SILVER);
    draw_string(220, 162, "100HZ", COLOR_HARD_LIGHT);
    draw_string(160, 172, "ARCH", COLOR_SILVER);
    draw_string(220, 172, "X86-32", COLOR_HARD_LIGHT);
}

// Kernel main
void kernel_main(unsigned int magic, unsigned int mb_addr)
{
    if(graphics_init(1024, 768, 32)) {
    } else {
        graphics_init_vga();
        init_forerunner_palette();
    }
    mem_init();
    mouse_init();
    boot_animation();
    draw_main_screen_highres();

    // NO sti, NO keyboard_init - pure polling
    while(1)
    {
        mouse_poll();
        char c = keyboard_poll();
        
        if(os_state == STATE_DASHBOARD)
        {
            if(c == '\n') {
                os_state = STATE_SHELL;
                clear_screen_graphics(COLOR_SPACE_BLACK);
                draw_rect(0, 0, screen.width, 20, COLOR_PANEL_BG);
                draw_line(0, 20, screen.width, 20, COLOR_FORERUNNER_GOLD);
                draw_string(10, 6, "SENTINELOS TERMINAL", COLOR_FORERUNNER_GOLD);
                draw_string(screen.width-120, 6, "ESC=DASHBOARD", COLOR_DIM_GOLD);
                shell_init();
            }
        }
        else if(os_state == STATE_SHELL)
        {
            if(c == 27) {
                os_state = STATE_DASHBOARD;
                draw_main_screen_highres();
            }
            else if(c) {
                shell_handle_key(c);
            }
        }
    }
}
// kernel.c
static int cursor_pos = 0;
static int current_color = 0x0F;


static inline unsigned char inb(unsigned short port) {
    unsigned char ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}


static inline void outb(unsigned short port, unsigned char val) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}


unsigned char bcd_to_bin(unsigned char bcd) {
    return ((bcd / 16) * 10) + (bcd % 16);
}

unsigned char get_rtc_register(int reg) {
    outb(0x70, reg);
    return inb(0x71);
}


void update_cursor(int pos) {
    outb(0x3D4, 0x0F);
    outb(0x3D5, (unsigned char)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (unsigned char)((pos >> 8) & 0xFF));
}


void scroll() {
    volatile char* video_memory = (char*) 0xB8000;
    
    
    for (int i = 0; i < (80 * 24); i++) {
        video_memory[i * 2] = video_memory[(i + 80) * 2];
        video_memory[i * 2 + 1] = video_memory[(i + 80) * 2 + 1];
    }

    
    for (int i = (80 * 24); i < (80 * 25); i++) {
        video_memory[i * 2] = ' ';
        video_memory[i * 2 + 1] = 0x07;
    }
    
    
    cursor_pos = 80 * 24;
}


void print(const char* str, int color) {
    int final_color = (color == 0) ? current_color : color;
    volatile char* video_memory = (char*) 0xB8000;

    for (int i = 0; str[i] != '\0'; i++) {

    if (cursor_pos >= 80 * 25) {
            scroll();
        }

        if (str[i] == '\n') {
            cursor_pos += 80 - (cursor_pos % 80);
        } else if (str[i] == '\b') {
            if (cursor_pos > 0) {
                cursor_pos--;
                video_memory[cursor_pos * 2] = ' ';
                video_memory[cursor_pos * 2 + 1] = final_color;
            }
        } else {
            video_memory[cursor_pos * 2] = str[i];
            video_memory[cursor_pos * 2 + 1] = final_color;
            cursor_pos++;
        }
    }
    update_cursor(cursor_pos);
}

void print_int(int n) {
    char str[3];
    str[0] = (n / 10) + '0';
    str[1] = (n % 10) + '0';
    str[2] = '\0';
    print(str, 0x0E); 
}

void display_time() {
    unsigned char second = bcd_to_bin(get_rtc_register(0x00));
    unsigned char minute = bcd_to_bin(get_rtc_register(0x02));
    unsigned char hour   = bcd_to_bin(get_rtc_register(0x04));

    print("\nSystem Time [HH:MM:SS] -> ", 0x0F);
    print_int(hour);   print(":", 0x0F);
    print_int(minute); print(":", 0x0F);
    print_int(second);
}


int str_equal(const char* s1, const char* s2) {
    int i = 0;
    while (s1[i] != '\0' && s2[i] != '\0') {
        if (s1[i] != s2[i]) return 0;
        i++;
    }
    return (s1[i] == s2[i]);
}

void command_cls() {
    volatile char* video_memory = (char*) 0xB8000;
    for (int i = 0; i < 80 * 25; i++) {
        video_memory[i * 2] = ' ';
        video_memory[i * 2 + 1] = 0x07;
    }
    cursor_pos = 0;
    update_cursor(0);
}

char get_char() {
    if (!(inb(0x64) & 1)) return 0; 
    unsigned char scancode = inb(0x60);
    if (scancode & 0x80) return 0; 
    switch(scancode) {
        case 0x1E: return 'a'; case 0x30: return 'b'; case 0x2E: return 'c';
        case 0x20: return 'd'; case 0x12: return 'e'; case 0x21: return 'f';
        case 0x22: return 'g'; case 0x23: return 'h'; case 0x17: return 'i';
        case 0x24: return 'j'; case 0x25: return 'k'; case 0x26: return 'l';
        case 0x32: return 'm'; case 0x31: return 'n'; case 0x18: return 'o';
        case 0x19: return 'p'; case 0x10: return 'q'; case 0x13: return 'r';
        case 0x1F: return 's'; case 0x14: return 't'; case 0x16: return 'u';
        case 0x2F: return 'v'; case 0x11: return 'w'; case 0x2D: return 'x';
        case 0x15: return 'y'; case 0x2C: return 'z';
        case 0x02: return '1'; case 0x03: return '2'; case 0x04: return '3';
        case 0x05: return '4'; case 0x06: return '5'; case 0x07: return '6';
        case 0x08: return '7'; case 0x09: return '8'; case 0x0A: return '9';
        case 0x0B: return '0';
        case 0x1C: return '\n'; case 0x39: return ' ';  case 0x0E: return '\b';
        default: return 0;
    }
}

void ask_password() {
    char pass_buffer[64];
    int i = 0;
    int authenticated = 0;

    while (!authenticated) {
        print("\nEnter Password: ", 0x0E); 
        i = 0;
        
        while (1) {
            char c = get_char();
            if (c != 0) {
                if (c == '\n') {
                    pass_buffer[i] = '\0';
                    break;
                } else if (c == '\b') {
                    if (i > 0) { i--; print("\b", 0); }
                } else if (i < 63) {
                    pass_buffer[i++] = c;
                    print("*", 0x0E);
                }
            }
        }

                // passwd : h
                if (str_equal(pass_buffer, "h")) {
            print("\nWelcome Halim\n", 0x0A); 
            authenticated = 1;
        } else {
            print("\nTry again.", 0x0C);
        }
    }
}

void reboot() {
    print("\nSystem is rebooting...", 0x0E);
    
    unsigned char temp;
    do {
        temp = inb(0x64);
    } while (temp & 0x02);

    outb(0x64, 0xFE);
}

void process_command(char* cmd) {
    if (str_equal(cmd, "help")) {
        print("\nHalimOS Commands: help, cls, time, red, blue, green, reboot, halim, info", 0x0F);
    } else if (str_equal(cmd, "reboot")){
        reboot();
    } else if (str_equal(cmd, "time")) {
        display_time();
    } else if (str_equal(cmd, "cls")) {
        command_cls();
    } else if (str_equal(cmd, "red")) {
        current_color = 0x0C; print("\nColor: Red", current_color);
    } else if (str_equal(cmd, "blue")) {
        current_color = 0x09; print("\nColor: Blue", current_color);
    } else if (str_equal(cmd, "green")) {
        current_color = 0x0A; print("\nColor: Green", current_color);
    } else if (str_equal(cmd, "halim")) {
        print("\nCreator: The Genius Halim!", 0x0D);
    } else if (str_equal(cmd, "info")) {
        print("\n====================================", 0x0B);
        print("\n        HnlimOS Kernel v2.1        ", 0x09); 
        print("\n------------------------------------", 0x0B);
        print("\n  Developer  : HALIM.DEV        ", 0x09); 
        print("\n  Architec   : x86 (32-bit)       ", 0x09);
        print("\n  Security   : Password Protected  ", 0x0C); 
        print("\n  Status     : System Stable       ", 0x09);
        print("\n====================================", 0x0B);
    } else if (cmd[0] != '\0') {
        print("\nUnknown: ", 0x0C); print(cmd, 0x0C);
    }
}

void start_shell() {
    char command[64];
    int i = 0;
    print("\nHalimOS Shell> ", 0x0B);
    while(1) {
        char c = get_char();
        if (c != 0) {
            if (c == '\n') {
                command[i] = '\0';
                process_command(command);
                print("\nHalimOS Shell> ", 0x0B);
                i = 0;
            } else if (c == '\b') {
                if (i > 0) { i--; print("\b", 0); }
            } else if (i < 63) {
                command[i++] = c;
                char str[2] = {c, '\0'};
                print(str, 0);
            }
        }
    }
}


void kmain() {
    command_cls();
    print("--- HalimOS Security Shield ---\n", 0x0B);
    ask_password();
    start_shell();
}

#include "stm32f1xx_hal.h"  // 或你的芯片头文件

extern UART_HandleTypeDef huart1;  // 串口句柄

void uart_send_char(char c) {
    HAL_UART_Transmit(&huart1, (uint8_t *)&c, 1, HAL_MAX_DELAY);
}

void uart_send_str(const char *s) {
    while (*s) {
        uart_send_char(*s++);
    }
}

void uart_send_hex(unsigned int num) {
    char hex_chars[] = "0123456789abcdef";
    char buffer[8];
    int i = 0;

    if (num == 0) {
        uart_send_char('0');
        return;
    }

    while (num && i < 8) {
        buffer[i++] = hex_chars[num % 16];
        num /= 16;
    }

    while (i--) {
        uart_send_char(buffer[i]);
    }
}

void uart_send_dec(int num) {
    char buffer[10];
    int i = 0;
    unsigned int n;

    if (num < 0) {
        uart_send_char('-');
        n = -num;
    } else {
        n = num;
    }

    if (n == 0) {
        uart_send_char('0');
        return;
    }

    while (n && i < 10) {
        buffer[i++] = (n % 10) + '0';
        n /= 10;
    }

    while (i--) {
        uart_send_char(buffer[i]);
    }
}




void my_printf(const char *fmt, ...) {
    char *arg_ptr = (char *)(&fmt);
    arg_ptr += sizeof(fmt);  // 指向第一个变参

    while (*fmt) {
        if (*fmt == '%') {
            fmt++;
            switch (*fmt) {
                case 'd': {
                    int val = *((int *)arg_ptr);
                    arg_ptr += sizeof(int);
                    uart_send_dec(val);
                    break;
                }
                case 'x': {
                    int val = *((int *)arg_ptr);
                    arg_ptr += sizeof(int);
                    uart_send_hex(val);
                    break;
                }
                case 'c': {
                    char ch = *((char *)arg_ptr);
                    arg_ptr += sizeof(int);  // 注意：char 也按 int 对齐
                    uart_send_char(ch);
                    break;
                }
                case 's': {
                    char *str = *((char **)arg_ptr);
                    arg_ptr += sizeof(char *);
                    uart_send_str(str);
                    break;
                }
                case '%': {
                    uart_send_char('%');
                    break;
                }
                default: {
                    uart_send_char('?');
                    break;
                }
            }
        } else {
            uart_send_char(*fmt);
        }
        fmt++;
    }
}

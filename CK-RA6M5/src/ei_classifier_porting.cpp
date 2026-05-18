/*
 * ei_classifier_porting.cpp
 *
 *  Created on: 1 May 2026
 *      Author: admin
 */

#include <sys/stat.h>
#include "edge-impulse-sdk/porting/ei_classifier_porting.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

// Báo cho AI biết hệ thống đang không bị hủy (Canceled)
__attribute__((weak)) EI_IMPULSE_ERROR ei_run_impulse_check_canceled() {
    return EI_IMPULSE_OK;
}

// Hàm in ra màn hình của AI (Trỏ về hàm printf tiêu chuẩn)
__attribute__((weak)) void ei_printf(const char *format, ...) {
    va_list myargs;
    va_start(myargs, format);
    vprintf(format, myargs);
    va_end(myargs);
}

__attribute__((weak)) void ei_printf_float(float f) {
    ei_printf("%f", f);
}

// Cấp phát và giải phóng RAM
__attribute__((weak)) void *ei_malloc(size_t size) {
    return malloc(size);
}

__attribute__((weak)) void *ei_calloc(size_t nitems, size_t size) {
    return calloc(nitems, size);
}

__attribute__((weak)) void ei_free(void *ptr) {
    free(ptr);
}

__attribute__((weak)) void DebugLog(const char* s) {
    ei_printf("%s", s);
}

// Hàm đếm thời gian (Tạm thời trả về 0 nếu chưa cần đo đạc tốc độ)
__attribute__((weak)) uint64_t ei_read_timer_ms() {
    return 0;
}

__attribute__((weak)) uint64_t ei_read_timer_us() {
    return 0;
}

// --- CÁC HÀM HỆ THỐNG GIẢ CẦY (SYSCALL STUBS) ---
// Giúp trình biên dịch không đòi hệ điều hành nữa

extern "C" void _exit(int status) {
    (void)status;
    while (1) {
        // Nếu chương trình bị lỗi nặng và gọi exit(), vi điều khiển sẽ treo ở đây
    }
}

extern "C" int _kill(int pid, int sig) {
    (void)pid;
    (void)sig;
    return -1;
}

extern "C" int _getpid(void) {
    return 1;
}

// --- CÁC HÀM XỬ LÝ FILE GIẢ CẦY ---

extern "C" int _close(int file) {
    (void)file;
    return -1;
}

extern "C" int _lseek(int file, int ptr, int dir) {
    (void)file;
    (void)ptr;
    (void)dir;
    return 0;
}

extern "C" int _read(int file, char *ptr, int len) {
    (void)file;
    (void)ptr;
    (void)len;
    return 0;
}

extern "C" int _write(int file, char *ptr, int len) {
    (void)file;
    (void)ptr;
    // Mẹo: Sau này nếu bạn muốn lệnh printf in được ra UART để xem log AI,
    // bạn chỉ cần nhét hàm HAL UART Transmit vào vị trí này là xong!
    // Hiện tại cứ giả vờ là đã ghi thành công 'len' byte.
    return len;
}

extern "C" int _fstat(int file, struct stat *st) {
    (void)file;
    if (st) {
        st->st_mode = S_IFCHR; // Báo đây là thiết bị character (kiểu như terminal)
    }
    return 0;
}

extern "C" int _isatty(int file) {
    (void)file;
    return 1; // Báo đây là một terminal
}

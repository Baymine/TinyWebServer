//
// Created by USTCer on 2023/1/29.
//
#include <stdio.h>
#include <string.h>
#include <stdarg.h>


#define SBUF_SIZE 128
char sbuf[SBUF_SIZE];

void MyPrintF(const char *format, ...) {
    va_list args;
    va_start (args, format);

    vsnprintf(sbuf, SBUF_SIZE, format, args);
    printf("%d\n", va_arg(args, int));
    va_end (args);

    printf("%s", sbuf);
}

void temp() {  }

int main() {
    MyPrintF("my name is %s,my age is %d\n", "bob", 18, 23);
    return 0;
}
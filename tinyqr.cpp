#include <stdio.h>
#include <Windows.h>
#include "qr.c"

int main()
{
    printf(" ____ __ __ _ _  _ __  ____ \n");
    printf("(_  _|  |  ( ( \\/ )  \\(  _ \\\n");
    printf("  )(  )(/    /)  (  O ))   /\n");
    printf(" (__)(__)_)__|__/ \\__\\|__\\_)\n");
    printf("Enter text to encode: ");

    char text[73]; // 73 characters + null terminator
    // scanf til newline
    scanf("%72[^\n]", text);

    // let user choose output file
    printf("Enter output file name (will be saved as bitmap): ");
    char filename[256];
    scanf("%255s", filename);


    parseMessage(filename, text, 0);
    printf("QR code saved to %s\n", filename);
    printf("2951 b\n");
    ExitProcess(0);
    return 0;
}
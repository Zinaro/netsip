#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc >= 3 && strcmp(argv[1], "-ip") == 0) {
        char command[100];
        snprintf(command, sizeof(command), "./ipscan %s", argv[2]);
        system(command);
    }
    else {
        system("./ips");
    }

    return 0;
}


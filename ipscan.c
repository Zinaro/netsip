#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <arpa/inet.h>
#include <netinet/ip_icmp.h>
#include <netdb.h>
#include <errno.h>

#define MAX_DEVICES 50

struct device_info {
    char ip_address[16];
    char mac_address[18];
    char hostname[NI_MAXHOST];
    int is_reachable;
};

int is_valid_ip(const char *ip) {
    struct sockaddr_in sa;
    return inet_pton(AF_INET, ip, &(sa.sin_addr)) != 0;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Please input an IP address. Using: ./ipscan <IP address>\n");
        return 1;
    }

    if (!is_valid_ip(argv[1])) {
        printf("Invalid IP address: %s\n", argv[1]);
        return 1;
    }

    struct device_info device;
    strcpy(device.ip_address, argv[1]);

    printf("IP Address: %s\n", device.ip_address);

    struct sockaddr_in sa;
    sa.sin_family = AF_INET;
    inet_pton(AF_INET, device.ip_address, &(sa.sin_addr));

    int ret = getnameinfo((struct sockaddr *)&sa, sizeof(sa), device.hostname, sizeof(device.hostname), NULL, 0, NI_NAMEREQD);
    if (ret != 0) {
        printf("Hostname lookup failed: %s\n", gai_strerror(ret));
        strcpy(device.hostname, "N/A");
    }

    printf("Hostname: %s\n", device.hostname);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("Socket creation error");
        return 1;
    }

    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout));

    struct sockaddr_in target;
    target.sin_family = AF_INET;
    target.sin_port = htons(80);
    inet_pton(AF_INET, device.ip_address, &(target.sin_addr));

    if (connect(sock, (struct sockaddr *)&target, sizeof(target)) == 0) {
        device.is_reachable = 1;
    } else {
        device.is_reachable = 0;
        printf("IP address not found: %s\n", device.ip_address);
    }

    close(sock);

    printf("Is Reachable: %s\n", device.is_reachable ? "Yes" : "No");

    return 0;
}


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netdb.h>
#include <errno.h>

#define MAX_DEVICES 50

struct device_info {
    char ip_address[16];
    char mac_address[18];
    char hostname[256];
    int is_reachable;
};


int main() {

    struct device_info devices[MAX_DEVICES];
    int device_count = 0;


    struct ifreq ifr;
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == -1) {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }

    struct arpreq arp;
    struct sockaddr_in *sin;
    char *iface;

    strncpy(ifr.ifr_name, "enp2s0", IFNAMSIZ);
    if (ioctl(sock, SIOCGIFHWADDR, &ifr) == -1) {
        perror("IOCTL SIOCGIFHWADDR error");
        close(sock);
        exit(EXIT_FAILURE);
    }
    sin = (struct sockaddr_in *)&ifr.ifr_addr;
    iface = inet_ntoa(sin->sin_addr);

    strcpy(arp.arp_dev, iface);
    arp.arp_ha.sa_family = ARPHRD_ETHER;

    FILE *fp = fopen("/proc/net/arp", "r");
    if (fp == NULL) {
        perror("File open error");
        close(sock);
        exit(EXIT_FAILURE);
    }

    char line[256];
    fgets(line, sizeof(line), fp);

    while (fgets(line, sizeof(line), fp) != NULL && device_count < MAX_DEVICES) {
        char ip_address[16];
        char mac_address[18];
        int hw_type, flags;
        if (sscanf(line, "%s 0x%x 0x%x %s", ip_address, &hw_type, &flags, mac_address) == 4) {
            if (hw_type == ARPHRD_ETHER && flags == ATF_COM) {
                strcpy(devices[device_count].ip_address, ip_address);
                strcpy(devices[device_count].mac_address, mac_address);
                device_count++;
            }
        }
    }

    fclose(fp);
    close(sock);

    for (int i = 0; i < device_count; i++) {
        struct sockaddr_in sa;
        memset(&sa, 0, sizeof(sa));
        sa.sin_family = AF_INET;
        inet_pton(AF_INET, devices[i].ip_address, &(sa.sin_addr));

        char host[NI_MAXHOST];
        int status = getnameinfo((struct sockaddr *)&sa, sizeof(sa), host, sizeof(host), NULL, 0, NI_NAMEREQD);
        if (status == 0) {
            strncpy(devices[i].hostname, host, sizeof(devices[i].hostname) - 1);
            devices[i].is_reachable = 1;
        } else {
            strncpy(devices[i].hostname, "", sizeof(devices[i].hostname) - 1);
            devices[i].is_reachable = 0;
        }
    }

    printf("devices on the network:\n");
    for (int i = 0; i < device_count; i++) {
        printf("IP: %s, MAC: %s, Host: %s, Status: %s\n",
               devices[i].ip_address, devices[i].mac_address, devices[i].hostname,
               devices[i].is_reachable ? "Accessible" : "Not Available" );
    }

    return 0;
}


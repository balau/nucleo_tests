#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "w5100.h"
#include "dhcp_client.h"

static
void print_ipaddr(const char *name, in_addr_t addr)
{
    struct in_addr in;

    in.s_addr = addr;
    printf("%s: %s\n", name, inet_ntoa(in));
}

static
int loop(void)
{
    struct dhcp_binding binding;
    uint8_t mac_addr[6];
    time_t next;

    printf("Press any key to continue...");
    getchar();
    printf("\n");

    w5100_read_regx(W5100_SHAR, mac_addr);
    dhcp_init(mac_addr, &binding);

    next = dhcp_bind(&binding);
    print_ipaddr("new client address", binding.client);
    print_ipaddr("gateway", binding.gateway);
    print_ipaddr("subnet", binding.subnet);
    print_ipaddr("DNS", binding.dns_server);
    printf("lease T1: %d seconds\n", (int)binding.lease_t1.tv_sec);
    printf("lease T2: %d seconds\n", (int)binding.lease_t2.tv_sec);
    printf("next call should be in %d seconds\n", (int)next);

    return 0;
}

int main(void)
{
    while(1)
    {
        loop();
    }
}


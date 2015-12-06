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
    int ret;

    printf("Press any key to continue...");
    getchar();
    printf("\n");

    w5100_read_regx(W5100_SHAR, binding.mac_addr);
    ret = dhcp_allocate(&binding);
    if (ret != 0)
    {
        if (ret == DHCP_ESYSCALL)
        {
            perror("dhcp_allocate");
        }
        else
        {
            fprintf(stderr, "error: dhcp_allocate: %d\n", ret);
        }
        return 1;
    }
    print_ipaddr("new client address", binding.client);
    print_ipaddr("gateway", binding.gateway);
    print_ipaddr("subnet", binding.subnet);
    print_ipaddr("DNS", binding.dns_server);
    printf("lease T1: %d seconds\n", (int)binding.lease_t1.tv_sec);
    printf("lease T2: %d seconds\n", (int)binding.lease_t2.tv_sec);
    w5100_write_regx(W5100_SIPR, &binding.client);
    w5100_write_regx(W5100_GAR, &binding.gateway);
    w5100_write_regx(W5100_SUBR, &binding.subnet);

    return 0;
}

int main(void)
{
    while(1)
    {
        loop();
    }
}


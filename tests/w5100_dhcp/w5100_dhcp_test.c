#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <time.h>
#include "w5100.h"
#include "w5100_dhcp.h"

static
void print_ipaddr(const char *name, in_addr_t addr)
{
    struct in_addr in;

    in.s_addr = addr;
    printf("%s: %s\n", name, inet_ntoa(in));
}

static
time_t next;

static
int loop(void)
{
    in_addr_t ip;

    next = w5100_dhcp_bind();
    w5100_read_regx(W5100_SIPR, &ip);
    print_ipaddr("IP", ip);
    print_ipaddr("DNS", w5100_getdns());
    printf("next: %d\n", (int)next);
    sleep(5);

    return 0;
}

int main(void)
{
    while(1)
    {
        loop();
    }
}


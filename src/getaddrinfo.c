/*
 * Copyright (c) 2015 Francesco Balducci
 *
 * This file is part of nucleo_tests.
 *
 *    nucleo_tests is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU Lesser General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    nucleo_tests is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public License
 *    along with nucleo_tests.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>

#define DNS_MSG_LEN 512
#define DNS_HEADER_LEN 12

#define TYPE_A        1
#define TYPE_NS       2
#define TYPE_MD       3
#define TYPE_MF       4
#define TYPE_CNAME    5
#define TYPE_SOA      6
#define TYPE_MB       7
#define TYPE_MG       8
#define TYPE_MR       9
#define TYPE_NULL    10
#define TYPE_WKS     11
#define TYPE_PTR     12
#define TYPE_HINFO   13
#define TYPE_MINFO   14
#define TYPE_MX      15
#define TYPE_TXT     16

#define QTYPE_AXFR  252
#define QTYPE_MAILA 253
#define QTYPE_MAILB 254
#define QTYPE_ALL   255

#define CLASS_IN     1
#define CLASS_CS     2
#define CLASS_CH     3
#define CLASS_HS     4

#define QCLASS_ANY 255

#define OPCODE_QUERY  (0<<3)
#define OPCODE_IQUERY (1<<3)
#define OPCODE_STATUS (2<<1)

#define RD (1<<0)

#define RCODE_NOERR   0
#define RCODE_EFMT    1
#define RCODE_ESRV    2
#define RCODE_ENAME   3
#define RCODE_EIMPL   4
#define RCODE_EREF    5

#define OFFSET_ID        0
#define OFFSET_OPCODE    2
#define OFFSET_RCODE     3
#define OFFSET_QDCOUNT   4
#define OFFSET_ANCOUNT   6
#define OFFSET_NSCOUNT   8
#define OFFSET_ARCOUNT  10
#define OFFSET_QUESTION 12

/* With respect to first byte after NAME in RR */
#define OFFSET_TYPE      0
#define OFFSET_CLASS     2
#define OFFSET_TTL       4
#define OFFSET_RDLENGTH  8
#define OFFSET_RDATA    10

in_addr_t getaddrinfo_dns_server = 0x08080808; /* google */

void freeaddrinfo(struct addrinfo * ai)
{
    struct addrinfo *next;

    for (; ai != NULL; ai = next)
    {
        if (ai->ai_addr != NULL)
        {
            free(ai->ai_addr);
        }
        if (ai->ai_canonname != NULL)
        {
            free(ai->ai_canonname);
        }
        next = ai->ai_next;
        free(ai);
    }
}

static
uint8_t *dns_query_append16(uint8_t *pq, uint16_t hval)
{
    uint16_t nval;

    nval = htons(hval);

    memcpy(pq, &nval, 2);

    return pq + 2;
}

static
uint16_t dns_response_get16(const uint8_t *pr)
{
    uint16_t nval;

    memcpy(&nval, pr, 2);

    return ntohs(nval);
}

static
int label_ispointer(const uint8_t *label)
{
    return (((*label) & 0xC0) == 0xC0);
}

static
int label_isend(const uint8_t *label)
{
    return ((*label) == 0x00);
}

static
int label_isstr(const uint8_t *label)
{
    return (((*label) > 0) && ((*label) < 0xC0));
}

static
uint16_t get_label_offset(const uint8_t *label)
{
    uint16_t offset;

    offset = dns_response_get16(label);
    offset &= ~0xC000;

    return offset;
}

static
uint8_t get_label_len(const uint8_t *label)
{
    return ((*label) & (~0xC0));
}

static
const uint8_t *get_label_str(const uint8_t *label)
{
    return &label[1];
}

static
const uint8_t *skip_labels(const uint8_t *label)
{
    int label_end;

    label_end = 0;

    while (!label_end)
    {
        if (label_isend(label))
        {
            label++;
            label_end = 1;
        }
        else if (label_ispointer(label))
        {
            label += 2;
            label_end = 1;
        }
        else if (label_isstr(label))
        {
            label += get_label_len(label) + 1;
        }
        else
        {
            label = NULL;
            label_end = 1;
        }
    }

    return label;
}

static
uint16_t parse_name(const uint8_t *msg, const uint8_t *pstart, char *dst)
{
    uint16_t str_len;
    int str_end;
    int first_label_str;
    const uint8_t *label;

    str_len = 0;
    str_end = 0;
    first_label_str = 1;
    label = pstart;

    while (!str_end)
    {
        if (label_ispointer(label))
        {
            label = msg + get_label_offset(label);
        }
        else if (label_isend(label))
        {
            if (dst != NULL)
            {
                *dst++ = '\0';
            }
            str_len++;
            str_end = 1;
        }
        else if (label_isstr(label))
        {
            uint8_t label_len;

            label_len = get_label_len(label);
            if (first_label_str)
            {
                first_label_str = 0;
            }
            else
            {
                if (dst != NULL)
                {
                    *dst++ = '.';
                }
                str_len++;
            }
            if (dst != NULL)
            {
                memcpy(dst, get_label_str(label), label_len);
                dst += label_len;
            }
            str_len += label_len;
            label += (label_len + 1);
        }
        else
        {
            /* malformed */
            str_len = 0;
            str_end = 1;
        }
    }
    return str_len;
}

static
uint16_t get_name_size(const uint8_t *msg, const uint8_t *pstart)
{
    return parse_name(msg, pstart, NULL);
}

static
char *new_name(const uint8_t *msg, const uint8_t *pstart, uint16_t *len)
{
    char *name;
    uint16_t name_len;

    name_len = get_name_size(msg, pstart);
    if (name_len == 0)
    {
        name = NULL;
    }
    else
    {
        name = malloc(name_len);
        parse_name(msg, pstart, name);
    }
    if (len != NULL)
    {
        *len = name_len;
    }

    return name;
}

static
int create_socket(in_addr_t dns_server)
{
    int sock;
    struct sockaddr_in server;
    struct timespec timeout = {1, 0}; /* 1s TODO: timeval? */

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == -1)
    {
        return -1;
    }
    
    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(struct timespec)) == -1)
    {
        close(sock);
        return -1;
    }
    if (setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(struct timespec)) == -1)
    {
        close(sock);
        return -1;
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(53);
    server.sin_addr.s_addr = dns_server;
    if (connect(sock, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
    {
        close(sock);
        return -1;
    }

    return sock;
}

static
int send_dns_query(int sock, const char *nodename, uint16_t *id)
{
    uint8_t dns_query[512];
    size_t dns_query_len;
    uint8_t *pq;
    uint8_t *plen;
    const char *pname;

    *id = rand();

    memset(dns_query, 0, sizeof(dns_query));
    dns_query_append16(&dns_query[OFFSET_ID], *id);
    dns_query[OFFSET_OPCODE] = OPCODE_QUERY | RD; /* QR|OPCODE|RD */
    dns_query[OFFSET_RCODE] = 0x00; /* RA|Z|RCODE */
    dns_query_append16(&dns_query[OFFSET_QDCOUNT], 1);
    dns_query_append16(&dns_query[OFFSET_ANCOUNT], 0);
    dns_query_append16(&dns_query[OFFSET_NSCOUNT], 0);
    dns_query_append16(&dns_query[OFFSET_ARCOUNT], 0);

    /* build QNAME */
    pq = &dns_query[OFFSET_QUESTION];
    pname = nodename;
    while (*pname != '\0')
    {
        plen = pq;
        pq++;
        while ((*pname != '.') && (*pname != '\0'))
        {
            *pq++ = *pname++;
        }
        *plen = pq - plen - 1;
        if (*pname == '.')
        {
            pname++;
        }
    }
    *pq++ = 0x00; /* end QNAME */
    pq = dns_query_append16(pq, TYPE_A);
    pq = dns_query_append16(pq, CLASS_IN);
    dns_query_len = (size_t)(pq - &dns_query[0]);

    if (send(sock, dns_query, dns_query_len, 0) < 0)
    {
        return EAI_SYSTEM;
    }

    return 0;
}

static
const uint8_t *skip_questions(const uint8_t *dns_response)
{
    uint16_t qdcount;
    const uint8_t *presp;
    uint16_t ientry;

    qdcount = dns_response_get16(&dns_response[OFFSET_QDCOUNT]);
    presp = &dns_response[OFFSET_QUESTION];
    for (ientry = 0; ientry < qdcount; ientry++)
    {
        presp = skip_labels(presp);
        if (presp == NULL)
        {
            break; /* malformed */ 
        }
        presp += (2 + 2); /* QTYPE + QCLASS */
    }
    if (ientry != qdcount)
    {
        presp = NULL; /* malformed */
    }

    return presp;
}

static
int recv_dns_response(
        int sock,
        uint16_t id,
        struct addrinfo **res)
{
    int ret;
    uint8_t dns_response[512];
    ssize_t dns_response_len;

    do
    {
        uint8_t rcode;
        uint16_t ancount;
        uint16_t ientry;
        const uint8_t *presp;
        struct addrinfo *ai_head;
        struct addrinfo *ai_tail;

        dns_response_len = recv(sock, dns_response, sizeof(dns_response), 0);
        if (dns_response_len < 0)
        {
            ret = EAI_SYSTEM;
            break;
        }
        if (dns_response_len < DNS_HEADER_LEN)
        {
            continue; /* malformed */
        }
        if (dns_response_get16(&dns_response[OFFSET_ID]) != id)
        {
            continue; /* ignore responses with wrong ID */
        }
        rcode = dns_response[OFFSET_RCODE] & 0x0F;
        if (rcode == RCODE_ENAME)
        {
            ret = EAI_NONAME;
            break;
        }
        else if (rcode != RCODE_NOERR)
        {
            ret = EAI_FAIL;
            break;
        }
        presp = skip_questions(dns_response);
        if (presp == NULL)
        {
            ret = EAI_FAIL; /* malformed */
            break;
        }
        ancount = dns_response_get16(&dns_response[OFFSET_ANCOUNT]);
        ai_head = NULL;
        ai_tail = NULL;
        for (ientry = 0; ientry < ancount; ientry++)
        {
            uint16_t len;
            uint16_t type;
            uint16_t class;
            uint16_t rdlength;
            char *name;

            name = new_name(dns_response, presp, &len);
            if (name == NULL)
            {
                ret = EAI_FAIL; /* malformed */
                break;
            }
            presp = skip_labels(presp);
            type = dns_response_get16(&presp[OFFSET_TYPE]);
            class = dns_response_get16(&presp[OFFSET_CLASS]);
            rdlength = dns_response_get16(&presp[OFFSET_RDLENGTH]);
            if ((type == TYPE_A) && (class == CLASS_IN))
            {
                struct addrinfo *ai_new;
                struct sockaddr_in *addr;

                /* TODO: check malloc return */

                ai_new = malloc(sizeof(struct addrinfo));

                ai_new->ai_flags = 0; /* TODO: from hints */
                ai_new->ai_family = AF_INET; /* TODO: from hints */
                ai_new->ai_socktype = SOCK_DGRAM; /* TODO: from hints */
                ai_new->ai_protocol = 0; /* TODO: from hints */
                /* addr */
                addr = malloc(sizeof(struct sockaddr_in));
                addr->sin_family = AF_INET;
                addr->sin_port = 0; /* TODO: from hints */
                memcpy(&addr->sin_addr.s_addr, &presp[OFFSET_RDATA], 4);
                ai_new->ai_addrlen = sizeof(struct sockaddr_in);
                ai_new->ai_addr = (struct sockaddr *)addr;

                ai_new->ai_canonname = name;
                ai_new->ai_next = NULL;

                /* insert in list */
                if (ai_head == NULL)
                {
                    ai_head = ai_new;
                    ai_tail = ai_new;
                }
                else
                {
                    ai_tail->ai_next = ai_new;
                    ai_tail = ai_new;
                }

            }
            else
            {
                free(name);
            }
            presp += (OFFSET_RDATA + rdlength);
        }
        if (ientry != ancount)
        {
            ret = EAI_FAIL; /* malformed */
            freeaddrinfo(ai_head);
            break;
        }
        *res = ai_head;
        ret = 0;
        break;
    } while(1);

    return ret;
}

int getaddrinfo(
        const char *nodename,
        const char *servname,
        const struct addrinfo *hints,
        struct addrinfo **res)
{
    int ret;
    int sock;
    uint16_t id;

    if ((nodename == NULL) && (servname == NULL))
    {
        return EAI_NONAME;
    }
    if (hints != NULL)
    {
        if (
                (hints->ai_family != AF_UNSPEC) &&
                (hints->ai_family != AF_INET) &&
                (hints->ai_family != 0)
           )
        {
            return EAI_FAMILY;
        }
        if (
                (hints->ai_socktype != SOCK_DGRAM) &&
                (hints->ai_socktype != SOCK_STREAM) &&
                (hints->ai_socktype != 0)
           )
        {
            return EAI_SOCKTYPE;
        }
    }

    sock = create_socket(getaddrinfo_dns_server);
    if (sock == -1)
    {
        return EAI_SYSTEM;
    }

    ret = send_dns_query(sock, nodename, &id);
    if (ret != 0)
    {
        close(sock);
        return ret;
    }

    ret = recv_dns_response(sock, id, res);
    close(sock);

    return ret;
}


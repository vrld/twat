/* This is a project for a seminar at the Hochschule fuer Gestaltung Karlsruhe.
 * Feel free to use it within the bounds of the following license:
 *
 * Copyright (c) 2010 Matthias Richter
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to ect to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name(s) of the above
 * copyright holders shall not be used in advertising or otherwise
 * to promote the sale, use or other dealings in this Software
 * without prior written authorization.
 */
#include "udp.h"
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

int udp_create_socket(struct sockaddr_in* addr, size_t size_addr, in_addr_t ip_addr, int port, int timeout)
{
    int sock;
    memset(addr, 0, size_addr);
    addr->sin_family = AF_INET;
    addr->sin_addr.s_addr = ip_addr;
    addr->sin_port = htons(port);

    sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock > 0 && timeout > 0) {
        struct timeval tv;
        tv.tv_sec = timeout;
        tv.tv_usec = 0;
        if (-1 == setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv))) {
            close(sock);
            sock = -1;
        }
    }

    return sock;
}

int udp_receive(int sock, char* buffer, size_t size, struct sockaddr_in* client, socklen_t size_client)
{
    int transfered = recvfrom(sock, buffer, size, 0,
            (struct sockaddr*)client, &size_client);
    if (transfered >= 0)
        buffer[transfered] = '\0';

    return transfered;
}

int udp_send(int sock, const char* msg, struct sockaddr_in* client, socklen_t size_client)
{
    int len_msg = strlen(msg);
    int transfered = sendto(sock, msg, len_msg, 0,
            (struct sockaddr*)client, size_client);
    return (transfered == len_msg);
}

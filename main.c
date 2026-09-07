#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

typedef uint8_t         u8;
typedef uint16_t        u16;
typedef uint32_t        u32;
typedef uint64_t        u64;

typedef int8_t          i8;
typedef int16_t         i16;
typedef int32_t         i32;
typedef int64_t         i64;

static void die(char *msg, i8 code)
{
        fprintf(stderr, "ERROR: %s failed\n", msg);
        exit(code);
}

int main()
{
        i32 sfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sfd == -1) die("socket", 1);

        u16 port = 8080;
        struct sockaddr_in sa = {0};
        socklen_t sa_len = sizeof(sa);

        sa.sin_family = AF_INET;
        sa.sin_addr.s_addr = INADDR_ANY;
        sa.sin_port = htons(port);

        if (bind(sfd, (struct sockaddr*)&sa, sa_len) < 0) die("bind", 1);
        if (listen(sfd, 5) < 0) die("listen", 1);

        printf("Server listening: %d\n", port);

        struct sockaddr_in ca = {0};
        socklen_t ca_len = sizeof(ca);
        i32 new_con = accept(sfd,
                        (struct sockaddr*)&ca,
                        &ca_len
        );

        char cl_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET,
                  &ca.sin_addr,
                  cl_ip,
                  sizeof(cl_ip)
        );

        printf("[+] Client from %s:%d\n",
                        cl_ip,
                        ntohs(ca.sin_port)
        );

        char buf[1024];
        ssize_t n = recv(new_con, buf, sizeof(buf) - 1, 0);
        if (n > 0) {
                buf[n] = '\0';
                printf("[+] Received: %s\n", buf);
        }

        close(sfd);
        close(new_con);

        return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <ifaddrs.h>
#include <arpa/inet.h>

static void die(char *msg, int status_code)
{
        perror(msg);
        exit(status_code);
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <net/if.h>

int get_ip(char *buf, size_t buflen)
{
        struct ifaddrs *ifaddr;
        struct ifaddrs *ifa;

        if (getifaddrs(&ifaddr) == -1)
                return -1;

        for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {

                if (!ifa->ifa_addr)
                        continue;

                if (ifa->ifa_addr->sa_family != AF_INET)
                        continue;

                if (ifa->ifa_flags & IFF_LOOPBACK)
                        continue;

                if (!(ifa->ifa_flags & IFF_UP))
                        continue;

                struct sockaddr_in *addr =
                        (struct sockaddr_in *)ifa->ifa_addr;

                if (inet_ntop(AF_INET,
                              &addr->sin_addr,
                              buf,
                              buflen) != NULL)
                {
                        freeifaddrs(ifaddr);
                        return 0;
                }
        }

        freeifaddrs(ifaddr);
        return -1;
}
int main()
{
        uint16_t port = 8080;
        struct sockaddr_in addr = {0};
        int opt = 1;
        socklen_t addrlen = sizeof(addr);

        int s_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (s_fd == -1) die("socket failed", 1);

        if (setsockopt(s_fd, 
                                SOL_SOCKET, 
                                SO_REUSEADDR|SO_REUSEPORT, 
                                &opt, 
                                sizeof(opt)))
        {
                die("setsockopt failed", 1);
        }

        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port);

        if (bind(s_fd, (struct sockaddr*)&addr, addrlen) < 0) die("bind failed", 1);
        if (listen(s_fd, 5) < 0) die("listen failed", 1);

        char ip[INET_ADDRSTRLEN];
        if (get_ip(ip, sizeof(ip)) == -1) {
                fprintf(stderr, "ERROR: No IPv4 interface found\n");
                return 1;
        }


        printf("Server is on %s:%d\n", ip, port);

        struct sockaddr_in cl_addr;
        socklen_t cl_len = sizeof(cl_addr);
        int new_con;
        new_con = accept(s_fd, 
                        (struct sockaddr*)&addr, 
                        &cl_len
                        );

        char cl_ip[INET_ADDRSTRLEN];
        inet_ntop(
                        AF_INET,
                        &cl_addr.sin_addr,
                        cl_ip,
                        sizeof(cl_ip)
                 );

        printf("[+] Client from %s:%d\n",
                        cl_ip,
                        ntohs(cl_addr.sin_port)
              );
        char buf[1024];
        ssize_t n = recv(new_con, buf, sizeof(buf) - 1, 0);
        if (n > 0) {
                buf[n] = '\0';
                printf("Received: %s\n", buf);
        }

        close(new_con);
        close(s_fd);

        return 0;
}

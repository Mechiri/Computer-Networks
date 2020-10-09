#define _POSIX_C_SOURCE 200809L


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char const *argv[])
{
        char hostname[100];
        struct addrinfo hostinfo;
        struct addrinfo *res, *addrinfo_iter;
        char port[10];
        int getaddrinfo_res;
        char *network_address;
        int socket_fd;
        char* read_buffer[256];
        ssize_t read_buffer_status;
        int connect_res;

        printf("Enter the hostname: ");
        scanf("%s", hostname);

        printf("Enter the port: ");
        scanf("%s", port);
        //check hostname
        if( (strlen(hostname)<0) || (strlen(hostname) >= 64) )
        {
                fprintf(stderr, "Hostname should not be less than zero and greater than 63\n");
                exit(1);
        }

        //assign memory to the host
        memset( &hostinfo, 0, sizeof(hostinfo) );
        hostinfo.ai_family = AF_UNSPEC; //both ipv4 & ipv6
        hostinfo.ai_socktype = SOCK_STREAM; //tcp

        //Get host details
        getaddrinfo_res = getaddrinfo( hostname, port, &hostinfo, &res );
        if(getaddrinfo_res != 0)
        {
                fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(getaddrinfo_res));
                exit(EXIT_FAILURE);
        }

        for(addrinfo_iter = res; addrinfo_iter != NULL; addrinfo_iter = addrinfo_iter->ai_next)
        {
                addrinfo_iter = res;
                network_address = inet_ntoa(((struct sockaddr_in*)addrinfo_iter->ai_addr)->sin_addr);
                printf("\n%s", network_address);

                //socket creation
                socket_fd = socket(addrinfo_iter->ai_family, addrinfo_iter->ai_socktype, addrinfo_iter->ai_protocol);
                if(socket_fd == -1)
                {
                        fprintf(stderr, "socket err: %d\n", socket_fd);
                }
                //connect
                connect_res = connect(socket_fd, addrinfo_iter->ai_addr, addrinfo_iter->ai_addrlen);
                if(connect_res !=0 )
                {
                        fprintf(stderr, "socket connection error: %d\n", connect_res);
                        close(connect_res);
                }
                else
                {
                        printf("Successfully connected\n");
                        break;
                }
        }

        read_buffer_status = read(socket_fd, read_buffer, 255);
        if(read_buffer_status == -1)
        {
                fprintf(stderr, "Read buffer error: %d\n", read_buffer_status);
                exit(1);
        }

        printf("The buffer read is: %s\n", read_buffer);
        close(socket_fd);
        return 0;
}
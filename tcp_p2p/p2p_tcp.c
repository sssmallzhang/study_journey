#include <errno.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <poll.h>
#include <sys/epoll.h>
#include <arpa/inet.h>



#define TARGET_IP                        "192.168.47.129"


int main(){

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    // addr of this host
    struct sockaddr_in thisaddr;
    thisaddr.sin_family = AF_INET;
    thisaddr.sin_port = htons(8000);
    thisaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (-1 == bind(sockfd, (struct sockaddr*)&thisaddr, sizeof(struct sockaddr))) {
        printf("bind failed\n");
        return -1;
    }

    // addr of target host
    struct sockaddr_in targetaddr;
    targetaddr.sin_family = AF_INET;
    targetaddr.sin_port = htons(8000);
    targetaddr.sin_addr.s_addr = inet_addr(TARGET_IP);
    
    while(1){
        int ret = connect(sockfd, (struct sockaddr*)&targetaddr, sizeof(struct sockaddr));
        if(ret == -1){
            printf("connect failed\n");
            usleep(1);
            continue;
        }
        if(ret == 0){
            char w_buffer[13] = {"I'm player!"};
            char r_buffer[20];
            memset(r_buffer, 0, 20);

            send(sockfd, w_buffer, 13, 0);
            recv(sockfd, r_buffer, 20, 0);

            printf("recv: %s\n", r_buffer);
        }

        close(sockfd);
        return 0;
    }
}
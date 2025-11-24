
#include <errno.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <poll.h>
#include <sys/epoll.h>

#define READ_BUFFER_SIZE         1024
#define WRITE_BUFFER_SIZE        1024
#define CONNECTION_LIST_SIZE     1024

int epfd = 0;

typedef int (*call_back_fun)(int fd);

struct connect_config{
	int fd;

	char rbuffer[READ_BUFFER_SIZE];
	int rlength;
	char wbuffer[WRITE_BUFFER_SIZE];
	int wlength;

	call_back_fun send_call_back;
	union{
		call_back_fun accept_call_back;
		call_back_fun recv_call_back;
	}in_action;

};

struct connect_config connection_list[CONNECTION_LIST_SIZE];


int recv_cb(int fd);
int send_cb(int fd);
int set_event(int fd, int event, int flag);

int event_register(int fd){
	connection_list[fd].fd = fd;
	connection_list[fd].in_action.recv_call_back = recv_cb;
	connection_list[fd].send_call_back = send_cb;
	// connection_list[clientfd].wbuffer = {0};
	// connection_listp[clientfd].rbuffer = {0};
	memset(connection_list[fd].wbuffer, 0, READ_BUFFER_SIZE);
	memset(connection_list[fd].rbuffer, 0, READ_BUFFER_SIZE);
	connection_list[fd].wlength = 0;
	connection_list[fd].rlength = 0;
}

int accept_cb(int fd){
	printf("sssssssss");
	struct sockaddr_in addr;
	int len = sizeof(addr);
	int clientfd = accept(fd, (struct sockaddr*)&addr, &len);
	printf("accept: %d\n", clientfd);

	event_register(clientfd);
	// connection_list[clientfd].fd = clientfd;
	// connection_list[clientfd].in_action.recv_call_back = recv_cb;
	// connection_list[clientfd].sen_call_back = send_cb;
	// // connection_list[clientfd].wbuffer = {0};
	// // connection_listp[clientfd].rbuffer = {0};
	// memset(connection_list[clientfd].wbuffer, 0, READ_BUFFER_SIZE);
	// memset(connection_list[clientfd].rbuffer, 0, READ_BUFFER_SIZE);
	// connection_list[clientfd].wlength = 0;
	// connection_list[clientfd].rlength = 0;
	set_event(clientfd, EPOLLIN, 1);
	return 0;
}

int recv_cb(int fd){

	int count = recv(fd, connection_list[fd].rbuffer, READ_BUFFER_SIZE, 0);
	if(count == 0){
		printf("close the connection\n");
		close(fd);

		epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
		return 0;
	}
	connection_list[fd].rlength = count;

	connection_list[fd].wlength = count;
	memcpy(connection_list[fd].wbuffer, connection_list[fd].rbuffer, connection_list[fd].wlength);

	printf("recv: %s\n", connection_list[fd].rbuffer);
	set_event(fd, EPOLLOUT, 0);

	return 0;
}

int send_cb(int fd){

	int count = send(fd, connection_list[fd].wbuffer, connection_list[fd].wlength, 0);
	connection_list[fd];
	
	
	set_event(fd, EPOLLIN, 0);
	return 0;
}



int set_event(int fd, int event, int flag){
    if(flag){
		struct epoll_event ev;
		ev.events = event;
		ev.data.fd = fd;
		epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);
	}else{
		struct epoll_event ev;
		ev.events = event;
		ev.data.fd = fd;
		
		epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev);
	}
	
}


int server_init(unsigned short port){
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in servaddr;
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY); // 0.0.0.0
	servaddr.sin_port = htons(2000); // 0-1023, 

	if (-1 == bind(sockfd, (struct sockaddr*)&servaddr, sizeof(struct sockaddr))) {
		printf("bind failed: %s\n", strerror(errno));
	}

	listen(sockfd, 10);
	printf("listen finshed: %d\n", sockfd); // 3 

    return sockfd;
}

int main(){
    
	epfd = epoll_create(1);
    unsigned short port = 2000;
    int sockfd = server_init(port);
    set_event(sockfd, EPOLLIN, 1);
	connection_list[sockfd].fd = sockfd;
	connection_list[sockfd].in_action.accept_call_back = accept_cb;
	while(1){

		struct epoll_event event[1024];
		int nready = epoll_wait(epfd, event, 1024, -1);

		int i;
		for(i = 0; i < nready; i++){
			
			int fd = event[i].data.fd;
			if(event[i].events & EPOLLIN){
				connection_list[fd].in_action.recv_call_back(fd);
			}
			if(event[i].events & EPOLLOUT){
				connection_list[fd].send_call_back(fd);
			}

		}


	}


}
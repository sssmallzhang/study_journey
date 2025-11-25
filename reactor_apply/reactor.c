
#include "server.h"

#define CONNECTION_LIST_SIZE     1024
#define MAX_PORT                 20

int epfd = 0;

struct connect_config connection_list[CONNECTION_LIST_SIZE];


int recv_cb(int fd);
int send_cb(int fd);
int set_event(int fd, int event, int flag);

int event_register(int fd){
	connection_list[fd].fd = fd;
	connection_list[fd].in_action.recv_call_back = recv_cb;
	connection_list[fd].send_call_back = send_cb;
	memset(connection_list[fd].wbuffer, 0, READ_BUFFER_SIZE);
	memset(connection_list[fd].rbuffer, 0, READ_BUFFER_SIZE);
	connection_list[fd].wlength = 0;
	connection_list[fd].rlength = 0;
	connection_list[fd].status = 0;
	return 0;
}

int accept_cb(int fd){
	struct sockaddr_in addr;
	int len = sizeof(addr);
	int clientfd = accept(fd, (struct sockaddr*)&addr, &len);
	printf("accept: %d\n", clientfd);

	event_register(clientfd);
	set_event(clientfd, EPOLLIN, 1);
	return 0;
}

int recv_cb(int fd){

	memset(connection_list[fd].rbuffer, 0, sizeof(connection_list[fd].rbuffer));
	int count = recv(fd, connection_list[fd].rbuffer, READ_BUFFER_SIZE, 0);
	if(count == 0){
		printf("close the connection\n");
		close(fd);

		epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
		return 0;
	}
	connection_list[fd].rlength = count;

	// connection_list[fd].wlength = count;
	// memcpy(connection_list[fd].wbuffer, connection_list[fd].rbuffer, connection_list[fd].wlength);

	// printf("recv: %s\n", connection_list[fd].rbuffer);

	http_request(connection_list + fd);

	set_event(fd, EPOLLOUT, 0);

	return 0;
}

int send_cb(int fd){
	if(connection_list[fd].status == 1){   // header stage
        send(fd, connection_list[fd].wbuffer, strlen(connection_list[fd].wbuffer), 0);
    }

    http_respon(connection_list + fd);

    if(connection_list[fd].status == 2){
		connection_list[fd].status = 0;
        connection_list[fd].wlength = 0;
		memset(connection_list[fd].wbuffer, 0, READ_BUFFER_SIZE);
        set_event(fd, EPOLLIN, 0);
    }
	// int count = send(fd, connection_list[fd].wbuffer, connection_list[fd].wlength, 0);
	// set_event(fd, EPOLLIN, 0);
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
	servaddr.sin_port = htons(port); // 0-1023, 

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

	int i = 1;
	for(i = 0; i < MAX_PORT; i++){
		int sockfd = server_init(port + i);
		set_event(sockfd, EPOLLIN, 1);
		connection_list[sockfd].fd = sockfd;
		connection_list[sockfd].in_action.accept_call_back = accept_cb;
	}
    // int sockfd = server_init(port);
    // set_event(sockfd, EPOLLIN, 1);
	// connection_list[sockfd].fd = sockfd;
	// connection_list[sockfd].in_action.accept_call_back = accept_cb;
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
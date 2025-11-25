
#include <errno.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <poll.h>
#include <sys/epoll.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>



#define READ_BUFFER_SIZE         1024
#define WRITE_BUFFER_SIZE        1024
#define CONNECTION_LIST_SIZE     1024


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

    int status;                          //控制输出流的状态机
};

int http_request(struct connect_config *connect_info);
int http_respon(struct connect_config *connect_info);
int set_event(int fd, int event, int flag);
#include "server.h"



int http_request(struct connect_config *connect_info){

    // printf("recv: %s\n",connect_info.rbuffer);
    memset(connect_info -> wbuffer, 0, WRITE_BUFFER_SIZE);
    connect_info -> wlength = 0;
    return 0;
}


int http_respon(struct connect_config *connect_info){

    int filefd = open("a.png", O_RDONLY);
    if(filefd == -1) printf("open failed\n");
    struct stat stat_buf;
    fstat(filefd, &stat_buf);
    if(connect_info -> status == 0){
        sprintf(connect_info -> wbuffer, "HTTP/1.1 200 OK\r\n"
			"Content-Type: image/png\r\n"
			"Accept-Ranges: bytes\r\n"
			"Content-Length: %ld\r\n"
			"Date: Tue, 30 Apr 2024 13:16:46 GMT\r\n\r\n"
            , stat_buf.st_size);
        
        connect_info -> status = 1;
    }

    else if(connect_info -> status == 1){

        int ret = sendfile(connect_info -> fd, filefd, NULL, stat_buf.st_size);
        if (ret == -1) {
			printf("errno: %d\n", errno);
		}

        connect_info -> status = 2;                      //发送完成，将状态机置为2
    }
    // else if(connect_info -> status == 2){

    //     connect_info -> status = 0;
    //     connect_info -> wlength = 0;
	// 	memset(connect_info->wbuffer, 0, READ_BUFFER_SIZE);
    // }
    
    return 0;

}

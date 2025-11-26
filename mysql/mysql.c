


#include <mysql.h>
#include <stdio.h>
#include <string.h>

#define DB_SERVER_IP          "192.168.47.128"
#define DB_SERVER_PORT        3306
#define DB_SERVER_PAWD        "mypwd"
#define DB_SEVER_USERNAME     "remote_user"
#define DB_SERVER_TABLE       "KING_DB"


#define SQL_INSERT_TABLE_USER "INSERT TBL_USER(U_NAME, U_GENGDER) VALUES('zhang', 'man');"
#define SQL_SELECT_TABLE_USER "select * from TBL_USER;"
#define SQL_DELETE_TBL_USER   "call PROC_DELETE_USER('lee');"


#define SQL_INSERT_IMG_USER   "INSERT TBL_USER(U_NAME, U_GENGDER, U_IMG) VALUES('zhang', 'man', ?);"
#define SQL_SELECT_IMG_USER   "select U_IMG from TBL_USER where U_NAME = 'zhang';"

#define IMG_FILE_LENGTH       (64*1024)

int DB_mysql_select(MYSQL *handle){
    //query
    if(mysql_real_query(handle, SQL_SELECT_TABLE_USER, strlen(SQL_SELECT_TABLE_USER))){
        printf("mysql_real_query : %s\n", mysql_error(handle));
        return -1;
    }
    
    //store
    MYSQL_RES *res = mysql_store_result(handle);
    if(res == NULL){
        return -2;
    }
    //row, fielid
    int rows = mysql_num_rows(res);
    int fields = mysql_num_fields(res);
    printf("rows:%d\n", rows);
    printf("fields: %d\n", fields);

    //fetch
    MYSQL_ROW row;
    while(row = mysql_fetch_row(res)){
        int i = 0;
        for(int i = 0; i < fields; i++){
            printf("%s\t", row[i]);
        }
        printf("\n");
    }


    mysql_free_result(res);


    return 0;

}



int read_image(char *filename, char *buffer){

    if(filename == NULL || buffer == NULL) return -1;
    FILE *fp = fopen(filename, "rb");
    if(fp == NULL){
        printf("openfile fail");
        return -2;
    }

    fseek(fp, 0, SEEK_END);
    int length = ftell(fp);
    fseek(fp, 0 ,SEEK_SET);

    int size = fread(buffer, 1, length, fp);
    if(size != length){
        printf("read fail size1: %d\n", size);
        return -3;
    }
    fclose(fp);
    return size;
}

int write_image(char *filename, char *buffer, int length){
    if(filename == NULL || buffer == NULL || length <= 0){
        printf("create file failed");
        return -1;
    }


    FILE *fp = fopen(filename, "wb+");
    if(fp == NULL){
        printf("openfile fail");
        return -2;
    }

    int size = fwrite(buffer, 1, length, fp);
    if(size != length){
        printf("write failed\n: %d", size);
        return -3;
    }
    fclose(fp);

    return size;

}

int mysql_write(MYSQL *handle, char *buffer, int length){
    if(handle == NULL || buffer == NULL || length <= 0){
        printf("paramter error");
        return -1;
    }
    MYSQL_STMT *stmt = mysql_stmt_init(handle);

    int ret = mysql_stmt_prepare(stmt, SQL_INSERT_IMG_USER, strlen(SQL_INSERT_IMG_USER));
    if(ret){
        printf("mysql_stmt_prepare : %s\n", mysql_error(handle));
        return -1;
    }
    MYSQL_BIND param = {0}; 
    param.buffer_type = MYSQL_TYPE_LONG_BLOB;
    param.buffer = NULL;
    param.is_null = 0;
    param.length = NULL;


    ret = mysql_stmt_bind_param(stmt, &param); 
    if(ret){
        printf("mysql_stmt_bind_param : %s\n", mysql_error(handle));
        return -3;
    }

    ret = mysql_stmt_send_long_data(stmt, 0, buffer, length);
    if(ret){
        printf("mysql_stmt_send_long_data : %s\n", mysql_error(handle));
        return -4;
    }


    ret = mysql_stmt_execute(stmt);
    if(ret){
        printf("mysql_stmt_execute : %s\n", mysql_error(handle));
        return -5;
    }
    ret = mysql_stmt_close(stmt);
    if(ret){
        printf("mysql_stmt_clode : %s\n", mysql_error(handle));
        return -6;
    }
    return 0;
}


int mysql_read(MYSQL *handle, char *buffer, int length){
    if(handle == NULL || buffer == NULL || length <= 0) return 0;
    MYSQL_STMT *stmt = mysql_stmt_init(handle);

    int ret = mysql_stmt_prepare(stmt, SQL_SELECT_IMG_USER, strlen(SQL_SELECT_IMG_USER));
    if(ret){
        printf("mysql_stmt_prepare : %s\n", mysql_error(handle));
        return -1;
    }
    MYSQL_BIND result = {0}; 

    result.buffer_type = MYSQL_TYPE_LONG_BLOB;
    long unsigned int total_length = 0;
    result.length = &total_length;

    ret = mysql_stmt_bind_result(stmt, &result);
    if(ret){
        printf("mysql_stmt_bind_result : %s\n", mysql_error(handle));
        return -2;
    }
    ret = mysql_stmt_execute(stmt);
    if(ret){
        printf("mysql_stmt_execute : %s\n", mysql_error(handle));
        return -3;
    }

    ret = mysql_stmt_store_result(stmt);
    if(ret){
        printf("mysql_stmt_store_result : %s\n", mysql_error(handle));
        return -4;
    }
    while(1){
        ret  = mysql_stmt_fetch(stmt);
        if(ret != 0 && ret != MYSQL_DATA_TRUNCATED)break;

        int start = 0;
        while(start < total_length){
            result.buffer = buffer+start;
            result.buffer_length = 1;
            mysql_stmt_fetch_column(stmt, &result, 0, start);
            start += result.buffer_length;
        }
    }
    mysql_stmt_close(stmt);
    return total_length;
}


int main(){
    MYSQL mysql;
    

    if(NULL == mysql_init(&mysql)) {
        printf("mysql_init : %s\n", mysql_error(&mysql));
        return -1; 
    }

    if(!mysql_real_connect(&mysql, DB_SERVER_IP, DB_SEVER_USERNAME, DB_SERVER_PAWD, DB_SERVER_TABLE, DB_SERVER_PORT, NULL, 0)){
        printf("mysql_real_connect : %s\n", mysql_error(&mysql));
        return -2;
    }
    // if(mysql_real_query(&mysql, SQL_DELETE_TBL_USER, strlen(SQL_DELETE_TBL_USER))){
    //     printf("mysql_real_query : %s\n", mysql_error(&mysql));
    //     return -3;
    // }
    // DB_mysql_select(&mysql);
#if 0
    if(mysql_real_query(&mysql, SQL_INSERT_TABLE_USER, strlen(SQL_INSERT_TABLE_USER))){
        printf("mysql_real_query : %s\n", mysql_error(&mysql));
    }
#endif
    printf("case-----> read immage and write mysql\n");
    char buffer[IMG_FILE_LENGTH] = {0};
    int length = read_image("微信截图_20251118230355.png", buffer);
    if(length <= 0){
        printf("read failed");
        return -1;
    }
    mysql_write(&mysql, buffer, length);

    printf("case-----> read mysql and write img\n");
    memset(buffer, 0, IMG_FILE_LENGTH);
    length = mysql_read(&mysql, buffer, IMG_FILE_LENGTH);
    write_image("a.png", buffer, length);


    mysql_close(&mysql);
    return 0;
}
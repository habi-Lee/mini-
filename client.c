/*===============================================
*   文件名称：tcp_serv.c
*   创 建 者：
*   创建日期：2024年04月15日
*   描    述：
================================================*/
#include <arpa/inet.h>
#include <dirent.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

void communicate_with_server(int client_socket);
int download(int sockfd, char *buf);
int upload(int sockfd, char *buf);

int main(int argc, char *argv[])
{

    // 创建套接字
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == fd)
    {
        perror("socket");
        return -1;
    }
    // 绑定套接字
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8888);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // 主动连接
    if (-1 == connect(fd, (struct sockaddr *)&addr, sizeof(addr)))
    {
        perror("connect");
        return -1;
    }

    communicate_with_server(fd);
    return 0;
}

void communicate_with_server(int client_socket)
{

    char buffer[1024];

    while (1)
    {
        // 用户输入消息

        scanf("%s", buffer);

        if (strcmp(buffer, "download") == 0)
        {
            send(client_socket, buffer, sizeof(buffer), 0);
            download(client_socket, buffer);
        }
        if (strcmp(buffer, "updata") == 0)
        {

            send(client_socket, buffer, sizeof(buffer), 0);
            upload(client_socket, buffer);
        }
        // 发送消息给服务器
        if (send(client_socket, buffer, sizeof(buffer), 0) < 0)
        {
            perror("send");
            break;
        }

        // 接收服务器响应

        int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);

        if (bytes_received < 0)
        {
            perror("recv");
            break;
        }
        else if (bytes_received == 0)
        {
            printf("Server disconnected.\n");
            break;
        }
        else
        {
            // 在接收到的数据后添加字符串结束符，以便使用 printf 正确打印
            buffer[bytes_received] = '\0';
            printf("%s\n", buffer);
            memset(buffer, 0, sizeof(buffer));
        }
    }
}

int download(int sockfd, char *buf)
{

    char filename[1024];
    printf("请你输入你要下载的文件名:\n");
    scanf("%s", buf);
    strcpy(filename, buf);

    // 发送文件名给服务器
    send(sockfd, buf, sizeof(buf), 0);

    // 接收服务器反馈的文件大小
    long size;
    recv(sockfd, &size, sizeof(size), 0);

    // 打开文件
    FILE *fp = fopen(filename, "wb");
    if (NULL == fp)
    {
        perror("fopen");
        return -1;
    }

    int sum = 0;
    char file_buffer[1024];

    while (sum < size)
    {
        int bytes_to_receive = (size - sum > sizeof(file_buffer)) ? sizeof(file_buffer) : size - sum;
        int ret = recv(sockfd, file_buffer, bytes_to_receive, 0);
        if (ret <= 0)
        {
            perror("recv during file download");
            fclose(fp);
            return -1;
        }

        sum += ret;
        fwrite(file_buffer, 1, ret, fp);
    }

    fclose(fp);
    return 0;
}
int upload(int sockfd, char *buf)
{
    char filename[1024];
    printf("请你输入你要上传的文件名\n");
    scanf("%s", buf);
    strcpy(filename, buf);

    // 发送文件名给服务器
    send(sockfd, buf, sizeof(buf), 0);

    // 发内容之前把文件大小发过去
    struct stat st;
    stat(filename, &st);
    send(sockfd, &st.st_size, sizeof(st.st_size), 0);

    FILE *fp = fopen(filename, "r");
    if (NULL == fp)
    {
        perror("fopen");
        return -1;
    }
    // 发送
    int ret;
    while ((ret = fread(buf, 1, sizeof(buf), fp)) > 0)
    {
        send(sockfd, buf, ret, 0);
    }
    fclose(fp);
    return 0;
}

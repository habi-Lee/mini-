/*===============================================
*   �ļ����ƣ�tcp_serv.c
*   �� �� �ߣ�
*   �������ڣ�2024��04��15��
*   ��    ����
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

    // �����׽���
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == fd)
    {
        perror("socket");
        return -1;
    }
    // ���׽���
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8888);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // ��������
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
        // �û�������Ϣ

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
        // ������Ϣ��������
        if (send(client_socket, buffer, sizeof(buffer), 0) < 0)
        {
            perror("send");
            break;
        }

        // ���շ�������Ӧ

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
            // �ڽ��յ������ݺ�����ַ������������Ա�ʹ�� printf ��ȷ��ӡ
            buffer[bytes_received] = '\0';
            printf("%s\n", buffer);
            memset(buffer, 0, sizeof(buffer));
        }
    }
}

int download(int sockfd, char *buf)
{

    char filename[1024];
    printf("����������Ҫ���ص��ļ���:\n");
    scanf("%s", buf);
    strcpy(filename, buf);

    // �����ļ�����������
    send(sockfd, buf, sizeof(buf), 0);

    // ���շ������������ļ���С
    long size;
    recv(sockfd, &size, sizeof(size), 0);

    // ���ļ�
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
    printf("����������Ҫ�ϴ����ļ���\n");
    scanf("%s", buf);
    strcpy(filename, buf);

    // �����ļ�����������
    send(sockfd, buf, sizeof(buf), 0);

    // ������֮ǰ���ļ���С����ȥ
    struct stat st;
    stat(filename, &st);
    send(sockfd, &st.st_size, sizeof(st.st_size), 0);

    FILE *fp = fopen(filename, "r");
    if (NULL == fp)
    {
        perror("fopen");
        return -1;
    }
    // ����
    int ret;
    while ((ret = fread(buf, 1, sizeof(buf), fp)) > 0)
    {
        send(sockfd, buf, ret, 0);
    }
    fclose(fp);
    return 0;
}

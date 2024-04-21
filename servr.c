#include <arpa/inet.h>
#include <dirent.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_USERS 100
#define MAX_USERNAME_LENGTH 20
#define MAX_PASSWORD_LENGTH 20
#define PORT 8888
#define MAX_PENDING_CONNECTIONS 5
#define BUFFER_SIZE 1024

void handle_client(int client_socket);
int loginUser(char username[], char password[]);
int registerUser(char username[], char password[]);
int upload(int sockfd, char *buf);
int download(int sockfd, char *buf);
void display_and_send_file_info(int client_sock, const char *dir_path);

int main()
{
    char buffer[BUFFER_SIZE];
    int bytes_received;
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    // 创建服务器套接字
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // 设置服务器地址和端口
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // 绑定服务器地址和端口
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    // 监听客户端连接请求
    if (listen(server_socket, MAX_PENDING_CONNECTIONS) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    while (1)
    {
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_socket < 0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        printf("Client connected: %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        while (1)
        {
            // 接收客户端消息

            bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);

            if (bytes_received < 0)
            {
                perror("recv");
                break;
            }
            else if (bytes_received == 0)
            {
                printf("Client disconnected.\n");
                break;
            }
            else
            {
                // 打印客户端消息
                printf("Received from client: %s\n", buffer);

                // 可以在这里对接收到的消息进行处理，例如回复客户端消息
                if (strcmp(buffer, "register") == 0)
                {
                    printf("enter register\n");
                    char user[MAX_USERNAME_LENGTH];
                    char passwd[MAX_PASSWORD_LENGTH];
                    memset(buffer, 0, sizeof(buffer));
                    strcpy(buffer, "请输入账号：");
                    send(client_socket, buffer, sizeof(buffer), 0);
                    recv(client_socket, buffer, sizeof(buffer), 0);
                    strcpy(user, buffer);
                    printf("%s", user);

                    memset(buffer, 0, sizeof(buffer));
                    strcpy(buffer, "请输入密码：");
                    send(client_socket, buffer, sizeof(buffer), 0);
                    recv(client_socket, buffer, sizeof(buffer), 0);
                    strcpy(passwd, buffer);
                    printf("%s", passwd);

                    registerUser(user, passwd);
                    strcpy(buffer, "注册成功！");
                    send(client_socket, buffer, sizeof(buffer), 0);
                }
                if (strcmp(buffer, "login") == 0)
                {
                    printf("enter register\n");
                    char user1[MAX_USERNAME_LENGTH];
                    char passwd1[MAX_PASSWORD_LENGTH];
                    memset(buffer, 0, sizeof(buffer));
                    strcpy(buffer, "请输入账号：");
                    send(client_socket, buffer, sizeof(buffer), 0);
                    recv(client_socket, buffer, sizeof(buffer), 0);
                    strcpy(user1, buffer);
                    printf("%s", user1);

                    memset(buffer, 0, sizeof(buffer));
                    strcpy(buffer, "请输入密码：");
                    send(client_socket, buffer, sizeof(buffer), 0);
                    recv(client_socket, buffer, sizeof(buffer), 0);
                    strcpy(passwd1, buffer);
                    printf("%s", passwd1);

                    int loginResult = loginUser(user1, passwd1);
                    if (loginResult == 1)
                    {
                        memset(buffer, 0, sizeof(buffer));
                        strcpy(buffer, "登陆成功");
                        send(client_socket, buffer, sizeof(buffer), 0);
                    }
                    else
                    {
                        memset(buffer, 0, sizeof(buffer));
                        strcpy(buffer, "登陆失败");
                        send(client_socket, buffer, sizeof(buffer), 0);
                    }
                }
                // 上传
                if (strcmp(buffer, "updata") == 0)
                {
                    printf("enter updata\n");
                    int flag;
                    flag = upload(client_socket, buffer);
                    if (flag == 0)
                    {
                        strcpy(buffer, "上传成功");
                        send(client_socket, buffer, sizeof(buffer), 0);
                    }
                    else
                    {
                        strcpy(buffer, "上传失败");
                        send(client_socket, buffer, sizeof(buffer), 0);
                    }
                }

                // 下载
                if (strcmp(buffer, "download") == 0)
                {
                    printf("enter download\n");
                    int flag;
                    flag = download(client_socket, buffer);

                    if (flag == 0)
                    {
                        strcpy(buffer, "下载成功");
                        send(client_socket, buffer, sizeof(buffer), 0);
                    }
                    else
                    {
                        strcpy(buffer, "下载失败");
                        send(client_socket, buffer, sizeof(buffer), 0);
                    }
                }

                // 查看目录
                if (strcmp(buffer, "show") == 0)
                {
                    printf("enter show\n");
                    char path[100];

                    memset(buffer, 0, sizeof(buffer));
                    strcpy(buffer, "请输入你要查看的路径：");
                    send(client_socket, buffer, sizeof(buffer), 0);
                    recv(client_socket, buffer, sizeof(buffer), 0);
                    strcpy(path, buffer);
                    memset(buffer, 0, sizeof(buffer));

                    display_and_send_file_info(client_socket, path);
                }
            }
        }
    }
    // 关闭客户端套接字
    close(client_socket);

    return 0;
}
struct User
{
    char username[MAX_USERNAME_LENGTH];
    char password[MAX_PASSWORD_LENGTH];
};

struct User users[MAX_USERS];
int numUsers = 0;

int registerUser(char username[], char password[])
{
    if (numUsers >= MAX_USERS)
    {
        return 0; // 注册用户已满
    }

    for (int i = 0; i < numUsers; i++)
    {
        if (strcmp(users[i].username, username) == 0)
        {
            return -1; // 用户名已存在
        }
    }

    strcpy(users[numUsers].username, username);
    strcpy(users[numUsers].password, password);
    numUsers++;

    return 1; // 注册成功
}

int loginUser(char username[], char password[])
{
    for (int i = 0; i < numUsers; i++)
    {
        if (strcmp(users[i].username, username) == 0 && strcmp(users[i].password, password) == 0)
        {
            return 1; // 登陆成功
        }
    }

    return 0; // 登陆失败
}

int upload(int sockfd, char *buf)
{
    char filename[1024];
    // 接收客户端要上传的文件名
    recv(sockfd, buf, sizeof(buf), 0);
    strcpy(filename, buf);

    // 接收客户端反馈的文件大小
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

int download(int client_sock, char *buf)
{
    char filename[1024];
    // 接收客户端要下载的文件名
    recv(client_sock, buf, sizeof(buf), 0);
    strcpy(filename, buf);
    // 服务器就要把这个文件打开读取内容发给客户端
    // 发内容之前把文件大小发过去
    struct stat st;
    stat(filename, &st);
    send(client_sock, &st.st_size, sizeof(st.st_size), 0);

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
        send(client_sock, buf, ret, 0);
    }
    fclose(fp);
    return 0;
}

void display_and_send_file_info(int client_sock, const char *dir_path)
{
    DIR *dir;
    struct dirent *ent;
    struct stat st;
    char buffer[1024];

    if ((dir = opendir(dir_path)) != NULL)
    {
        while ((ent = readdir(dir)) != NULL)
        {
            char full_path[PATH_MAX];
            snprintf(full_path, PATH_MAX, "%s/%s", dir_path, ent->d_name);

            if (stat(full_path, &st) == -1)
            {
                snprintf(buffer, BUFFER_SIZE, "Failed to get file stats: %s\n", ent->d_name);
                send(client_sock, buffer, strlen(buffer), 0);
                continue;
            }

            if (S_ISDIR(st.st_mode))
            {
                snprintf(buffer, BUFFER_SIZE, "%s is a directory.\n", ent->d_name);
            }
            else if (S_ISREG(st.st_mode))
            {
                snprintf(buffer, BUFFER_SIZE, "Name: %s, Size: %ld bytes, Mode: %o, Owner: %d, Group: %d\n",
                         ent->d_name, st.st_size, st.st_mode & 07777, (int)st.st_uid, (int)st.st_gid);
            }
            else
            {
                snprintf(buffer, BUFFER_SIZE, "%s is not a regular file or directory.\n", ent->d_name);
            }

            send(client_sock, buffer, strlen(buffer), 0);
        }
        closedir(dir);
    }
    else
    {
        perror("Failed to open directory");
        return;
    }
}

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

    // �����������׽���
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // ���÷�������ַ�Ͷ˿�
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // �󶨷�������ַ�Ͷ˿�
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    // �����ͻ�����������
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
            // ���տͻ�����Ϣ

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
                // ��ӡ�ͻ�����Ϣ
                printf("Received from client: %s\n", buffer);

                // ����������Խ��յ�����Ϣ���д�������ظ��ͻ�����Ϣ
                if (strcmp(buffer, "register") == 0)
                {
                    printf("enter register\n");
                    char user[MAX_USERNAME_LENGTH];
                    char passwd[MAX_PASSWORD_LENGTH];
                    memset(buffer, 0, sizeof(buffer));
                    strcpy(buffer, "�������˺ţ�");
                    send(client_socket, buffer, sizeof(buffer), 0);
                    recv(client_socket, buffer, sizeof(buffer), 0);
                    strcpy(user, buffer);
                    printf("%s", user);

                    memset(buffer, 0, sizeof(buffer));
                    strcpy(buffer, "���������룺");
                    send(client_socket, buffer, sizeof(buffer), 0);
                    recv(client_socket, buffer, sizeof(buffer), 0);
                    strcpy(passwd, buffer);
                    printf("%s", passwd);

                    registerUser(user, passwd);
                    strcpy(buffer, "ע��ɹ���");
                    send(client_socket, buffer, sizeof(buffer), 0);
                }
                if (strcmp(buffer, "login") == 0)
                {
                    printf("enter register\n");
                    char user1[MAX_USERNAME_LENGTH];
                    char passwd1[MAX_PASSWORD_LENGTH];
                    memset(buffer, 0, sizeof(buffer));
                    strcpy(buffer, "�������˺ţ�");
                    send(client_socket, buffer, sizeof(buffer), 0);
                    recv(client_socket, buffer, sizeof(buffer), 0);
                    strcpy(user1, buffer);
                    printf("%s", user1);

                    memset(buffer, 0, sizeof(buffer));
                    strcpy(buffer, "���������룺");
                    send(client_socket, buffer, sizeof(buffer), 0);
                    recv(client_socket, buffer, sizeof(buffer), 0);
                    strcpy(passwd1, buffer);
                    printf("%s", passwd1);

                    int loginResult = loginUser(user1, passwd1);
                    if (loginResult == 1)
                    {
                        memset(buffer, 0, sizeof(buffer));
                        strcpy(buffer, "��½�ɹ�");
                        send(client_socket, buffer, sizeof(buffer), 0);
                    }
                    else
                    {
                        memset(buffer, 0, sizeof(buffer));
                        strcpy(buffer, "��½ʧ��");
                        send(client_socket, buffer, sizeof(buffer), 0);
                    }
                }
                // �ϴ�
                if (strcmp(buffer, "updata") == 0)
                {
                    printf("enter updata\n");
                    int flag;
                    flag = upload(client_socket, buffer);
                    if (flag == 0)
                    {
                        strcpy(buffer, "�ϴ��ɹ�");
                        send(client_socket, buffer, sizeof(buffer), 0);
                    }
                    else
                    {
                        strcpy(buffer, "�ϴ�ʧ��");
                        send(client_socket, buffer, sizeof(buffer), 0);
                    }
                }

                // ����
                if (strcmp(buffer, "download") == 0)
                {
                    printf("enter download\n");
                    int flag;
                    flag = download(client_socket, buffer);

                    if (flag == 0)
                    {
                        strcpy(buffer, "���سɹ�");
                        send(client_socket, buffer, sizeof(buffer), 0);
                    }
                    else
                    {
                        strcpy(buffer, "����ʧ��");
                        send(client_socket, buffer, sizeof(buffer), 0);
                    }
                }

                // �鿴Ŀ¼
                if (strcmp(buffer, "show") == 0)
                {
                    printf("enter show\n");
                    char path[100];

                    memset(buffer, 0, sizeof(buffer));
                    strcpy(buffer, "��������Ҫ�鿴��·����");
                    send(client_socket, buffer, sizeof(buffer), 0);
                    recv(client_socket, buffer, sizeof(buffer), 0);
                    strcpy(path, buffer);
                    memset(buffer, 0, sizeof(buffer));

                    display_and_send_file_info(client_socket, path);
                }
            }
        }
    }
    // �رտͻ����׽���
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
        return 0; // ע���û�����
    }

    for (int i = 0; i < numUsers; i++)
    {
        if (strcmp(users[i].username, username) == 0)
        {
            return -1; // �û����Ѵ���
        }
    }

    strcpy(users[numUsers].username, username);
    strcpy(users[numUsers].password, password);
    numUsers++;

    return 1; // ע��ɹ�
}

int loginUser(char username[], char password[])
{
    for (int i = 0; i < numUsers; i++)
    {
        if (strcmp(users[i].username, username) == 0 && strcmp(users[i].password, password) == 0)
        {
            return 1; // ��½�ɹ�
        }
    }

    return 0; // ��½ʧ��
}

int upload(int sockfd, char *buf)
{
    char filename[1024];
    // ���տͻ���Ҫ�ϴ����ļ���
    recv(sockfd, buf, sizeof(buf), 0);
    strcpy(filename, buf);

    // ���տͻ��˷������ļ���С
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

int download(int client_sock, char *buf)
{
    char filename[1024];
    // ���տͻ���Ҫ���ص��ļ���
    recv(client_sock, buf, sizeof(buf), 0);
    strcpy(filename, buf);
    // ��������Ҫ������ļ��򿪶�ȡ���ݷ����ͻ���
    // ������֮ǰ���ļ���С����ȥ
    struct stat st;
    stat(filename, &st);
    send(client_sock, &st.st_size, sizeof(st.st_size), 0);

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

#include "tcpserver.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/stat.h>	/* S_IXXX flags */
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <syslog.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>	/* open() and O_XXX flags */
#include <sstream>
#include <iostream>
#include <fstream>

const int client_buffer_size_max = 1024;

void TCPServer::sighandler(int signal)
{
    syslog(LOG_NOTICE, "HANDLER!!!!!\n");
    if(signal == SIGTERM)
    {
        syslog(LOG_NOTICE, "receive terminate signal\n");
        exit(0);
    }
    else if(signal == SIGHUP)
    {
        syslog(LOG_NOTICE, "receive hang up signal\n");
        exit(0);
    }
}

void TCPServer::exec()
{
    int pid;
    struct sigaction sigact;

    pid = fork();
    switch(pid)
    {
    case 0:
        setsid();
        chdir("/");
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);

        memset(&sigact, 0, sizeof(sigact));
        sigact.sa_handler = &sighandler;
        sigaction(SIGTERM, &sigact, 0);
        sigaction(SIGHUP, &sigact, 0);

        openlog("mytcpserv", LOG_PID, LOG_USER);
        mainloop();
        closelog();
        exit(0);

    case -1:
        printf("Error: Start Daemon failed (%s)\n", strerror(errno));
        break;

    default:
        printf("Server started. PID=%d\n", pid);
        break;
    }
}

void TCPServer::mainloop()
{
    int sockfd;
    int sock;
    int result;
    struct sockaddr_in sa;
    char buf[client_buffer_size_max];

    sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sockfd == -1)
    {
        syslog(LOG_NOTICE, "[SOCKET] No connection\n");
        exit(1);
    }

    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = htonl(INADDR_ANY);
    sa.sin_port = htons(8800);
    result = bind(sockfd, (struct sockaddr *)&sa, sizeof(sa));
    if(result == -1)
    {
        syslog(LOG_NOTICE, "[BIND] no connection\n");
        exit(2);
    }

    listen(sockfd, 1);
    while(true)
    {
        sock = accept(sockfd, NULL, NULL);
        if(sock == -1)
        {
            syslog(LOG_NOTICE, "[ACCEPT] ... wtf?!\n");
            exit(3);
        }

        while(true)
        {
            result = recv(sock, buf, client_buffer_size_max, 0);
            if (result == -1)
            {
                syslog(LOG_NOTICE, "recv failed: %d\n", result);  // ошибка получения данных
                close(sock);
                break;
            }
            else if (result == 0)
            {
                syslog(LOG_NOTICE, "connection closed...\n");  // соединение закрыто клиентом
                break;
            }
            writeToFile(buf);
            send(sock, buf, result, 0);
        }
        close(sock);
    }
    close(sockfd);
    syslog(LOG_NOTICE, "Connect closed\n");
}

void TCPServer::writeToFile(char buf[])
{
    char pathToLog[512];
    sprintf(pathToLog, "%s/MyTCPServLog", getenv("HOME"));

    std::ofstream f(pathToLog, std::ios::app);
    if(f.is_open())
    {
        f<< buf <<std::endl;
        f.close();
    }
    else
        syslog(LOG_NOTICE, "[LOG] Logfile doesn't open");
}

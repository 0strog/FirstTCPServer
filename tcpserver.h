#ifndef TCPSERVER_H
#define TCPSERVER_H

class TCPServer
{
protected:
    void mainloop();
    void writeToFile(char buf[]);
    static void sighandler(int signal);

public:
    void exec();
};

#endif // TCPSERVER_H

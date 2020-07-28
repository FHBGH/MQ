#include  <unistd.h>
#include  <sys/types.h>       /* basic system data types */
#include  <sys/socket.h>      /* basic socket definitions */
#include  <netinet/in.h>      /* sockaddr_in{} and other Internet defns */
#include  <arpa/inet.h>       /* inet(3) functions */
#include <netdb.h> /*gethostbyname function */
#include "requestque.h"
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#define MAXLINE 1024

void handle(int connfd);
void create(int socketfd ,string topic,bool ack );
int main(int argc, char **argv)
{
    char * servInetAddr = "9.135.10.161";
    int servPort = 6000;
    char buf[MAXLINE];
    int connfd;
    struct sockaddr_in servaddr;



    connfd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(servPort);
    inet_pton(AF_INET, servInetAddr, &servaddr.sin_addr);

    if (connect(connfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        perror("connect error");
        return -1;
    }
    printf("welcome to echoclient\n");
    handle(connfd);     /* do it all */
    /*while(1){
        string topic;
        cin>>topic;
        create(connfd,topic,true);
    }*/
    close(connfd);
    printf("exit\n");
    exit(0);
}

void create(int socketfd ,string topic,bool ack = false) {
    Head head;
    head.cmd=CREATE;
    head.ack=ack;
    head.topicL=topic.size();
    char buffer[1024];
    memcpy(buffer,&head,sizeof(Head));
    memcpy(buffer+sizeof(Head),topic.c_str(),head.topicL);
    int n = 0;
    if(ack == true) {
        write(socketfd,buffer,sizeof(Head)+head.topicL);
        n = read(socketfd,buffer,1024);
        if(n < 0) {
            cout<<"read fail"<<endl;
            return ;
        }
        Head* rsp =(Head*) buffer;
        if(rsp->cmd==RSP){
            if(rsp->ret == OK)
                cout<< "create topic succ : "<<topic<<endl;
            else if(rsp->ret == TOPICED)
                cout<<"create topic fail"<<endl;
            else
                cout<<"unhnwon ret"<<endl;
            return ;
        } 
        else
        {
            cout<<"unexcept cmd"<<endl;
            return ;
        }
        
    }
    else {
        write(socketfd,buffer,sizeof(Head)+head.topicL);
        cout<<"send create cmd succ"<<endl;
        return ;
    }
    return ;
}

void handle(int sockfd)
{
    char sendline[MAXLINE], recvline[MAXLINE];
    int n;
    Head head;
        head.cmd = PUSH;
        head.ack=1;
        head.topicL= 5;
        memcpy(sendline,&head,sizeof(head));
        memcpy(sendline+sizeof(head),"first",head.topicL);

        if (fgets(sendline+sizeof(head)+head.topicL, MAXLINE-sizeof(head)-head.topicL, stdin) == NULL) {
            cout<<"NULL"<<endl;
            //read eof
        }
    int i=10;
    for (;i>0;i--) {

        
        /*
        //也可以不用标准库的缓冲流,直接使用系统函数无缓存操作
        if (read(STDIN_FILENO, sendline, MAXLINE) == 0) {
            break;//read eof
        }
        */

        n = write(sockfd, sendline,sizeof(head)+head.topicL+10);
        std::cout<<"write" <<n<<" byte"<<endl;
        n = read(sockfd, recvline, MAXLINE);
        if (n == 0) {
            printf("echoclient: server terminated prematurely\n");
            break;
        }
        Head * re =  (Head*) recvline;
        if(re->cmd==RSP){
            if(re->ret==NO_TOPIC)
            std::cout<< "no topic"<<endl;
            else
            {
                std::cout<<"write ok" <<endl;               
            }
            
        }
        else{
            std::cout<<"unexcept"<<endl;
        }

        //write(STDOUT_FILENO, recvline, n);
        //如果用标准库的缓存流输出有时会出现问题
        //fputs(recvline, stdout);
    }
}


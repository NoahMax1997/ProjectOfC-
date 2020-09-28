#include "std_head.h"
pthread_t send_thread_id;
pthread_t receive_thread_id;
int sockfd;
struct sockaddr_in servaddr;
char sendline[MAX_LINE];
char recvline[MAX_LINE];
string nowTime()
{
    string ret="";
     // 基于当前系统的当前日期/时间
    time_t now = time(0);
   // 把 now 转换为字符串形式
    tm *ltm = localtime(&now);
    stringstream ss;
    ss<<ltm->tm_hour+8<<":"<<ltm->tm_min<<":"<<ltm->tm_sec;
    ss>>ret;
    return ret;
}

ssize_t readline(int fd, char *vptr, size_t maxlen)
{
	ssize_t	n, rc;
	char	c, *ptr;
 
	ptr = vptr;
	for (n = 1; n < maxlen; n++) {
		if ( (rc = read(fd, &c,1)) == 1) {
			*ptr++ = c;
			if (c == '\n')
				break;	/* newline is stored, like fgets() */
		} else if (rc == 0) {
			*ptr = 0;
			return(n - 1);	/* EOF, n - 1 bytes were read */
		} else
			return(-1);		/* error, errno set by read() */
	}
 
	*ptr = 0;	/* null terminate like fgets() */
	return(n);
}
void initSocket(){
    string str="127.0.0.1";
    cout<<"请输入服务器所在ip地址:";
    cin>>str;
    if(str=="1"){
        str="127.0.0.1";
    }
	/*(1) 创建套接字*/
	if((sockfd = socket(AF_INET , SOCK_STREAM , 0)) == -1)
	{
		perror("socket error");
		exit(1);
	}//if
 
	/*(2) 设置链接服务器地址结构*/
	bzero(&servaddr , sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORT);
	if(inet_pton(AF_INET , str.c_str() , &servaddr.sin_addr) < 0)
	{
		printf("inet_pton error for %s\n",str.c_str());
		exit(1);
	}//if
 
	/*(3) 发送链接服务器请求*/
	if( connect(sockfd , (struct sockaddr *)&servaddr , sizeof(servaddr)) < 0)
	{
		perror("connect error");
		exit(1);
	}//if
}
void* sendMessage(void *arg){
    cout<<"in sendMessage"<<endl;
    while(1){
        while(fgets(sendline , MAX_LINE , stdin) != NULL)	
        {  
            // cout<<"in client sendMessage :"<<sendline;
            string send_str=sendline;
            string tmp_str="["+nowTime()+"]:"+send_str;
            // cout<<"client send_str:"<<send_str<<endl;
            // cout<<"prepare send message:"<<send_str;
            write(sockfd , tmp_str.c_str() , tmp_str.length());
            memset(sendline,MAX_LINE,'\0');
        }
    }
    
}
void* receiveMessage(void *arg){
    cout<<"in receiveMessage"<<endl;
    while(1){
        while(readline(sockfd , recvline , MAX_LINE) != 0)
        {
            cout<<"server:";
            if(fputs(recvline , stdout) == EOF)
            {
                perror("fputs error");
                exit(1);
            }//if 
            memset(recvline,MAX_LINE,'\0');  
        }//if
    }
    
}
void startSendThread(){
    if(pthread_create(&send_thread_id, NULL,sendMessage, NULL)==-1){
        printf("startSendThread create error!\n");
    }
    string tmp_str="SendThread is over";
    char* p=(char*)tmp_str.c_str();
    // pthread_exit(p);
    return;
}
void startReceiveThread(){
    if(pthread_create(&receive_thread_id, NULL,receiveMessage, NULL)==-1){
        printf("startReceiveThread create error!\n");
    }
    string tmp_str="ReceiveThread is over";
    char* p=(char*)tmp_str.c_str();
    // pthread_exit(p);
    return;
}
void stopSocket(){
    /*(5) 关闭套接字*/
    void * p=NULL;
    pthread_join(send_thread_id,&p);  
    cout<<p<<endl;
    pthread_join(receive_thread_id, &p); 
    cout<<p<<endl;
	close(sockfd);
}
int main(int argc , char ** argv)
{
	initSocket();
    startSendThread();
    startReceiveThread();
    stopSocket();
    cout<<"end main"<<endl;
    return 0;
}

#include "std_head.h"

/*声明服务器地址和客户链接地址*/
struct sockaddr_in servaddr, cliaddr;
/*声明服务器监听套接字和客户端链接套接字*/
int listenfd, connfd;
pid_t childpid;
/*声明缓冲区*/
char buf[MAX_LINE];
socklen_t clilen;
pthread_t send_thread_id_list[MAX_CONNECT];
pthread_t receive_thread_id_list[MAX_CONNECT];
int pthread_num_list[MAX_CONNECT]={0};
int pool_full = 0;
map<string, int> send_pool;
map<int, string> reverse_send_pool;
map<int, char *> receive_buff_pool;
map<int, char *> send_buff_pool;
pthread_mutex_t mutex;
pthread_mutex_t file_mutex;
ofstream  ofile;
vector<int> connid_pool;
int connid_pool_index = 0;
string nowTime(){
    string ret = "";
    // 基于当前系统的当前日期/时间
    time_t now = time(0);
    // 把 now 转换为字符串形式
    tm *ltm = localtime(&now);
    stringstream ss;
    ss << ltm->tm_hour + 8 << ":" << ltm->tm_min << ":" << ltm->tm_sec;
    ss >> ret;
    return ret;
}
char *hexToCharIP(struct in_addr addrIP){
    char *ip;
    unsigned int intIP;
    memcpy(&intIP, &addrIP, sizeof(unsigned int));
    int a = (intIP >> 24) & 0xFF;
    int b = (intIP >> 16) & 0xFF;
    int c = (intIP >> 8) & 0xFF;
    int d = intIP & 0xFF;
    if ((ip = (char *)malloc(16 * sizeof(char))) == NULL)
    {
        return NULL;
    }
    sprintf(ip, "%d.%d.%d.%d", d, c, b, a);
    return ip;
}

void initSocket(){
    /*(1) 初始化监听套接字listenfd*/
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket error");
        exit(1);
    } //if
    ofile.open("./message_log",ios::app);
    if(!ofile.is_open()){
		cout<<"message_log open fail"<<endl;
	}
    /*(2) 设置服务器sockaddr_in结构*/
    bzero(&servaddr, sizeof(servaddr));
    pthread_mutex_init(&mutex,NULL);
    pthread_mutex_init(&file_mutex,NULL);
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); //表明可接受任意IP地址
    servaddr.sin_port = htons(PORT);
    /*(3) 绑定套接字和端口*/
    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("bind error");
        exit(1);
    } //if

    /*(4) 监听客户请求*/
    if (listen(listenfd, LISTENQ) < 0)
    {
        perror("listen error");
        exit(1);
    } //if
    for (int i = 0; i < MAX_CONNECT; i++)
    {
        connid_pool.push_back(0);
    }
    /*(5) 接受客户请求*/
}
void* receiveMessage(void* args){
    int * tmp_index=(int*) args;
    // cout << "new receiveMessage thread tmp_index="<< *tmp_index<< endl;
    // close(listenfd);
    //str_echo
    ssize_t n;
    while ((n = read(connid_pool[*tmp_index], receive_buff_pool[connid_pool[*tmp_index]], MAX_LINE)) > 0)
    {
        // cout<<"receive message"<<endl;
        // write(connid_pool[connid_pool_index-1] , recvline , n);
        ofile<<receive_buff_pool[connid_pool[*tmp_index]];
        cout << "client["<<reverse_send_pool[connid_pool[*tmp_index]]<<"]:";
        if (fputs(receive_buff_pool[connid_pool[*tmp_index]], stdout) == EOF)
        {
            perror("fputs error");
            exit(1);
        } //if
        memset(receive_buff_pool[connid_pool[*tmp_index]], MAX_LINE, '\0');
    } 
}
void* sendMessage(void* args)
{
    int * tmp_index=(int*) args;
    // cout << "new sendMessage thread tmp_index=" <<*tmp_index<< endl;
    // close(listenfd);
    int tmp_connid = connid_pool[*tmp_index];
    //str_echo
    pthread_mutex_lock(&mutex);
    while (fgets(send_buff_pool[tmp_connid], MAX_LINE, stdin) != NULL)
    {
        pthread_mutex_unlock(&mutex);
        string send_str=send_buff_pool[tmp_connid];
        int position = -1;
        if ((position = send_str.find('@')) != string::npos)
        {
            // cout<<"send_str:"<<send_str<<"position="<<position<<endl;
            string send_data = send_str.substr(0, position);
            send_data+='\n';
            // cout << "send_data:" << send_data << endl;
            string send_client_data = send_str.substr(position + 1, send_str.length() - position-2);
            // cout << "send_client_data:" << send_client_data <<"length="<<send_client_data.length()<< endl;
            // for(map<string,int>::iterator it=send_pool.begin();it!=send_pool.end();it++){
            //     cout<<"ip_data:"<<it->first<<"length="<<it->first.length()<<endl;
            //     if(send_client_data==it->first) cout<<"Yes"<<endl;
            // }
            if (send_pool.find(send_client_data) != send_pool.end())
            {
                ofile<<"to one:["+nowTime() + "]:" <<send_data;
                int t=send_pool[send_client_data];
                tmp_connid=t;
                // cout << "tmp_connid:" << tmp_connid << endl;
                string tmp_str="["+nowTime() + "]:" + send_data;
                write(tmp_connid, tmp_str.c_str(), tmp_str.length());
                // cout<<"sfdf"<<endl;
            }
            else
            {
                cout << "not fount ip and port" << endl;
            }
        }
        else
        {
            // cout<<send_pool.size()<<endl;
            cout << "*message had send to all client*"<<endl;
            ofile<<"to all:["+nowTime() + "]:" + send_str<<endl;
            for(map<string,int>::iterator it=send_pool.begin();it!=send_pool.end();it++){
                // cout << "send to all client:"<<it->first<<endl;
                tmp_connid=it->second;
                // cout << "tmp_connid:" << tmp_connid << endl;
                string tmp_str="["+nowTime() + "]:" + send_str;
                // cout<<"client send_str:"<<send_str<<endl;
                // cout<<"prepare send message:"<<send_str;
                write(tmp_connid, tmp_str.c_str(), tmp_str.length());  
            }
            send_str = "";
        }
        memset(send_buff_pool[tmp_connid],MAX_LINE,'\0');
    }
}
void acceptListen(void *args){
    while (1)
    {
        if (send_pool.size() > MAX_CONNECT)
        {
            pool_full = 1;
            continue;
        }
        clilen = sizeof(cliaddr);
        int is_change = 0;

        if ((connid_pool[connid_pool_index] = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen)) < 0)
        {
            perror("accept error");
            exit(1);
        } //if
        connid_pool_index++;
        string str_ip = hexToCharIP(cliaddr.sin_addr);
        stringstream ss;
        ss << cliaddr.sin_port;
        string str_port;
        ss >> str_port;
        string temp = str_ip + ":" + str_port;
        if (send_pool.find(temp) == send_pool.end())
        {  
            //  cout<<"temp:"<<temp<<endl;
            send_pool[temp] = connid_pool[connid_pool_index - 1];
            reverse_send_pool[connid_pool[connid_pool_index - 1]]=temp;
            receive_buff_pool[connid_pool[connid_pool_index - 1]] = new char[MAX_LINE];
            pthread_num_list[connid_pool_index - 1]=connid_pool_index - 1;
            send_buff_pool[connid_pool[connid_pool_index - 1]] = new char[MAX_LINE];
            is_change = 1;
            // cout<<"size:"<<send_pool.size()<<endl;
        }
        if (is_change == 1)
        {
            is_change = 0;
            // cout<<"connid_pool_index:"<<connid_pool_index<<endl;
            // string tmp_ip_data=hexToCharIP(cliaddr.sin_addr)+":"+cliaddr.sin_port;
            if(pthread_create(&send_thread_id_list[connid_pool_index],NULL,sendMessage,&pthread_num_list[connid_pool_index - 1])==-1){
                printf("create send_thread  error[connid_pool_index=%d]\n",connid_pool_index);
            }
            if(pthread_create(&receive_thread_id_list[connid_pool_index],NULL,receiveMessage,&pthread_num_list[connid_pool_index - 1])==-1){
                printf("create receive_thread  error[connid_pool_index=%d]\n",connid_pool_index);
            }
        }
    }
}
int main(int argc, char **argv){
    initSocket();
    // sendMessage();
    acceptListen(NULL);
    // receiveMessage(NULL);
    /*(6) 关闭监听套接字*/
    close(listenfd);
    ofile.close();
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <pthread.h>

void *do_chat(void *); //채팅 메세지를 보내는 함수
int pushClient(int, char *); //새로운 클라이언트가 접속했을 때 클라이언트 정보 추가
int popClient(int); //클라이언트가 종료했을 때 클라이언트 정보 삭제

pthread_t thread;
pthread_mutex_t mutex;

#define MAX_CLIENT 10
#define CHATDATA 1024
#define INVALID_SOCK -1
#define PORT 9000

struct chatClient{
	int c_socket;
	char nickname[CHATDATA];
};
struct chatClient clientList[MAX_CLIENT];
char	escape[ ] = "exit";
char	greeting[ ] = "Welcome to chatting room\n";
char	CODE200[ ] = "Sorry No More Connection\n";

int main(int argc, char *argv[ ])
{

	int c_socket, s_socket;
	struct sockaddr_in s_addr, c_addr;
	int	len;
	int	i, j, n;
	int	res;
	char nickname[CHATDATA];
	if(pthread_mutex_init(&mutex, NULL) != 0) {
		printf("Can not create mutex\n");
		return -1;
	}
	s_socket = socket(PF_INET, SOCK_STREAM, 0);
	memset(&s_addr, 0, sizeof(s_addr));
	s_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	s_addr.sin_family = AF_INET;
	s_addr.sin_port = htons(PORT);
	if(bind(s_socket, (struct sockaddr *)&s_addr, sizeof(s_addr)) == -1) {
		printf("Can not Bind\n");
		return -1;
	}

	if(listen(s_socket, MAX_CLIENT) == -1) {
		printf("listen Fail\n");
		return -1;
	}
	for(i = 0; i < MAX_CLIENT; i++)
		clientList[i].c_socket = INVALID_SOCK;
	while(1) {
		len = sizeof(c_addr);
		c_socket = accept(s_socket, (struct sockaddr *) &c_addr, &len);
		memset(nickname, 0 , sizeof(nickname));
		if((n = read(c_socket, nickname, sizeof(nickname))) < 0){
			printf("nickname read fail.\n");
			return -1;
		}	
		printf("%s chat in\n", nickname);
		res = pushClient(c_socket, nickname);
		if(res < 0) {
			write(c_socket, CODE200, strlen(CODE200));
			close(c_socket);
		} else {
			write(c_socket, greeting, strlen(greeting));
			pthread_create(&thread, NULL, do_chat, (void *) &c_socket);
		}
	}
}

void *do_chat(void *arg)
{
	int c_socket = *((int *)arg);
	char chatData[CHATDATA];
        char S_Data[CHATDATA];
	int i, n;
        char *token = NULL;
	char *myname = NULL;
	char *tonickname = NULL;
	char *message = NULL;
	while(1) {
		memset(chatData, 0, sizeof(chatData));
		if((n = read(c_socket, chatData, sizeof(chatData))) > 0) {
			printf("receive : %s\n", chatData);
			if(strncasecmp(chatData, "/w", 2) == 0){	
				token = strtok(chatData, " "); 
				tonickname = strtok(NULL, " ");
                                myname = strtok(NULL, " ");
				message = strtok(NULL, "\0");
				for(i = 0; i < MAX_CLIENT; i++) {
					if(strncasecmp(clientList[i].nickname, tonickname, strlen(tonickname)) == 0)	{		
                                                sprintf(S_Data,"[귓속말(%s -> [%s])] : %s \n",myname, tonickname, message); 
						write(clientList[i].c_socket, S_Data, strlen(S_Data));	
                                      }
				}
				if(i == MAX_CLIENT){
						sprintf(chatData,"no such user\n");
						write(c_socket,chatData,strlen(chatData));
				}
			}else{
				for(i = 0; i < MAX_CLIENT; i++) 
					write(clientList[i].c_socket, chatData, n);
			}
			if(strstr(chatData, escape) != NULL) {			        
				popClient(c_socket);
				break;
			}
		}
	}
}
int pushClient(int c_socket, char *nickname) {
	int i;
	for(i = 0; i < MAX_CLIENT; i++) {
		pthread_mutex_lock(&mutex);
		if(clientList[i].c_socket == INVALID_SOCK) {
			clientList[i].c_socket = c_socket;
			memset(clientList[i].nickname, 0, CHATDATA);
			strcpy(clientList[i].nickname, nickname);
			pthread_mutex_unlock(&mutex);
			return i;
		}
		pthread_mutex_unlock(&mutex);
	}
	if(i == MAX_CLIENT)
		return -1;
}		

int popClient(int c_socket)
{			
	int i;	 
	close(c_socket); 
	pthread_mutex_lock(&mutex);
	for(i = 0; i < MAX_CLIENT; i++) {
		if(c_socket == clientList[i].c_socket) {
			clientList[i].c_socket = INVALID_SOCK;
			printf("%s chat out\n", clientList[i].nickname);
			break;
		}
	}
	pthread_mutex_unlock(&mutex);
	return 0;
}

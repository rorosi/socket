#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>

#define PORT 10000

char buffer[100] = "안녕하세요, 만나서 반가워요!\n";
char nbuffer[100] = "내 이름은 서버야!\n";
char abuffer[100] = "난 1살이야!\n";
char rcvBuffer[100];
char *rcvBuffer2[100];
char *rcvBuffer3[100];

int main(){
	int c_socket, s_socket;
	struct sockaddr_in s_addr, c_addr;
	int len;
	int n;
	// 1. 서버 소켓 생성
	//서버 소켓 = 클라이언트의 접속 요청을 처리(허용)해 주기 위한 소켓
	s_socket = socket(PF_INET, SOCK_STREAM, 0); //TCP/IP 통신을 위한 서버 소켓 생성
	
	//2. 서버 소켓 주소 설정
	memset(&s_addr, 0, sizeof(s_addr)); //s_addr의 값을 모두 0으로  초기화
	s_addr.sin_addr.s_addr = htonl(INADDR_ANY); //IP 주소 설정
	s_addr.sin_family = AF_INET;
	s_addr.sin_port = htons(PORT);

	//3. 서버 소켓바인딩
	if(bind(s_socket,(struct sockaddr *) &s_addr, sizeof(s_addr)) == -1){ 
		//바인딩 작업 실패 시, Cannot Bind 메시지 출력 후 프로그램 종료
		printf("Cannot Bind\n");
		return -1;
	}
	
	//4.listen() 함수 실행
	if(listen(s_socket, 5) == -1){
		printf("listen Fail\n");
		return -1;
	}

	//5. 클라이언트 요청 처리
	// 요청을 허용한 후, Hello World 메세지를 전송함
	while(1){ //무한 루프
		int count = 0;   
		int i = 0;
		int result;
		len = sizeof(c_addr);
		printf("클라이언트 접속을 기다리는 중....\n");
		c_socket = accept(s_socket, (struct sockaddr *)&c_addr, &len); 
		//클라이언트의 요청이 오면 허용(accept)해 주고, 해당 클라이언트와 통신할 수 있도록 클라이언트 소켓(c_socket)을 반환함.
		printf("/client is connected\n");
		printf("클라이언트 접속 허용\n");
		while(1){
			n = read(c_socket, rcvBuffer, sizeof(rcvBuffer));
			printf("클라이언트가 보낸 메세지: %s\n", rcvBuffer);
			if(strncasecmp(rcvBuffer, "quit", 4) == 0 || strncasecmp(rcvBuffer, "kill server", 11) == 0){
				break;
				write(c_socket, rcvBuffer, n); //클라이언트에게 buffer의 내용을 전송함
			}else if(strcmp(rcvBuffer, "안녕하세요\n") == 0){
				write(c_socket, buffer, strlen(buffer));
				for(i=0; i<sizeof(rcvBuffer); i++)
					rcvBuffer[i] = '\0';
				continue;												//다시 반복문으로 돌아가기 위해 선언
			}else if(strcmp(rcvBuffer, "이름이 뭐야?\n") == 0){
				write(c_socket, nbuffer, strlen(nbuffer));
				for(i=0; i<sizeof(rcvBuffer); i++)
					rcvBuffer[i] = '\0';
				continue;
			}else if(strcmp(rcvBuffer, "몇살이니?\n") == 0){
				write(c_socket, abuffer, strlen(abuffer));
				for(i=0; i<sizeof(rcvBuffer); i++)
					rcvBuffer[i] = '\0';
				continue;
			}else if(strncasecmp(rcvBuffer,"strlen ",6) == 0){
				rcvBuffer[strlen(rcvBuffer) - 1] = '\0';	
				char *ptr = strtok(rcvBuffer," ");   //공백으로 구분
				i=0;
				count=0;
				while(ptr != NULL){					  //문자열을 각각 배열에 저장
				rcvBuffer2[i] = ptr;
				i++;
				ptr = strtok(NULL," ");
				}
				count = strlen(rcvBuffer2[1]);
				printf("문자열의 길이 : %d\n",count);
				for(i=0; i<sizeof(rcvBuffer); i++)
					rcvBuffer[i] = '\0';
			}else if(strncasecmp(rcvBuffer,"strcmp ",6) == 0 ){	
				rcvBuffer[strlen(rcvBuffer) - 1] = '\0';			
				char *ptr = strtok(rcvBuffer," ");   //공백으로 구분
				i=0;
				while(ptr != NULL){					  //문자열을 각각 배열에 저장
				rcvBuffer3[i] = ptr;
				i++;
				ptr = strtok(NULL," ");
				}
				result = strcmp(rcvBuffer3[1],rcvBuffer3[2]);
				printf("두 문자열의 strcmp 결과 값: %d\n",result);
				for(i=0; i<sizeof(rcvBuffer); i++)
					rcvBuffer[i] = '\0';
			}
			write(c_socket,rcvBuffer,n);
			for(i=0; i<sizeof(rcvBuffer); i++)
					rcvBuffer[i] = '\0';
		}
		close(c_socket);
		if (strncasecmp(rcvBuffer, "kill server", 11) == 0)
			break;
	}
	close(s_socket);
	return 0;	
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define DEFAULT_PROTOCOL 0
#define BUF_SIZE 100

int readLine(int, char*);

int main(int argc, char *argv[]) {
	int sfd, cfd, port, clientLen;
	FILE *fp;
	char msg_recv[BUF_SIZE], msg_send[BUF_SIZE];
	struct sockaddr_in serveraddr, clientaddr;
	struct hostent *hp;
	char *haddrp;

	signal(SIGCHLD, SIG_IGN);	/* 자식프로세스가 종료되면 부모프로세스에 보내짐, 시그널 받으면 무시 */

	if(argc != 2) {
		fprintf(stderr, "Usage: %s <port> \n", argv[0]);
		exit(0);
	}

	port = atoi(argv[1]);	/* 포트 번호 받기 */
	sfd = socket(AF_INET, SOCK_STREAM, DEFAULT_PROTOCOL);	/* 인터넷 소켓 만들기 */

	bzero((char *)&serveraddr, sizeof(serveraddr));		/* 배열. 구조체 변수의 메모리 영역을 0으로 채워서 초기화  */
	serveraddr.sin_family = AF_INET;	/* 인터넷 주소 체계 설정 */
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);		/* 문제 없음 */
	serveraddr.sin_port = htons((unsigned short)port);	/* 포트 번호 설정 */

	bind(sfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));	/* 소켓에 이름(주소) 주기 */
	listen(sfd, 5);	/* 소켓 큐 생성 */

	while(1) {
		clientLen = sizeof(clientaddr);		/* clientaddr 크기를 clientLen에 넣어줌 */
		cfd = accept(sfd, (struct sockaddr *)&clientaddr, &clientLen);		/* 클라이언트 접속 요청 함수 */
		haddrp = inet_ntoa(clientaddr.sin_addr);	/* 인터넷 주소 반환 */
		printf("Server: %s(%d) connected.\n", haddrp, clientaddr.sin_port);

		if(fork() == 0) {
			readLine(cfd, msg_recv);
			fp = fopen(msg_recv, "r");
			if(fp == NULL) {
				write(cfd, "File is not here.", 20);
			} else {
				while(fgets(msg_send, BUF_SIZE, fp) != NULL) {
					write(cfd, msg_send, strlen(msg_send) + 1);
				}
			}
			close(cfd);
			exit(0);
		} else {
		        close(cfd);
		}
	}
}

int readLine(int fd, char *str) {
	int n;
	do {
		n = read(fd, str, 1);
	} while( n>0 && *str++ != '\0' );

	return ( n>0 );
}

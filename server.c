#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <errno.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <netdb.h> 
#include <arpa/inet.h>
#include <errno.h> 
#include <time.h> 
#include <sys/wait.h> 
#include <pthread.h> 
#include <signal.h>
#include <sys/poll.h>
#include "llist.h"
   
#define BACKLOG 10   
#define MAXDATASIZE 100
#define nlen 21

struct msg
{
	int type;
 	char buf[100];
	int size;
	int s;
};
char *PORT[] = {"1400", "1401", "1402"};
int *games[100];
int port, k;

int check_gr(int *play_gr)
{
	int res = 0, i, j;
	
	if ((*(play_gr) == *(play_gr + 3)) && (*(play_gr) == *(play_gr+6)) && (*(play_gr) != 0)) {
		res = 1;
	}
	if ((*(play_gr + 1) == *(play_gr+4)) && (*(play_gr + 1) == *(play_gr+7)) && (*(play_gr+1) != 0)) {
		res = 1;
	}
	if ((*(play_gr + 2) == *(play_gr+5)) && (*(play_gr+2) == *(play_gr + 8)) && (*(play_gr+2) != 0)) {
		res = 1;
	}
	
	if ((*(play_gr) == *(play_gr+1)) && (*(play_gr) == *(play_gr+2)) && (*(play_gr) != 0)) {
		res = 1;
	}
	if ((*(play_gr+3) == *(play_gr+4)) && (*(play_gr+3) == *(play_gr+5)) && (*(play_gr+3) != 0)) {
		res = 1;
	}
	if ((*(play_gr+6) == *(play_gr+7)) && (*(play_gr+6) == *(play_gr+8)) && (*(play_gr+6) != 0)) {
		res = 1;
	}
	
	if ((*(play_gr) == *(play_gr+4)) && (*(play_gr) == *(play_gr+8)) && (*(play_gr) != 0)) {
		res = 1;
	}
	if ((*(play_gr+6) == *(play_gr+4)) && (*(play_gr+6) == *(play_gr+2)) && (*(play_gr+6)!= 0)) {
		res = 1;
	}
	int fl = 0;
	if (res != 1){
		for (i = 0; i < 3; i++)
		{
			for (j = 0; j < 3; j++)
			{
				if (*(play_gr+i*3 + j) == 0)
				{
					fl = 1;
					break;
				}
				
			}
		}
	} else{
		fl = 1;
	}
	if (fl == 0)
	{
		res = 2;
	}
	
	return res;
}

void sigchld_handler(int s) 
{ 
	while(waitpid(-1, NULL, WNOHANG) > 0); 
}

void *get_in_addr(struct sockaddr *sa) 
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr); 
}

int servers[3];

void connect_to_server()
{
	struct msg *message = (struct msg*)malloc(sizeof(struct msg));
    struct addrinfo hints, *servinfo, *p; 
    int rv; 
    char s[INET6_ADDRSTRLEN];
    k = 0;

    memset(&hints, 0, sizeof hints); 
    hints.ai_family = AF_UNSPEC; 
    hints.ai_socktype = SOCK_STREAM; 
    int i;
    for (i = 0; i < 3; i++){
		if (i != port){
			if ((rv = getaddrinfo("127.0.0.1", PORT[i], &hints, &servinfo)) != 0) { 
				fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv)); 
				fflush(stderr);
				exit(1); 
			}
			for(p = servinfo; p != NULL; p = p->ai_next) { 
				if ((servers[i] = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) { 
					perror("server: socket"); 
					fflush(stderr);
					continue; 
				}
				if (connect(servers[i], p->ai_addr, p->ai_addrlen) == -1) { 
					close(servers[i]); 
					servers[i] = 0;
					perror("server: connect"); 
					fflush(stderr);
					continue; 
				} else{
					inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s); 
					freeaddrinfo(servinfo);
					k++;
					message->type = 15; 	
					memset(message->buf, '\0', strlen(message->buf)); 
					message->s = 0;
					message->size = 0;
					send(servers[i], message, sizeof(struct msg), 0);
				}
				break;
			}
		}
	}
}

void game(int op1, int op2, int game_number, int new)
{
	int play_gr[3][3];
	int i, j, numbytes;
	char str_c[9], msg_s_c[9] = {};
	struct msg *message = (struct msg*)malloc(sizeof(struct msg));
	if (new == 1){
		fprintf(stderr, "game %d start\n", game_number);
		fflush(stderr);
		if (send(op2, &game_number, sizeof(int), 0) == -1) {
			perror("send");
			fflush(stderr);
		}
							
		if (send(op1, &game_number, sizeof(int), 0) == -1) {
			perror("send");
			fflush(stderr);
		}
	} else{
		fprintf(stderr, "game %d restart\n", game_number);
		fflush(stderr);
	}
	for (i = 0; i < 3; i++) {	
		for (j = 0; j < 3; j++) {
			if (games[game_number] != NULL){
				play_gr[j][i] = games[game_number][(i * 3) + j];
				sprintf(str_c, "%d", play_gr[j][i]);
				strcat(msg_s_c, str_c);
			}
			else
				play_gr[i][j] = 0;
		}
	}
	if (games[game_number] != NULL){
		strcpy(message->buf, msg_s_c);
		message->type = 0;	
		message->s = 0;
		message->size = 0;
		send(op1, message, sizeof(struct msg), 0);
		send(op2, message, sizeof(struct msg), 0);
	}
	signal(SIGPIPE, SIG_IGN);
	time_t t1 = time(NULL);
	time_t t2 = time(NULL);
	while(1){
		message->type = 0;
		if (time(NULL) - t1 > 10){
			message->type = 11;
			memset(message->buf, '\0', strlen(message->buf)); 
			message->s = 0;
			message->size = 0;
			send(op2, message, sizeof(struct msg), 0);
			fprintf(stderr, "op2 was disconnected\n");
			fflush(stderr);
			exit(0);
		}
		if (time(NULL) - t2 > 10){
			message->type = 11;
			memset(message->buf, '\0', strlen(message->buf)); 
			message->s = 0;
			message->size = 0;
			send(op1, message, sizeof(struct msg), 0);
			fprintf(stderr, "op2 was disconnected\n");
			fflush(stderr);
			exit(0);
		}
		if ((numbytes = recv(op1, message, sizeof(struct msg), MSG_DONTWAIT)) != -1) { 
			if (message->type == 4){
				int i, j;
				int chk;
				i = message->s / 10;
				j = message->s % 10;
				if (message->size == 0){
					if (i < 3 && i >= 0 && j < 3 && j >= 0){								
						play_gr[i][j] = 1;
						if (send(op2, message, sizeof(struct msg), 0) == -1) {
							perror("send");
							fflush(stderr);
						}					
						chk = check_gr(&play_gr[0][0]);
						char str[9], msg_s[9] = {};
						message->s = game_number;
						for(j = 0; j < 3; j++)
							for(i = 0; i < 3; i++){
								sprintf(str, "%d", play_gr[i][j]);
								strcat(msg_s, str);
							}
						strcpy(message->buf, msg_s);

						for(i = 0; i < 3; i++){
							if ((i != port) && (servers[i] != 0)){
								if (send(servers[i], message, sizeof(struct msg), 0) == -1){
									perror("send");
									fflush(stderr);
									close(servers[i]);
									servers[i] = 0;						
								}
							}
						}
						
						if (chk == 1) {
							message->s = 1;
							memset(message->buf, '\0', strlen(message->buf)); 
							message->size = 0;
							if (send(op1, message, sizeof(struct msg), 0) == -1) {
								perror("send");
								fflush(stderr);
							}
							message->s = 0;
							if (send(op2, message, sizeof(struct msg), 0) == -1) {
								perror("send");
								fflush(stderr);
							}
							close(op1);
							close(op2);
							break;
						} else {
							if (chk == 2) {
								memset(message->buf, '\0', strlen(message->buf)); 
								message->size = 0;
								message->s = 2;
								if (send(op1, message, sizeof(struct msg), 0) == -1) {
									perror("send");
									fflush(stderr);
								}
								if (send(op2, message, sizeof(struct msg), 0) == -1) {
									perror("send");
									fflush(stderr);
								}
								close(op1);
								close(op2);
								break;
							} else {
								memset(message->buf, '\0', strlen(message->buf)); 
								message->size = 0;
								message->s = 3;
								if (send(op1, message, sizeof(struct msg), 0) == -1) {
									perror("send");
									fflush(stderr);
								}
								if (send(op2, message, sizeof(struct msg), 0) == -1) {
									perror("send");
									fflush(stderr);
								}
							}
						}
					}
				} else{
					fprintf(stderr, "incorrect message\n", i + 1);
					fflush(stderr);
				}	
			}
			if (message->type == 1){
				if (message->size == 0 && message->s == 0)
					send(op2, message, sizeof(struct msg), 0);
				else{
					fprintf(stderr, "incorrect message\n");
					fflush(stderr);
				}	
			}
			if (message->type == 8){
				if (message->size == 0 && message->s == 0)
					t1 = time(NULL);
				else{
					fprintf(stderr, "incorrect message\n");
					fflush(stderr);
				}
			}
		} 
		message->type = 0;
		if ((numbytes = recv(op2, message, sizeof(struct msg), MSG_DONTWAIT)) != -1){
			if (message->type == 4){
				int i, j;
				int chk;
				i = message->s / 10;
				j = message->s % 10;
				if (message->size == 0){
					if (i < 3 && i >= 0 && j < 3 && j >= 0){
						play_gr[i][j] = 2;
						if (send(op1, message, sizeof(struct msg), 0) == -1) {
							perror("send");
							fflush(stderr);
						}
																
						chk = check_gr(&play_gr[0][0]);
						char str[9], msg_s[9] = {};
						message->s = game_number;
						for(j = 0; j < 3; j++)
							for(i = 0; i < 3; i++){
								sprintf(str, "%d", play_gr[i][j]);
								strcat(msg_s, str);
							}
						strcpy(message->buf, msg_s);
						for(i = 0; i < 3; i++){
							if ((i != port) && (servers[i] != 0))
								if (send(servers[i], message, sizeof(struct msg), 0) == -1){
									perror("send");
									fprintf(stderr, "server %d crush\n", i + 1);
									fflush(stderr);
									close(servers[i]);
									servers[i] = 0;						
								}
						}
						if (chk == 1) {
							message->s = 1;
							if (send(op2, message, sizeof(struct msg), 0) == -1) {
								perror("send");
								fflush(stderr);
							}
							message->s = 0;
							if (send(op1, message, sizeof(struct msg), 0) == -1) {
								perror("send");
								fflush(stderr);
							}
							close(op1);
							close(op2);
							fprintf(stderr, "game %d end\n", game_number);
							fflush(stderr);
							break;
						} else {
							if (chk == 2){
								message->s = 2;
								if (send(op1, message, sizeof(struct msg), 0) == -1) {
									perror("send");
									fflush(stderr);
								}
								if (send(op2, message, sizeof(struct msg), 0) == -1) {
									perror("send");
									fflush(stderr);
								}
								close(op1);
								close(op2);
								fprintf(stderr, "game %d end\n", game_number);
								fflush(stderr);
								break;
							} else {
								message->s = 3;
								if (send(op1, message, sizeof(struct msg), 0) == -1) {
									perror("send");
									fflush(stderr);
								}
								if (send(op2, message, sizeof(struct msg), 0) == -1) {
									perror("send");
									fflush(stderr);
								}
							}
						}
					}	
				} else{
					fprintf(stderr, "incorrect message\n");
					fflush(stderr);
				}	
			}
			if (message->type == 1){
				if (message->size == 0 && message->s == 0)
					send(op1, message, sizeof(struct msg), 0);
				else{
					fprintf(stderr, "incorrect message\n");
					fflush(stderr);
				}	
			}
			if (message->type == 8){
				if (message->size == 0 && message->s == 0)
					t2 = time(NULL);
				else{
					fprintf(stderr, "incorrect message\n");
					fflush(stderr);
				}	
			}
		}						
	}
}

void *server_send(void *args)
{
	struct msg *message = (struct msg*)malloc(sizeof(struct msg));
	while(1){
		for (int i = 0; i < 3; i++){
			if ((servers[i] != 0) && (i != port)){
				if (recv(servers[i], message, sizeof(struct msg), MSG_DONTWAIT) != -1){
						if (message->type != 0){
							message->type = 0;
							if (send(servers[i], message, sizeof(struct msg), 0) == -1){
								if (errno == 32){
									perror("send");
									fprintf(stderr, "server %d crush", i);
									fflush(stderr);
									close(servers[i]);
									servers[i] = 0;	
								}	
							} else{
								if (strlen(message->buf) <= 9){
									int *game = (int*)malloc(sizeof(int) * 9);
									char ch;
									int temp;
									for (int j = 0; j < 9; j++){
										ch = message->buf[j];
										temp = atoi(&ch);
										game[j] = temp;
									}
									games[message->s] = game;
								}
							}
					}
				}
			}
		}
	}
}

struct list *root, *ptrlist, *reconnect_list;

int main(void) 
{ 
	int sockfd, new_fd, numbytes, error, con; 
	struct addrinfo hints, *servinfo, *p; 
    struct sockaddr_storage their_addr;  
	socklen_t sin_size; 
	struct sigaction sa; 
	int yes=1; 
	char s[INET6_ADDRSTRLEN]; 
    int rv, count = 0, count_r, op1, op2;

	signal(SIGPIPE, SIG_IGN);
	memset(&hints, 0, sizeof hints); 
	hints.ai_family = AF_UNSPEC; 
	hints.ai_socktype = SOCK_STREAM; 
    hints.ai_flags = AI_PASSIVE;    
	int i;
	errno = EADDRINUSE;

	for (i = 0; i < 3; i++){
		servers[i] = 0;
		if (errno == EADDRINUSE){
			if ((rv = getaddrinfo(NULL, PORT[i], &hints, &servinfo)) != 0) { 
				fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv)); 
				return 1; 
			}
			for(p = servinfo; p != NULL; p = p->ai_next) { 
				if ((sockfd = socket(p->ai_family, p->ai_socktype, 
					p->ai_protocol)) == -1) { 
					perror("server: socket"); 
					continue; 
				}
				if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
					perror("setsockopt");
					exit(1);
				}
				if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
					close(sockfd); 
					perror("server: bind"); 
					continue;
				} else {
					errno = 0;
					port = i;
					if (i == 0)
						freopen( "stdout1.log", "w", stderr);
					if (i == 1)
						freopen( "stdout2.log", "w", stderr);
					if (i == 2)
						freopen( "stdout3.log", "w", stderr);
				}
				break; 
			}
		}
	}
	if (p == NULL)  {
		fprintf(stderr, "server: failed to bind\n"); 
		fflush(stderr);
		return 2;
	}
	freeaddrinfo(servinfo);   

	if (listen(sockfd, BACKLOG) == -1) { 
		perror("listen");
		fflush(stderr);
		exit(1);
	}
    
    sa.sa_handler = sigchld_handler;    
	sigemptyset(&sa.sa_mask); 
	sa.sa_flags = SA_RESTART; 
	
	if (sigaction(SIGCHLD, &sa, NULL) == -1) { 
		perror("sigaction");
		fflush(stderr);
		exit(1);
	}
    printf("server: waiting for connections...\n");
	int flg = 0, flg_r = 0;
	
	connect_to_server();
	pthread_t thread;
	pthread_create(&thread, NULL, server_send, NULL);
	
	int game_n = 0;
    while(1) {
		struct msg *message = (struct msg*)malloc(sizeof(struct msg));
		sin_size = sizeof their_addr; 
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size); 
		if (new_fd == -1) { 
			perror("accept"); 
			fflush(stderr);
			continue;
		}
		inet_ntop(their_addr.ss_family, 
		get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
		if ((numbytes = recv(new_fd, message, sizeof(struct msg), 0)) == -1) { 
			exit(1);
		}
		if (message->type == 15){ //подключается другой сервер
			if (k == port)
				k++;
			servers[k] = new_fd;
			k++;
		}
		if (message->type == 3) { //создает игру и добавляет в списоr
			if (message->s == 0){
				if (strlen(message->buf) > 20 || strlen(message->buf) < 4){
					message->type = -1;
					send(new_fd, message, sizeof(struct msg), 0);
					fprintf(stderr, "game name too big or too small", i);
					fflush(stderr);
				} else {
					send(new_fd, message, sizeof(struct msg), 0);
					if(flg == 0) {
						root = init(new_fd, message->buf);
						count = 1;
						flg = 1;
					} else {
						addelem(root, new_fd, message->buf);
						count = count + 1;
					}
				}
			}
			else{
				fprintf(stderr, "incorrect message\n", i + 1);
				fflush(stderr);
			}
		} else {
			if (message->type == 2) { //подключение к игре
				if(count != 0) {
					char c[50];
					ptrlist = root;
					message->size = count;
					if (send(new_fd, message, sizeof(struct msg), 0) == -1) {
						perror("send");
						fflush(stderr);
					}
					do {
						if (send(new_fd, ptrlist->name, strlen(ptrlist->name) , 0) == -1) {
							perror("send");
						}
						ptrlist = ptrlist->ptr;
						if ((numbytes = recv(new_fd, c, 49, 0)) == -1) { 
							perror("recv"); 
						}
					} while(ptrlist != NULL);
					int game_number = game_n;
					game_n++;
					if ((numbytes = recv(new_fd, message, sizeof(struct msg), 0)) == -1) { 
						perror("recv"); 
						exit(1); 
					}
				
					ptrlist = listfind(root, message->buf);
					op1 = ptrlist->fd;
					op2 = new_fd;
					
						
					if(ptrlist != root) {
						deletelem(ptrlist, root);
					} else {
						root = deletehead(root);
						flg = 0;
					}
					count--;
					if (!fork()){
						game(op1, op2, game_number, 1);
						exit(0);
					}	
				} else {
					message->size = 0;
					message->type = 0;
					message->s = 0;
					memset(message->buf, '\0', strlen(message->buf));
					if (send(new_fd, message, sizeof(struct msg), 0) == -1) {
						perror("send");
					}
				}
			}
		}
		if (message->type == 10){
			char str[10];
			sprintf(str, "%d", message->s);
			if(flg_r == 0) {
				reconnect_list = init(new_fd, str);
				count_r = 1;
				flg_r = 1;
			} else {
				if((ptrlist = listfind(reconnect_list, str)) == NULL){
					addelem(reconnect_list, new_fd, str);
					count_r = count_r + 1;
				} else {
					if (!fork()){
						for(i = 0; i < 3; i++){
							if ((i != port) && (servers[i] != 0))
								message->type = 0;
								errno = 0; 	
								memset(message->buf, '\0', strlen(message->buf)); 
								message->s = 0;
								message->size = 0;
								if (send(servers[i], message, sizeof(struct msg), 0) == -1){
									if (errno == 32){
										perror("send");
										fflush(stderr);
										close(servers[i]);
										servers[i] = 0;	
									}
								}
						}
						if(message->size == 1)
							game(new_fd, ptrlist->fd, message->s, 0);
						else
							game(ptrlist->fd, new_fd, message->s, 0);
					}
				}
			}
		}
	}
	return 0; 
}

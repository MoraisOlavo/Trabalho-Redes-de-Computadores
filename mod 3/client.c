#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <pthread.h>

void* ouvirServidor(void* args){
	int client=*(int*)args;
	char buff[4098];
	while(1){
		recv(client,buff,sizeof(buff),0);
		printf("%s\n",buff);
	}
	pthread_exit(NULL);
}

int main(){
	struct sockaddr_in saddr;
	int client=socket(AF_INET, SOCK_STREAM, 0);
	if(client<0){
		printf("Erro ao criar o socket\n");
		return 1;
	}

	saddr.sin_family=AF_INET;
	saddr.sin_port=htons(8080);
	saddr.sin_addr.s_addr=inet_addr("127.0.0.1");

	int is_connected=0;
	char msg[4098];
	pthread_t thread;

	while(1){
		fgets(msg,4096,stdin);
		if(strcmp(msg,"/connect\n")==0){
			if(is_connected==1){
				printf("Você já está conectado\n");
			}else{
				if(connect(client, (struct sockaddr*) &saddr, sizeof(saddr))<0){
					printf("Erro ao conectar no servidor\n");
					return 1;
				}
				printf("Conexão bem sucedida\n");
				is_connected=1;
				pthread_create(&thread, NULL, ouvirServidor, (void *)&client);
			}
		}else if(strcmp(msg,"/ping\n")==0){
			if(is_connected==1){
				if(send(client, msg, strlen(msg)+1,0)==-1){
					printf("Erro ao enviar /ping\n");
					return 1;
				}
			}else{
				printf("Usuário desconectado, ping não é possível\n");
			}
		}else if(strcmp(msg,"/quit\n")==0){
			if(is_connected==1){
				if(send(client, msg, strlen(msg)+1,0)==-1){
					printf("Erro ao enviar /quit\n");
					return 1;
				}
				close(client);
				is_connected=0;
				pthread_cancel(thread);
				break;
			}else{
				printf("Usuário já está desconectado\n");
			}
		}else{
			if(is_connected==1){
				if(send(client,msg,strlen(msg)+1,0)==-1){
					printf("Erro ao enviar mensagem\n");
					return 1;
				}
			}else{
				printf("Usuário desconectado, não é possível enviar mensagem\n");
			}
		}
	}

	return 0;
}

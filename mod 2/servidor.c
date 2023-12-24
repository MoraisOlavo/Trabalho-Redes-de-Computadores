#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>

#define MAX_CLIENTS 2

int clients[MAX_CLIENTS];

void* clientHandle(void* args){
	int server=*(int*) args;	
	struct sockaddr_in caddr;
	int csize=sizeof(caddr);
	int client=accept(server,(struct sockaddr*)&caddr,&csize);
	int num_client=-1;

	pthread_t thread;
	pthread_create(&thread,NULL, clientHandle, args);

	for(int i=0;i<MAX_CLIENTS;i++){
		if(clients[i]==-1){
			clients[i]=client;
			num_client=i;
			break;
		}
	}

	if(num_client==-1){
		printf("Servidor cheio\n");
	}

	char buff[4097];
	while(1 && num_client!=-1){
		if(recv(client, buff, sizeof(buff),0)<=0){
			printf("Erro ao receber mensagem do %d\n",client);
			break;
		}

		if(strcmp(buff,"/ping\n")==0){
			printf("Recebido um /ping do %d\n",client);
			strcpy(buff,"servidor: pong\n");
			if(send(client,buff,strlen(buff)+1,0)<0){
				printf("Erro ao enviar pong para o cliente %d\n",client);
				break;
			}
		}else if(strcmp(buff,"/quit\n")==0){
			printf("/quit recebido do cliente %d\n",client);
			clients[num_client]=-1;
			break;
		}else{
			printf("%d: %s\n",client,buff);
			char aux[4096];
			sprintf(aux, "%d: ", client);
			strcat(aux,buff);
			for(int i=0;i<MAX_CLIENTS;i++){
				if(clients[i]!=-1 && clients[i]!=client){
					if(send(clients[i],aux,strlen(aux)+1,0)<0){
						printf("Erro ao enviar mensagens aos outros clients\n");
						break;
					}
				}
			}
		}
	}

	close(client);
	pthread_exit(NULL);
}

int main(){
	struct sockaddr_in saddr= {
		.sin_family=AF_INET,
		.sin_addr.s_addr=htonl(INADDR_ANY),
		.sin_port= htons(8080)
	};

	int server=socket(AF_INET, SOCK_STREAM,0);
	if(server<0){
		printf("Erro ao criar o sockeet\n");
		return 1;
	}

	if(bind(server, (struct sockaddr *) &saddr,sizeof(saddr))<0){
		printf("Erro ao vincular o socket\n");
		return 1;
	}

	if(listen(server,MAX_CLIENTS)<0){
		printf("Erro ao anunicar o aceite de conexões\n");
		return 1;
	}

	printf("Aguardando conexões\n");

	for(int i=0;i<MAX_CLIENTS;i++){
		clients[i]=-1;
	}

	pthread_t thread;
	pthread_create(&thread, NULL, clientHandle, (void *)&server);

	while(1){

	}	
	return 0;
}

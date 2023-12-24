#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>

#define MAX_CLIENTS 256
#define MAX_CHANNELS 10

struct usuario{
	char ip [INET_ADDRSTRLEN];
	char nickname[51];
};

struct usuario nicknames[MAX_CLIENTS*MAX_CHANNELS];

char channels_names [MAX_CHANNELS][51];

int clients[MAX_CHANNELS][MAX_CLIENTS];

void* clientHandle(void* args){
	int server=*(int*) args;	
	char buff[4097];
	struct sockaddr_in caddr;
	int csize=sizeof(caddr);
	int client=accept(server,(struct sockaddr*)&caddr,&csize);
	int num_client=-1;
	int num_channel=-1;
	int num_nick=-1;

	pthread_t thread;
	pthread_create(&thread,NULL, clientHandle, args);

	if(recv(client, buff, sizeof(buff),0)<=0){
		printf("Erro ao receber nickname do %d\n",client);
		return;
	}
	printf("nickname recebido do client %d: %s\n",client, buff);

	for(int i=0;i<MAX_CHANNELS*MAX_CLIENTS;i++){
		if(strcmp(nicknames[i].ip,"")==0){
			inet_ntop(AF_INET, &(caddr.sin_addr), nicknames[i].ip, INET_ADDRSTRLEN);
			strcpy(nicknames[i].nickname,buff);
			num_nick=i;
			break;
		}
	}

	if(num_nick==-1){
		printf("Servidor cheio\n");
	}

	while(1 && num_nick!=-1){
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
			strcpy(nicknames[num_nick].ip,"");
			strcpy(nicknames[num_nick].nickname,"");

			if(num_channel!=-1 && num_client!=-1){
				clients[num_channel][num_client]=-1;
			}
			break;
		}else if(strncmp(buff,"/join ",5)==0){
			for(int i=0;i<MAX_CHANNELS;i++){
				if(strcmp(channels_names[i],buff+7)==0){
					num_channel=i;
					printf("Canal encontrado na posicao %d\n",i);
					break;
				}
			}

			if(num_channel==-1){
				printf("Canal não encontrado, procurando canal livre\n");
				for(int i=0;i<MAX_CHANNELS;i++){
					if(strcmp(channels_names[i],"")==0){
						num_channel=i;
						printf("Canal livre encontrado na posicao %d\n",i);
						break;
					}
				}

				if(num_channel==-1){
					printf("Canal não existe e não há espaço para ser criado\n");
					break;
				}
				strcpy(channels_names[num_channel],buff+7);
			}

			num_client=-1;
			for(int i=0;i<MAX_CLIENTS;i++){
				if(clients[num_channel][i]==-1){
					clients[num_channel][i]=client;
					num_client=i;
					break;
				}
			}

			if(num_client==-1){
				printf("Canal cheio\n");
				break;
			}
		}else{
			printf("%d: %s\n",client,buff);
			char aux[4096];
			sprintf(aux, "%s: ", nicknames[num_nick].nickname);
			strcat(aux,buff);
			for(int i=0;i<MAX_CLIENTS;i++){
				if(clients[num_channel][i]!=-1 && clients[num_channel][i]!=client){
					if(send(clients[num_channel][i],aux,strlen(aux)+1,0)<0){
						printf("Erro ao enviar mensagens aos outros clients\n");
						break;
					}
				}
			}
		}
	}

	printf("Encerrando comunicação com %d\n",client);
	close(client);
	pthread_exit(NULL);
}

int main(){
	struct sockaddr_in saddr= {
		.sin_family=AF_INET,
		.sin_addr.s_addr=htonl(INADDR_ANY),
		.sin_port= htons(8080)
	};

	for(int i=0;i<MAX_CHANNELS;i++){
		for(int j=0;j<MAX_CLIENTS;j++){
			strcpy(nicknames[i*MAX_CHANNELS+j].nickname,"");
			strcpy(nicknames[i*MAX_CHANNELS+j].ip,"");
			clients[i][j]=-1;
		}
		strcpy(channels_names[i],"");	
	}

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

	pthread_t thread;
	pthread_create(&thread, NULL, clientHandle, (void *)&server);

	while(1){

	}	
	return 0;
}

#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>

void* clientHandle(void* args){
	int server=*(int*) args;	
	struct sockaddr_in caddr;
	int csize=sizeof(caddr);
	int client=accept(server,(struct sockaddr*)&caddr,&csize);

	pthread_t thread;
	pthread_create(&thread,NULL, clientHandle, args);

	char buff[4097];
	while(1){
		if(recv(client, buff, sizeof(buff),0)<=0){
			printf("Erro ao receber mensagem do %d\n",client);
			break;
		}

		if(strcmp(buff,"/sair\n")==0){
			printf("Encerrando conexão com %d\n",client);
			break;
		}

		printf("%d: %s\n",client,buff);
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

	if(listen(server,0)<0){
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

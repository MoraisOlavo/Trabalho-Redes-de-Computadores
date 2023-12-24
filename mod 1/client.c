#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

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

	if(connect(client, (struct sockaddr*) &saddr,sizeof(saddr))<0){
		printf("Erro ao conectar no servidor\n");
		return 1;
	}else{
		printf("Conexão bem sucedida\n");
	}

	char msg[4097];
	while(1){
		fgets(msg, sizeof(msg), stdin);
		printf("msg lida com fgets: %s\n",msg);
		if(send(client, msg, strlen(msg)+1,0)==-1){
			printf("Erro ao enviar mensagem\n");
			return 1;
		}

		if(strcmp(msg,"/sair\n")==0){
			printf("Saindo\n");
			break;		
		}
	}
	
	close(client);
	return 0;
}

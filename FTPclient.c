#include "FTPheader.h"

int main(int argc,char* argv[])
{
	if (argc < 3) {printf("%s\n","ERROR: Please input the host name and the port number as arguments");return -1;}
	if (strlen(argv[2])>5){printf("%s\n","ERROR: Please input the host name then the port number as the 2 arguments, make sure the port number is valid");return -1;}
	//1. socket
	int server_fd = socket(AF_INET,SOCK_STREAM,0);
	if(server_fd<0)
	{
		perror("Socket: ");
		return -1;
	}

	struct sockaddr_in server_address;
	memset(&server_address,0,sizeof(server_address));

	struct hostent *server = gethostbyname(argv[1]);
    bcopy((char *)server->h_addr_list[0], (char *)&server_address.sin_addr.s_addr, server->h_length); 
    int port = atoi(argv[2]);

	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(port);
	//2. connect
	if(connect(server_fd,(struct sockaddr*)&server_address,sizeof(server_address))<0)
	{
		perror("Connect :");
		return -1;
	}

	char message[100];
	char sermessage[100];
	int sending=0;
	char cmd[10];
	

	while(1)
	{
		while(!sending)
		{
			printf("ftp>");
			fgets(message,100,stdin);
			message[strcspn(message,"\n")]=0; //lets add it
			sscanf(message,"%s",cmd);
			if(!strcmp(cmd,"QUIT") || !strcmp(cmd,"USER")|| !strcmp(cmd,"PASS") || !strcmp(cmd,"PRINT") || !strcmp(cmd,"LS") || !strcmp(cmd,"CD") || !strcmp(cmd,"PWD") || !strcmp(cmd,"NEWU"))
				sending = 1;
			else if(!strcmp(cmd,"PUT"))
			{
				if(!strlen(message+4)) printf("Please enter a file name\n");
				else if(access(message+4,F_OK)!=-1) sending = 1;
				else printf("No file with such name exists\n");
			}
			else if(!strcmp(cmd,"GET"))
			{
				if(strlen(message+4)>0) sending = 1;
				else printf("Please enter a file name\n");
			}
			else if(!strcmp(cmd,"!CD"))
			{
				if(!chdir(message+4)) printf("%s\n","Directory Changed");
				else printf("%s\n","Directory not found");
			}
			else if(!strcmp(cmd,"!LS"))
				system("ls *");
			else if(!strcmp(cmd,"!PWD"))
				system("pwd *");
			else
				printf("command not found\n");
		}
		//3. send/rec
		send(server_fd,message,strlen(message),0);
		sending = 0;
		// Exit
		if(strcmp(cmd,"QUIT")==0) break;
		// Receive
		memset(sermessage,0,sizeof(sermessage));
		if(recv(server_fd,sermessage,sizeof(sermessage)-1,0)>0)
		{
			if(strcmp(cmd,"GET") && strcmp(cmd,"PUT"))
				printf("%s\n", sermessage);
			if(!strcmp(cmd,"GET"))
			{
				if(!strcmp(sermessage,"nonexisted")) printf("No file with such name exists\n");
				else
				{
					sscanf(sermessage,"%s",cmd);
					if(!strcmp(cmd,"existed") && strlen(sermessage)<15) // makes sure there is no issue serverside
					{
						// socket
						int tserver_fd = socket(AF_INET,SOCK_STREAM,0);
						if(tserver_fd<0)
							printf("Socket ERROR\n");
						else
						{	
							char tport[6];
							struct sockaddr_in serverAddress;
							memset(&serverAddress,0,sizeof(serverAddress));
							serverAddress.sin_family = AF_INET;
							bcopy((char *)server->h_addr_list[0], (char *)&serverAddress.sin_addr.s_addr, server->h_length);
							sscanf(sermessage+8,"%s",tport);
							serverAddress.sin_port = htons(atoi(tport));
							// connect
							if(connect(tserver_fd,(struct sockaddr*)&serverAddress,sizeof(serverAddress))<0)
								printf("Connect ERROR\n");
							else
							{
								int fp = open(message+4,O_CREAT|O_WRONLY,0666);
								if(fp != -1)
								{
									memset(sermessage,0,sizeof(sermessage));
									while(recv(tserver_fd,sermessage,sizeof(sermessage)-1,0)>0)
									{
										write(fp, sermessage, strlen(sermessage));
										memset(sermessage,0,sizeof(sermessage));
									}
								}
								else printf("File ERROR\n"); 
								close(fp);
								close(tserver_fd);
								printf("File received\n");
							}
						}
					}
					else printf("%s\n",sermessage);
				}
			}
			if(!strcmp(cmd,"PUT"))
			{
				if(!strcmp(sermessage,"nonexisted")) printf("ERROR\n");
				else
				{
					sscanf(sermessage,"%s",cmd);
					if(!strcmp(cmd,"existed") && strlen(sermessage)<15) //makes sure there is no issue serverside
					{
						// socket
						int tserver_fd = socket(AF_INET,SOCK_STREAM,0);
						if(tserver_fd<0)
								printf("Socket ERROR\n");
						else
						{	
							char tport[6];
							struct sockaddr_in serverAddress;
							memset(&serverAddress,0,sizeof(serverAddress));
							serverAddress.sin_family = AF_INET;
							bcopy((char *)server->h_addr_list[0], (char *)&serverAddress.sin_addr.s_addr, server->h_length);
							sscanf(sermessage+8,"%s",tport);
							serverAddress.sin_port = htons(atoi(tport));
							// connect
							if(connect(tserver_fd,(struct sockaddr*)&serverAddress,sizeof(serverAddress))<0)
								printf("Connect ERROR\n");
							else
							{
								FILE* fp = fopen(message+4,"r");
								if(fp!=NULL)
								{	
									char path [1024];
									int nFlag = 0;
									do
									{
										strcpy(path,"");
										if(fgets(path,1024,fp) != NULL)
											strcpy(message,path);
										else nFlag = 1;
										send(tserver_fd,message,strlen(message),0);
										strcpy(message,"");
									} while (!nFlag);
									fclose(fp);
								}
								else printf("File ERROR\n");
							}
							printf("%s\n", "File sent\n");
							close(tserver_fd);
						}
					}
					else printf("%s\n",sermessage);
				}
			}
		}
	}

	//4. close
	close(server_fd);
	return 0;
}
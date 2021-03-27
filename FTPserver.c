#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <pwd.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#define PORT 9000

#define MAX_USERS 20
#define MAX_CLIENTS 8
char user_list[MAX_USERS][20];
char pass_list[MAX_USERS][20]; 

int serveClient(int client_fd,short *uauth,short *auth,int *p0,int *p1)
{ 
	char message[100];
	char sermessage[100];
	char cmd[10];
	memset(message,0,sizeof(message));
	if(recv(client_fd,message,sizeof(message)-1,0)>0)
	{
		sscanf(message,"%s",cmd);
		if(!strcmp(cmd,"QUIT"))
		{
			auth=0;
			uauth=0;
			printf("Client disconnected...! \n");
			return 1;
		}
		else if(!strcmp(cmd,"USER"))
		{
			*uauth=0;
			*auth=0; 
			for (int i=0;i<*p1;i++)
			{
				if(strcmp(message+5,user_list[i])==0 && strcmp("NULL",pass_list[*uauth-1])!=0)
				{
					*uauth = i+1;
					strcpy(sermessage,"Username OK, password required");
					break;
				}
				else strcpy(sermessage,"Username does not exist or is taken by another user");
			}
		}
		else if(!strcmp(cmd,"PASS"))
		{
			if (*uauth>0)
			{
				if (strcmp("NULL",pass_list[*uauth-1])==0)
				{
					if (strcmp("NULL",message+5)==0)
						strcpy(sermessage,"Please use another PASS");
					else
					{
						strcpy(message+5,pass_list[*uauth-1]);
						strcpy(sermessage,"New PASS assigned to the USER");
						*auth = *uauth;
					}
				}
				else if (strcmp(message+5,pass_list[*uauth-1])==0)
				{
					*auth=*uauth;
					strcpy(sermessage,"Authentication Complete");
				}
				else
					strcpy(sermessage,"Wrong password");
			}
			else
				strcpy(sermessage,"Set USER first");
		}
		else if(!strcmp(cmd,"NEWU"))
		{
			*uauth=0;
			*auth=0; 
			if (*p1<MAX_USERS)
			{
				for (int i=0;i<*p1;i++)
				{
					if(strcmp(message+5,user_list[i])==0)
					{
						*uauth = -1;
						strcpy(sermessage,"Username already taken");
						break;
					}
				}
				if (*uauth == -1) *uauth = 0;
				else
				{
					if (strlen(message)>5)
					{
						strcpy(user_list[++*p1],message+5);
						strcpy(pass_list[  *p1],"NULL");
						*uauth = *p1;
						strcpy(sermessage,"USER created, use PASS to set your password");
					}
					else strcpy(sermessage,"Please specify the username using NEWU <user>");
				}
			}
			else strcpy(sermessage,"Max no. of users reached!");
		}
		else
		{
			if (*auth>0)
			{
				if(!strcmp(cmd,"PRINT"))
				{
					printf("%s \n", message+6);
					strcpy(sermessage,"PRINTED");
				}
				else if(!strcmp(cmd,"PUT"))
				{
				}
				else if(!strcmp(cmd,"GET"))
				{
					FILE* fp = fopen(message+4,"r");
					if(access(message+4,F_OK)!=-1)
					{
						if(fp!=NULL)
						{
							int tport = PORT + client_fd + *p0;
							if(fork() == 0)
							{
								struct sockaddr_in serverAddress,clientAddress;
								serverAddress.sin_family = AF_INET;
								serverAddress.sin_addr.s_addr = INADDR_ANY;
								serverAddress.sin_port = htons(tport);
								int tserver_fd = socket(AF_INET, SOCK_STREAM, 0);
								if (tserver_fd < 0) 
								{
									printf("Socket ERROR\n");
									strcpy(sermessage,"Socket ERROR");
									send(client_fd,sermessage,strlen(sermessage),0);
									return 0;
								}
								if (bind(tserver_fd, (struct sockaddr *) &serverAddress, sizeof(serverAddress))<0) 
								{
									*p0 = (*p0+MAX_CLIENTS)%(MAX_CLIENTS*32);
									printf("%i\n",*p0);
									printf("Bind ERROR\n");
									strcpy(sermessage,"Bind ERROR");
									close(tserver_fd);
									send(client_fd,sermessage,strlen(sermessage),0);
									return 0;
								}
								if (listen(tserver_fd, 5) < 0)
								{
									printf("Listen ERROR\n");
									strcpy(sermessage,"Listen ERROR");
									close(tserver_fd);
									send(client_fd,sermessage,strlen(sermessage),0);
									return 0;
								}
								int tsock = accept(tserver_fd,NULL,NULL);
								if (tsock < 0) 
								{
									printf("Accept ERROR\n");
									strcpy(sermessage,"Accept ERROR");
									send(tsock,sermessage,strlen(sermessage),0);
									return 0;
								}
								else
								{
									char path [1024];
									int nFlag = 0;
									do
									{
										strcpy(path,"");
										if(fgets(path,1024,fp) != NULL)
											strcpy(sermessage,path);
										else nFlag = 1;
										send(tsock,sermessage,strlen(sermessage),0);
										strcpy(sermessage,"");
									} while (!nFlag);
								}
								fclose(fp);
								close(tsock);
								close(tserver_fd);
								return 0;
							}
							else{sprintf(sermessage,"%s %d","existed",tport);}
						}
						else strcpy(sermessage,"nonexisted");
					}
					else strcpy(sermessage,"nonexisted");
				}
				else if(!strcmp(cmd,"CD"))
				{
					if(!chdir(message+3)) strcpy(sermessage,"Directory Changed");
					else strcpy(sermessage,"Directory not found");
				}
				else if(!strcmp(cmd,"LS"))
				{
					FILE* fp = popen("ls","r");
					if(fp != NULL) 
					{
						char path [100];
						char fret [100];
						strcpy(path,"");
						strcpy(fret,"");
						while (fgets(path,100,fp) != NULL) strcat(fret,path);		
						strcpy(path,"Successfully executed! Result below:\n");
						strcpy(sermessage,strcat(path,fret));
						pclose(fp);
					}
					else strcpy(sermessage,"Wrong Command Usage!");
				}
				else if(!strcmp(cmd,"PWD"))
				{
					FILE* fp = popen("pwd","r");
					if(fp != NULL) 
					{
						char path [100];
						char fret [100];
						strcpy(path,"");
						strcpy(fret,"");
						while (fgets(path,100,fp) != NULL) strcat(fret,path);		
						strcpy(path,"Successfully executed! Result below:\n");
						strcpy(sermessage,strcat(path,fret));
						pclose(fp);
					}
				}
			}
			else 
				strcpy(sermessage,"please authorise your account first");
		}
	}
	send(client_fd,sermessage,strlen(sermessage),0);
	return 0;
}

int main()
{
	// default usernames
	strcpy(user_list[0],"hessa");
	strcpy(user_list[1],"mo");
	strcpy(user_list[2],"user");
	strcpy(user_list[3],"prof");
	// default passwords
	strcpy(pass_list[0], "hammad");
	strcpy(pass_list[1], "abualh");
	strcpy(pass_list[2], "pass");
	strcpy(pass_list[3], "zaki");

	int p[3]={0,4,0}; // {Bind Resolution, no. of users, no. of current clients}

	struct sockaddr_in server_address;

	//1. socket creation 
	int server_fd = socket(AF_INET,SOCK_STREAM,0);
	if(server_fd<0)
	{
		perror("Creating Socket failed..\n");
		return -1;
	}

	//Assigning the IP, PORT 
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(PORT);
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	
	//2. binding the newly created socket
	if(bind(server_fd,(struct sockaddr*) &server_address,sizeof(server_address))<0)
	{
		perror("socket bind failed..\n");
		return -1;
	}
	//3. server is listening 
	if(listen(server_fd,2)<0)
	{
		perror("Listen failed..\n");
		return -1;
	}

	printf("Server listening and waiting for a connection..\n");

	struct sockaddr_in client_address;				//we to pass this to accept method to get client info
	int client_address_len = sizeof(client_address); // accept also needs client_address length

	// select() prep
	fd_set full_fdset, ready_fdset; //file descriptor lists 
	FD_ZERO(&full_fdset); //clear sets
	FD_SET(server_fd,&full_fdset);

    int max_fd = server_fd; 
	short UAUTH[max_fd];
	short AUTH[max_fd];

	for (int i; i<max_fd; i++){AUTH[i]=UAUTH[i]=0;}

	while(1)
	{
		int quit = 0;
		ready_fdset = full_fdset;
		//waiting for the activity 
		if(select(max_fd+1,&ready_fdset,NULL,NULL,NULL)<0)
		{
			perror("Select function error");
			return -1;
		}

		for(int j=0; j<=max_fd;j++)
		{
			if(FD_ISSET(j,&ready_fdset))
			{
				if(j==server_fd)
				{
					if(p[2]++<MAX_CLIENTS)
					{
						printf("Client connected...!\n");
						int client_fd = accept(server_fd,NULL,NULL);
						FD_SET(client_fd,&full_fdset);
						if(client_fd>max_fd) max_fd = client_fd;
						UAUTH[client_fd] = 0;
						AUTH[client_fd] = 0;
					}
				}
				else
				{
					quit = serveClient(j,&(UAUTH[j]),&(AUTH[j]),&p[0],&p[1]);
					if (quit) 
						{p[2]--;FD_CLR(j,&full_fdset);break;}
				}
			}
		}
	}
	close(server_fd);
	return 0;
}
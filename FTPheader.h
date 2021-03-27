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
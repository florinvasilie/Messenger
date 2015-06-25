#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include "user.h"



static void *treat(void *); /* functia executata de fiecare thread ce realizeaza comunicarea cu clientii */
void raspunde(void *);
void menu(void * arg);
int verifyLogin(thData *tdL);
void sendMessage(char* touser, char* message);
void readMessage(int inID, void * arg); 
void readArchive(int inID, void * arg);
int replystatus=0;
char usersconnected[200];
char index1=0,index2=0;
 

int main ()
{
	struct sockaddr_in server;	// structura folosita de server
	struct sockaddr_in from;
 
	
	int sd;		//descriptorul de socket 
	int pid;
	pthread_t th[100];    //Identificatorii thread-urilor care se vor crea
	int i=0;
  	

	/* crearea unui socket */
	if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
   	{
		printf ("[server]Eroare la socket().\n");
      		return errno;
    	}
  	/* utilizarea optiunii SO_REUSEADDR */
  	int on=1;
  	setsockopt(sd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));
  
  	/* pregatirea structurilor de date */
  	bzero (&server, sizeof (server));
  	bzero (&from, sizeof (from));
  
  	/* umplem structura folosita de server */
  	/* stabilirea familiei de socket-uri */
    	server.sin_family = AF_INET;	
  	/* acceptam orice adresa */
    	server.sin_addr.s_addr = htonl (INADDR_ANY);
  	/* utilizam un port utilizator */
   	server.sin_port = htons (PORT);
  
  	/* atasam socketul */
  	if (bind (sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1)
    	{
      		printf ("[server]Eroare la bind().\n");
      		return errno;
    	}

  	/* punem serverul sa asculte daca vin clienti sa se conecteze */
  	if (listen (sd, 2) == -1)
    	{
      		printf ("[server]Eroare la listen().\n");
      		return errno;
    	}
	
	load(); //aici incarc din fisier
		
  	/* servim in mod concurent clientii...folosind thread-uri */
  
   	 while (1)
  	 {
      		int client;
      		thData * td; //parametru functia executata de thread     
      		int length = sizeof (from);

      		printf ("[server]Asteptam la portul %d...\n",PORT);
      		fflush (stdout);

      		/* acceptam un client (stare blocanta pina la realizarea conexiunii) */
      		if ( (client = accept (sd, (struct sockaddr *) &from, &length)) < 0)
		{
	  		printf ("[server]Eroare la accept().\n");
	  		continue;
		}
	
        	/* s-a realizat conexiunea, se astepta mesajul */
    	
		int idThread; //id-ul threadului
		int cl; //descriptorul intors de accept
	


		td=(struct thData*)malloc(sizeof(struct thData));	
	
		td->idThread=i++;
		td->cl=client;
	
	
		pthread_create(&th[i], NULL, &treat, td);      
					
	}//while    
};
				
static void *treat(void * arg)
{		
		struct thData tdL; 
		tdL= *((struct thData*)arg);	
		printf ("[thread]- %d - Asteptam mesajul...\n", tdL.idThread);
		fflush (stdout);		 
		pthread_detach(pthread_self());		
		raspunde(&tdL);
		/* am terminat cu acest client, inchidem conexiunea */
		close (&tdL);
  		
};


void raspunde(void *arg)
{
	struct thData tdL; 
	tdL= *((struct thData*)arg);
	verifyLogin(&tdL);
	menu(&tdL);
}


int verifyLogin(thData * tdL)
{

	int size, rsize, contor = 0, i = 0;
	char msg[2000], rmsg[2000], user[100], pass[100];
	char * tmp;
	bzero (&msg, sizeof(msg));
	bzero (&rmsg, sizeof(rmsg));
	bzero (&user, sizeof(user));
	bzero (&pass, sizeof(pass));
	strcpy(msg, "\nBine ai venit la Offline Messenger!\n\tIntrodu Username si Password pentru logare.");
	
	while(contor != -1)
	{	
		if(contor == 0)
			strcat(msg, "\nUsername:\n ");
		else
			strcat(msg, "\nPassword:\n ");

		size = strlen(msg);
		if(write(tdL->cl, &size, sizeof(int)) <= 0)
		{
			printf ("[Server] error sending msg to [Client].\n");
		}
		if(write(tdL->cl, msg, size) <= 0)
		{
			printf ("[Server] error sending msg to [Client].\n");
		}

		if(rsize = read(tdL->cl, &size, sizeof(int)) < 0 )
		{
			printf ("[Server] error reading msg from [Client].\n");
		}
		if(rsize = read(tdL->cl, rmsg, size) < 0 )
		{
			printf ("[Server] error reading msg from [Client].\n");
		}
		printf("[Server] Client response: %s\n", rmsg);
		
		rmsg[strlen(rmsg) -1] = '\0';
		
		if(contor == 0)
		{
			for(i = 0; i < MAX; i++)
			{	
				if(strlen(u[i].name) < 0)
					break;
				if(strcmp(rmsg, u[i].name) == 0)
				{
					contor = 1;
					tmp = u[i].name;
				}
			}
			
		}
		else
		{ 
			
			
			for(i = 0; i < MAX; i++)
			{
				if(strlen(u[i].pass) < 0)
					break;

				if((strcmp(rmsg, u[i].pass) == 0) && (strcmp(tmp,u[i].name)== 0))
				{
					contor = -1;
					printf ("[Server] User %s s-a logat cu succes!\n", u[i].name);
					tdL->id = u[i].id;
					strcpy(tdL->name,tmp);
					strcat(usersconnected,tmp);
					strcat(usersconnected,"\n");
					
				}
			}
		}
	
		bzero (&msg, sizeof(msg));
		bzero (&rmsg, sizeof(rmsg));
	}

	return 1;
}

void menu(void * arg)
{
	struct thData tdL; 
	tdL= *((struct thData*)arg);
	int size, rsize, contor = 0, i = 0,j=0,k=0;
	char msg[2000], rmsg[2000];
	char commsend[4],commread[4],commarchive[6],commonusers[8];
	char command[100];
	char touser[100];
	char message[1500];
	char msgDeTrimis[2000];
	bzero (&msg, sizeof(msg));
	bzero (&rmsg, sizeof(rmsg));
	bzero (&msgDeTrimis, sizeof(msgDeTrimis));

	strcpy(msgDeTrimis, tdL.name);
	strcat(msgDeTrimis,": "); 

	if (u[tdL.id].mesajenoi==1)
		strcpy(msg,"\nAveti un mesaj nou !: read\n");
	else
		strcpy(msg,"\nIntroduceti optiunea dorita: on.users, send, read, arhiva\n");	

	if (replystatus==1)
	{
		strcpy(msg,"\nFolositi comanda send pentru a raspunde\n");
		replystatus=0;
	}

	

	size = strlen(msg);
	if(write(tdL.cl, &size, sizeof(int)) <= 0)
	{
		printf ("[Server] error menu1 sending msg to [Client].\n");
	}
	if(write(tdL.cl, msg, size) <= 0)
	{
		printf ("[Server] error menu2 sending msg to [Client].\n");
	}
	
	if(rsize = read(tdL.cl, &size, sizeof(int)) < 0 )
	{
		printf ("[Server] error reading msg from [Client].\n");
	}
	if(rsize = read(tdL.cl, rmsg, size) < 0 )
	{
		printf ("[Server] error reading msg from [Client].\n");
	}

	printf("[Server] Client response menu: %s\n", rmsg);
		
	rmsg[strlen(rmsg) -1] = '\0';
		
	for (i=0;i<strlen(rmsg);i++)
	{	
		if (contor <=1)
		{
			if (rmsg[i]==' ')
			{
				contor++;
				continue;		
			}
		}
		if (contor == 0)
		{
			command[i]=rmsg[i];	//aici sunt comenzile
		}
		if (contor == 1)
		{
			touser[j++]=rmsg[i]; // aici userul catre cine vreau sa trimit
		}	
		if (contor > 1)
		{
			message[k++]=rmsg[i];  //aici mesajul
		}
 							
	}
	
	strcpy(commsend,"send");
	strcpy(commread,"read");
	strcpy(commarchive,"arhiva");
	strcpy(commonusers,"on.users");
	
	strcat(msgDeTrimis,message);
		
		
	if (strcmp(command,commsend)==0)
	{
		printf("Comanda1 trimisa catre server este: %s\n",command); //aici daca comanda e send
		strcat(u[tdL.id].arhiva[index2],msgDeTrimis);
		strcat(u[tdL.id].arhiva[index2],"\n");
		index2++;
		sendMessage(touser,msgDeTrimis);
		menu(&tdL);	
			
	}
	
	if (strcmp(command,commread)==0)
	{
		printf("Comanda2 trimisa catre server este: %s\n",command); //comanda read
		readMessage(tdL.id,&tdL); 
	}
	
	if (strcmp(command,commarchive)==0)
	{	
		readArchive(tdL.id,&tdL);
	}

	if (strcmp(command,commonusers)==0)
	{
		onlineUsers(&tdL);
	}				
			

}



void sendMessage(char* touser, char* message)
{
	int k = getIdByName(touser);
	int i=0;
	strcpy(u[k].mesaj[i], message);
	
	strcat(u[k].arhiva[index1],message);
	strcat(u[k].arhiva[index1],"\n");
	i++;
	index1++;
	
	u[k].mesajenoi=1; 
	
	
}


void onlineUsers(void * arg)
{
	struct thData tdL; 
	tdL= *((struct thData*)arg);
	char msg[4000],rmsg[4000];
	char back[100],commback[100];
	int size,rsize,j;
	bzero(&msg,4000);
	
	
	strcat(msg,usersconnected);
	strcat(msg,"\nTastati back pentru a reveni\n");
	
	size = strlen(msg);
	if(write(tdL.cl, &size, sizeof(int)) <= 0)
	{
		printf ("[Server] error menu1 sending msg to [Client].\n");
	}
	
	if(write(tdL.cl, msg, size) <= 0)
	{
		printf ("[Server] error menu3 sending msg to [Client].\n");
	}

	if(rsize = read(tdL.cl, &size, sizeof(int)) < 0 )
	{
			printf ("[Server] error reading msg from [Client].\n");
	}
	if(rsize = read(tdL.cl, rmsg, size) < 0 )
	{
		printf ("[Server] error reading msg from [Client].\n");
	}

	printf("[Server] Client response menu reply: %s\n", rmsg);
	
	
	rmsg[strlen(rmsg) -1] = '\0';
	strcat(back,rmsg);
	strcpy(commback,"back");
	if (strcmp(back,commback)==0)
	{
		menu(&tdL);
		
	}
	else
	{
		menu(&tdL);
	}

}


void readArchive(int inID, void * arg)
{
	struct thData tdL; 
	tdL= *((struct thData*)arg);
	char msg[4000],rmsg[4000];
	char back[100],commback[100];
	int size,rsize,j;
	bzero(&msg,4000);
	
	for(j=0;j<400;j++)
	{
		if (strlen(u[inID].arhiva[j]) == 0)
			break;
		
		strcat(msg,u[inID].arhiva[j]);
		strcat(msg,"\n");
	}
	
	strcat(msg,"\nTastati back pentru a reveni\n");
	size = strlen(msg);
	if(write(tdL.cl, &size, sizeof(int)) <= 0)
	{
		printf ("[Server] error menu1 sending msg to [Client].\n");
	}
	
	if(write(tdL.cl, msg, size) <= 0)
	{
		printf ("[Server] error menu3 sending msg to [Client].\n");
	}

	if(rsize = read(tdL.cl, &size, sizeof(int)) < 0 )
	{
			printf ("[Server] error reading msg from [Client].\n");
	}
	if(rsize = read(tdL.cl, rmsg, size) < 0 )
	{
		printf ("[Server] error reading msg from [Client].\n");
	}

	printf("[Server] Client response menu reply: %s\n", rmsg);
	
	
	rmsg[strlen(rmsg) -1] = '\0';
	strcat(back,rmsg);
	strcpy(commback,"back");
	if (strcmp(back,commback)==0)
	{
		menu(&tdL);
		
	}
	else
	{
		menu(&tdL);
	}


}

void readMessage(int inID, void * arg)
{
	struct thData tdL; 
	tdL= *((struct thData*)arg);
	char msg[4000],rmsg[4000];
	char reply[100],commreply[100];
	int size,rsize,j;
	bzero(&msg,4000);
	
	for(j=0;j<100;j++)
	{
		if (strlen(u[inID].mesaj[j]) == 0)
			break;
		
		strcat(msg,u[inID].mesaj[j]);
		strcat(msg,"\n");
	}

	strcat(msg,"\nTastati reply pentru a raspunde/cancel pentru a revenit la optiuni\n");
	size = strlen(msg);
	if(write(tdL.cl, &size, sizeof(int)) <= 0)
	{
		printf ("[Server] error menu1 sending msg to [Client].\n");
	}
	
	if(write(tdL.cl, msg, size) <= 0)
	{
		printf ("[Server] error menu3 sending msg to [Client].\n");
	}

	if(rsize = read(tdL.cl, &size, sizeof(int)) < 0 )
	{
			printf ("[Server] error reading msg from [Client].\n");
	}
	if(rsize = read(tdL.cl, rmsg, size) < 0 )
	{
		printf ("[Server] error reading msg from [Client].\n");
	}

	printf("[Server] Client response menu reply: %s\n", rmsg);
	
	u[inID].mesajenoi=0;
	
	rmsg[strlen(rmsg) -1] = '\0';
	strcat(reply,rmsg);
	strcpy(commreply,"reply");
	if (strcmp(reply,commreply)==0)
	{
		replystatus=1;
		menu(&tdL);
		
	}
	else
	{
		replystatus=0;
		menu(&tdL);
	}

	
}  

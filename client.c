#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>

/* codul de eroare returnat de anumite apeluri */
extern int errno;

/* portul de conectare la server*/
int port;

int main (int argc, char *argv[])
{
  int sd;			// descriptorul de socket
  struct sockaddr_in server;	// structura folosita pentru conectare 
  		// mesajul trimis
  int nr=0;
  char buf[10];

  /* exista toate argumentele in linia de comanda? */
  if (argc != 3)
    {
      printf ("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
      return -1;
    }

  /* stabilim portul */
  port = atoi (argv[2]);

  /* cream socketul */
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("Eroare la socket().\n");
      return errno;
    }

  /* umplem structura folosita pentru realizarea conexiunii cu serverul */
  /* familia socket-ului */
  server.sin_family = AF_INET;
  /* adresa IP a serverului */
  server.sin_addr.s_addr = inet_addr(argv[1]);
  /* portul de conectare */
  server.sin_port = htons (port);
  
  /* ne conectam la server */
  if (connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1)
    {
      perror ("[client]Eroare la connect().\n");
      return errno;
    }

	int size, rsize;
	char msg[2000], rmsg[2000];

	while(1)
	{
		bzero (&msg, sizeof(msg));
		bzero (&rmsg, sizeof(rmsg));
		if(rsize = read(sd, &size, sizeof(size)) < 0 )
		{
			printf ("[Client] error reading msg from [Server].\n");
		}
		if(rsize = read(sd, rmsg, size) < 0 )
		{
			printf ("[Client] error reading msg from [Server].\n");
		}
		
		printf("%s",rmsg);
		bzero (&rmsg, sizeof(rmsg));
		read (0, msg, sizeof(msg));

		size = strlen(msg);
		if(write(sd, &size, sizeof(size)) <= 0)
		{
			printf ("[Client] error sending msg to [Server].\n");
		}
		if(write(sd, msg, size) <= 0)
		{
			printf ("[Client] error sending msg to [Server].\n");
		}

	}





  /* inchidem conexiunea, am terminat */
  close (sd);
}

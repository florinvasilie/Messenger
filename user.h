#include <stdio.h>
#include <string.h>
#include <strings.h>
#define MAX 20

/* portul folosit */
#define PORT 2908

/* codul de eroare returnat de anumite apeluri */
extern int errno;


typedef struct thData{
	int idThread; //id-ul thread-ului tinut in evidenta de acest program
	int cl; //descriptorul intors de accept
	int id; //id-ul userului conectat
	char name[100]; //numele userului conectat
}thData;


typedef struct User
{
	char name[100];
	char pass[100];
	int id;
	char mesaj[100][1000]; // toate mesajele citite se sterg de aici
	char arhiva[400][2000]; //toate mesajele citite ajung aici
	int mesajenoi;
}User;
User u[MAX];
char name[100];
char pass[100];
int id = 0;

void AfisUsers();
int getIdByName(char * name);
int getStructMessageindex(int inID);

void load(){
	
	int i = 0;
	FILE *file;
	file = fopen("user.txt", "r");
	while(!feof(file))
	{
		u[i].id = id;
		fscanf(file, "%s %s", u[i].name, u[i].pass);
		id++;
		i++;
	}
	fclose(file);
	//AfisUsers();
}

void AfisUsers()
{
	int i;
	for(i = 0; i < MAX; i++)
	{ 
		if(strlen(u[i].name) < 1)
			break;
		printf("Name: %s\nPass: %s\nID: %d\n", u[i].name, u[i].pass, u[i].id);
	}
}

int getIdByName(char * name)
{
	int i, outID = 0;

	for(i = 0; i < MAX; i++)
	{
		if(strcmp(name, u[i].name) == 0 ) 
		{
			outID = u[i].id;
			return outID;
		}
	}
	return outID;
}


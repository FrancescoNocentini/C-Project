#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>
#include <sys/unistd.h>
#include <time.h>
#include <signal.h>

#define PSTOP rand() % 100
#define PINT rand() % 10000
#define PCONT rand() % 10
#define PUSR1 rand() % 10

void sendSTOP(int, int);
void sendINT(int, int);
void sendCONT(int, int);
void sendUSR1(int, int);

int main(int argc, char *argv[]){
	int fdFailures, r, i = 0, pid[3], pidChoice;
	char tmp[50], buffer[20], line[64];
	bool start = false;
	FILE* fp;
	char const* fileName = "./pid.txt";
	
	do{	
		time_t t = time(NULL);
		struct tm tm = *localtime(&t);
		sprintf(tmp,"%d.%d.%d", tm.tm_hour, tm.tm_min, tm.tm_sec);	//creo la stringa per confrontarla con quella del padre
		if(strcmp(tmp, argv[1]) == 0){	//se il tempo preso dal processo e' uguale a quello passato dal padre
			start = true;
		}else{
			sleep(1);
		}
	}while(start == false);

	//apro file failures.log
	fdFailures = open("failures.log",O_WRONLY|O_CREAT,0666);
	if(fdFailures < 0){
		printf("Errore apertura failures.log da genFallimenti\n");
	}
	//apro file pid.txt per salvare i pid dei tre PFC
	fp = fopen(fileName, "r");
	if(fp == NULL){
		printf("Errore apertura pid.txt da generatoreFallimenti\n");
	}
	memset(tmp, 0x00, 50);
	memset(buffer, 0x00, 20);
	for(i = 0; i < 3; i++){
		fgets(line, sizeof(line), fp);	//leggo riga
		pid[i] = atoi(line);	//converto il pid in int
	}
	fclose(fp);

	srand(time(NULL));
	i = 0;
	
	do{
		sleep(1);
		//azzero perche' ho utilizzato tmp e buffer sopra
		memset(tmp, 0x00, 50);
		memset(buffer, 0x00, 20);
		
		r = rand() % 3;	//genero intero random compreso tra 0 e 2, 0 = PFC1, 1 = PFC2, 2 = PFC3

		if(r == 0){
			pidChoice = pid[0];	//pid di PFC1 in pidChoice
		}else if(r == 1){
			pidChoice = pid[1];	//pid di PFC2 in pidChoice
		}else{
			pidChoice = pid[2];	//pid di PFC3 in pidChoice
		}

		r = PSTOP;	//genero intero random compreso tra 0 e 99
		if(r == 1){
			sendSTOP(pidChoice, fdFailures);
		}
		
		r = PINT;	//genero intero random compreso tra 0 e 9999
		if(r == 1){
			sendINT(pidChoice, fdFailures);
		}
		
		r = PCONT;	//genero intero random compreso tra 0 e 9
		if(r == 1){
			sendCONT(pidChoice, fdFailures);
		}
		
		r = PUSR1;	//genero intero random compreso tra 0 e 9
		if(r == 1){
			sendUSR1(pidChoice, fdFailures);
		}
	}while(true);

	close(fdFailures);

	return 0;
}

void sendSTOP(int pidChoice, int fdFailures){
	char tmp[50];
	int charsWritten;
	//invio segnale stop al processo scelto
	kill(pidChoice, SIGSTOP);
	//scrivo azione sul file di log
	sprintf(tmp,"Segnale SIGSTOP a processo %d\n", pidChoice);
	fflush(NULL);
	charsWritten = write(fdFailures, tmp, strlen(tmp));
	fflush(NULL);
	if(charsWritten != strlen(tmp)){
		printf("Errore scrittura file failures di sigstop\n");
	}
}
void sendINT(int pidChoice, int fdFailures){
	char tmp[50];
	int charsWritten;
	//invio segnale int al processo scelto
	kill(pidChoice, SIGINT);
	//scrivo azione sul file di log
	sprintf(tmp,"Segnale SIGINT a processo %d\n", pidChoice);
	fflush(NULL);
	charsWritten = write(fdFailures, tmp, strlen(tmp));
	fflush(NULL);
	if(charsWritten != strlen(tmp)){
		printf("Errore scrittura file failures di sigint\n");
	}
}
void sendCONT(int pidChoice, int fdFailures){
	char tmp[50];
	int charsWritten;
	//invio segnale cont al processo scelto
	kill(pidChoice, SIGCONT);
	//scrivo azione sul file di log
	sprintf(tmp,"Segnale SIGCONT a processo %d\n", pidChoice);
	fflush(NULL);
	charsWritten = write(fdFailures, tmp, strlen(tmp));
	fflush(NULL);
	if(charsWritten != strlen(tmp)){
		printf("Errore scrittura file failures di sigcont\n");
	}	
}
void sendUSR1(int pidChoice, int fdFailures){
	char tmp[50];
	int charsWritten;
	//invio segnale usr1 al processo scelto
	kill(pidChoice, SIGUSR1);
	//scrivo azione sul file di log
	sprintf(tmp,"Segnale SIGUSR1 a processo %d\n", pidChoice);
	fflush(NULL);
	charsWritten = write(fdFailures, tmp, strlen(tmp));
	fflush(NULL);
	if(charsWritten != strlen(tmp)){
		printf("Errore scrittura file failures di sigusr1\n");
	}	
}

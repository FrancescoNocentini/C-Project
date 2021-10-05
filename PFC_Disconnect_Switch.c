#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <fcntl.h>

void writePid1(int, int);
void writePid2(int, int);
void writePid3(int, int);
void getStatus(int, int, int, int, int);
void sigusr2(int sig);
bool readMessage = false;

int main(int argc, char *argv[]){	//in argv[0] nome processo, in argv[1] percorso G18.txt
	int pid[6], fdMsg, i, fdLog, charsWritten, charsRead, fdPid;
	char tmp[20], nameFile[] = "./transducers,./PFC1,./PFC2,./PFC3,./WES,./generatoreFallimenti,", *token = strtok(nameFile, ","), writeLog[60];
	
	signal(SIGUSR2, sigusr2);	//quando riceve SIGUSR1 gestiscilo con l'handler sigusr1
	
	//ottengo l'ora corrente e la metto nella struttura tm
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	
	//aggiungo un ritardo a piacimento per sincronizzare tutti i processi
	tm.tm_sec = tm.tm_sec + 2;	//voglio aggiungere 2 secondi
	if(tm.tm_sec > 59){	//se aggiungendo 2 secondi vado al minuto successivo
		tm.tm_min = tm.tm_min + 1;	//aumento il minuto
		if(tm.tm_min > 59){	//se aggiungendo il minuto vado all'ora successiva
			tm.tm_hour = tm.tm_hour + 1;	//aumento l'ora
			tm.tm_min = tm.tm_min - 60;	//aggiusto i minuti
		}
		tm.tm_sec = tm.tm_sec - 60;	//aggiusto i secondi
	}
	
	sprintf(tmp, "%d.%d.%d", tm.tm_hour, tm.tm_min, tm.tm_sec);	//creo la stringa con il tempo da passare ai figli
	printf("Orario avvio applicazione: %s\n", tmp);

	//apro file switch.log
	fdLog = open("./switch.log", O_WRONLY|O_CREAT, 0666);
	if(fdLog < 0){
		printf("Errore apertura switch.log da PFC_Disconnect_Switch\n");
	}
	
	//apro file msg.log
	fdMsg = open("./msg.log", O_RDONLY|O_CREAT, 0666);
	if(fdMsg < 0){
		printf("Errore apertura msg.log da PFC_Disconnect_Switch\n");
	}
	
	//apro file pid.txt
	fdPid = open("./pid.txt", O_CREAT|O_WRONLY, 0666);
	if(fdPid < 0){
		printf("Errore apertura pid.txt da PFC_Disconnect_Switch");
	}
	
	//creo i processi
	for(i = 0; i < 6; i++){
		pid[i] = fork();
		if(pid[i] == 0){	//figlio

			execl(token, token, tmp, argv[1] ,(char*) NULL);	//in argv[1] ho G18, alcuni lo useranno altri no
		}
		token = strtok(NULL, ",");	//prossimo elemento
		//scrivo il pid che mi interessa su pid.txt
		if(i == 1){	//per i=1 ho pid PFC1
			writePid1(fdPid, pid[i]);
		}else if(i == 2){	//i=2 ho pid PFC2
			writePid2(fdPid, pid[i]);
		}else if(i == 3){	//i=3 ho pid PFC3
			writePid3(fdPid, pid[i]);
		}
	}
	
	close(fdPid);

	do{
		do{
			usleep(5*1000);	//dormo 5ms fintanto che readMessage != true
		}while(readMessage == false);
		memset(tmp, 0x00, 20);
		//leggo il messaggio su msg.txt
		charsRead = read(fdMsg, tmp, 6);	//provo a leggere "Errore"

		if(charsRead != 6){
			printf("Errore lettura msg.txt da PFC_Disconnect_Switch\n");
		}
		if(strcmp(tmp,"ERRORE") == 0){	//se leggo errore
			//check status devo andare a leggere i pid del processo
			charsRead = read(fdMsg, tmp, 1);	//leggo lo spazio
			if(charsRead != 1){
				printf("Errore lettura msg.txt da PFC_Disconnect_Switch\n");
			}
			
			getStatus(fdMsg, pid[1], pid[2], pid[3], fdLog);
			
			charsRead = read(fdMsg, tmp, 1);	//leggo il carattere newline
			if(charsRead != 1){
				printf("Errore lettura msg.txt da PFC_Disconnect_Switch\n");
			}
			
		}else{	//messaggio emergenza
			strcpy(writeLog, "Ricevuto messaggio emergenza, termino l'applicazione\n");
			fflush(NULL);	//puntatore a NULL la scrittura avviene per tutti i files aperti in output
			charsWritten = write(fdLog, writeLog, strlen(writeLog));
			fflush(NULL);	//puntatore a NULL la scrittura avviene per tutti i files aperti in output
			if(charsWritten < 0){
				printf("Errore scrittura messaggio emergenza su switch.log\n");
			}
			for(i = 0; i < 6; i++){
				kill(pid[i], SIGINT);	//segnale interruzione a tutti i processi generati con la fork
			}
			close(fdMsg);
			exit(0);	//termina se stesso
		}
	}while(true);
	close(fdLog);
	return 0;
}

void sigusr2(int sig){
	readMessage = true;
}

void writePid1(int fdPid, int pid){
	int charsWritten;
	char pidChar[20];
	//scrivo lo stato di PFC1
	sprintf(pidChar, "%dPFC1\n", pid);
	fflush(NULL);
	charsWritten = write(fdPid, pidChar, strlen(pidChar));
	fflush(NULL);
	if(charsWritten < strlen(pidChar)){
		printf("Errore scrittura pid PFC1 su pid.txt da PFC_Disconnect_Switch\n");
	}
}
void writePid2(int fdPid, int pid){
	int charsWritten;
	char pidChar[20];
	//scrivo lo stato di PFC2
	sprintf(pidChar, "%dPFC2\n", pid);
	fflush(NULL);
	charsWritten = write(fdPid, pidChar, strlen(pidChar));
	fflush(NULL);
	if(charsWritten < strlen(pidChar)){
		printf("Errore scrittura pid PFC3 su pid.txt da PFC_Disconnect_Switch\n");
	}
}
void writePid3(int fdPid, int pid){
	int charsWritten;
	char pidChar[20];
	//scrivo lo stato di PFC3
	sprintf(pidChar, "%dPFC3\n", pid);
	fflush(NULL);
	charsWritten = write(fdPid, pidChar, strlen(pidChar));
	fflush(NULL);
	if(charsWritten < strlen(pidChar)){
		printf("Errore scrittura pid PFC3 su pid.txt da PFC_Disconnect_Switch\n");
	}
}

void getStatus(int fdMsg, int pid1, int pid2, int pid3, int fdLog){
		int charsRead, charsWritten, pidMsg;
		char tmp[20], buffer[10], writeLog[60];
		FILE *fp;
		
		charsRead = read(fdMsg, tmp, 4);	//leggo il nome del processo che ha generato l'errore
		if(charsRead != 4){
			printf("Errore lettura msg.txt da PFC_Disconnect_Switch\n");
		}
		//con il nome del processo posso prendere il suo pid
		if(strncmp(tmp,"PFC1", 4) == 0){	//ho letto PFC1, comparo al piu' i primi 4 caratteri, ad un certo punto ne legge uno un piu'
			pidMsg = pid1;
		}else if(strncmp(tmp,"PFC2", 4) == 0){	//ho letto PFC2, comparo al piu' i primi 4 caratteri, ad un certo punto ne legge uno un piu
			pidMsg = pid2;
		}else{	//ho letto PFC3
			pidMsg = pid3;
		}
		//con il pid in pidMsg posso andare a controllare lo stato
		sprintf(tmp, "/proc/%d/status", pidMsg);
		fp = fopen(tmp, "r");
		if(fp == NULL){
			printf("Errore apertura proc/pid/status da PFC_Disconnect_Switch\n");
		}
		do{
			fscanf(fp, "%s%s\n", tmp, buffer);	//in tmp ho state:, in buffer ho S,T o altro
		}while(strcmp(tmp, "State:") != 0);	//fino a che non leggo state
		
		//scrivo state: poi lo stato del processo ed il suo pid su switch.log
		sprintf(writeLog, "%s %s %d\n", tmp, buffer, pidMsg);
		fflush(NULL);
		charsWritten = write(fdLog, writeLog, strlen(writeLog));
		fflush(NULL);
		if(charsWritten < 0){
			printf("Errore scrittura su switch.log da PFC_Disconnect_Switch\n");
		}
		fclose(fp);
		readMessage = false;
}

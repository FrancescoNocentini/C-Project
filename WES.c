#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#include<fcntl.h>
#include<sys/unistd.h>
#include<stdbool.h>
#include <signal.h>
#include <time.h>

double getPFC1(int *, int *, bool *);
double getPFC2(int *, int *, bool *);
double getPFC3(int *, int *, bool *);


void errPFC1(int);
void errPFC2(int);
void errPFC3(int);
void emerg(int);
void ok(int);

int main(int argc, char *argv[]){
	int fdLog, numeroLineaLetto1 = 0, numeroLineaLetto2 = 0, numeroLineaLetto3 = 0;
	bool start = false, checkEOF1 = false, checkEOF2 = false, checkEOF3 = false;
	char tmp[20],line1[64], line2[64], line3[64];
	double d1, d2, d3;
	
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
	
	//apro status.log dove registra quello che fa
	fdLog = open("status.log", O_WRONLY|O_CREAT, 0666);
	if(fdLog < 0){
		printf("Errore apertura file fdLog da WES\n");
	}

	do{
		sleep(1);
		int currentLine1 = 0;
		int currentLine2 = 0;
		int currentLine3 = 0;
		checkEOF1 = false;
		checkEOF2 = false;
		checkEOF3 = false;
		memset(line1, 0x00, 64);
		memset(line2, 0x00, 64);
		memset(line2, 0x00, 64);
		
		d1 = getPFC1(&currentLine1, &numeroLineaLetto1, &checkEOF1);	//leggo speed
		sprintf(line1, "%f", d1);	//converto speed in stringa
		
		d2 = getPFC2(&currentLine2, &numeroLineaLetto2, &checkEOF2);	//leggo speed
		sprintf(line2, "%f", d2);	//converto speed in stringa
		
		d3 = getPFC3(&currentLine3, &numeroLineaLetto3, &checkEOF3);	//leggo speed
		sprintf(line3, "%f", d3);	//converto speed in stringa

		if(!checkEOF1 || !checkEOF2 || !checkEOF3){	//ho letto un valore nuovo in almeno uno speed.log
			//confronto quello che ho letto
			if(strcmp(line2,line3) == 0 && strcmp(line1,line2) != 0){	//speedPFC1 discorde
				errPFC1(fdLog);
			}else if(strcmp(line1,line3) == 0 && strcmp(line2,line3) != 0){	//speedPFC2 discorde
				errPFC2(fdLog);
			}else if(strcmp(line1,line2) == 0 && strcmp(line3,line1) != 0){	//speedPFC3 discorde
				errPFC3(fdLog);
			}else if(strcmp(line1,line2) != 0 && strcmp(line2,line3) != 0){	//tutti discordi
				emerg(fdLog);
			}else if(strcmp(line1,line2) == 0 && strcmp(line2,line3) == 0){
				ok(fdLog);
			}
		}else if(checkEOF1 && checkEOF2 && checkEOF3){	//tutti a fine file, nessun valore nuovo letto
			//printf("No confronto, tutti EOF\n");
		}
	}while(true);
	close(fdLog);
	return 0;
}

void errPFC1(int fdLog){
	int fdMsgLog, charsWritten;
	char strErr1[38] = "ERRORE PFC1\n";
	
	printf("ERRORE PFC1\n");
	fflush(NULL);	//puntatore a NULL la scrittura avviene per tutti i files aperti in output
	charsWritten = write(fdLog, strErr1, strlen(strErr1));	//scrivo messaggio errore su file log
	fflush(NULL);	//puntatore a NULL la scrittura avviene per tutti i files aperti in output
	if(charsWritten < 0){
		printf("Errore scrittura strErr1\n");
	}
	//apro file che deve leggere PFC_Disconnect_Switch
	fdMsgLog = open("msg.log", O_WRONLY|O_CREAT|O_APPEND, 0666);
	if(fdMsgLog < 0){
		printf("Errore apertura file msg.txt da WES\n");
	}
	fflush(NULL);	//puntatore a NULL la scrittura avviene per tutti i files aperti in output
	charsWritten = write(fdMsgLog, strErr1, strlen(strErr1));	//scrivo messaggio errore su file msg.txt
	fflush(NULL);	//puntatore a NULL la scrittura avviene per tutti i files aperti in output
	if(charsWritten < 0){
		printf("Errore scrittura strErr1 su msg.txt\n");
	}
	close(fdMsgLog);
	
	kill(getppid(), SIGUSR2);	//mando segnale SIGUSR2 al padre PFC_Disconnect_Switch
}

void errPFC2(int fdLog){
	int fdMsgLog, charsWritten;
	char strErr2[38] = "ERRORE PFC2\n";
	
	printf("ERRORE PFC2\n");
	fflush(NULL);	//puntatore a NULL la scrittura avviene per tutti i files aperti in output
	charsWritten = write(fdLog, strErr2, strlen(strErr2));	//scrivo messaggio errore su file log
	fflush(NULL);	//puntatore a NULL la scrittura avviene per tutti i files aperti in output
	if(charsWritten < 0){
		printf("Errore scrittura strErr2\n");
	}
	fdMsgLog = open("msg.log", O_WRONLY|O_CREAT|O_APPEND, 0666); //apro file msg.log in sola scrittura, lo creo se non esiste
	if(fdMsgLog < 0){
		printf("Errore apertura file msg.txt da WES\n");
	}
	fflush(NULL);	//puntatore a NULL la scrittura avviene per tutti i files aperti in output
	charsWritten = write(fdMsgLog, strErr2, strlen(strErr2));	//scrivo messaggio errore su file msg.txt
	fflush(NULL);	//puntatore a NULL la scrittura avviene per tutti i files aperti in output
	if(charsWritten < 0){
		printf("Errore scrittura strErr2 su msg.txt\n");
	}
	close(fdMsgLog);
	
	kill(getppid(), SIGUSR2);	//mando segnale SIGUSR2 al padre PFC_Disconnect_Switch
}

void errPFC3(int fdLog){
	int fdMsgLog, charsWritten;
	char strErr3[38] = "ERRORE PFC3\n";
	
	printf("ERRORE PFC3\n");
	fflush(NULL);	//puntatore a NULL la scrittura avviene per tutti i files aperti in output
	charsWritten = write(fdLog, strErr3, strlen(strErr3));	//scrivo messaggio errore su file log
	fflush(NULL);	//puntatore a NULL la scrittura avviene per tutti i files aperti in output
	if(charsWritten < 0){
		printf("Errore scrittura strErr2\n");
	}
	fdMsgLog = open("msg.log", O_WRONLY|O_CREAT|O_APPEND, 0666); //apro file msg.log in sola scrittura, lo creo se non esiste
	if(fdMsgLog < 0){
		printf("Errore apertura file msg.txt da WES\n");
	}
	fflush(NULL);	//puntatore a NULL la scrittura avviene per tutti i files aperti in output
	charsWritten = write(fdMsgLog, strErr3, strlen(strErr3));	//scrivo messaggio errore su file msg.txt
	fflush(NULL);	//puntatore a NULL la scrittura avviene per tutti i files aperti in output
	if(charsWritten < 0){
		printf("Errore scrittura strErr3 su msg.txt\n");
	}
	close(fdMsgLog);
	
	kill(getppid(), SIGUSR2);	//mando segnale SIGUSR2 al padre PFC_Disconnect_Switch
}

void emerg(int fdLog){
	int fdMsgLog, charsWritten;
	char strEmerg[35] = "EMERGENZA\n";
	
	printf("EMERGENZA\n");
	fflush(NULL);	//puntatore a NULL la scrittura avviene per tutti i files aperti in output
	charsWritten = write(fdLog, strEmerg, strlen(strEmerg));	//scrivo messaggio errore su file log
	fflush(NULL);	//puntatore a NULL la scrittura avviene per tutti i files aperti in output
	if(charsWritten < 0){
		printf("Errore scrittura strEmerg\n");
	}
	fdMsgLog = open("msg.log", O_WRONLY|O_CREAT|O_APPEND, 0666); //apro file msg.log in sola scrittura, lo creo se non esiste
	if(fdMsgLog < 0){
		printf("Errore apertura file msg.txt da WES\n");
	}
	fflush(NULL);	//puntatore a NULL la scrittura avviene per tutti i files aperti in output
	charsWritten = write(fdMsgLog, strEmerg, strlen(strEmerg));	//scrivo messaggio errore su file msg.txt
	fflush(NULL);	//puntatore a NULL la scrittura avviene per tutti i files aperti in output
	if(charsWritten < 0){
		printf("Errore scrittura strEmerg su msg.txt\n");
	}
	close(fdMsgLog);
	
	kill(getppid(), SIGUSR2);	//mando segnale SIGUSR2 al padre PFC_Disconnect_Switch
}

void ok(int fdLog){
	int charsWritten;
	char strOk[23] = "OK\n";
	
	printf("OK\n");
	fflush(NULL);	//puntatore a NULL la scrittura avviene per tutti i files aperti in output
	charsWritten = write(fdLog, strOk, strlen(strOk));	//scrivo messaggio errore su file log
	fflush(NULL);	//puntatore a NULL la scrittura avviene per tutti i files aperti in output
	if(charsWritten < 0){
		printf("Errore scrittura strOk\n");
	}
}


double getPFC1(int *currentLine1, int *numeroLineaLetto1, bool *checkEOF1){
		//leggo speed in speedPFC1.log
		FILE* file1;
		bool speed1 = false;
		char const* fileName1 = "./speedPFC1.log";
		char line1[64];
		double speed;
		
		file1 = fopen(fileName1, "a+"); //APERTURA FILE
		if(file1 == NULL){
			printf("Errore apertura speedPFC1.log da WES\n");
		}
		
		while(speed1 == false){ //LEGGE FILE
			fgets(line1, sizeof(line1), file1);		//leggo una riga
			
			if(*currentLine1 < *numeroLineaLetto1){	//e' una riga che ho gia' letto
				*currentLine1 = *currentLine1 + 1;
			}else if(feof(file1)){	//ho letto ma sono a fine file, niente di nuovo da leggere
				//printf("EOF1\n");
				speed1 = true;
				*checkEOF1 = true;	//sono arrivato EOF
				strcpy(line1,"999999.9");	//assegno stringa a caso, se gli altri sono andati avanti significa che sono rimasto indietro e devo generare un errore
				sscanf(line1, "%lf", &speed);	//lo converto in double mettendolo in speed
			}else{	//sono arrivato alla riga che voglio leggere
				//printf("Letto speedPFC1: %s\n", line1);
				*numeroLineaLetto1 = *numeroLineaLetto1 + 1;
				speed1 = true;
				sscanf(line1, "%lf", &speed);	//lo converto in double mettendolo in speed
			}
		} 
		fclose(file1);
		return speed;
}

double getPFC2(int *currentLine2, int *numeroLineaLetto2, bool *checkEOF2){
		//leggo speed in speedPFC2.log
		FILE* file2;
		bool speed2 = false;
		char const* fileName2 = "./speedPFC2.log";
		char line2[64];
		double speed;
		
		file2 = fopen(fileName2, "a+"); //APERTURA FILE
		if(file2 == NULL){
			printf("Errore apertura speedPFC2.log da WES\n");
		}
		
		while(speed2 == false){ //LEGGE FILE
			fgets(line2, sizeof(line2), file2);		//leggo una riga
			
			if(*currentLine2 < *numeroLineaLetto2){	//e' una riga che ho gia' letto
				*currentLine2 = *currentLine2 + 1;
			}else if(feof(file2)){	//ho letto ma sono a fine file, niente di nuovo da leggere
				//printf("EOF2\n");
				speed2 = true;
				*checkEOF2 = true;	//sono arrivato EOF
				strcpy(line2,"888888.8");	//assegno stringa a caso, se gli altri sono andati avanti significa che sono rimasto indietro e devo generare un errore
				sscanf(line2, "%lf", &speed);	//lo converto in double mettendolo in speed
			}else{	//sono arrivato alla riga che voglio leggere
				//printf("Letto speedPFC2: %s\n", line2);
				*numeroLineaLetto2 = *numeroLineaLetto2 + 1;
				speed2 = true;
				sscanf(line2, "%lf", &speed);	//lo converto in double mettendolo in speed
			}
		} 
		fclose(file2);
		return speed;
}	
	
double getPFC3(int *currentLine3, int *numeroLineaLetto3, bool *checkEOF3){
		//leggo speed in speedPFC3.log
		FILE* file3;
		bool speed3 = false;
		char const* fileName3 = "./speedPFC3.log";
		char line3[64];
		double speed;
		
		file3 = fopen(fileName3, "a+"); //APERTURA FILE
		if(file3 == NULL){
			printf("Errore apertura speedPFC3.log da WES\n");
		}
		
		while(speed3 == false){ //LEGGE FILE
			fgets(line3, sizeof(line3), file3);		//leggo una riga
			
			if(*currentLine3 < *numeroLineaLetto3){	//e' una riga che ho gia' letto
				*currentLine3 = *currentLine3 + 1;
			}else if(feof(file3)){	//ho letto ma sono a fine file, niente di nuovo da leggere
				//printf("EOF3\n");
				speed3 = true;
				*checkEOF3 = true;	//sono arrivato EOF
				strcpy(line3,"777777.7");	//assegno stringa a caso, se gli altri sono andati avanti significa che sono rimasto indietro e devo generare un errore
				sscanf(line3, "%lf", &speed);	//lo converto in double mettendolo in speed
			}else{	//sono arrivato alla riga che voglio leggere
				//printf("Letto speedPFC3: %s\n", line3);
				*numeroLineaLetto3 = *numeroLineaLetto3 + 1;
				speed3 = true;
				sscanf(line3, "%lf", &speed);	//lo converto in double mettendolo in speed
			}
		} 
		fclose(file3);
		return speed;
}

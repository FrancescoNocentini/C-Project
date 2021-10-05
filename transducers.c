#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/unistd.h>
#include <sys/stat.h>

void readFile(int, int *, int *);

int main(int argc, char *argv[]){
	int fdLog1, clientFd, serverLen, result, fdPipe, fdLog2, fdLog3, charsRead, charsWrite, numeroLineaLetto = 0, currentLine = 0;
	char tmp[14], buffer[20];
	bool start = false;
	
	//ciclo per sincronizzare i processi
	do{
		time_t t = time(NULL);
		struct tm tm = *localtime(&t);
		sprintf(tmp, "%d.%d.%d", tm.tm_hour, tm.tm_min, tm.tm_sec);
		if(strcmp(tmp, argv[1]) == 0){
			start = true;
		}else{
			sleep(1);	
		}
	}while(start == false);
	
	//apro speedPFC1.log
	fdLog1 = open("speedPFC1.log", O_CREAT|O_WRONLY, 0666);
	if(fdLog1 < 0){
		printf("Errore apertura speePFC1.log da transducers\n");
	}
	//apro speedPFC2.log
	fdLog2 = open("speedPFC2.log", O_CREAT|O_WRONLY, 0666);
	if(fdLog2 < 0){
		printf("Errore apertura speedPFC2.log da transducers\n");
	}
	
	//apro speedPFC3.log
	fdLog3 = open("speedPFC3.log", O_CREAT|O_WRONLY, 0666);
	if(fdLog3 < 0){
		printf("Errore apertura speedPFC3.log da transducers\n");
	}
	
	//creo socket
	struct sockaddr_un serverUNIXAddress;
	struct sockaddr* serverSockAddrPtr;
	serverSockAddrPtr = (struct sockaddr*) &serverUNIXAddress;
	serverLen = sizeof(serverUNIXAddress);
	
	clientFd = socket(AF_UNIX, SOCK_STREAM, 0);
	serverUNIXAddress.sun_family = AF_UNIX;
	strcpy(serverUNIXAddress.sun_path, "socketPFC1");
	
	do{
		result = connect(clientFd, serverSockAddrPtr, serverLen);
		if(result == -1){	//se fallisce riprovo tra 5ms
			usleep(5*1000);
		}
	}while(result == -1);
	
	//creo pipe non bloccante
	unlink("pipePFC2");
	mknod("pipePFC2", S_IFIFO, 0);
	chmod("pipePFC2", 0660);
	fdPipe = open("pipePFC2", O_RDONLY|O_NONBLOCK);	//ha successo anche se non aperta in scrittura 

	do{
		usleep(5*1000);	//dormo 5ms

		//lettura socket
		charsRead = read(clientFd, tmp, strlen(tmp));
		if(charsRead <= 1){	//se non legge niente restituisce comunque 1
			//printf("Socket non ha letto niente\n");
		}else{
			//scrittura su file
			sprintf(buffer, "%s\n", tmp);
			fflush(NULL);
			charsWrite = write(fdLog1, buffer, strlen(buffer));
			fflush(NULL);
			if(charsWrite != strlen(buffer)){
				printf("Scrittura socket su file fallita\n");
			}else{
				//printf("Letto socket\n");
			}
		}
		
		//lettura pipe
		charsRead = read(fdPipe, tmp, strlen(tmp));
		if(charsRead <= 1){
			//printf("Pipe non ha letto niente %d\n",charsRead);
		}else{
			sprintf(buffer, "%s\n", tmp);
			fflush(NULL);
			charsWrite = write(fdLog2, buffer, strlen(buffer));
			fflush(NULL);
			if(charsWrite != strlen(buffer)){
				printf("Scrittura pipe su file fallita\n");
			}else{
				//printf("Letto pipe\n");
			}
		}
		currentLine = 0;
		readFile(fdLog3, &currentLine, &numeroLineaLetto); 
	
	}while(true);
	close(fdLog1);
	close(fdLog2);
	close(fdLog3);
	close(clientFd);
	close(fdPipe);
	unlink("pipePFC2");

	return 0;
}

void readFile(int fdLog3, int *currentLine, int *numeroLineaLetto){
	int charsWriteFile;
	char line[64];
	char const* fileName = "./speed.txt";
	bool speed = false;
	FILE* file;

	file = fopen(fileName, "r+"); //Apertura file

	if(file == NULL){
		printf("Errore apertura speed.txt da transducers\n");
	}
	
	while(speed == false){ //Legge file
		fgets(line, sizeof(line), file);		//leggo una riga
		if(*currentLine < *numeroLineaLetto){	//e' una riga che ho gia' letto
			*currentLine = *currentLine + 1;
		}else if(feof(file)){	//ho letto ma sono a fine file, niente di nuovo da leggere
			//printf("File non ha letto niente\n");
			speed = true;
		}else{	//sono arrivato alla riga che voglio leggere
			fflush(NULL);
			charsWriteFile = write(fdLog3, line, strlen(line));
			fflush(NULL);
			*numeroLineaLetto = *numeroLineaLetto + 1;
			if(charsWriteFile != strlen(line)){
				printf("Scrittura file su file fallita\n");
			}else{
				//printf("Letto file\n");
			}
			speed = true;
		}
	} 
	fclose(file);	
}

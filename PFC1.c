#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/types.h>
#include <time.h>
#include <signal.h>

#define _GNU_SOURCE

double degreesToRadians(double);
double distanceInMetres(double, double, double, double);
double getLatitude(int);
double getLongitude(int);
double computeSpeed(int, double*, double*);
void sigusr1(int);
bool shifting = false;

int main(int argc, char *argv[]){

	int fd, charsRead, serverFd, clientFd, serverLen, clientLen, nWrite;
	double latitudeOld, longitudeOld, speed = 0;
	char buffer [10], tmp[14], *gpgll = "$GPGLL,";
	bool start = false;
	latitudeOld = 0;
	longitudeOld = 0;
	
	signal(SIGUSR1, sigusr1);	//quando riceve SIGUSR1 gestiscilo con l'handler sigusr1
	
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
	
	//apro G18.txt
	fd = open(argv[2], O_RDONLY, 0666);
	if( fd < 0){	//se apertura fallisce
		printf("Errore apertura file G18 da PFC1\n");
		exit(1);
	}
	
	//creo socket
	struct sockaddr_un serverUNIXAddress;
	struct sockaddr_un clientUNIXAddress;
	
	struct sockaddr* serverSockAddrPtr;
	struct sockaddr* clientSockAddrPtr;
	
	serverSockAddrPtr = (struct sockaddr*) &serverUNIXAddress;
	serverLen = sizeof(serverUNIXAddress);
	
	clientSockAddrPtr = (struct sockaddr*) &clientUNIXAddress;
	clientLen = sizeof(clientUNIXAddress);
	
	serverFd = socket(AF_UNIX, SOCK_STREAM, 0);
	serverUNIXAddress.sun_family = AF_UNIX;
	strcpy(serverUNIXAddress.sun_path, "socketPFC1");
	unlink("socketPFC1");
	
	bind(serverFd, serverSockAddrPtr, serverLen);
	listen(serverFd, 1);	//1 sola richiesta in coda, solo il transducers si connette con PFC1

	//clientFd = accept(serverFd, clientSockAddrPtr, &clientLen);	//si blocca in attesa della connect
	clientFd = accept4(serverFd, clientSockAddrPtr, &clientLen, SOCK_NONBLOCK);	//si blocca in attesa della connect
	
	do{
		sleep(1);
		bool find = false;
		do{
			charsRead = read(fd,buffer,7); //leggo primi 7 caratteri per capire in che riga sono
			if(strcmp(buffer, gpgll) == 0){	//se ho letto $GPGLL,
				//calcolo la velocita'
				speed = computeSpeed(fd, &longitudeOld, &latitudeOld);
				
				//invia la speed tramite socket
				sprintf(tmp, "%f", speed);
				fflush(NULL);	//puntatore a NULL la scrittura avviene per tutti i files aperti in output
				nWrite = write(clientFd, tmp, strlen(tmp));
				fflush(NULL);	//puntatore a NULL la scrittura avviene per tutti i files aperti in output
				if(nWrite == -1){
					printf("Errore scrittura socket da parte di PFC1\n");
				}
				do{
					charsRead = read(fd, buffer, 1);
				}while(buffer[0] != '\n');	//leggo fino alla riga successiva
				find = true;	
			}else{
				do{
					charsRead = read(fd, buffer, 1);
				}while(buffer[0] != '\n');	//leggo fino alla riga successiva
			}
		}while(!find);
		
	}while(charsRead != 0); //sono arrivato a fine file
	fflush(NULL);	//puntatore a NULL la scrittura avviene per tutti i files aperti in output
	close(fd);
	close(clientFd);

	return 0;
}

double computeSpeed(int fd, double *longitudeOld, double *latitudeOld){
	double latitudeNew, longitudeNew, distance;
	int shiftedSpeed;

	latitudeNew = getLatitude(fd);
	longitudeNew = getLongitude(fd);
	// computa la speed calcolando la distanza che coincide con la velocitò, v = s / t ed il tempo è di 1s

	if(*longitudeOld != 0 && *latitudeOld != 0){	//se non è la prima volta che calcolo la speed
		distance = distanceInMetres(*latitudeOld, *longitudeOld, latitudeNew, longitudeNew);
	}else{	//prima volta che calcolo speed allora deve essere 0
		distance = 0;
	}
	if(shifting == true){	//la variabile globale e' true se ho ricevuto un SIGUSR1
		shifting = false;
		//devo shiftare a sinistra di 2 bit, prima converto valore ad intero
		shiftedSpeed = (int) distance;
		shiftedSpeed = shiftedSpeed << 2;
		distance = shiftedSpeed;
	}
	
	*latitudeOld = latitudeNew;
	*longitudeOld = longitudeNew;
	
	return distance;

}

double getLatitude(int fd){
	//leggo latitudine
	bool test = false;
	int i = 0, j = 0, charsRead, latitude[10];
	char buffer[10];
	double latitudeNew = 0;
	
	do{
		charsRead = read(fd,buffer,1);	//leggo un carattere della latitudine
		if(buffer[0] != ',' && buffer[0] != '.'){
			latitude[i] = atoi(buffer);	//inserisco il carattere letto nell'array latitudine
			i++;
		}else if(buffer[0] != ',' && buffer[0] == '.'){
			//se trovo un . non faccio niente
		}else{
			test = true;
		}
	}while(test == false);	//leggo la latitudine

	for(i=0;i<8;i++){
		j = pow(10,7-i);	//potenze per convertirlo nell'intero giusto
		latitudeNew = latitudeNew + latitude[i] * j;	//moltiplico il valore dentro l'array per la sua potenza di 10 corretta (in base alla posizione) e poi lo sommo al precedente valore di latitudeNew
	}
	charsRead = read(fd,buffer,1);	//leggo N o S
	if(buffer[0] == 'S'){	//se leggo S devo invertire il segno
		latitudeNew = latitudeNew * (-1);
	}

	j = pow(10,6);	//converto latitudeNew nella notazione che mi serve per calcolare la velocità
	latitudeNew = latitudeNew / j;
	
	charsRead = read(fd,buffer,1); //leggo la , dopo N o S
	return latitudeNew;
}

double getLongitude(int fd){
	//leggo longitudine
	bool test = false;
	int i = 0, j = 0, charsRead, longitude[10];
	double longitudeNew = 0;
	char buffer[10];
	
	do{
		charsRead = read(fd,buffer,1);	//leggo un carattere della longitudine
		if(buffer[0] != ',' && buffer[0] != '.'){
			longitude[i] = atoi(buffer);	//inserisco il carattere letto nell'array longitudine
			i++;
		}else if(buffer[0] != ',' && buffer[0] == '.'){
			//se trovo un . non faccio niente
		}else{
			test = true;
		}
	}while(test == false);	//leggo la longitudine
	
	for(i=0;i<9;i++){
		j = pow(10,8-i);	//potenze per convertirlo nell'intero giusto
		longitudeNew = longitudeNew + longitude[i] * j;	//moltiplico il valore dentro l'array per la sua potenza di 10 corretta (in base alla posizione) e poi lo sommo al precedente valore di longitudeNew
	}
	charsRead = read(fd,buffer,1);	//leggo E o W
	if(buffer[0] == 'W'){	//se leggo W devo invertire il segno
		longitudeNew = longitudeNew * (-1);
	}

	j = pow(10,6);	//converto longitudeNew nella notazione che mi serve per calcolare la velocità
	longitudeNew = longitudeNew / j;

	charsRead = read(fd,buffer,1); //leggo la , dopo E o W
	return longitudeNew;
}


double degreesToRadians(double degrees){
	return degrees * M_PI / 180;
}
double distanceInMetres(double latitudeOld, double longitudeOld, double latitudeNew, double longitudeNew){
	double raggioTerra = 6368000;	//approssimato, a  genova raggio in km 6367.863
	double dLat, dLon, latOld, latNew, a, c;
	dLat = degreesToRadians(latitudeNew - latitudeOld);
	dLon = degreesToRadians(longitudeNew - longitudeOld);
	latOld = degreesToRadians(latitudeOld);
	latNew = degreesToRadians(latitudeNew);
	a = sin(dLat/2) * sin(dLat/2) + sin(dLon/2) * sin(dLon/2) * cos(latOld) * cos(latNew);
	c = 2 * atan2(sqrt(a), sqrt(1-a));
	return raggioTerra * c;
}
void sigusr1(int sig){
	shifting = true;
}

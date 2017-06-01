/* Power Board test program */

#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <errno.h>
#include "pbif.h"

using namespace std;

#define BOARD_NAME_LEN 128

typedef struct options_s {
	char board[BOARD_NAME_LEN];
	bool readState;
	int IthN;
	float IthVal;
	float Vbias;
	int VoutN;
	float VoutVal;
	int storeVout;
	bool storeAllVout;
	bool restoreAllVout;
	bool on;
} options_t;
options_t OPTIONS;


void dump(unsigned char *buffer, int size)
{
	int i, j;
	
	for (i=0; i<size;){
		for (j=0; j<16; j++){
			printf(" %02x", buffer[i]);
			i++;
		}
		printf("\n");
	}
}

void print_help()
{
	printf("pbctrl [options] board_address\n"
		"\n"
		"Options list:\n"
		"\t -state\n"
		"\t -Ith <channel> <value[V]>\n"
		"\t -Vbias <value[V]> - Note: MUST be negative\n"
		"\t -Vout <channel> <value[V]>\n"
		"\t -store <channel>\n"
		"\t -storeall\n"
		"\t -restoreall\n"
		"\t -on\n"
		"\n"
	);
}

/*
	Read options from command line
	return -1 if error, 0 if OK
*/
int readopt(int argc, char *argv[])
{
	int pc=1;

	if (argc<3)
		return -1;

	errno=0;			//  needed for strto* functions
	
	OPTIONS.readState = false;
	OPTIONS.IthN = -1;
	OPTIONS.Vbias = 1;
	OPTIONS.VoutN = -1;
	OPTIONS.storeVout = -1;
	OPTIONS.storeAllVout = false;
	OPTIONS.restoreAllVout = false;
	OPTIONS.on = false;
	
	while (pc<argc){
		if (strcmp(argv[pc], "-state")==0){
			OPTIONS.readState=true;
		} else if (strcmp(argv[pc], "-Ith")==0){
			if (pc>=(argc-2))
				return -1;
			OPTIONS.IthN = strtol(argv[++pc], NULL, 10);
			OPTIONS.IthVal = strtof(argv[++pc], NULL);
			if (OPTIONS.IthN<0 || OPTIONS.IthN>7 || OPTIONS.IthVal<0)
				return -1; 
		} else if (strcmp(argv[pc], "-Vbias")==0){
			if (pc>=(argc-1))
				return -1;
			OPTIONS.Vbias = strtof(argv[++pc], NULL);
			if (OPTIONS.Vbias>0)
				return -1;
		} else if (strcmp(argv[pc], "-Vout")==0){
			if (pc>=(argc-2))
				return -1;
			OPTIONS.VoutN = strtol(argv[++pc], NULL, 10);
			OPTIONS.VoutVal = strtof(argv[++pc], NULL);
			if (OPTIONS.VoutN<0 || OPTIONS.VoutN>7 || OPTIONS.VoutVal<0)
				return -1; 
		} else if (strcmp(argv[pc], "-store")==0){
			if (pc>=(argc-1))
				return -1;
			OPTIONS.storeVout = strtol(argv[++pc], NULL, 10);
			if (OPTIONS.storeVout<0)
				return -1;
		} else if (strcmp(argv[pc], "-storeall")==0){
			OPTIONS.storeAllVout = true;
		} else if (strcmp(argv[pc], "-restoreall")==0){
			OPTIONS.restoreAllVout = true;
		} else if (strcmp(argv[pc], "-on")==0){
			OPTIONS.on = true;
		} else {
			break;
		}
		pc++;
	}
	if (pc>=argc || argv[pc][0] == '-')
		return -1;

	strncpy(OPTIONS.board, argv[pc], BOARD_NAME_LEN);
	return 0;
}

void printState(powerboard::pbstate_t *pbStat)
{
	printf("\nPower board state:\n");

	for (int i=0; i<NUM_TSENSOR; i++)
		printf("T[%d]:%5.1f ", i, pbStat->T[i]);
	printf("\n");

	for (int i=0; i<8; i++){
		printf("CH%d:%s ", i, (pbStat->chOn & 1<<i) ? "Off":"ON ");
		printf("Vset:%4.2f ", pbStat->Vout[i]); 
		printf("Vmon:%4.2f ", pbStat->Vmon[i]); 
		printf("Imon:%5.3f\n", pbStat->Imon[i]); 
	}	

	printf("\n\n");
}


int main(int argc, char**argv)
{
	powerboard::pbstate pbStat;

	if (readopt(argc, argv)<0){
		print_help();
		exit(0);
	}

	try {
		PBif *board = new PBif(OPTIONS.board);
		powerboard *pb = board->pb;

		// check board connection
		if (!pb->isReady()){
			printf("Power board unconnected or off\n");
			exit(0);
		}

		if (OPTIONS.IthN>=0)
			pb->setIth(OPTIONS.IthN, OPTIONS.IthVal);
		if (OPTIONS.Vbias<=0){
			pb->setVbias(OPTIONS.Vbias);
			if (OPTIONS.Vbias!=0.0)
				pb->enVbias(true);
			else
				pb->enVbias(false);
		}
		if (OPTIONS.VoutN>=0)
			pb->setVout(OPTIONS.VoutN, OPTIONS.VoutVal);
		if (OPTIONS.storeVout>=0)
			pb->storeVout(OPTIONS.storeVout);
		if (OPTIONS.storeAllVout)
			pb->storeAllVout();
		if (OPTIONS.restoreAllVout)
			pb->restoreAllVout();
		if (OPTIONS.on)
			pb->onAllVout();
	
		if (OPTIONS.readState){
			pb->getState(&pbStat);
			printState(&pbStat);
		}

		delete board;

	} catch (std::exception& e) {	
		cout << e.what() << endl;
	}

	return 0;
}

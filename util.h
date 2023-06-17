//#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/select.h>
#include <errno.h>
#include <pthread.h>

#define TAM 10
#define FUSERS "fusers.txt"
#define FITEMS "fitems.txt"
#define FPROMOTERS "fpromoters.txt"
#define BACK_FIFO "BACKFIFO"
#define BACK_FIFO2 "BACKFIFO2"
#define BACK_FIFO3 "BACKFIFO3"
#define BACK_FIFO4 "BACKFIFO4"
#define CLIENT_FIFO "CLIENTE%d"

typedef struct Item item, *item_ptr;
struct Item{
    int id;
    char name[50]; 
    char catg[50]; 
    int value; 
    int buyN; 
    int time; 
    char userOwner[50]; 
    char userLbuyer[50];
    char msg[50];
    int lastBid;
    
    
    pid_t cli_pid;
};

typedef struct User user, *user_ptr;
struct User{
  
    char nome[50];
    char pass[50];
    char msg[50];
    int saldo;

    pid_t pid;
     
};


typedef struct backendCliente backCli, *backCli_ptr;
struct backendCliente{
  
  int *numCli;
  int maxCli;
  int *numItens;
  int maxItens;
  int *numPromo;
  int maxPromo;
  int stop;

  int **pipe_arr;
  
  pid_t* prom_pids;

  char** promo_names;

  user_ptr array_clientes;

  item_ptr array_itens;

  pthread_mutex_t *trinco;
};

typedef struct licitacao licit, *licit_ptr;
struct licitacao{
  int id;
  int valor;
  char buyerName[50];
  char msg[50];

  pid_t pid;
};

typedef struct informacao info, *info_ptr;
struct informacao{
  pid_t pid;

  int valor;
  int prazo;
  int time;

  char msg[20];
  char userName[20];
  char categoria[20];
  char buffer[1024];

};



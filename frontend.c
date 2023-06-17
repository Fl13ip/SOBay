#include "util.h"
#include "users_lib.h" 

int is_number(char const* arg)
{
   int n;
   return (sscanf(arg, "%d", &n) == 1);
}

int max(int a, int b, int c) {
  if(a >= b && a >= c)
    return a;
    
  if(b >= a && b >= c)
    return b;
    
  if(c >= a && c >= b)
    return c;
  
  return 0;
}


int main(int argc, char *argv[]){
    int i=0;
    int fd_back_fifo, fd_back_fifo2, fd_back_fifo3, fd_back_fifo4, fd_cli_fifo, res_size;

    user cli_dados;
    item item_cli;
    licit cli_licit;
    info cli_info;
    
    
    setbuf(stdout,NULL);
    
    char *arr[TAM];
    char str[50];
    char nomeFifoCliente[100];
    char buffer[500];
    
    if(argc != 3){
      printf("Erro. <username> <password>\n");
      exit(1);
    }

    if (access(BACK_FIFO, F_OK)){
		  printf("[WARNING] Backend is not running\n");
		  exit(1);
	  }

    // Recepção das credenciais do Utilizador
    strcpy(cli_dados.nome,argv[1]);
    strcpy(cli_dados.pass,argv[2]);
    strcpy(cli_dados.msg,"V");
    cli_dados.pid=getpid(); 

    // FIFO CLIENTE
    sprintf (nomeFifoCliente,CLIENT_FIFO,getpid());
    if (mkfifo(nomeFifoCliente,0666) == -1)  { 
      printf("Erro abrir fifo");
      return 1;
    }

    // FIFO BACKEND
    fd_back_fifo = open(BACK_FIFO,O_RDWR);
    if(fd_back_fifo == -1){
        printf("[ERRO] Abrir fifo do backend.\n");
        close(fd_back_fifo);
        exit(1);
    }

    // FIFO BACKEND2
    fd_back_fifo2 = open(BACK_FIFO2,O_RDWR);
    if(fd_back_fifo2 == -1){
        printf("[ERRO] Abrir fifo do backend2.\n");
        close(fd_back_fifo2);
        exit(1);
    }

    // FIFO BACKEND3
    fd_back_fifo3 = open(BACK_FIFO3,O_RDWR);
    if(fd_back_fifo3 == -1){
        printf("[ERRO] Abrir fifo do backend3.\n");
        close(fd_back_fifo3);
        exit(1);
    }

    // FIFO BACKEND4
    fd_back_fifo4 = open(BACK_FIFO4,O_RDWR);
    if(fd_back_fifo4 == -1){
        printf("[ERRO] Abrir fifo do backend3.\n");
        close(fd_back_fifo4);
        exit(1);
    }

    printf("\nBem vindo %s - PID[%d]", cli_dados.nome, cli_dados.pid);
    write(fd_back_fifo, &cli_dados, sizeof(user));

    // Abrir fifo cliente para ler
    fd_cli_fifo = open(nomeFifoCliente,O_RDWR);
    if(fd_cli_fifo == -1){
      printf("\n[ERRO] Abrir fifo cliente.");
      close(fd_cli_fifo);
      exit(1);
    }
    printf("\n[%s]\n",nomeFifoCliente);
    res_size = read(fd_cli_fifo, &cli_dados, sizeof(user));

    //--------------------------
    if(strcmp(cli_dados.msg,"VALIDO")==0){
      printf("\nLogin validado.\n");

    //------Select-----------
    int nfd;
    fd_set read_fds; 

      do{

        FD_ZERO(&read_fds);
        FD_SET(STDIN_FILENO,& read_fds);
        FD_SET(fd_cli_fifo,& read_fds);
        
        nfd = select(fd_cli_fifo+1,&read_fds,NULL,NULL,NULL); 

        if(FD_ISSET(0, & read_fds)){ 
          fgets(str,sizeof(str),stdin);
          str[strlen(str)-1] = '\0';

          if(strcmp(str,"exit")==0||strcmp(str,"sair")==0){ // exit / sair
            strcpy(cli_dados.msg,"E");
            write(fd_back_fifo, &cli_dados, sizeof(user));
            unlink(nomeFifoCliente);
            printf("\n[Bye]\n");
            exit(0);
          }  

          char * token = strtok(str, " ");

          while(token != NULL){
            arr[i]=token;
            token = strtok(NULL, " ");
            ++i;
          }

          if(strcmp(arr[0],"sell")==0){ // sell
            if(i == 6){ 
              
              if(is_number(arr[3]) && is_number(arr[4]) && is_number(arr[5])){

                strcpy(item_cli.name,arr[1]);
                strcpy(item_cli.catg,arr[2]);
                strcpy(item_cli.userOwner,cli_dados.nome);
                item_cli.value = atoi(arr[3]);
                item_cli.buyN = atoi(arr[4]);
                item_cli.time = atoi(arr[5]);
                item_cli.cli_pid = getpid();

              }else{
                printf("\nErro. sell <nome-item> <categoria> <preco-base> <preco-compre-ja> <duracao>\n");
                break;
              }
              if(item_cli.value < item_cli.buyN){

                strcpy(item_cli.msg,"S");
                write(fd_back_fifo2,&item_cli, sizeof(item));
                res_size = read(fd_cli_fifo, &item_cli, sizeof(item));
                if(res_size != sizeof(item)){
                  printf("\n[ERRO] Ler do fifo do cliente.");
                }
                if(strcmp(item_cli.msg,"VALIDO")==0)
                  printf("\n[SUCESSO] - Id Item: [%d] \n",item_cli.id);
                else if(strcmp(item_cli.msg,"FULL")==0)
                  printf("\n[NUM MAX ITENS ATINGIDO]");

              } else {
                printf("\nErro. Preco base tem que ser inferior ao compre ja!\n");
              }
            
            }else
              printf("\nErro. sell <nome-item> <categoria> <preco-base> <preco-compre-ja> <duracao>\n");    
          
          }else if(strcmp(arr[0],"list")==0){ // list
            if(i == 1){

              strcpy(cli_info.msg,arr[0]);
              cli_info.pid = cli_dados.pid;
              write(fd_back_fifo4,&cli_info, sizeof(info));
              res_size = read(fd_cli_fifo, &cli_info, sizeof(info));
              if(res_size != sizeof(info)){
                printf("\n[ERRO] Ler do fifo do cliente.");
              }
              printf("%s\n",cli_info.buffer);

            }
            else    
              printf("\nErro. list\n");
          
          }else if(strcmp(arr[0],"licat")==0){ // licat
            if(i == 2){

              strcpy(cli_info.msg,arr[0]);
              strcpy(cli_info.categoria,arr[1]);
              cli_info.pid = cli_dados.pid;
              write(fd_back_fifo4,&cli_info, sizeof(info));
              res_size = read(fd_cli_fifo, &cli_info, sizeof(info));
              if(res_size != sizeof(info)){
                printf("\n[ERRO] Ler do fifo do cliente.");
              }
              printf("%s\n",cli_info.buffer);

            }
            else 
              printf("\nErro. licat <nome-categoria>\n");
          
          }else if(strcmp(arr[0],"lisel")==0){ // lisel
            if(i == 2){

              strcpy(cli_info.msg,arr[0]);
              strcpy(cli_info.userName,arr[1]);
              cli_info.pid = cli_dados.pid;
              write(fd_back_fifo4,&cli_info, sizeof(info));
              res_size = read(fd_cli_fifo, &cli_info, sizeof(info));
              if(res_size != sizeof(info)){
                printf("\n[ERRO] Ler do fifo do cliente.");
              }
              printf("%s\n",cli_info.buffer);

            }
            else 
              printf("\nErro. lisel <nome-categoria>\n");
          
          }else if(strcmp(arr[0],"lival")==0){ // lival
            if(i == 2){

              strcpy(cli_info.msg,arr[0]);
              if(is_number(arr[1])){

                cli_info.valor = atoi(arr[1]);
                cli_info.pid = cli_dados.pid;
                write(fd_back_fifo4,&cli_info, sizeof(info));
                res_size = read(fd_cli_fifo, &cli_info, sizeof(info));
                if(res_size != sizeof(info)){
                  printf("\n[ERRO] Ler do fifo do cliente.");
                }
                printf("%s\n",cli_info.buffer);

              } else 
                printf("\nO valor introduzido tem que ser um numero inteiro.\n"); 
              
            }
            else 
              printf("\nErro. lival <preco-maximo>\n");
          
          }else if(strcmp(arr[0],"litime")==0){ // litime
            if(i == 2){

              strcpy(cli_info.msg,arr[0]);
              if(is_number(arr[1])){

                cli_info.time = atoi(arr[1]);
                cli_info.pid = cli_dados.pid;
                write(fd_back_fifo4,&cli_info, sizeof(info));
                res_size = read(fd_cli_fifo, &cli_info, sizeof(info));
                if(res_size != sizeof(info)){
                  printf("\n[ERRO] Ler do fifo do cliente.");
                }
                printf("%s\n",cli_info.buffer);

              } else 
                printf("\nO valor introduzido tem que ser um numero inteiro.\n"); 

            }
            else
              printf("\nErro. litime <hora-em-segundos>\n");
          
          }else if(strcmp(arr[0],"time")==0){ // time
            if(i == 1)
              printf("\nComando time validado.");
            else 
              printf("\nErro. time");
          
          }else if(strcmp(arr[0],"buy")==0){ // buy
            if(i == 3){

              if(is_number(arr[1]) && is_number(arr[2])){
                loadUsersFile(FUSERS);

                cli_licit.valor = atoi(arr[2]);
                if(cli_licit.valor<=getUserBalance(cli_dados.nome) && cli_licit.valor>0){

                  cli_licit.id = atoi(arr[1]);
                  cli_licit.pid = cli_dados.pid;
                  strcpy(cli_licit.buyerName,cli_dados.nome);

                  strcpy(cli_licit.msg,"L");
                  write(fd_back_fifo3,&cli_licit, sizeof(licit));
                  res_size = read(fd_cli_fifo, &cli_licit, sizeof(licit));
                  if(res_size != sizeof(licit)){
                    printf("\n[ERRO] Ler do fifo do cliente.");
                  }
                  if(strcmp(cli_licit.msg,"VALIDO")==0)
                    printf("\n[SUCESSO]\n");
                  else 
                    printf("\n[NAO FOI POSSIVEL LICITAR O ITEM[%d]]\n",cli_licit.id);

                  } else {
                    printf("\nDinheiro insuficiente ou valor introduzido menor que zero.\n");
                  }

              } else {
                printf("\nErro. buy <id> <valor>\n");
              }

            }
            else 
              printf("\nErro. buy <id> <valor>\n");
          
          }else if(strcmp(arr[0],"cash")==0){ // cash
            if(i == 1){
              loadUsersFile(FUSERS);
              printf("\nCash: [%d]\n",getUserBalance(cli_dados.nome));
            }
            else 
              printf("\nErro. cash\n");
          
          }else if(strcmp(arr[0],"add")==0){ // add
            if(i == 2){
              if(is_number(arr[1])){
                int add = atoi(arr[1]);
                if(add>0){
                  loadUsersFile(FUSERS);
                  updateUserBalance(cli_dados.nome,getUserBalance(cli_dados.nome)+add);
                  printf("\n[Quantia Atualizada]-[%d]\n",getUserBalance(cli_dados.nome));
                  saveUsersFile(FUSERS);
                } else {
                  printf("\nValor a adicionar tem de ser superior a zero.\n");
                }
              } else {
                printf("\nValor introduzido tem de ser um numero inteiro.\n");
              }
            }
            else 
              printf("\nErro. add <valor>\n");
          
          }else 
              printf("\nComando desconhecido\n");
          i=0;
        }

        if(FD_ISSET(fd_cli_fifo,& read_fds)){
          int size = read(fd_cli_fifo, &buffer, sizeof(buffer));
          printf("\n------------------------\n");
          if(strcmp(buffer,"kick")==0){
            unlink(nomeFifoCliente);
            printf("\n[Foi kickado pelo Backend]\n");
            exit(0);
          } else if(strcmp(buffer,"close")==0){
            unlink(nomeFifoCliente);
            printf("\n[Backend vai encerrar dentro de momentos]\n");
            sleep(1);
            printf("\n[BYE]\n");
            exit(0);
          } else
            printf("\n%s\n\n",buffer);
        }

      }while(1);

    }else if(strcmp(cli_dados.msg,"INVALIDO")==0)
      printf("\nLogin invalido.\n");
     else if(strcmp(cli_dados.msg,"AGAIN")==0)
      printf("\nEste User ja se encontra online.\n");

    return 0;
}
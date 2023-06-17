#include "util.h"
#include "users_lib.h"

void exit_handler(int signum) {
  
}

user_ptr add_cli(int *numCli, user_ptr array_cli, user_ptr new){
  if(*numCli == 0){
    
    array_cli = malloc(sizeof(user));
    if(array_cli == NULL){
      printf("\n[ERRO] Alloc de memoria array_cli\n");
      free(array_cli);
      return NULL;
    }

  } else {

    user_ptr aux;
    aux = realloc(array_cli, sizeof(user)*(*numCli + 1));
    if(aux == NULL){
      printf("\n[ERRO] Alloc de memoria aux");
      free(aux);
      return NULL;
    }
    
    array_cli = aux;

  }

  // Copiar info
  strcpy(array_cli[*numCli].nome, new->nome);
  strcpy(array_cli[*numCli].pass, new->pass);
  array_cli[*numCli].saldo = getUserBalance(new->nome);
  array_cli[*numCli].pid = new->pid;

  (*numCli)++;
  
  return array_cli;

}

item_ptr add_item(int *numItens, item_ptr array_itens, item_ptr new){
  
  if(*numItens == 0){
    
    array_itens = malloc(sizeof(item));
    if(array_itens == NULL){
      printf("\n[ERRO] Alloc de memoria array_itens.");
      free(array_itens);
      return NULL;
    }

  } else  {

    item_ptr aux;
    aux = realloc(array_itens, sizeof(item)*(*numItens + 1));
    if(aux == NULL){
      printf("\n[ERRO] realloc de memoria aux.");
      free(aux);
      return NULL;
    }
    
    array_itens = aux;

  }
  
  // Copiar info
  strcpy(array_itens[*numItens].name, new->name);
  strcpy(array_itens[*numItens].catg, new->catg);
  strcpy(array_itens[*numItens].userOwner, new->userOwner);
  strcpy(array_itens[*numItens].userLbuyer, "-");
  array_itens[*numItens].value = new->value;
  array_itens[*numItens].buyN = new->buyN;
  array_itens[*numItens].time = new->time;
  array_itens[*numItens].lastBid = 0;

  // Nova info
  array_itens[*numItens].id = 100+(*numItens+1);
  new->id = array_itens[*numItens].id;

  (*numItens)++;
  return array_itens;
  
}

item_ptr eli_item(int *numItens, item_ptr array_itens, int id){
  int index=-1;
  for(int i=0; i<*numItens; i++){
    if(array_itens[i].id==id){
      index=i;
      break;
    }
  }

  if(index==-1){
    printf("\n[ERRO INDEX ITENS]");
    return NULL; 
  }

  memmove(&array_itens[index], &array_itens[index+1], (*numItens-index)*sizeof(item));

  (*numItens)--;
  return array_itens;

}

user_ptr eli_cli(int *numClientes, user_ptr array_cli, int pid){ 
  int index=-1;
  for(int i=0; i<*numClientes; i++){
    if(array_cli[i].pid==pid){
      index=i;
      break;
    }
  }

  if(index==-1){
    printf("\n[ERRO INDEX CLIENTES]");
    return NULL;
  }

  memmove(&array_cli[index], &array_cli[index+1], (*numClientes-index)*sizeof(user));

  (*numClientes)--;
  return array_cli;
}

void lista_prom(char **array_promo, int num_promo){

  printf("\nNumero de Promotores: [%d]\n",num_promo);

  for(int i=0; i<num_promo; i++){ 
    printf("\n[%s]\n",array_promo[i]);
  }

}

void lista_cli(user_ptr array_cli, int numCli){
  loadUsersFile(FUSERS);
  printf("\nClientes[%d]:\n", numCli);

	for(int i = 0; i< numCli; i++)
		printf("\nCliente-[%s] - ID Cliente-[%d] - Password-[%s] - Saldo-[%d]", 
    array_cli[i].nome, 
    array_cli[i].pid, 
    array_cli[i].pass, 
    getUserBalance(array_cli[i].nome));


}

void lista_item(item_ptr array_itens, int numItens){
  printf("\nItens[%d]:\n", numItens);

  for(int i=0; i<numItens; i++)
    printf("\nItem: [%s] \n ID ITEM: [%d] \n CATEGORIA: [%s] \n VALOR: [%d] \n COMPRA JA: [%d] \n VENDEDOR: [%s] \n ULTIMA BET: [%d] \n ULTIMO LICITADOR: [%s]\n TEMPO: [%d]\n",
    array_itens[i].name,
    array_itens[i].id,
    array_itens[i].catg,
    array_itens[i].value,
    array_itens[i].buyN,
    array_itens[i].userOwner,
    array_itens[i].lastBid,
    array_itens[i].userLbuyer,
    array_itens[i].time);

}

int isBetValid(int id, int value, item_ptr array_itens, int numItens){
  
  for(int i=0; i<numItens; i++){
    if(array_itens[i].id==id){
      if(array_itens[i].lastBid==0){
        if(array_itens[i].value<value)
          return 1;
      }else if(array_itens[i].lastBid<value)
        return 1;
    } 
  }
  return 0;

}

int getItemId(int id, item_ptr array_itens, int numItens){
  int index = -1;

  for(int i=0; i<numItens; i++){
    if(array_itens[i].id==id){
      index = i;
      return index;
    } 
  }
  return index;
}

void * threadRecebeCli(void *dados){
  backCli *backCliData = (backCli *)dados;

  user_ptr auxCliente_ptr = backCliData->array_clientes;
  user cli_dados;

  struct sigaction action;
  action.sa_handler = &exit_handler; 
  action.sa_flags = 0;
  sigaction(SIGUSR1, &action, NULL);

  int res_size,fd_back_fifo,fd_cli_fifo;
  int flag=0;

  char nome_fifo_cli[30];

  fd_back_fifo = open(BACK_FIFO, O_RDWR);
  if(fd_back_fifo == -1){
    printf("\n[ERRO] Abrir fifo backend.");
    exit(1);
  }

  while (backCliData->stop) {
    // Lê credenciais do Cliente
    res_size = read(fd_back_fifo, &cli_dados, sizeof(user));

    if(res_size != sizeof(user)){
      printf("\n[ERRO] Ler do fifo do backend.");
    } 

    if(strcmp(cli_dados.msg,"V")==0){
      loadUsersFile(FUSERS);
      if (isUserValid(cli_dados.nome, cli_dados.pass) == 1){
			// Testar para evitar mais que uma log de um Cliente
      for(int i=0; i<(*backCliData->numCli); i++){
        if(strcmp(backCliData->array_clientes[i].nome,cli_dados.nome)==0) // Ver se já está logado
          flag=1;
      }

      if(!flag){
        //Guardar novo Cliente e os seus dados
          pthread_mutex_lock(backCliData->trinco);
          auxCliente_ptr = add_cli(backCliData->numCli, backCliData->array_clientes, &cli_dados);
          backCliData->array_clientes = auxCliente_ptr;
          pthread_mutex_unlock(backCliData->trinco);  
        
        //Responder ao Cliente
          sprintf(nome_fifo_cli,CLIENT_FIFO,cli_dados.pid);
          strcpy(cli_dados.msg,"VALIDO");
          fd_cli_fifo = open(nome_fifo_cli, O_WRONLY);
          if(fd_cli_fifo == -1){
            printf("\n[ERRO] Abrir fifo do cliente.");
            //exit(1);
          }
          write(fd_cli_fifo, &cli_dados, sizeof(user));
          close(fd_cli_fifo);
          printf("\n[%s] is now online.\n",cli_dados.nome);

        } else {

          //Responder ao Cliente
          sprintf(nome_fifo_cli,CLIENT_FIFO,cli_dados.pid);
          strcpy(cli_dados.msg,"AGAIN");
          fd_cli_fifo = open(nome_fifo_cli, O_WRONLY);
          if(fd_cli_fifo == -1){
            printf("\n[ERRO] Abrir fifo do cliente.");
            //exit(1);
          }
          write(fd_cli_fifo, &cli_dados, sizeof(user));
          close(fd_cli_fifo);

        }

      } else {

        printf("\n[User [%s] - [%d], tentativa de login falhada.]\n",
          cli_dados.nome,
          cli_dados.pid);

        //Responder ao Cliente
        sprintf(nome_fifo_cli,CLIENT_FIFO,cli_dados.pid);
        strcpy(cli_dados.msg,"INVALIDO");
        fd_cli_fifo = open(nome_fifo_cli, O_WRONLY);
        if(fd_cli_fifo == -1){
          printf("\n[ERRO] Abrir fifo do cliente.");
          //exit(1);
        }
        write(fd_cli_fifo, &cli_dados, sizeof(user));
        close(fd_cli_fifo);
      }

    }

    //------------------------------
    if(strcmp(cli_dados.msg,"E")==0){
      loadUsersFile(FUSERS);
      if(isUserValid(cli_dados.nome, cli_dados.pass) == 1){

        //Remove Cliente
        pthread_mutex_lock(backCliData->trinco);
        auxCliente_ptr = eli_cli(backCliData->numCli, backCliData->array_clientes, cli_dados.pid);
        if(auxCliente_ptr != NULL){
          backCliData->array_clientes = auxCliente_ptr;
          pthread_mutex_unlock(backCliData->trinco);
          printf("\n[%s] is now offline.\n",cli_dados.nome);
        } else {
          pthread_mutex_unlock(backCliData->trinco);
          printf("\n[PID NAO ENCONTRADO]");
        }       

      } else {
        printf("\n[ERRO]-[E]");
      }

    }
    
  }
  pthread_exit(NULL);

}

void * threadRecebeItem(void *dados){
  backCli *backCliData = (backCli *)dados;

  item_ptr auxItem_ptr = backCliData->array_itens;
  user cli_dados;
  item item_cli;

  pthread_t thread_leilao[30];

  int res_size, fd_back_fifo, fd_cli_fifo;

  char nome_fifo_cli[30];
  char buffer[150];

  fd_back_fifo = open(BACK_FIFO2, O_RDWR);
  if(fd_back_fifo == -1){
    printf("\n[ERRO] Abrir fifo backend 2.");
    exit(1);
  }

  while (backCliData->stop) {
    // Lê credenciais do Cliente
    res_size = read(fd_back_fifo, &item_cli, sizeof(item));
    if(res_size != sizeof(item))
      printf("\n[ERRO] Ler fifo do backend 2.");
    
    if(strcmp(item_cli.msg,"S")==0){
      if((*backCliData->numItens)<backCliData->maxItens){
        //Guardar novo Item a licitar e os seus dados
        pthread_mutex_lock(backCliData->trinco);
        auxItem_ptr = add_item(backCliData->numItens, backCliData->array_itens, &item_cli);
        backCliData->array_itens = auxItem_ptr;
        pthread_mutex_unlock(backCliData->trinco);
        strcpy(item_cli.msg,"VALIDO");

        //Responder ao Cliente
        sprintf(nome_fifo_cli,CLIENT_FIFO,item_cli.cli_pid);
        fd_cli_fifo = open(nome_fifo_cli, O_WRONLY);
        if(fd_cli_fifo == -1){
          printf("\n[ERRO] Abrir fifo do cliente.");
          //exit(1);
        }
        write(fd_cli_fifo, &item_cli, sizeof(item));
        close(fd_cli_fifo);
        printf("\n[Item [%s]-REGISTADO]\n",item_cli.name);

        //Avisar todos os Clientes de novo Leilão
        sprintf(buffer,"Novo Item em Leilao  \n Nome: [%s] \n Id: [%d] \n Valor Inicial: [%d] \n Compre Ja: [%d] \n Tempo: [%d]\n",
          item_cli.name,
          backCliData->array_itens[(*backCliData->numItens)-1].id,
          item_cli.value,
          item_cli.buyN,
          item_cli.time);

        for(int i=0; i<(*backCliData->numCli); i++){

          sprintf(nome_fifo_cli,CLIENT_FIFO,backCliData->array_clientes[i].pid);
          fd_cli_fifo = open(nome_fifo_cli, O_WRONLY);
          if(fd_cli_fifo == -1){
            printf("\n[ERRO] Abrir fifo do cliente.");
            //exit(1);
          }
          write(fd_cli_fifo, &buffer, sizeof(buffer));
          close(fd_cli_fifo);

        }

      } else {

        printf("\n[MAX ITENS ATINGIDO]");
        
        //Responder ao Cliente
        sprintf(nome_fifo_cli,CLIENT_FIFO,item_cli.cli_pid);
        strcpy(item_cli.msg,"FULL");
        fd_cli_fifo = open(nome_fifo_cli, O_WRONLY);
        if(fd_cli_fifo == -1){
          printf("\n[ERRO] Abrir fifo do cliente.");
          //exit(1);
        }
        write(fd_cli_fifo, &item_cli, sizeof(item));
        close(fd_cli_fifo);

      }
     
    }
  
  }
  pthread_exit(NULL);
  
}

void * threadLeilao(void *dados){
  backCli *backCliData = (backCli *)dados;
  item_ptr auxItem_ptr = backCliData->array_itens;

  int fd_cli_fifo;

  char nome_fifo_cli[30];
  char buffer[500];

  while(backCliData->stop){
    if((*backCliData->numItens) != 0){

      for(int b=0; b<(*backCliData->numItens); b++){
        if(backCliData->array_itens[b].time==0){


          FILE *file = fopen(FITEMS, "r");
          if (file == NULL) {
            perror("\n[ERRO] FITEMS\n"); 
          }

          //Vamos ver se o Ficheiro está vazio
          if(feof(file)) {
            printf("\n[FITEMS VAZIO]\n");
          } else {

            FILE *temp_file = fopen("temp.txt", "w");
            char *word;
            char line[500];

            while(fgets(buffer, sizeof(buffer), file)){
   
              strcpy(line,buffer);
              word = strtok(buffer, " ");
              int num = atoi(word);
            
              if (num != backCliData->array_itens[b].id){
                fprintf(temp_file, "%s", line);
              }

            }

            fclose(file);
            fclose(temp_file);

            file = fopen(FITEMS, "w");
            temp_file = fopen("temp.txt", "r");

            while (fgets(buffer, sizeof(buffer), temp_file)) {
              fprintf(file, "%s", buffer);
            }

            fclose(file);
            fclose(temp_file);

            strcpy(buffer,"");

          }

          printf("\nTIMEOUT - [%s]\n",backCliData->array_itens[b].name);
          
          if(strcmp(backCliData->array_itens[b].userLbuyer,"-")!=0){// Testar se houveram Licitações no Item
            
            loadUsersFile(FUSERS);// Vamos remover o dinheiro do Ultimo Licitador e adicionar ao Vendedor do Item
            if(updateUserBalance(backCliData->array_itens[b].userLbuyer,getUserBalance(backCliData->array_itens[b].userLbuyer)-backCliData->array_itens[b].lastBid)==-1){
              printf("\n[ERRO] Remover dinheiro de Cliente.\n");
            }
            if(updateUserBalance(backCliData->array_itens[b].userOwner,getUserBalance(backCliData->array_itens[b].userOwner)+backCliData->array_itens[b].lastBid)==-1){
              printf("\n[ERRO] Adicionar dinheiro ao Cliente.\n");
            }
            saveUsersFile(FUSERS);

            //Avisar o Cliente vencedor do Leilão
            sprintf(buffer,"Compra Concluida! Item: [%s] - Valor Pago: [%d]",
              backCliData->array_itens[b].name,
              backCliData->array_itens[b].lastBid);

            for(int i=0; i<(*backCliData->numCli); i++){
              if(strcmp(backCliData->array_clientes[i].nome,backCliData->array_itens[b].userLbuyer)==0){
                sprintf(nome_fifo_cli,CLIENT_FIFO,backCliData->array_clientes[i].pid);
                fd_cli_fifo = open(nome_fifo_cli, O_WRONLY);
                if(fd_cli_fifo == -1){
                  printf("\n[ERRO] Abrir fifo do cliente.");
                  //exit(1);
                }
                write(fd_cli_fifo, &buffer, sizeof(buffer));
                close(fd_cli_fifo);
              }
            }

            //Avisar o Cliente que vendeu o Item
            sprintf(buffer,"Venda Concluida! Item: [%s] - Valor Recebido: [%d]",
              backCliData->array_itens[b].name,
              backCliData->array_itens[b].lastBid);

            for(int i=0; i<(*backCliData->numCli); i++){
              if(strcmp(backCliData->array_clientes[i].nome,backCliData->array_itens[b].userOwner)==0){
                sprintf(nome_fifo_cli,CLIENT_FIFO,backCliData->array_clientes[i].pid);
                fd_cli_fifo = open(nome_fifo_cli, O_WRONLY);
                if(fd_cli_fifo == -1){
                  printf("\n[ERRO] Abrir fifo do cliente.");
                  //exit(1);
                }
                write(fd_cli_fifo, &buffer, sizeof(buffer));
                close(fd_cli_fifo);
              }
            }

            
          }

          // Preparar mensagem para enviar ao Clientes
          if(backCliData->array_itens[b].lastBid==0)
            sprintf(buffer,"Leilao TERMINADO! - Item: [%s] - Id: [%d] - Sem comprador.",
            backCliData->array_itens[b].name,
            backCliData->array_itens[b].id);
          else
            sprintf(buffer,"Leilao TERMINADO! - Item: [%s] - Id: [%d] - Vendido por: [%d]",
              backCliData->array_itens[b].name,
              backCliData->array_itens[b].id,
              backCliData->array_itens[b].lastBid);

          // Avisar todos os Clientes de que acabou o Leilão do Item
          for(int j=0; j<(*backCliData->numCli); j++){
            sprintf(nome_fifo_cli,CLIENT_FIFO,backCliData->array_clientes[j].pid);
            fd_cli_fifo = open(nome_fifo_cli, O_WRONLY);
            if(fd_cli_fifo == -1){
              printf("\n[ERRO] Abrir fifo do cliente.");
              //exit(1);
            }
            write(fd_cli_fifo, &buffer, sizeof(buffer));
            close(fd_cli_fifo);
          }

          // Remover o Item do Array de Itens
          pthread_mutex_lock(backCliData->trinco);
          auxItem_ptr = eli_item(backCliData->numItens, backCliData->array_itens, backCliData->array_itens[b].id);
          if(auxItem_ptr != NULL){ // Item removido com sucesso
            backCliData->array_itens = auxItem_ptr;
            pthread_mutex_unlock(backCliData->trinco);
          } else {// Item não removido
            pthread_mutex_unlock(backCliData->trinco);
            printf("\n[ID NAO ENCONTRADO]");
          }
          

        } else if(backCliData->array_itens[b].buyN <= backCliData->array_itens[b].lastBid){ // Valor compra Já
            if(strcmp(backCliData->array_itens[b].userLbuyer,"-")!=0){
              loadUsersFile(FUSERS);
              if(updateUserBalance(backCliData->array_itens[b].userLbuyer,getUserBalance(backCliData->array_itens[b].userLbuyer)-backCliData->array_itens[b].buyN)==-1){// Aqui tiramos só o valor do BuyN pq o Cliente pode ter feito uma licitação acima do valor compre já
                printf("\n[ERRO] Remover dinheiro de Cliente.\n");
              }
              if(updateUserBalance(backCliData->array_itens[b].userOwner,getUserBalance(backCliData->array_itens[b].userOwner)+backCliData->array_itens[b].buyN)==-1){
              printf("\n[ERRO] Adicionar dinheiro ao Cliente.\n");
              }
              saveUsersFile(FUSERS);

              printf("\n[COMPRE JA]-[ATIVADO]-[%s]-[%d]-[%s]\n",
              backCliData->array_itens[b].name,
              backCliData->array_itens[b].buyN,
              backCliData->array_itens[b].userLbuyer);

              //Avisar o Cliente vencedor do Leilão
              sprintf(buffer,"Compra Concluida! Item: [%s] - Valor Pago: [%d]",backCliData->array_itens[b].name,backCliData->array_itens[b].buyN);
              
              for(int i=0; i<(*backCliData->numCli); i++){
                if(strcmp(backCliData->array_clientes[i].nome,backCliData->array_itens[b].userLbuyer)==0){
                  sprintf(nome_fifo_cli,CLIENT_FIFO,backCliData->array_clientes[i].pid);
                  fd_cli_fifo = open(nome_fifo_cli, O_WRONLY);
                  if(fd_cli_fifo == -1){
                    printf("\n[ERRO] Abrir fifo do cliente.");
                    //exit(1);
                  }
                  write(fd_cli_fifo, &buffer, sizeof(buffer));
                  close(fd_cli_fifo);
                }
              }

              //Avisar o Cliente que vendeu o Item
              sprintf(buffer,"Venda Concluida! Item: [%s] - Valor Recebido: [%d]",
                backCliData->array_itens[b].name,
                backCliData->array_itens[b].buyN);

              for(int i=0; i<(*backCliData->numCli); i++){
                if(strcmp(backCliData->array_clientes[i].nome,backCliData->array_itens[b].userOwner)==0){
                  sprintf(nome_fifo_cli,CLIENT_FIFO,backCliData->array_clientes[i].pid);
                  fd_cli_fifo = open(nome_fifo_cli, O_WRONLY);
                  if(fd_cli_fifo == -1){
                    printf("\n[ERRO] Abrir fifo do cliente.");
                    //exit(1);
                  }
                  write(fd_cli_fifo, &buffer, sizeof(buffer));
                  close(fd_cli_fifo);
                }
              }

              // Preparar mensagem para enviar ao Clientes
              sprintf(buffer,"Leilao TERMINADO! - Item: [%s] - Id: [%d] - Vendido por: [%d]",
                backCliData->array_itens[b].name,
                backCliData->array_itens[b].id,
                backCliData->array_itens[b].buyN);

              // Avisar todos os Clientes de que acabou o Leilão do Item
              for(int j=0; j<(*backCliData->numCli); j++){
                sprintf(nome_fifo_cli,CLIENT_FIFO,backCliData->array_clientes[j].pid);
                fd_cli_fifo = open(nome_fifo_cli, O_WRONLY);
                if(fd_cli_fifo == -1){
                  printf("\n[ERRO] Abrir fifo do cliente.");
                  //exit(1);
                }
                write(fd_cli_fifo, &buffer, sizeof(buffer));
                close(fd_cli_fifo);
              }

              // Remover o Item do Array de Itens
              pthread_mutex_lock(backCliData->trinco);
              auxItem_ptr = eli_item(backCliData->numItens, backCliData->array_itens, backCliData->array_itens[b].id);
              if(auxItem_ptr != NULL){ // Item removido com sucesso
                backCliData->array_itens = auxItem_ptr;
                pthread_mutex_unlock(backCliData->trinco);
              } else { // Item não removido
                pthread_mutex_unlock(backCliData->trinco);
                printf("\n[ID NAO ENCONTRADO]");
              }
            }

        } else {
          
          if(backCliData->array_itens[b].time==10){
          
            char msg[500];
            // Preparar a mensagem para os Clientes
            if(backCliData->array_itens[b].lastBid==0){
              sprintf(msg,"\n\nATENCAO! [%d] segundos restantes: \nItem: [%s] \n ID ITEM: [%d] \n VALOR: [%d] \n ULTIMA BET: [-] \n COMPRA JA: [%d]",
                backCliData->array_itens[b].time,
                backCliData->array_itens[b].name,
                backCliData->array_itens[b].id,
                backCliData->array_itens[b].value,
                backCliData->array_itens[b].buyN);
            } else 
              sprintf(msg,"\n\nATENCAO! [%d] segundos restantes: \nItem: [%s] \n ID ITEM: [%d] \n ULTIMA BET: [%d] \n COMPRA JA: [%d]",
                  backCliData->array_itens[b].time,
                  backCliData->array_itens[b].name,
                  backCliData->array_itens[b].id,
                  backCliData->array_itens[b].lastBid,
                  backCliData->array_itens[b].buyN);

            // Avisar todos os Clientes
            for(int j=0; j<(*backCliData->numCli); j++){
              sprintf(nome_fifo_cli,CLIENT_FIFO,backCliData->array_clientes[j].pid);
              fd_cli_fifo = open(nome_fifo_cli, O_WRONLY);
              if(fd_cli_fifo == -1){
                printf("\n[ERRO] Abrir fifo do cliente.");
                //exit(1);
              }
              write(fd_cli_fifo, &msg, sizeof(msg));
              close(fd_cli_fifo);
            }

          }
          //-----------------------
          if(backCliData->array_itens[b].time<=10)
            printf("\n[%s]-[%d]",backCliData->array_itens[b].name,backCliData->array_itens[b].time);

          pthread_mutex_lock(backCliData->trinco);
          backCliData->array_itens[b].time -= 1;
          pthread_mutex_unlock(backCliData->trinco);
        
        }
      }
    } else {
      //printf("\n[NO ITEMS]");
    }
    sleep(1);
  }
  pthread_exit(NULL);
}

void * threadLicitador(void *dados){
  backCli *backCliData = (backCli *)dados;

  licit cli_licit;

  int res_size, fd_back_fifo, fd_cli_fifo;

  char nome_fifo_cli[30];

  fd_back_fifo = open(BACK_FIFO3, O_RDWR);
  if(fd_back_fifo == -1){
    printf("\n[ERRO] Abrir fifo backend 3.");
    exit(1);
  }

  while(backCliData->stop){
    if((*backCliData->numItens) != 0){

      res_size = read(fd_back_fifo, &cli_licit, sizeof(licit));
      if(res_size != sizeof(licit))
        printf("\n[ERRO] Ler fifo do backend 3.");

      if(strcmp(cli_licit.msg,"L")==0){

        if(isBetValid(cli_licit.id, cli_licit.valor, backCliData->array_itens, (*backCliData->numItens))==1){

          int index = getItemId(cli_licit.id,backCliData->array_itens, (*backCliData->numItens));
          
          if(index!=-1){

            pthread_mutex_lock(backCliData->trinco);
            strcpy(backCliData->array_itens[index].userLbuyer,cli_licit.buyerName);
            backCliData->array_itens[index].lastBid = cli_licit.valor;
            pthread_mutex_unlock(backCliData->trinco);
            printf("\n[BET] [%s] - [%s] - [%d]\n",backCliData->array_itens[index].name,
            cli_licit.buyerName,
            cli_licit.valor);

            //Responder ao Cliente
            strcpy(cli_licit.msg,"VALIDO");
            sprintf(nome_fifo_cli,CLIENT_FIFO,cli_licit.pid);
            fd_cli_fifo = open(nome_fifo_cli, O_WRONLY);
            if(fd_cli_fifo == -1){
              printf("\n[ERRO] Abrir fifo do cliente.");
              //exit(1);
            }
            write(fd_cli_fifo, &cli_licit, sizeof(licit));
            close(fd_cli_fifo);

          } else { // Bet not valid

            //Responder ao Cliente
            strcpy(cli_licit.msg,"INVALIDO");
            sprintf(nome_fifo_cli,CLIENT_FIFO,cli_licit.pid);
            fd_cli_fifo = open(nome_fifo_cli, O_WRONLY);
            if(fd_cli_fifo == -1){
              printf("\n[ERRO] Abrir fifo do cliente.");
              //exit(1);
            }
            write(fd_cli_fifo, &cli_licit, sizeof(licit));
            close(fd_cli_fifo);

          }

        } else { // Bet not valid
            strcpy(cli_licit.msg,"INVALIDO");
            sprintf(nome_fifo_cli,CLIENT_FIFO,cli_licit.pid);
            fd_cli_fifo = open(nome_fifo_cli, O_WRONLY);
            if(fd_cli_fifo == -1){
              printf("\n[ERRO] Abrir fifo do cliente.");
              //exit(1);
            }
            write(fd_cli_fifo, &cli_licit, sizeof(licit));
            close(fd_cli_fifo);
        }
      }


    }
  }
  pthread_exit(NULL);

}

void * threadTrataInfo(void *dados){
  backCli *backCliData = (backCli *)dados;

  info cli_info;

  int res_size, fd_back_fifo, fd_cli_fifo;

  char nome_fifo_cli[30];
  char buffer[1024];

  fd_back_fifo = open(BACK_FIFO4, O_RDWR);
  if(fd_back_fifo == -1){
    printf("\n[ERRO] Abrir fifo backend 2.\n");
    exit(1);
  }

  while (backCliData->stop){
    // Lê estrutura info
    res_size = read(fd_back_fifo, &cli_info, sizeof(info));
    if(res_size != sizeof(info))
      printf("\n[ERRO] Ler fifo do backend 4.\n");

    if(strcmp(cli_info.msg,"list")==0){

      if((*backCliData->numItens)>0){ 

        char line[500];
        for(int i=0; i<(*backCliData->numItens); i++){

          if(backCliData->array_itens[i].lastBid != 0)
            sprintf(line,"\n\nItem: [%s] \n ID ITEM: [%d] \n CATEGORIA: [%s] \n VENDEDOR: [%s] \n ULTIMA BET: [%d] \n ULTIMO LICITADOR: [%s] \n COMPRA JA: [%d] \n TEMPO: [%d]",
              backCliData->array_itens[i].name,
              backCliData->array_itens[i].id,
              backCliData->array_itens[i].catg,
              backCliData->array_itens[i].userOwner,
              backCliData->array_itens[i].lastBid,
              backCliData->array_itens[i].userLbuyer,
              backCliData->array_itens[i].buyN,
              backCliData->array_itens[i].time);
          else 
            sprintf(line,"\n\nItem: [%s] \n ID ITEM: [%d] \n CATEGORIA: [%s] \n VALOR: [%d] \n VENDEDOR: [%s] \n ULTIMA BET: [SEM BET] \n ULTIMO LICITADOR: [-] \n COMPRA JA: [%d] \n TEMPO: [%d]",
              backCliData->array_itens[i].name,
              backCliData->array_itens[i].id,
              backCliData->array_itens[i].catg,
              backCliData->array_itens[i].value,
              backCliData->array_itens[i].userOwner,  
              backCliData->array_itens[i].buyN,
              backCliData->array_itens[i].time);

          strcat(buffer,line); 

        }

        //Responder ao Cliente
        strcpy(cli_info.buffer,buffer);
        sprintf(nome_fifo_cli,CLIENT_FIFO,cli_info.pid);
        fd_cli_fifo = open(nome_fifo_cli, O_WRONLY);
        if(fd_cli_fifo == -1){
          printf("\n[ERRO] Abrir fifo do cliente.");
          //exit(1);
        }
        write(fd_cli_fifo, &cli_info, sizeof(info));
        close(fd_cli_fifo);
        strcpy(buffer,"");

      } else {

        sprintf(buffer,"\n[Nao existem itens a ser leiloados de momento]\n");
        //Responder ao Cliente
        strcpy(cli_info.buffer,buffer);
        sprintf(nome_fifo_cli,CLIENT_FIFO,cli_info.pid);
        fd_cli_fifo = open(nome_fifo_cli, O_WRONLY);
        if(fd_cli_fifo == -1){
          printf("\n[ERRO] Abrir fifo do cliente.");
          //exit(1);
        }
        write(fd_cli_fifo, &cli_info, sizeof(info));
        close(fd_cli_fifo);
        strcpy(buffer,"");

      }

    } else if(strcmp(cli_info.msg,"licat")==0){

      if((*backCliData->numItens)>0){
        char line[500];
        int flag=0;
        for(int i=0; i<(*backCliData->numItens); i++){
          if(strcmp(backCliData->array_itens[i].catg,cli_info.categoria)==0){
            if(backCliData->array_itens[i].lastBid != 0)
              sprintf(line,"\n\nItem: [%s] \n ID ITEM: [%d] \n VENDEDOR: [%s] \n ULTIMA BET: [%d] \n ULTIMO LICITADOR: [%s] \n COMPRA JA: [%d] \n TEMPO: [%d]",
              backCliData->array_itens[i].name,
              backCliData->array_itens[i].id,
              backCliData->array_itens[i].userOwner,
              backCliData->array_itens[i].lastBid,
              backCliData->array_itens[i].userLbuyer,
              backCliData->array_itens[i].buyN,
              backCliData->array_itens[i].time);            
            else 
              sprintf(line,"\n\nItem: [%s] \n ID ITEM: [%d] \n VALOR: [%d] \n VENDEDOR: [%s] \n ULTIMA BET: [SEM BET] \n ULTIMO LICITADOR: [-] \n COMPRA JA: [%d] \n TEMPO: [%d]",
              backCliData->array_itens[i].name,
              backCliData->array_itens[i].id, 
              backCliData->array_itens[i].value,
              backCliData->array_itens[i].userOwner,  
              backCliData->array_itens[i].buyN,
              backCliData->array_itens[i].time);

            strcat(buffer,line);
            flag=1;
          }
        }

        if(flag){
          //Responder ao Cliente
          strcpy(cli_info.buffer,buffer);
          sprintf(nome_fifo_cli,CLIENT_FIFO,cli_info.pid);
          fd_cli_fifo = open(nome_fifo_cli, O_WRONLY);
          if(fd_cli_fifo == -1){
            printf("\n[ERRO] Abrir fifo do cliente.");
            //exit(1);
          }
          write(fd_cli_fifo, &cli_info, sizeof(info));
          close(fd_cli_fifo);
          strcpy(buffer,"");
        } else {
          sprintf(buffer,"\n[Nao existem itens desta categoria a serem leiloados]\n");
          //Responder ao Cliente
          strcpy(cli_info.buffer,buffer);
          sprintf(nome_fifo_cli,CLIENT_FIFO,cli_info.pid);
          fd_cli_fifo = open(nome_fifo_cli, O_WRONLY);
          if(fd_cli_fifo == -1){
            printf("\n[ERRO] Abrir fifo do cliente.");
            //exit(1);
          }
          write(fd_cli_fifo, &cli_info, sizeof(info));
          close(fd_cli_fifo);
          strcpy(buffer,"");
        }
        

      } else {

        sprintf(buffer,"\n[Nao existem itens a ser leiloados de momento]\n");
        //Responder ao Cliente
        strcpy(cli_info.buffer,buffer);
        sprintf(nome_fifo_cli,CLIENT_FIFO,cli_info.pid);
        fd_cli_fifo = open(nome_fifo_cli, O_WRONLY);
        if(fd_cli_fifo == -1){
          printf("\n[ERRO] Abrir fifo do cliente.");
          //exit(1);
        }
        write(fd_cli_fifo, &cli_info, sizeof(info));
        close(fd_cli_fifo);
        strcpy(buffer,"");

      }

    } else if(strcmp(cli_info.msg,"lisel")==0){

      if((*backCliData->numItens)>0){
        char line[500];
        int flag=0;
        for(int i=0; i<(*backCliData->numItens); i++){
          if(strcmp(backCliData->array_itens[i].userOwner,cli_info.userName)==0){
            if(backCliData->array_itens[i].lastBid != 0)
              sprintf(line,"\n\nItem: [%s] \n ID ITEM: [%d] \n CATEGORIA: [%s] \n ULTIMA BET: [%d] \n ULTIMO LICITADOR: [%s] \n COMPRA JA: [%d] \n TEMPO: [%d]",
              backCliData->array_itens[i].name,
              backCliData->array_itens[i].id,
              backCliData->array_itens[i].catg, 
              backCliData->array_itens[i].lastBid,
              backCliData->array_itens[i].userLbuyer,
              backCliData->array_itens[i].buyN,
              backCliData->array_itens[i].time);
            else 
              sprintf(line,"\n\nItem: [%s] \n ID ITEM: [%d] \n CATEGORIA: [%s] \n VALOR: [%d] \n ULTIMA BET: [SEM BET] \n ULTIMO LICITADOR: [-] \n COMPRA JA: [%d] \n TEMPO: [%d]",
              backCliData->array_itens[i].name,
              backCliData->array_itens[i].id,
              backCliData->array_itens[i].catg,
              backCliData->array_itens[i].value, 
              backCliData->array_itens[i].buyN,
              backCliData->array_itens[i].time);

            strcat(buffer,line);
            flag=1;
          }
        }

        if(flag){

          //Responder ao Cliente
          strcpy(cli_info.buffer,buffer);
          sprintf(nome_fifo_cli,CLIENT_FIFO,cli_info.pid);
          fd_cli_fifo = open(nome_fifo_cli, O_WRONLY);
          if(fd_cli_fifo == -1){
            printf("\n[ERRO] Abrir fifo do cliente.");
            //exit(1);
          }
          write(fd_cli_fifo, &cli_info, sizeof(info));
          close(fd_cli_fifo);
          strcpy(buffer,"");

        } else {

          sprintf(buffer,"\n[Nao existem itens deste User a serem leiloados]\n");
          //Responder ao Cliente
          strcpy(cli_info.buffer,buffer);
          sprintf(nome_fifo_cli,CLIENT_FIFO,cli_info.pid);
          fd_cli_fifo = open(nome_fifo_cli, O_WRONLY);
          if(fd_cli_fifo == -1){
            printf("\n[ERRO] Abrir fifo do cliente.");
            //exit(1);
          }
          write(fd_cli_fifo, &cli_info, sizeof(info));
          close(fd_cli_fifo);
          strcpy(buffer,"");

        }
        
      } else {

        sprintf(buffer,"\n[Nao existem itens a ser leiloados de momento]\n");
        //Responder ao Cliente
        strcpy(cli_info.buffer,buffer);
        sprintf(nome_fifo_cli,CLIENT_FIFO,cli_info.pid);
        fd_cli_fifo = open(nome_fifo_cli, O_WRONLY);
        if(fd_cli_fifo == -1){
          printf("\n[ERRO] Abrir fifo do cliente.");
          //exit(1);
        }
        write(fd_cli_fifo, &cli_info, sizeof(info));
        close(fd_cli_fifo);
        strcpy(buffer,"");

      }
      
    } else if(strcmp(cli_info.msg,"lival")==0){

      if((*backCliData->numItens)>0){
        char line[500];
        int flag=0;
        for(int i=0; i<(*backCliData->numItens); i++){
          if(backCliData->array_itens[i].buyN<=cli_info.valor){
            if(backCliData->array_itens[i].lastBid != 0)
              sprintf(line,"\n\nItem: [%s] \n ID ITEM: [%d] \n CATEGORIA: [%s] \n ULTIMA BET: [%d] \n ULTIMO LICITADOR: [%s] \n COMPRA JA: [%d] \n TEMPO: [%d]",
              backCliData->array_itens[i].name,
              backCliData->array_itens[i].id,
              backCliData->array_itens[i].catg, 
              backCliData->array_itens[i].lastBid,
              backCliData->array_itens[i].userLbuyer,
              backCliData->array_itens[i].buyN,
              backCliData->array_itens[i].time);
            else 
              sprintf(line,"\n\nItem: [%s] \n ID ITEM: [%d] \n CATEGORIA: [%s] \n VALOR: [%d] \n ULTIMA BET: [SEM BET] \n ULTIMO LICITADOR: [-] \n COMPRA JA: [%d] \n TEMPO: [%d]",
              backCliData->array_itens[i].name,
              backCliData->array_itens[i].id,
              backCliData->array_itens[i].catg,
              backCliData->array_itens[i].value, 
              backCliData->array_itens[i].buyN,
              backCliData->array_itens[i].time); 

            strcat(buffer,line);
            flag=1;
          }
        }

        if(flag){

          //Responder ao Cliente
          strcpy(cli_info.buffer,buffer);
          sprintf(nome_fifo_cli,CLIENT_FIFO,cli_info.pid);
          fd_cli_fifo = open(nome_fifo_cli, O_WRONLY);
          if(fd_cli_fifo == -1){
            printf("\n[ERRO] Abrir fifo do cliente.");
            //exit(1);
          }
          write(fd_cli_fifo, &cli_info, sizeof(info));
          close(fd_cli_fifo);
          strcpy(buffer,"");

        } else {

          sprintf(buffer,"\n[Nao existem itens deste valor a serem leiloados]\n");
          //Responder ao Cliente
          strcpy(cli_info.buffer,buffer);
          sprintf(nome_fifo_cli,CLIENT_FIFO,cli_info.pid);
          fd_cli_fifo = open(nome_fifo_cli, O_WRONLY);
          if(fd_cli_fifo == -1){
            printf("\n[ERRO] Abrir fifo do cliente.");
            //exit(1);
          }
          write(fd_cli_fifo, &cli_info, sizeof(info));
          close(fd_cli_fifo);
          strcpy(buffer,"");

        }

      } else {

        sprintf(buffer,"\n[Nao existem itens a ser leiloados de momento]\n");
        //Responder ao Cliente
        strcpy(cli_info.buffer,buffer);
        sprintf(nome_fifo_cli,CLIENT_FIFO,cli_info.pid);
        fd_cli_fifo = open(nome_fifo_cli, O_WRONLY);
        if(fd_cli_fifo == -1){
          printf("\n[ERRO] Abrir fifo do cliente.");
          //exit(1);
        }
        write(fd_cli_fifo, &cli_info, sizeof(info));
        close(fd_cli_fifo);
        strcpy(buffer,"");

      }

    } else if(strcmp(cli_info.msg,"litime")==0){ 

      if((*backCliData->numItens)>0){
        char line[500];
        int flag=0;
        for(int i=0; i<(*backCliData->numItens); i++){
          if(backCliData->array_itens[i].time==cli_info.time){
            if(backCliData->array_itens[i].lastBid != 0)
              sprintf(line,"\n\nItem: [%s] \n ID ITEM: [%d] \n CATEGORIA: [%s] \n ULTIMA BET: [%d] \n ULTIMO LICITADOR: [%s] \n COMPRA JA: [%d]",
              backCliData->array_itens[i].name,
              backCliData->array_itens[i].id,
              backCliData->array_itens[i].catg, 
              backCliData->array_itens[i].lastBid,
              backCliData->array_itens[i].userLbuyer,
              backCliData->array_itens[i].buyN);
            else 
              sprintf(line,"\n\nItem: [%s] \n ID ITEM: [%d] \n CATEGORIA: [%s] \n VALOR: [%d] \n ULTIMA BET: [SEM BET] \n ULTIMO LICITADOR: [-] \n COMPRA JA: [%d]",
              backCliData->array_itens[i].name,
              backCliData->array_itens[i].id,
              backCliData->array_itens[i].catg,
              backCliData->array_itens[i].value, 
              backCliData->array_itens[i].buyN);

            strcat(buffer,line);
            flag=1;
          }
        }

        if(flag){

          //Responder ao Cliente
          strcpy(cli_info.buffer,buffer);
          sprintf(nome_fifo_cli,CLIENT_FIFO,cli_info.pid);
          fd_cli_fifo = open(nome_fifo_cli, O_WRONLY);
          if(fd_cli_fifo == -1){
            printf("\n[ERRO] Abrir fifo do cliente.");
            //exit(1);
          }
          write(fd_cli_fifo, &cli_info, sizeof(info));
          close(fd_cli_fifo);
          strcpy(buffer,"");

        } else {

          sprintf(buffer,"\n[Nao existem itens com este tempo a serem leiloados]\n");
          //Responder ao Cliente
          strcpy(cli_info.buffer,buffer);
          sprintf(nome_fifo_cli,CLIENT_FIFO,cli_info.pid);
          fd_cli_fifo = open(nome_fifo_cli, O_WRONLY);
          if(fd_cli_fifo == -1){
            printf("\n[ERRO] Abrir fifo do cliente.");
            //exit(1);
          }
          write(fd_cli_fifo, &cli_info, sizeof(info));
          close(fd_cli_fifo);
          strcpy(buffer,"");

        }

      } else {

        sprintf(buffer,"\n[Nao existem itens a ser leiloados de momento]\n");
        //Responder ao Cliente
        strcpy(cli_info.buffer,buffer);
        sprintf(nome_fifo_cli,CLIENT_FIFO,cli_info.pid);
        fd_cli_fifo = open(nome_fifo_cli, O_WRONLY);
        if(fd_cli_fifo == -1){
          printf("\n[ERRO] Abrir fifo do cliente.");
          //exit(1);
        }
        write(fd_cli_fifo, &cli_info, sizeof(info));
        close(fd_cli_fifo);
        strcpy(buffer,"");

      }
      
    }

  }
  pthread_exit(NULL);
  
}

void * threadTrataPromo(void *dados){

  backCli *backCliData = (backCli *)dados; 

  int res_size, duration, fd_cli_fifo;
  int j=0;

  float discount;

  char prom_info[50];
  char *arr[2];

  char nome_fifo_cli[30];
  char buffer[500];
  char cat[30];

  fd_set read_fds;
  int nfds=0;
  int ret; 

  while(backCliData->stop){

    if((*backCliData->numPromo)!=0){

      FD_ZERO(&read_fds);

      // Adicionar os descritores
      for(int i=0; i<(*backCliData->numPromo); i++){ 
        FD_SET(backCliData->pipe_arr[i][0], &read_fds); 
        nfds = backCliData->pipe_arr[i][0] > nfds ? backCliData->pipe_arr[i][0] : nfds; 
      }
      nfds++; 

      // Esperar que alguma das promoções seja lançada      
      ret = select(nfds, &read_fds, NULL, NULL, NULL);
      if (ret < 0) {
        printf("\n[ERRO] Select PromoThread.\n");
        //pthread_exit(NULL);
      }
    
      // Ler os pipes que têm dados disponíveis
      for(int i=0; i<(*backCliData->numPromo); i++){
        if (FD_ISSET(backCliData->pipe_arr[i][0], &read_fds)) {
          
          res_size = read(backCliData->pipe_arr[i][0], &prom_info, sizeof(prom_info));
          if(res_size != sizeof(prom_info)){
            //printf("\n[ERRO] Ler do PromFifo.\n"); -> Entra sempre aqui
          }

          if(strcmp(prom_info,"")!=0){

            prom_info[strlen(prom_info)-1] = '\0';

            char *token = strtok(prom_info, " ");
            while (token != NULL){
              arr[j] = token;
              token = strtok(NULL, " ");
              ++j;
            }

            char time[5];
            memcpy(time, arr[2], 2);// Resolver problemas de output dos promotores
            discount = atoi(arr[1]); 
            discount = discount/100; 
            strcpy(cat,arr[0]); 
            
            printf("\n[%s][%s %s %s]\n",backCliData->promo_names[i],arr[0],arr[1],time);
            fflush(stdout);
            strcpy(prom_info,"");

            for(int b=0; b<(*backCliData->numItens); b++){  

              if(strcmp(backCliData->array_itens[b].catg,cat)==0){  
                
                if(backCliData->array_itens[b].lastBid==0){ // Só alteramos preço de Itens sem licitações 
                  
                  if(backCliData->array_itens[b].value<backCliData->array_itens[b].buyN-backCliData->array_itens[b].buyN*discount){

                    pthread_mutex_lock(backCliData->trinco);
                    backCliData->array_itens[b].buyN = backCliData->array_itens[b].buyN-backCliData->array_itens[b].buyN*discount;;
                    pthread_mutex_unlock(backCliData->trinco);

                    sprintf(buffer,"[!!!] %s - Desconto: [%s%c] - Categoria: [%s]",
                      backCliData->promo_names[i],
                      arr[1],
                      37,
                      cat);

                    for(int k=0; k<(*backCliData->numCli); k++){

                      if(strcmp(backCliData->array_clientes[k].nome,backCliData->array_itens[k].userOwner)==0){

                        sprintf(nome_fifo_cli,CLIENT_FIFO,backCliData->array_clientes[k].pid);
                        fd_cli_fifo = open(nome_fifo_cli, O_WRONLY);
                        if(fd_cli_fifo == -1){
                          printf("\n[ERRO] Abrir fifo do cliente.");
                          //exit(1);
                        }
                        write(fd_cli_fifo, &buffer, sizeof(buffer));
                        close(fd_cli_fifo);

                      }

                    }

                  } 
                
                }

              }           
              
            }

          }
          
        }
        j=0;
      }

    }

  }

  pthread_exit(NULL);

}

int main(int argc, char *argv[]){
  setbuf(stdout, NULL);

  int fd[2];
  pipe(fd);

  int i = 0;
  int fd_back_fifo, fd_back_fifo2, fd_back_fifo3, fd_back_fifo4, res_size, MaxClientes, MaxItens;
  int numCli=0;
  int numItens=0;
  int numPromo=0;

  char buffer[200];
  char *arr[2];
  char *MaxClientes_str, *MaxItens_str;

  user_ptr array_cli = NULL;
  item_ptr array_ite = NULL;

  backCli backCliData;

  pthread_t thread_recebeCli;
  pthread_t thread_recebeItem;
  pthread_t thread_leilao;
  pthread_t thread_licitador;
  pthread_t thread_info;
  pthread_t thread_promo;
  pthread_mutex_t trinco;

  if (access(BACK_FIFO, F_OK) == 0){// Ver se Backend já está a rodar ou não
    printf("[WARNING] Backend is already running\n");
    exit(1);
  }

  if (argc != 1){
    printf("Erro. ./backend\n");
    exit(1);
  }

  // criar fifo backend-cliente
  if (access(BACK_FIFO, F_OK) != 0){
    mkfifo(BACK_FIFO, 0600);
  }

  // criar fifo backend-cliente2
  if (access(BACK_FIFO2, F_OK) != 0){
    mkfifo(BACK_FIFO2, 0600);
  }

  // criar fifo backend-cliente3
  if (access(BACK_FIFO3, F_OK) != 0){
    mkfifo(BACK_FIFO3, 0600);
  }

  // criar fifo backend-cliente4
  if (access(BACK_FIFO4, F_OK) != 0){
    mkfifo(BACK_FIFO4, 0600);
  }

  backCliData.maxCli=20;
  backCliData.maxItens=30;
  backCliData.maxPromo=10;

  // Abrir fifo backend-cliente
  fd_back_fifo = open(BACK_FIFO, O_RDWR);
  if (fd_back_fifo == -1)
  {
    printf("\n[ERRO] Abrir fifo do backend.");
    exit(1);
  }

  // Abrir fifo backend-cliente2
  fd_back_fifo2 = open(BACK_FIFO2, O_RDWR);
  if (fd_back_fifo2 == -1)
  {
    printf("\n[ERRO] Abrir fifo do backend2.");
    exit(1);
  }

  // Abrir fifo backend-cliente3
  fd_back_fifo3 = open(BACK_FIFO3, O_RDWR);
  if (fd_back_fifo3 == -1)
  {
    printf("\n[ERRO] Abrir fifo do backend3.");
    exit(1);
  }

  // Abrir fifo backend-cliente4
  fd_back_fifo4 = open(BACK_FIFO4, O_RDWR);
  if (fd_back_fifo4 == -1)
  {
    printf("\n[ERRO] Abrir fifo do backend3.");
    exit(1);
  }

  // Preencher estruturas das Threads
  pthread_mutex_init(&trinco,NULL);

  backCliData.stop = 1;
  backCliData.trinco = &trinco;
  backCliData.maxCli = 20;
  backCliData.numCli = &numCli;
  backCliData.numItens = &numItens;
  backCliData.numPromo = &numPromo;
  backCliData.array_clientes = array_cli;
  backCliData.array_itens = array_ite;
  backCliData.promo_names = NULL;
  backCliData.prom_pids = NULL;
  backCliData.pipe_arr = NULL;

  //Ler ficheiro de Itens

  FILE *file = fopen(FITEMS, "r");
  if (file == NULL)
  {
    printf("\n[ERRO] Ficheiro fitems.");
    exit(1);
  }

  while(fgets(buffer, sizeof(buffer), file) != NULL){

    if(numItens == 0){
    
    backCliData.array_itens = malloc(sizeof(item));
    if(backCliData.array_itens == NULL){
      printf("\n[ERRO] Alloc de memoria array_itens.");
      free(backCliData.array_itens);
      exit(1);
    }

    } else  {

      item_ptr aux;
      aux = realloc(backCliData.array_itens, sizeof(item)*(numItens + 1));
      if(aux == NULL){
        printf("\n[ERRO] realloc de memoria aux.");
        free(aux);
        exit(1);
      }
      
      backCliData.array_itens = aux;

    }

    int value;
    sscanf(buffer, "%d %s %s %d %d %d %s %s", 
    &backCliData.array_itens[numItens].id, 
    backCliData.array_itens[numItens].name, 
    backCliData.array_itens[numItens].catg,
    &value,
    &backCliData.array_itens[numItens].buyN, 
    &backCliData.array_itens[numItens].time, 
    backCliData.array_itens[numItens].userOwner,
    backCliData.array_itens[numItens].userLbuyer);
    
    if(strcmp(backCliData.array_itens[numItens].userLbuyer,"-")==0){// Caso não tenha havido Licitador
      backCliData.array_itens[numItens].value = value;
      backCliData.array_itens[numItens].lastBid = 0;
    }else{ //Caso haja Licitador
      backCliData.array_itens[numItens].lastBid = value;
      backCliData.array_itens[numItens].value = 0;
    } 
      

    numItens++;
  }

  fclose(file);

  //Debug
  printf("\n[ITENS CARREGADOS DO FICHEIRO]\n");
  lista_item(backCliData.array_itens,numItens);
  //-------
  
  //---------------------
  printf("\n[A INICIAR PROGRAMA]\n");
  sleep(2);

  //Threads 
  pthread_create(&thread_recebeCli,NULL,threadRecebeCli,&backCliData);
  pthread_create(&thread_recebeItem,NULL,threadRecebeItem,&backCliData);
  pthread_create(&thread_leilao,NULL,threadLeilao,&backCliData);
  pthread_create(&thread_licitador,NULL,threadLicitador,&backCliData);
  pthread_create(&thread_info,NULL,threadTrataInfo,&backCliData); 

  //Lançar Promotores presentes no ficheiro
  FILE* fp = fopen(FPROMOTERS, "r");
  if (fp == NULL) {
    perror("Error opening file");
    return 1;
  }

  if(feof(fp)) {
    printf("\n[FPROMOTERS VAZIO]\n");
  } else {

    char line[50];  
    
    int str_len=50;

    //Contar o número de promotores presentes no ficheiro
    while (fgets(line, sizeof(line), fp) != NULL)
    {
      numPromo++;
    }

    if(numPromo!=0){

      //Caso haja mais que 10 promotores no ficheiro
      if(numPromo>backCliData.maxPromo){
        printf("\n[FPROMOTERS EXCEDE O NUMERO MAXIMO DE PROMOTORES]\n");
        numPromo=backCliData.maxPromo;//Ficamos só com o 10
      }

      rewind(fp);

      // Alocar memória para o array com o nome dos promotores
      backCliData.promo_names = (char**)malloc(numPromo * sizeof(char*));
      if(backCliData.promo_names == NULL){
        printf("\n[ERRO] Alloc de memoria promo_names\n");
        free(backCliData.promo_names);
      }
 
      backCliData.pipe_arr = malloc(numPromo * sizeof(int *)); // Número de Promotores

      for (int c = 0; c < numPromo; c++) {

        backCliData.promo_names[c] = (char*)malloc(str_len * sizeof(char));

        backCliData.pipe_arr[c] = malloc(2 * sizeof(int)); // Número de colunas
        
        if (pipe(backCliData.pipe_arr[c]) != 0) {
          printf("\n[ERRO] Pipes\n.");
          return 1;
        }

      }
      
      int index=0;
      while (fgets(line, sizeof(line), fp) != NULL) {
        
        if(line[strlen(line)-1] == '\n')
          line[strlen(line)-1] = '\0';

        if(index < numPromo)
          strcpy(backCliData.promo_names[index], line);//Alocar os nomes no array

        index++;
        
      }

      fclose(fp); 

      //Alocar memória para o array de pid's
      backCliData.prom_pids = malloc(numPromo * sizeof(pid_t));

      //Lançar o promotores
      for(int c=0; c<numPromo; c++){

        pid_t pid = fork();
        if(pid == 0){ 
          close(1);  
          dup(backCliData.pipe_arr[c][1]); // Redirecionamento 
          close(backCliData.pipe_arr[c][0]);
          close(backCliData.pipe_arr[c][1]);
          execl(backCliData.promo_names[c], backCliData.promo_names[c], NULL);
          printf("\n[ERRO] Impossivel executar [%s].\n",backCliData.promo_names[c]);
          exit(1);
        } else if(pid > 0){ // Vamos guardar o pid do Promotor executado no array
          close(backCliData.pipe_arr[c][1]);
          backCliData.prom_pids[c] = pid;
          printf("\nPromotor: [%s] executado com sucesso.\n",backCliData.promo_names[c]);
        } else {
          printf("\n[ERRO] Criar child process.\n");
          return 1;
        }

      }

    } else {
      fclose(fp);
      printf("\n[FPROMOTERS VAZIO]\n");
    } 

    pthread_create(&thread_promo,NULL,threadTrataPromo,&backCliData);

  }

  //--------------------------------------

  //Comandos
  while(strcmp(buffer,"close")!=0){
    printf("\n\n-> ");
    fgets(buffer, sizeof(buffer), stdin);
    buffer[strlen(buffer)-1] = '\0';

    char *token = strtok(buffer, " ");
    while (token != NULL){
      arr[i] = token;
      token = strtok(NULL, " ");
      ++i;
    }

    if (strcmp(arr[0], "close") == 0){

      // Avisar todos os Clientes de que o Programa vai fechar
      char nome_fifo_cli[30];
      char msg[20];
      strcpy(msg,"close");
      for(int j=0; j<(*backCliData.numCli); j++){
        sprintf(nome_fifo_cli,CLIENT_FIFO,backCliData.array_clientes[j].pid);
        int fd_cli_fifo = open(nome_fifo_cli, O_WRONLY);
        if(fd_cli_fifo == -1){
          printf("\n[ERRO] Abrir fifo do cliente.");
          exit(1);
        }
        write(fd_cli_fifo, &msg, sizeof(msg));
        close(fd_cli_fifo);
      }
      pthread_mutex_lock(backCliData.trinco);
      backCliData.stop=0;
      pthread_mutex_unlock(backCliData.trinco);
      break;
    }
      
    else if (strcmp(arr[0], "users") == 0){
      if (i == 1){
        lista_cli(backCliData.array_clientes,numCli);
      }
    else
      printf("\nErro. users");
    } 
    else if (strcmp(arr[0], "list") == 0){
      if (i == 1)
        lista_item(backCliData.array_itens,numItens);
      else
        printf("\nErro. list");
    }
    else if (strcmp(arr[0], "kick") == 0){
      int aux=0;
      if (i == 2){
        for(int i=0; i<(*backCliData.numCli); i++){
          if(strcmp(backCliData.array_clientes[i].nome,arr[1])==0){
            // Avisar User que foi kickado 
            char nome_fifo_cli[30];
            char msg[20];
            strcpy(msg,"kick");
            sprintf(nome_fifo_cli,CLIENT_FIFO,backCliData.array_clientes[i].pid);
            int fd_cli_fifo = open(nome_fifo_cli, O_WRONLY);
            if(fd_cli_fifo == -1){
              printf("\n[ERRO] Abrir fifo do cliente.");
              exit(1);
            }
            write(fd_cli_fifo, &msg, sizeof(msg));
            close(fd_cli_fifo);

            // Remover User do Array
            user_ptr auxCliente_ptr = backCliData.array_clientes;
            char nome[20];
            strcpy(nome,backCliData.array_clientes[i].nome);
            pthread_mutex_lock(backCliData.trinco);
            auxCliente_ptr = eli_cli(backCliData.numCli, backCliData.array_clientes, backCliData.array_clientes[i].pid);
            if(auxCliente_ptr != NULL){
              backCliData.array_clientes = auxCliente_ptr;
              pthread_mutex_unlock(backCliData.trinco);
              printf("\n[%s] was kicked.\n",nome);
              aux=1;
            } else {
              pthread_mutex_unlock(backCliData.trinco);
              printf("\n[USER NAO ENCONTRADO]");
            }

          } 
        }
        if(!aux){
          printf("\n[USER [%s] NAO ENCONTRADO]\n",arr[1]);
        }
      }
      else
        printf("\nErro. kick <username>");

    }
    else if (strcmp(arr[0], "prom") == 0){
      if (i == 1)
        lista_prom(backCliData.promo_names,numPromo);
      else
        printf("\nErro. prom");
    }
    else if (strcmp(arr[0], "reprom") == 0){
      if (i == 1){
        for(int c=0; c<numPromo; c++){
          printf("\n[%d]\n",backCliData.prom_pids[c]);
          kill(backCliData.prom_pids[c],SIGUSR1);
        }
        numPromo=0;
      }
      else
        printf("\nErro. reprom");
    }
    else if (strcmp(arr[0], "cancel") == 0){
      int flag=0;
      if(i == 2) {
        for(int c=0; c<numPromo; c++){
          if(strcmp(backCliData.promo_names[c],arr[1])==0 && !flag){

            pthread_mutex_lock(backCliData.trinco);

            kill(backCliData.prom_pids[c],SIGUSR1);
            memmove(&backCliData.prom_pids[c], &backCliData.prom_pids[c + 1], (numPromo - c - 1) * sizeof(int));
            backCliData.prom_pids[numPromo-1]=0;

            free(backCliData.promo_names[c]);
            memmove(&backCliData.promo_names[c], &backCliData.promo_names[c+1], (numPromo - c -1) * sizeof(char *));
            backCliData.promo_names[numPromo-1]=NULL;

            memmove(&backCliData.pipe_arr[c], &backCliData.pipe_arr[c+1], (numPromo - c - 1) * sizeof(int[2]));
            backCliData.pipe_arr[numPromo-1][0] = backCliData.pipe_arr[numPromo-1][1] = 0;

            numPromo--;

            pthread_mutex_unlock(backCliData.trinco); 

            flag=1;

            printf("\nPromotor: [%s] removido com sucesso.\n",arr[1]);
            
          }

        }

        if(!flag)
          printf("\nPromotor: [%s] nao encontrado\n",arr[1]);

      } else   
        printf("\nErro. cancel <nome-do-executavel-do-promotor>");
     }
     else
      printf("\nComando desconhecido\n");

    i = 0;
  }
  
  //---Sinais----
  pthread_kill(thread_recebeCli, SIGUSR1);  
  pthread_kill(thread_recebeItem, SIGUSR1); 
  pthread_kill(thread_leilao, SIGUSR1); 
  pthread_kill(thread_licitador, SIGUSR1); 
  pthread_kill(thread_info, SIGUSR1); 
  //pthread_kill(thread_promo, SIGUSR1);
  //-------------

  //pthread join's
  pthread_join(thread_recebeCli,NULL); 
  pthread_join(thread_recebeItem,NULL); 
  pthread_join(thread_leilao,NULL); 
  pthread_join(thread_licitador,NULL); 
  pthread_join(thread_info,NULL); 
  //pthread_join(thread_promo,NULL); 


  printf("\n[THREADS FINALIZADAS]\n");
  
  //Guardar Itens ainda em leilão no respectivo ficheiro

  if((*backCliData.numItens)!=0){// Caso existam Itens para guardar no ficheiro

    FILE *fp = fopen(FITEMS, "w");   
    if (fp == NULL){
      perror("Error opening file");
      return 1;
    }

    for (i=0; i < (*backCliData.numItens); i++){ 
      if(backCliData.array_itens[i].lastBid==0){
        fprintf(fp, "%d %s %s %d %d %d %s %s\n",
        backCliData.array_itens[i].id,
        backCliData.array_itens[i].name,
        backCliData.array_itens[i].catg,
        backCliData.array_itens[i].value,
        backCliData.array_itens[i].buyN,
        backCliData.array_itens[i].time,
        backCliData.array_itens[i].userOwner,
        backCliData.array_itens[i].userLbuyer);
      }else{
        fprintf(fp, "%d %s %s %d %d %d %s %s\n",
        backCliData.array_itens[i].id,
        backCliData.array_itens[i].name,
        backCliData.array_itens[i].catg,
        backCliData.array_itens[i].lastBid,
        backCliData.array_itens[i].buyN,
        backCliData.array_itens[i].time,
        backCliData.array_itens[i].userOwner,
        backCliData.array_itens[i].userLbuyer);
      } 
    }

    fclose(fp); 
  }
  
  unlink(BACK_FIFO);
  unlink(BACK_FIFO2);
  unlink(BACK_FIFO3);
  unlink(BACK_FIFO4);

return 0;
}

#ifndef _USERS_H_
#define _USERS_H_

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>


int loadUsersFile(char * pathname);
// Le os dados do utilizadores (username, password, saldo)
// O armazenamento dos utilizadores é gerido internamente por esta biblioteca
// Retorna    o número de utilizadores lidos
//           -1 em caso de erro 


int saveUsersFile(char * filename);
// Grava os dados dos utilizadores em ficheiro.
// Retorna    0 em caso de sucesso
//           -1 em caso de erro

 
int isUserValid(char * username, char * password);
// Verifica se o utilizador existe (se estava entre os que foram lidos por loadUsersFile) e a password está certa 
// Retorna   -1 em caso de erro
//            0 se o utilizador não existir/password inválida
//            1 se o utilizador existe e apassword está certa


int getUserBalance(char * username);
// Obtem o saldo do utilizador
// Retorna    o saldo do utilziador indicado
//           -1 em caso de erro 


int updateUserBalance(char * username, int value);
// Atualiza o saldo do utilizador
// Retorna    0 em caso de sucesso
//           -1 em caso de erro 



const char * getLastErrorText();
// Retorna o ponteiro para uma mensagem descritiva acerca da última operação desta biblioteca
// A invocação de uma destas funções (excepto esta) apaga sempre a mensagem anterior mesmo que não haja erro nenhum

#endif //_USERS_H_

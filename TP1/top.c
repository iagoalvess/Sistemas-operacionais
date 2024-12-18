#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <pwd.h>
#include <time.h>
#include <pthread.h>



void* loop_infinito() {

     while(1) {

        system("clear");

        FILE* fp = popen("cat /proc/*/status 2>/dev/null", "r");
        
        if(fp == NULL)
            perror("Erro ao executar o comando\n");
        
        printf("PID            | User           | PROCNAME       | Estado         |\n");
        printf("---------------|----------------|----------------|----------------|\n");

        char linha[500];
        int pid = -1;
        char proc_nome[20] = "";
        char estado = '\0';
        char user[20] = "";
        
        int vinte = 0;
        while (fgets(linha, sizeof(linha), fp) != NULL) {
             
                char chave[20];
                char valor[20];
                    
                sscanf(linha, "%s %s", chave, valor);
                    
                if(strcmp(chave, "Name:") == 0)
                    strcpy(proc_nome, valor);
                else if(strcmp(chave, "State:") == 0)
                    estado = valor[0];
                else if(strcmp(chave, "Pid:") == 0) {

                    char *final_ptr;
                    pid = (int) strtol(valor, &final_ptr, 10);
                }
                else if(strcmp(chave, "Uid:") == 0) {

                    char *final_ptr;
                    user[0] = 'a';
                    int id_user = (int) strtol(valor, &final_ptr, 10);
                    struct passwd *s = getpwuid(id_user);
                    strcpy(user, s->pw_name);  
                }

                if (strcmp(proc_nome, "") != 0 && estado != '\0' && pid != -1 && strcmp(user, "") != 0) {

                    printf("%-14d | %-14s | %-14s | %-14c |\n", pid, user, proc_nome, estado);
                    pid = -1; strcpy(proc_nome, ""); 
                    estado = '\0'; strcpy(user, "");
                    vinte = vinte + 1;
                }

                if (vinte == 20) {
                    
                   break;
                }
                
            }
            
            sleep(1);
            pclose(fp);
    }

}

void* input () {

    while(1) {  

        int pid;
        int tipo_sinal;
      
        scanf("%i %i", &pid, &tipo_sinal);
      
        if (kill(pid, tipo_sinal) == 0) {
                printf("Sinal enviado para o processo %d\n", pid);
        } else {
                perror("Erro ao enviar sinal");
        }

    }
}



int main () {


    pthread_t thread1, thread2;

    if (pthread_create(&thread1, NULL, loop_infinito, NULL) != 0) {
        perror("Erro ao criar thread 1");
        return 1;
    }

    if (pthread_create(&thread2, NULL, input, NULL) != 0) {
        perror("Erro ao criar thread 2");
        return 1;
    }



    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pwd.h>
#include <time.h>

int main () {


    while(1) {

        system("clear");

        int lista_id[50];
        char id[40];

        FILE* fp = popen("ls /proc | grep '^[0-9]\\+$'", "r");

        if(fp == NULL)
            perror("Erro ao executar o comando\n");
        
        int indice = 0;

        while (fgets(id, sizeof(id), fp) != NULL) {

            int num = atoi(id);
            lista_id[indice] = num;
            indice = indice + 1;
        }

        pclose(fp);

        int vinte = 0;

        printf("PID        | User       | PROCNAME   | Estado     |\n");
        printf("-----------|------------|------------|------------|\n");

        for(int i = 0; i < sizeof(lista_id) / sizeof(lista_id[0]); i++) {

            char caminho[20];
            sprintf(caminho, "cat /proc/%i/status", lista_id[i]);
            
            freopen("/dev/null", "w", stderr);
            FILE* fp_id = popen(caminho, "r");
            
            if(fp_id == NULL) {
                perror("Erro ao executar o comando\n");
                i++;
                continue;
            }

            int pid = -1;
            char proc_nome[20] = "";
            char estado = '\0';
            char user[20] = "";

            char linha[256];
    
            while (fgets(linha, sizeof(linha), fp_id) != NULL) {
                
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
                    
                    int id_user = (int) strtol(valor, &final_ptr, 10);
                    struct passwd *s = getpwuid(id_user);
                    strcpy(user, s->pw_name);  
                }

            
        
                if (strcmp(proc_nome, "") != 0 && estado != '\0' && pid != -1 && strcmp(user, "") != 0)
                    break;

            }

            if (pid == -1) {
                i++;
                continue;
            }
                

            printf("%-10d | %-10s | %-10s | %-10c |\n", pid, user, proc_nome, estado);


            pclose(fp_id);
            
            vinte++;
        
            if(vinte == 20)
                break;
        }

        sleep(1);
    }
    

    return 0;
}
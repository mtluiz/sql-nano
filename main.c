#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char* buffer;
    size_t buffer_length;
    ssize_t input_length;
} Ibuffer;

Ibuffer* newIbuffer(){
    Ibuffer* ibuffer = malloc(sizeof(Ibuffer));
    ibuffer->buffer= NULL;
    ibuffer->buffer_length = 0;
    ibuffer->input_length = 0;
    return ibuffer;
}

void printTerminal() { printf("db > "); }

void readInput( Ibuffer* ibuffer ){
    ssize_t bytesRead = getline(&(ibuffer->buffer), &(ibuffer->buffer_length), stdin);

    if(bytesRead <= 0){
        printf("Erro durante leitura\n");
        exit(EXIT_FAILURE);
    }
    
    ibuffer->input_length = bytesRead - 1;
    ibuffer->buffer[bytesRead -1] = 0;
}

void closeInputBuffer(Ibuffer* ibuffer){
    free(ibuffer->buffer);
    free(ibuffer);
}

int main(int argc, char* argv[]){
    Ibuffer* ibuffer = newIbuffer();
    while(true){
        printTerminal();
        readInput( ibuffer );
        if (strcmp(ibuffer->buffer, ".sair")) {
            closeInputBuffer(ibuffer);
            exit(EXIT_SUCCESS);
        } else {
            printf("Comando desconhecido '%s'.\n", ibuffer->buffer);
        }
    }
}

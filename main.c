#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 255
typedef struct
{
    char *buffer;
    size_t buffer_length;
    ssize_t input_length;
} Ibuffer;

typedef enum
{
    META_COMMAND_SUCCESS,
    META_COMMAND_UNRECOGNIZED_COMMAND
} MetaCommandResult;

typedef enum
{
    PREPARE_SUCCESS,
    PREPARE_UNRECOGNIZED_STATEMENT,
    PREPARE_SYNTAX_ERROR
} PrepareResult;

typedef enum
{
    STATEMENT_INSERT,
    STATEMENT_SELECT
} StatementType;

typedef struct
{
    uint32_t id;
    char username[COLUMN_USERNAME_SIZE];
    char email[COLUMN_EMAIL_SIZE];
} Row;
typedef struct
{
    StatementType type;
    Row rowToInsert;
} Statement;

Ibuffer *newIbuffer()
{
    Ibuffer *ibuffer = malloc(sizeof(Ibuffer));
    ibuffer->buffer = NULL;
    ibuffer->buffer_length = 0;
    ibuffer->input_length = 0;
    return ibuffer;
}
void printTerminal() { printf("db > "); }

void readInput(Ibuffer *ibuffer)
{
    ssize_t bytesRead = getline(&(ibuffer->buffer), &(ibuffer->buffer_length), stdin);

    if (bytesRead <= 0)
    {
        printf("Erro durante leitura\n");
        exit(EXIT_FAILURE);
    }

    ibuffer->input_length = bytesRead - 1;
    ibuffer->buffer[bytesRead - 1] = 0;
}

void closeInputBuffer(Ibuffer *ibuffer)
{
    free(ibuffer->buffer);
    free(ibuffer);
}

MetaCommandResult doMetaCommand(Ibuffer *ibuffer)
{
    if (strcmp(ibuffer->buffer, ".sair") == 0)
    {
        exit(EXIT_SUCCESS);
    }
    else
    {
        return META_COMMAND_UNRECOGNIZED_COMMAND;
    }
}

PrepareResult prepareStatement(Ibuffer *ibuffer, Statement *statement)
{
    if (strncmp(ibuffer->buffer, "insert", 6) == 0)
    {
        statement->type = STATEMENT_INSERT;
        int assignedArgs = sscanf(ibuffer->buffer, "insert %d %s %s", &(statement->rowToInsert.id), statement->rowToInsert.username, statement->rowToInsert.email);
        if (assignedArgs < 3)
        {
            return PREPARE_SYNTAX_ERROR;
        }
        return PREPARE_SUCCESS;
    }

    if (strncmp(ibuffer->buffer, "select", 6) == 0)
    {
        statement->type = STATEMENT_SELECT;
        return PREPARE_SUCCESS;
    }

    return PREPARE_UNRECOGNIZED_STATEMENT;
}

void executeStatement(Statement *statement)
{
    switch (statement->type)
    {
    case (STATEMENT_INSERT):
        printf("Aqui faremos um insert");
        break;
    case (STATEMENT_SELECT):
        printf("Aqui faremos um select");
        break;
    };
}

int main(int argc, char *argv[])
{
    Ibuffer *ibuffer = newIbuffer();
    while (true)
    {

        printTerminal();

        readInput(ibuffer);

        if (ibuffer->buffer[0] == '.')
        {
            switch (doMetaCommand(ibuffer))
            {
            case (META_COMMAND_SUCCESS):
                continue;
            case (META_COMMAND_UNRECOGNIZED_COMMAND):
                printf("Comando desconhecido '%s'.\n", ibuffer->buffer);
            }
        }

        Statement statement;
        switch (prepareStatement(ibuffer, &statement))
        {
        case (PREPARE_SUCCESS):
            break;
        case (PREPARE_UNRECOGNIZED_STATEMENT):
            printf("Palavra-chave desconhecida no inicio de '%s'. \n ", ibuffer->buffer);
            continue;
        }

        executeStatement(&statement);
        printf("Executado. \n");
    }
}

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

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
    EXECUTE_SUCCESS,
    EXECUTE_FULL_TABLE
} ExecuteResult;

typedef enum
{
    STATEMENT_INSERT,
    STATEMENT_SELECT
} StatementType;

typedef struct
{
    uint32_t id;
    char username[COLUMN_USERNAME_SIZE + 1];
    char email[COLUMN_EMAIL_SIZE + 1];
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

#define size_of_attribute(Struct, Attribute) sizeof(((Struct *)0)->Attribute)

const uint32_t ID_SIZE = size_of_attribute(Row, id);
const uint32_t USERNAME_SIZE = size_of_attribute(Row, username);
const uint32_t EMAIL_SIZE = size_of_attribute(Row, email);
const uint32_t ID_OFFSET = 0;
const uint32_t USERNAME_OFFSET = ID_OFFSET + ID_SIZE;
const uint32_t EMAIL_OFFSET = USERNAME_OFFSET + USERNAME_SIZE;
const uint32_t ROW_SIZE = ID_SIZE + USERNAME_SIZE + EMAIL_SIZE;
const uint32_t PAGE_SIZE = 4096;

#define TABLE_MAX_PAGES 100

const uint32_t ROWS_PER_PAGE = PAGE_SIZE / ROW_SIZE;
const uint32_t TABLE_MAX_ROWS = ROWS_PER_PAGE * TABLE_MAX_PAGES;

typedef struct
{
    uint32_t num_rows;
    void *pages[TABLE_MAX_PAGES];
} Table;

void print_row(Row *row)
{
    printf("(%d, %s, %s)\n", row->id, row->username, row->email);
}

void serialize_row(Row *source, void *destination)
{
    memcpy(destination + ID_OFFSET, &(source->id), ID_SIZE);
    memcpy(destination + USERNAME_OFFSET, &(source->username), USERNAME_SIZE);
    memcpy(destination + EMAIL_OFFSET, &(source->email), EMAIL_SIZE);
}

void deserialize_row(void *source, Row *destination)
{
    memcpy(&(destination->id), source + ID_OFFSET, ID_SIZE);
    memcpy(&(destination->username), source + USERNAME_OFFSET, USERNAME_SIZE);
    memcpy(&(destination->email), source + EMAIL_OFFSET, EMAIL_SIZE);
}

void *row_slot(Table *table, uint32_t row_num)
{
    uint32_t page_num = row_num / ROWS_PER_PAGE;
    void *page = table->pages[page_num];
    if (page == NULL)
    {
        page = table->pages[page_num] = malloc(PAGE_SIZE);
    }

    uint32_t row_offset = row_num % ROWS_PER_PAGE;
    uint32_t byte_offset = row_offset * ROW_SIZE;
    return page + byte_offset;
}

Table *new_table()
{
    Table *table = (Table *)malloc(sizeof(Table));
    table->num_rows = 0;

    for (u_int32_t i = 0; i < TABLE_MAX_PAGES; i++)
    {
        table->pages[i] = NULL;
    }
    return table;
}

void free_table(Table *table)
{
    for (int i = 0; table->pages[i]; i++)
    {
        free(table->pages[i]);
    }
    free(table);
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

MetaCommandResult doMetaCommand(Ibuffer *ibuffer, Table *table)
{
    if (strcmp(ibuffer->buffer, ".sair") == 0)
    {
        closeInputBuffer(ibuffer);
        free_table(table);
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

ExecuteResult execute_insert(Statement *statement, Table *table)
{
    if (table->num_rows >= TABLE_MAX_ROWS)
        return EXECUTE_FULL_TABLE;

    Row *row_to_insert = &(statement->rowToInsert);

    serialize_row(row_to_insert, row_slot(table, table->num_rows));
    table->num_rows += 1;

    return EXECUTE_SUCCESS;
}

ExecuteResult execute_select(Statement *statement, Table *table)
{
    Row row;
    for (uint32_t i = 0; i < table->num_rows; i++)
    {
        deserialize_row(row_slot(table, i), &row);
        print_row(&row);
    }
    return EXECUTE_SUCCESS;
}

ExecuteResult execute_statement(Statement *statement, Table *table)
{
    switch (statement->type)
    {
    case (STATEMENT_INSERT):
        return execute_insert(statement, table);
    case (STATEMENT_SELECT):
        return execute_select(statement, table);
    }
}

ExecuteResult execute_statement()
{
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
    Table *table = new_table();
    Ibuffer *ibuffer = newIbuffer();
    while (true)
    {

        printTerminal();

        readInput(ibuffer);

        if (ibuffer->buffer[0] == '.')
        {
            switch (doMetaCommand(ibuffer, table))
            {
            case (META_COMMAND_SUCCESS):
                continue;
            case (META_COMMAND_UNRECOGNIZED_COMMAND):
                printf("Comando desconhecido '%s'.\n", ibuffer->buffer);
            }
        }

        Statement statement;
        switch (prepare_statement(ibuffer, &statement))
        {
        case (PREPARE_SUCCESS):
            break;
        case (PREPARE_SYNTAX_ERROR):
            printf("Syntax error. Could not parse statement.\n");
            continue;
        case (PREPARE_UNRECOGNIZED_STATEMENT):
            printf("Unrecognized keyword at start of '%s'.\n",
                   ibuffer->buffer);
            continue;
        }

        switch (execute_statement(&statement, table))
        {
        case (EXECUTE_SUCCESS):
            printf("Executed.\n");
            break;
        case (EXECUTE_FULL_TABLE):
            printf("Error: Table full.\n");
            break;

            executeStatement(&statement);
            printf("Executado. \n");
        }
    }

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#define MAX_RL_BUFSIZE 512
#define SHELL_TOK_BUFSIZE 64
#define SHELL_TOK_DELIM " \t\r\n\a"
#define MAX_FILENAME_SZ 256
#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN "\x1b[36m"
#define ANSI_COLOR_RESET "\x1b[0m"
// Clearing the shell using escape sequences
#define clear() printf("\033[H\033[J")

int shell_cd(char **args);
int shell_help(char **args);
int shell_quit(char **args);

int shell_num_builtins();
// Greeting shell during startup
void init_shell()
{
    clear();
    printf(ANSI_COLOR_YELLOW "\n\n\n\n******************"
                             "************************");
    printf("\n\n\n\t****WELCOME TO MINISHELL****");
    printf(ANSI_COLOR_RED "\n\n\t    use at your own risk");
    printf(ANSI_COLOR_YELLOW "\n\n\n\n*******************"
                             "***********************");
    char *username = getenv("USER");
    printf("\n\n\nUSER is: " ANSI_COLOR_CYAN "@%s" ANSI_COLOR_RESET, username);
    printf("\n\n\n\n\n\n");
    sleep(2);
    clear();
}
char *builtin_str[] = {
    "cd",
    "help",
    "quit"};

int (*builtin_func[])(char **) = {
    &shell_cd,
    &shell_help,
    &shell_quit};

void shell_loop(void);
char *shell_read_line(void);
char **shell_split_line(char *line);
int shell_launch(char **args);
int shell_execute(char **args);
void file_handler(char filename[MAX_FILENAME_SZ]);

/****** Main *******/
int main(int argc, char **argv)
{
    init_shell();

    if (argc == 1)
    {

        shell_loop();
    }
    else if (argc != 2)
    {
        printf("Usage: %s batchfile -- for batch mode", argv[0]);
        exit(1);
    }
    else
    {
        file_handler(argv[1]);
    }
    return EXIT_SUCCESS;
}

void shell_loop(void)
{
    char *input_line;
    char **args;
    while (1)
    {
        printf(ANSI_COLOR_GREEN "antoniadis_8761> " ANSI_COLOR_RESET);
        input_line = shell_read_line();

        args = shell_split_line(input_line);

        shell_execute(args);

        free(input_line);
        free(args);
    }
}

char *shell_read_line(void)
{
    int bufsize = MAX_RL_BUFSIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * bufsize);
    int c;

    if (!buffer)
    {
        fprintf(stderr, "MiniShell: allocation error\n");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        c = getchar();

        // If we hit EOF, replace it with a null character and return.
        if (c == EOF || c == '\n')
        {
            buffer[position] = '\0';
            return buffer;
        }
        else
        {
            buffer[position] = c;
        }
        position++;

        // If we have exceeded the buffer, exit.
        if (position >= bufsize)
        {
            fprintf(stderr, "MiniShell: command has more than 512 characters\n");
            exit(EXIT_FAILURE);
        }
    }
}

char **shell_split_line(char *line)
{
    int bufsize = SHELL_TOK_BUFSIZE, position = 0;
    char **toks = malloc(bufsize * sizeof(char *));
    char *tok;

    if (!toks)
    {
        fprintf(stderr, "MiniShell: allocation error\n");
        exit(EXIT_FAILURE);
    }
    tok = strtok(line, SHELL_TOK_DELIM);

    while (tok != NULL)
    {
        toks[position] = tok;
        position++;

        if (position >= bufsize)
        {
            bufsize += SHELL_TOK_BUFSIZE;
            toks = realloc(toks, bufsize);
            if (!toks)
            {
                fprintf(stderr, "MiniShell: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        tok = strtok(NULL, SHELL_TOK_DELIM);
    }
    toks[position] = NULL;
    return toks;
}

int shell_execute(char **args)
{

    if (args[0] == NULL)
    {
        // An empty command was entered.
        return 1;
    }

    /*Check if input is quit*/
    if (strcmp(args[0], builtin_str[shell_num_builtins() - 1]) == 0)
    {
        return (*builtin_func[shell_num_builtins() - 1])(args);
    }

    int i = 0, cnt = 1, cnt2 = 0;
    char **args_buf;
    while (1)
    {

        if (args[i] == NULL || !(strcmp(args[i], "&&")) || !(strcmp(args[i], ";")))
        {

            if ((args_buf = (char **)malloc((cnt) * sizeof(char *))) == NULL)
            {

                perror("Failed to allocate");
                exit(1);
            }

            if (i - cnt + 1 == 0)
            {
                for (int k = 0; k < cnt - 1; ++k)
                {
                    args_buf[k] = strdup(args[k]);
                }
            }

            else
            {
                for (int k = 0; k < cnt - 1; ++k)
                {
                    args_buf[k] = strdup(args[i - cnt + 1 + k]);
                }
            }

            args_buf[cnt - 1] = NULL;

            if (shell_launch(args_buf) == -1)
            {
                if (args[i] != NULL && !(strcmp(args[i], "&&")))
                {
                    free(args_buf);
                    return -1;
                }
            }
            free(args_buf);
            cnt = 0;
        }
        if (args[i] == NULL)
            break;
        cnt++;
        i++;
    }

    return -1;
}

int shell_launch(char **args)
{
    pid_t pid, wpid;
    int i, status;

    pid = fork();
    if (pid == 0)
    {
        for (i = 0; i < shell_num_builtins() - 1; i++) //except quit
        {
            if (strcmp(args[0], builtin_str[i]) == 0)
            {
                return (*builtin_func[i])(args);
            }
        }
        
        if (execvp(args[0], args) == -1)
        {
            printf("%s: command not found\n", args[0]);
        }
        exit(EXIT_FAILURE);
    }
    else if (pid < 0)
    {
        perror("MiniShell");
    }
    /*Parent process*/
    else
    {
        do
        {
            wpid = waitpid(pid, &status, WUNTRACED);

        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        if (status)
            return -1;
    }
    return 1;
}

int shell_num_builtins()
{
    return sizeof(builtin_str) / sizeof(char *);
}

int shell_cd(char **args)
{
    if (args[1] == NULL)
    {
        fprintf(stderr, "MiniShell: expected argument to \"cd\"\n");
    }
    else
    {
        if (chdir(args[1]) != 0)
        {
            perror("MiniShell");
        }
    }
    return 1;
}

int shell_help(char **args)
{
    int i;

    printf("Antoniadis Moschos 8761\n");

    printf("The following are built in:\n");

    for (i = 0; i < shell_num_builtins(); i++)
    {
        printf("  %s\n", builtin_str[i]);
    }

    printf("Use man for information on other programs.\n");
    return 1;
}

int shell_quit(char **args)
{
    printf("Bye bye!\n\n");
    exit(EXIT_SUCCESS);
}

void file_handler(char filename[MAX_FILENAME_SZ])
{
    FILE *batch;
    char input_line[MAX_RL_BUFSIZE];
    char **args;
    int lineNo = 1;
    if ((batch = fopen(filename, "r+")) == NULL)
    {
        printf("MiniShell: Batchfile not found\a\n");

        exit(EXIT_FAILURE);
    }
    printf(ANSI_COLOR_MAGENTA "****Starting the execution of batchfile****\n" ANSI_COLOR_RESET);

    while (fgets(input_line, MAX_RL_BUFSIZE, batch))
    { // read a line

        printf(ANSI_COLOR_GREEN "****** %d.: %s" ANSI_COLOR_RESET, lineNo++, input_line); // print the command number
        if (input_line[0] == '\n')
        {
            printf("--empty line--\n\n");
            continue;
        }
        args = shell_split_line(input_line);
        printf("\n");

        shell_execute(args);

        printf("\n");
    }
    printf(ANSI_COLOR_MAGENTA "**END of file**\n" ANSI_COLOR_RESET);
    /*Check if last input is quit*/
    if (strcmp(args[0], builtin_str[shell_num_builtins() - 1]) != 0)
    {
        printf("No quit detected. Bye!\n\n");
        exit(EXIT_SUCCESS);
    }

    exit(0);
}
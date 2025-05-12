#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>

int cd(char* path);
int pwd();
void multi_pipe(char *command);
char *remove_whitespace(char *str);
void logical_command(char *logic);
void background(int check);
int single_command(char *command);
void split_args(char *input, char **args);

int cd(char* path) {
    path += 3;
    path = remove_whitespace(path);
    if (chdir(path) != 0) {
        perror("cd 오류");
        return 1;
    }
    return 0;
}

int pwd() {
    char cwd[100];
    if (getcwd(cwd, sizeof(cwd))) {
        printf("%s\n", cwd);
        return 0;
    } else {
        perror("pwd 오류");
        return 1;
    }
}

void multi_pipe(char *command) {
    char *cmds[10];
    int num = 0;

    char *token = strtok(command, "|");
    while (token != NULL && num < 10) {
        token = remove_whitespace(token);
        if (strlen(token) > 0) {
            cmds[num++] = token;
        }
        token = strtok(NULL, "|");
    }

    int prev_fd = -1;

    for (int i = 0; i < num; i++) {
        char *args[100];
        split_args(cmds[i], args);

        int pipe_fd[2];
        if (i < num - 1 && pipe(pipe_fd) < 0) {
            perror("pipe 실패");
            exit(1);
        }

        pid_t pid = fork();
        if (pid == 0) {
            if (prev_fd != -1) {
                dup2(prev_fd, 0);
                close(prev_fd);
            }
            if (i < num - 1) {
                close(pipe_fd[0]);
                dup2(pipe_fd[1], 1);
                close(pipe_fd[1]);
            }

            execvp(args[0], args);
            perror("명령 실행 실패");
            exit(1);
        } 
        else {
            if (prev_fd != -1) {
                close(prev_fd);
            }
            if (i < num - 1) {
                close(pipe_fd[1]);
                prev_fd = pipe_fd[0];
            }
            waitpid(pid, NULL, 0);
        }
    }
}

char *remove_whitespace(char *str) {
    while (*str == ' ' || *str == '\t' || *str == '\n') {
        str++;
    }

    char *end = str + strlen(str) - 1;

    while (end > str && (*end == ' ' || *end == '\t' || *end == '\n')) {
        *end = '\0';
        end--;
    }
    return str;
}

void logical_command(char *logic) {
    char *curr = logic;
    int last = 0;         
    int run_next = 1;

    while (*curr != '\0') {
        char *next_and = strstr(curr, "&&");
        char *next_or  = strstr(curr, "||");
        char *next_semi = strchr(curr, ';');

        char *next = NULL;
        int type = 0;

        if (next_and && (!next || next_and < next)) {
            next = next_and;
            type = 1;
        }

        if (next_or && (!next || next_or < next)) {
            next = next_or;
            type = 2;
        }

        if (next_semi && (!next || next_semi < next)) {
            next = next_semi;
            type = 3;
        }

        char *segment = curr;
        if (next != NULL) {
            *next = '\0';
        }

        segment = remove_whitespace(segment);

        if (run_next && strlen(segment) > 0) {
            last = single_command(segment);
        }

        if (next != NULL) {
            if (type == 1) {
                run_next = (last == 0);
            }
            else if (type == 2) {
                run_next = (last != 0);   
            } 
            else {
                run_next = 1;             
            }
            curr = next + (type == 3 ? 1 : 2);  
        } 
        else {
            break;
        }
    }
}

void background(int check) {
    int saved_errno = errno;
    pid_t pid;
    int status;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        printf("[백그라운드 종료] pid: %d\n", pid);
        fflush(stdout);
    }

    errno = saved_errno;
}

void split_args(char *input, char **args) {
    int i = 0;
    char *token = strtok(input, " ");

    while (token != NULL && i < 99) {
        args[i++] = remove_whitespace(token);
        token = strtok(NULL, " ");
    }
    args[i] = NULL;
}

int single_command(char *command) {
    command = remove_whitespace(command);

    int background = 0;
    size_t len = strlen(command);
    if (len > 0 && command[len - 1] == '&') {
        background = 1;
        command[len - 1] = '\0';  
        command = remove_whitespace(command);
    }

    if (strcmp(command, "exit") == 0) {
        exit(0);
    }
    if (strncmp(command, "cd ", 3) == 0) {
        return cd(command);
    }
    if (strcmp(command, "pwd") == 0) {
        return pwd();
    }
    if (strchr(command, '|')) {
        multi_pipe(command);
        return 0;
    }

    char *args[100];
    split_args(command, args);

    pid_t pid = fork();
    if (pid == 0) {
        execvp(args[0], args);
        perror("실행 실패");
        exit(1);
    } 
    else {
        if (!background) {
            int status;
            waitpid(pid, &status, 0);
            return WEXITSTATUS(status);
        } 
        else {
            printf("[백그라운드 실행] pid: %d\n", pid);
            return 0;  
        }
    }
}

int main() {
    signal(SIGCHLD, background);

    char command[100];
    
    while (1) {
        char cwd[100];
        getcwd(cwd, sizeof(cwd));
        printf("%s$ ", cwd);
        fflush(stdout);

        if (!fgets(command, sizeof(command), stdin)) {
            break;
        }

        command[strcspn(command, "\n")] = 0;

        logical_command(command);
    }

    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/wait.h>

void cd(char* path){
    path += 3;
    if (chdir(path) != 0) {
        perror("cd 오류");
    }
}

void pwd() {
    char cwd[1000];
    if (getcwd(cwd, sizeof(cwd))) {
        printf("%s\n", cwd);
    } 
    else {
        perror("pwd 오류");
    }
}

void other_commands(char *command) {
    char *args[100];
    int i = 0;

    char *token = strtok(command, " ");
    while (token != NULL && i < 99)
    {
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork 실패");
    } 
    else if (pid == 0) {
        execvp(args[0], args);
        perror("명령 실행 실패");
        exit(1);
    } 
    else {
        waitpid(pid, NULL, 0);
    }
}

int main() {
    char command[1000];

    while (1) {
        char cwd[1000];
        getcwd(cwd, sizeof(cwd));
        printf("%s$ ", cwd);
        fflush(stdout);

        fgets(command, sizeof(command), stdin);
        command[strcspn(command, "\n")] = 0;

        if (strcmp(command, "exit") == 0) {
            break;
        }
        else if (strncmp(command, "cd ", 3) == 0) {
            cd(command);
        }
        else if (strcmp(command, "pwd") == 0) {
            pwd();
        }
        else {
            other_commands(command);
        }
    }

    return 0;
}

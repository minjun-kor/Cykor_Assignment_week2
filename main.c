#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/wait.h>

void split_args(char *input, char **args) {
    int i = 0;
    char *token = strtok(input, " ");
    while (token != NULL && i < 99) {
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;
}

void cd(char* path){
    path += 3;
    if (chdir(path) != 0) {
        perror("cd 오류");
    }
}

void pwd() {
    char cwd[100];
    if (getcwd(cwd, sizeof(cwd))) {
        printf("%s\n", cwd);
    } 
    else {
        perror("pwd 오류");
    }
}

void other_commands(char *command) {
    char *args[100];
    split_args(command, args);

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

void one_pipe(char *command) {
    char *input1 = strtok(command, "|");
    char *input2 = strtok(NULL, "|");

    if (input1 == NULL || input2 == NULL) {
        fprintf(stderr, "파이프 명령어 형식 오류\n");
        return;
    }

    char *args1[100], *args2[100];
    split_args(input1, args1);
    split_args(input2, args2);

    int fd[2];
    pipe(fd);

    pid_t pid1 = fork();
    if (pid1 == 0) {
        dup2(fd[1], STDOUT_FILENO);
        close(fd[0]);
        close(fd[1]);
        execvp(args1[0], args1);
        perror("파이프 왼쪽 실행 실패");
        exit(1);
    }

    pid_t pid2 = fork();
    if (pid2 == 0) {
        dup2(fd[0], STDIN_FILENO);
        close(fd[1]);
        close(fd[0]);
        execvp(args2[0], args2);
        perror("파이프 오른쪽 실행 실패");
        exit(1);
    }

    close(fd[0]);
    close(fd[1]);
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);

}


int main() {
    char command[100];

    while (1) {
        char cwd[100];
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
        else if (strchr(command, '|') != NULL) {
            one_pipe(command);
        }
        else {
            other_commands(command);
        }
    }

    return 0;
}

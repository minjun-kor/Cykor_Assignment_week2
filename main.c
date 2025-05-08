#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>


void cd(char* path){
    path += 3;
    if (chdir(path) != 0) {
        perror("cd 오류");
    }
}

void pwd() {
    char cwd[1000];
    if (!(getcwd(cwd, sizeof(cwd)))) {
        printf("%s\n", cwd);
    } 
    else {
        perror("pwd 오류");
    }
}

int main() {
    char command[1000];

    while (1) {
        char cwd[1000];
        getcwd(cwd, sizeof(cwd));
        printf("%s$ ", cwd);

        fgets(command, sizeof(command), stdin);
        command[strcspn(command, "\n")] = 0;

        if (strcmp(command, "exit") == 0){
            break;
        }
        else if (strncmp(command, "cd ", 3) == 0) {
            cd(command);
        }
        else if (strcmp(command, "pwd") == 0) {
            pwd();
        }
        else {
            printf("지원하지 않는 명령어입니다: %s\n", command);
        }
    }

    return 0;
}

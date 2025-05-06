#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

int main() {
    char command[256];

    while (1) {
        char cwd[256];
        getcwd(cwd, sizeof(cwd));
        printf("%s$ ", cwd);

        fgets(command, sizeof(command), stdin);
        command[strcspn(command, "\n")] = 0;

        if (strcmp(command, "exit") == 0) {
            break;
        }

        if (strncmp(command, "cd ", 3) == 0) {
            char *path = command + 3;

            if (chdir(path) != 0) {
                perror("cd 오류");
            }

            continue;
        }

        printf("지원하지 않는 명령어입니다: %s\n", command);
    }

    return 0;
}

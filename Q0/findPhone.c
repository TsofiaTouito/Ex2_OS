#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
    // Validate the number of arguments
    if (argc < 2 || argc > 3)
    {
        fprintf(stderr, "Invalid number of arguments. Usage: %s <pattern>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int pipefd[2]; // Pipe file descriptors for communication between processes

    // Create a pipe
    if (pipe(pipefd) == -1)
    {
        perror("Pipe creation failed");
        return EXIT_FAILURE;
    }

    // Fork for the grep process
    pid_t grepPid = fork();
    if (grepPid == 0)
    {
        printf("Grep process ID: %d\n", getpid());
        dup2(pipefd[1], STDOUT_FILENO); // Redirect stdout to pipe
        close(pipefd[0]); // Close the read end of the pipe
        close(pipefd[1]); // Close the write end (after duplicating)
        execlp("grep", "grep", argv[1], "phoneBook.txt", NULL); // Execute grep
        perror("Grep execution failed");
        return EXIT_FAILURE;
    }
    else if (grepPid < 0)
    {
        perror("Fork for grep failed");
        return EXIT_FAILURE;
    }

    // Fork for the cut process
    pid_t cutPid = fork();
    if (cutPid == 0)
    {
        printf("Cut process ID: %d\n", getpid());
        dup2(pipefd[0], STDIN_FILENO); // Redirect stdin from pipe
        close(pipefd[0]); // Close the read end (after duplicating)
        close(pipefd[1]); // Close the write end of the pipe
        execlp("cut", "cut", "-d,", "-f2", NULL); // Execute cut
        perror("Cut execution failed");
        return EXIT_FAILURE;
    }
    else if (cutPid < 0)
    {
        perror("Fork for cut failed");
        return EXIT_FAILURE;
    }

    // Fork for the sed process
    pid_t sedPid = fork();
    if (sedPid == 0)
    {
        printf("Sed process ID: %d\n", getpid());
        dup2(pipefd[0], STDIN_FILENO); // Redirect stdin from pipe
        close(pipefd[0]); // Close the read end of the pipe
        close(pipefd[1]); // Close the write end of the pipe
        execlp("sed", "sed", "s/ //g", NULL); // Execute sed
        perror("Sed execution failed");
        return EXIT_FAILURE;
    }
    else if (sedPid < 0)
    {
        perror("Fork for sed failed");
        return EXIT_FAILURE;
    }

    printf("Parent process ID: %d\n", getpid());
    close(pipefd[0]); // Close the read end of the pipe
    close(pipefd[1]); // Close the write end of the pipe

    // Wait for all child processes to finish
    waitpid(grepPid, NULL, 0);
    waitpid(cutPid, NULL, 0);
    waitpid(sedPid, NULL, 0);

    return EXIT_SUCCESS;
}

/**
 * Unix Pipeline Implementation with Process Hierarchy
 * 
 * This program implements a three-stage Unix pipeline equivalent to:
 * cat scores | grep <search_term> | sort
 * 
 * It creates a process hierarchy with three processes:
 * - Parent process: executes 'cat scores' to read the file content
 * - First child process: executes 'grep <search_term>' to filter lines
 * - Grandchild process: executes 'sort' to alphabetically sort the results
 * 
 * The program takes one command-line argument:
 * - <search_term>: The term to search for in the file
 * 
 * It uses two pipes to connect the three processes:
 * - First pipe: connects cat (parent) to grep (child)
 * - Second pipe: connects grep (child) to sort (grandchild)
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    // Check if the correct number of arguments is provided
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <search_term>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    int pipefd1[2], pipefd2[2];  // File descriptors for the two pipes
    
    // Create the first pipe (parent to child)
    if (pipe(pipefd1) == -1) {
        perror("pipe1");
        exit(EXIT_FAILURE);
    }
    
    // Fork the first child process
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork1");
        exit(EXIT_FAILURE);
    }
    
    if (pid == 0) {
        // First child: executes grep
        // Create the second pipe (child to grandchild)
        if (pipe(pipefd2) == -1) {
            perror("pipe2");
            exit(EXIT_FAILURE);
        }
        
        // Fork the grandchild process
        pid_t pid2 = fork();
        if (pid2 < 0) {
            perror("fork2");
            exit(EXIT_FAILURE);
        }
        
        if (pid2 == 0) {
            // Child's child (grandchild): executes sort
            // Close unused pipe ends
            close(pipefd1[0]);
            close(pipefd1[1]);
            close(pipefd2[1]);
            
            // Redirect stdin to read from the second pipe
            dup2(pipefd2[0], STDIN_FILENO);
            close(pipefd2[0]);
            
            // Execute the sort command
            execlp("sort", "sort", (char *)NULL);
            perror("execlp sort");
            exit(EXIT_FAILURE);
        } else {
            // Child process continues: executes grep
            // Close unused pipe ends
            close(pipefd1[1]);
            close(pipefd2[0]);
            
            // Redirect stdin to read from the first pipe
            dup2(pipefd1[0], STDIN_FILENO);
            
            // Redirect stdout to write to the second pipe
            dup2(pipefd2[1], STDOUT_FILENO);
            
            // Close the original file descriptors
            close(pipefd1[0]);
            close(pipefd2[1]);
            
            // Execute the grep command with the search term
            execlp("grep", "grep", argv[1], (char *)NULL);
            perror("execlp grep");
            exit(EXIT_FAILURE);
        }
    } else {
        // Parent process: executes cat
        // Close unused pipe end
        close(pipefd1[0]);
        
        // Redirect stdout to write to the first pipe
        dup2(pipefd1[1], STDOUT_FILENO);
        close(pipefd1[1]);
        
        // Execute the cat command on the scores file
        execlp("cat", "cat", "scores", (char *)NULL); // Fixed filename from scores.txt to scores
        perror("execlp cat");
        exit(EXIT_FAILURE);
    }
    
    return 0;
}
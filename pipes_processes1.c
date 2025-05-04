/**
 * Two-Way Pipe Communication Between Processes with Concatenated Outputs
 * 
 * This program implements the following workflow:
 * 1. P1 (parent) takes a string from the prompt and passes it to P2 (child)
 * 2. P2 concatenates the received string with "howard.edu" and prints to stdout
 * 3. P2 prompts user for a second input string
 * 4. P2 passes both the concatenated first string and the second input to P1
 * 5. P1 builds the final string by concatenating: firstOutput + secondInput + "gobison.org"
 * 6. P1 prints the complete concatenated string to stdout
 * 
 * Example flow:
 * - Input to P1: "adel"
 * - Output from P2: "adelhoward.edu"
 * - Input to P2: "hamad"
 * - Final output from P1: "adelhoward.eduhamadgobison.org"
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>

int main()
{
    // File descriptors for the two pipes
    int p1ToP2Pipe[2];  // Used for P1 -> P2 communication
    int p2ToP1Pipe[2];  // Used for P2 -> P1 communication
    
    // Fixed domain strings to concatenate
    char howardDomain[] = "howard.edu";
    char bisonDomain[] = "gobison.org";
    
    // Buffer for user input
    char userInput[100];
    
    // Create both pipes
    if (pipe(p1ToP2Pipe) == -1 || pipe(p2ToP1Pipe) == -1)
    {
        fprintf(stderr, "Pipe creation failed\n");
        return 1;
    }
    
    // Get first input from user in P1
    printf("Enter first string: ");
    scanf("%s", userInput);
    
    // Create child process
    pid_t processId = fork();
    
    if (processId < 0)
    {
        fprintf(stderr, "Fork failed\n");
        return 1;
    }
    
    // P1 (Parent process) code
    else if (processId > 0)
    {
        // Close unused pipe ends
        close(p1ToP2Pipe[0]);  // Close reading end of P1->P2 pipe
        close(p2ToP1Pipe[1]);  // Close writing end of P2->P1 pipe
        
        // Send first input string to P2
        write(p1ToP2Pipe[1], userInput, strlen(userInput) + 1);
        close(p1ToP2Pipe[1]);  // Close after writing
        
        // Read combined string from P2
        char combinedString[300];  // Larger buffer to hold concatenated strings
        read(p2ToP1Pipe[0], combinedString, 300);
        
        // Extract the first part (P2's output) and second part (new input)
        char *parts[2];
        parts[0] = strtok(combinedString, "|");  // Using | as delimiter between parts
        parts[1] = strtok(NULL, "|");
        
        // Reconstruct the full string: firstOutput + secondInput
        char finalOutput[400];
        strcpy(finalOutput, parts[0]);  // Copy the first part (P2's output)
        strcat(finalOutput, parts[1]);  // Append the second part (new input)
        
        // Concatenate gobison.org to create the complete final string
        strcat(finalOutput, bisonDomain);
        
        // P1 prints the final concatenated string
        printf("P1 Final Output: %s\n", finalOutput);
        
        close(p2ToP1Pipe[0]);  // Close reading pipe
        
        // Wait for child to finish
        wait(NULL);
    }
    
    // P2 (Child process) code
    else
    {
        // Close unused pipe ends
        close(p1ToP2Pipe[1]);  // Close writing end of P1->P2 pipe
        close(p2ToP1Pipe[0]);  // Close reading end of P2->P1 pipe
        
        // Read string from P1
        char receivedString[100];
        read(p1ToP2Pipe[0], receivedString, 100);
        close(p1ToP2Pipe[0]);  // Close after reading
        
        // Create the first concatenated string (original + howard.edu)
        char firstOutput[200];
        strcpy(firstOutput, receivedString);
        strcat(firstOutput, howardDomain);
        
        // P2 prints its concatenated string
        printf("P2 Output: %s\n", firstOutput);
        
        // Prompt for second user input
        char secondInput[100];
        printf("Enter second string: ");
        scanf("%s", secondInput);
        
        // Combine both strings with a delimiter to send back to P1
        char combinedString[300];
        sprintf(combinedString, "%s|%s", firstOutput, secondInput);
        
        // Send combined string to P1
        write(p2ToP1Pipe[1], combinedString, strlen(combinedString) + 1);
        close(p2ToP1Pipe[1]);  // Close after writing
        
        exit(0);  // Child process exits
    }
    
    return 0;
}
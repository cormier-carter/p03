/*
 * main.c - gshell, the Gaucho Shell
 *
 * This is the only file you edit and submit. main() and the prompt loop
 * are done for you. Every other function below has a comment describing
 * what it does. Work through the checkpoints in the writeup in order:
 *
 *   Checkpoint 1: tokenize, parse_pipeline, and the exit built-in
 *   Checkpoint 2: run_command and run_pipeline for a single command
 *   Checkpoint 3: input and output redirection
 *   Checkpoint 4: pipelines
 */

#define _POSIX_C_SOURCE 200809L
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/** A 100-character line can also have a newline and a '\0'. */
#define MAX_LINE 102

/** A 100-character line has at most 50 whitespace-separated tokens */
#define MAX_TOKENS 50

/** A pipeline can never have more commands than the line has tokens */
#define MAX_COMMANDS MAX_TOKENS

/**
 * One command of a pipeline. For example, the line
 *
 *     /bin/sort -r < in.txt | /usr/bin/head -n 3
 *
 * is a pipeline of two commands. The first has argv {"/bin/sort", "-r",
 * NULL}, input_file "in.txt", and output_file NULL.
 */
typedef struct {
    /** The executable path and its arguments, followed by NULL (as execve expects) */
    char *argv[MAX_TOKENS + 1];
    /** The file named after <, or NULL if the command has no input redirection */
    char *input_file;
    /** The file named after >, or NULL if the command has no output redirection */
    char *output_file;
} command_t;

/**
 * Splits `line` into its whitespace-separated tokens and stores a pointer
 * to each one in `tokens`. Changes the contents of `line`.
 *
 * Returns the number of tokens, or -1 if the array would overflow.
 */
static int tokenize(char *line, char *tokens[]) {
    const char *whitespace = " \t\n\r\f\v";
    char *saveptr = NULL;
    char *token = strtok_r(line, whitespace, &saveptr);
    int count = 0;

    while (token != NULL) {
        if (count == MAX_TOKENS)
            return -1;
        tokens[count] = token;
        count++;
        token = strtok_r(NULL, whitespace, &saveptr);
    }
    return count;
}

/**
 * Groups `tokens` into the commands of a pipeline and fills in one
 * command_t per command in `commands`.
 *
 * Returns the number of commands, or -1 if the tokens are not a valid
 * pipeline.
 */
static int parse_pipeline(char *tokens[], int num_tokens, command_t commands[]) {
    if (num_tokens <= 0)
        return -1;

    int index = 0;
    int num_commands = 0;

    while (index < num_tokens) {
        command_t *cmd = &commands[num_commands];
        int argc = 0;
        cmd->input_file = NULL;
        cmd->output_file = NULL;

        while (index < num_tokens && strcmp(tokens[index], "|") != 0) {
            char *token = tokens[index];
            index++;

            if (strcmp(token, "<") == 0 || strcmp(token, ">") == 0) {
                if (index == num_tokens)
                    return -1;

                char *filename = tokens[index];
                if (strcmp(filename, "<") == 0 || strcmp(filename, ">") == 0 ||
                    strcmp(filename, "|") == 0)
                    return -1;

                if (token[0] == '<') {
                    if (cmd->input_file != NULL)
                        return -1;
                    cmd->input_file = filename;
                } else {
                    if (cmd->output_file != NULL)
                        return -1;
                    cmd->output_file = filename;
                }
                index++;
            } else {
                cmd->argv[argc] = token;
                argc++;
            }
        }

        if (argc == 0)
            return -1;
        cmd->argv[argc] = NULL;
        num_commands++;

        if (index < num_tokens) {
            index++;
            if (index == num_tokens)
                return -1;
        }
    }
    return num_commands;
}

/**
 * Starts `cmd` in a child process. The child reads its standard input
 * from `in_fd` and writes its standard output to `out_fd`, unless the
 * command redirects them to a file. Does not wait for the child.
 *
 * Returns the child's process ID, or -1 if no child was started.
 */
__attribute__((unused))
static pid_t run_command(command_t *cmd, int in_fd, int out_fd) {
    // TODO (Checkpoints 2 and 3)
    return -1;
}

/**
 * Runs the `num_commands` commands of a pipeline, with each command's
 * standard output connected to the next command's standard input. Then
 * waits for all of them and prints each one's exit status, in order.
 */
static void run_pipeline(command_t commands[], int num_commands) {
    // TODO (Checkpoints 2 and 4)
}

/**
 * Runs one line of input: either the exit built-in, or a pipeline of
 * commands. Prints "Invalid command" to stderr if the line is not a valid
 * pipeline.
 */
static void parse_and_run_command(char *line) {
    char *tokens[MAX_TOKENS];
    int num_tokens = tokenize(line, tokens);
    if (num_tokens == 1 && strcmp(tokens[0], "exit") == 0)
        exit(0);

    command_t commands[MAX_COMMANDS];
    int num_commands = parse_pipeline(tokens, num_tokens, commands);
    if (num_commands == -1) {
        fprintf(stderr, "Invalid command\n");
        return;
    }

    run_pipeline(commands, num_commands);
}

int main(void) {
    char line[MAX_LINE];

    while (1) {
        printf("> ");
        fflush(stdout);
        if (fgets(line, sizeof(line), stdin) == NULL)
            break;
        // fgets keeps the '\n' at the end of the line. strcspn(line, "\n")
        // returns the index of the first '\n' in line (or of the final '\0'
        // if there is none), so this replaces the newline with '\0'.
        line[strcspn(line, "\n")] = '\0';
        parse_and_run_command(line);
    }

    return 0;
}

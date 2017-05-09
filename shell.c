#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>

// Homemade booleans
#define FALSE	0
#define	TRUE	1

// Shell constants and built-in commands
char* prompt = ">>>";
char* exitCommand = "exit";
char* batchCommand = "batch";
char* promptCommand = "prompt";
char* aliasCommand = "alias";
char* runBackgroundSymbol = "&";

pid_t thisPID;
pid_t childPID;

void runShell();
void readLine(char lineBuffer[]);
int parseWords(char line[], char* words[]);
void executeCommand(char* command, char* options[], int runBackground);
void terminateProcess();

int main() {
	// Szve own pid
	thisPID = getpid();

	// Install signal handler for child process termination
	signal(SIGINT, terminateProcess);

	runShell();

	return 0;
}

// Runs main loop of shell
void runShell() {
	char line[256];		// Holds entire user input line
	char* words[20];	// Holds line split into words
	char* command;		// Command to execute
	int status;

	while (1) {
		// Get status from any terminated child processes
		waitpid(-1, &status, WNOHANG);

		// Show user prompt
		printf("%s ", prompt);

		readLine(line);

		// Find and execute command
		int runBackground = FALSE;
		if (line[0] != '\0') {
			int numWords = parseWords(line, words);
			command = words[0];

			// Determine if command process should run in background
			if (strcmp(words[numWords - 1], runBackgroundSymbol) == 0) {
				runBackground = TRUE;
				words[numWords - 1] = '\0'; // Remove symbol from words
			}

			executeCommand(command, words, runBackground);
		}
	}
}

// Reads an entire line into a specified array
void readLine(char lineBuffer[]) {
	int i = 0;
	char c;
	while ((c = getchar()) != '\n')
		lineBuffer[i++] = c;

	lineBuffer[i] = '\0';
}

// Runs a command, including built in functions, optionally run in background
void executeCommand(char* command, char* options[], int runBackground) {	
	if (strcmp(command, exitCommand) == 0) {			// Exit
		_exit(0);
	} else if (strcmp(command, batchCommand) == 0) {	// Run batch file

	} else if (strcmp(command, promptCommand) == 0) {	// Change prompt
		prompt = options[1];
	} else if (strcmp(command, aliasCommand) == 0) {	// Create alias

	} else {
		pid_t childPID = fork();

		// Error in fork call
		if (childPID == -1) {
			perror("Fork");
			_exit(1);
		}

		// Invalid command
		if (childPID == 0) {
			execvp(command, options);
			perror("Invalid command");
			_exit(1);
		}

		// Wait for process to stop if not running in background
		if (!runBackground) {
			int status;
			waitpid(childPID, &status, 0);
		}
	}	
}

// Kill a child process
void terminateProcess() {
	kill(childPID, SIGKILL);
}

// Split a line into words
int parseWords(char line[], char* words[]) {
	int currentWord = 0;
	char* word;
	char* token;
	int insideQuotes = FALSE;

	token = strtok(line, " ");
	while (token != NULL) {
		if (insideQuotes) {
			word = strcat(word, token);
			word = strcat(word, " ");

			if (word[strlen(word) - 1] == '\"') {
				word[strlen(word) - 1] = '\0';
				words[currentWord] = word;
				currentWord++;
				insideQuotes = FALSE;
			}
		} else {
			if (token[0] == '\"') {
				word = &token[1];
				insideQuotes = TRUE;

				if (word[strlen(word) - 1] == '\"') {
					word[strlen(word) - 1] = '\0';
					words[currentWord] = word;
					currentWord++;
					insideQuotes = FALSE;
				}
			} else {
				words[currentWord] = strcat(token, "\0");
				currentWord++;
			}
		}

		token = strtok(NULL, " ");
	}

	words[currentWord] = NULL;

	return currentWord;
}

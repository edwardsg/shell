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

char aliasList[256][2][100];
int aliasCount = 0;

void runShell();
void readLine(char lineBuffer[]);
int parseWords(char line[], char* words[]);
void executeCommand(char* command, char* options[], int runBackground);
void exitShell();
void runBatchFile(char* filePath);
void runLine(char line[]);
int shouldRunInBackground(char* words[], int numWords);
void changePrompt(char* newPrompt);
void createAlias(char* alias, char* command);
void terminateProcess();

int main() {
	// Szve own pid
	thisPID = getpid();

	// Install signal handler for child process termination
	signal(SIGINT, terminateProcess);

	// Start the shell
	runShell();

	return 0;
}

// Runs main loop of shell
void runShell() {
	char line[256];		// Holds entire user input line
	int status;

	while (1) {
		// Get status from any terminated child processes
		waitpid(-1, &status, WNOHANG);

		// Show user prompt
		printf("%s ", prompt);

		// Get input line
		readLine(line);

		// Run command
		runLine(line);
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
		exitShell();	
	} else if (strcmp(command, batchCommand) == 0) {	// Run batch file
		runBatchFile(options[1]);
	} else if (strcmp(command, promptCommand) == 0) {	// Change prompt
		changePrompt(options[1]);
	} else if (strcmp(command, aliasCommand) == 0) {	// Create alias
		createAlias(options[1], options[2]);
	} else {
		childPID = fork();

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

// Exit the shell program
void exitShell() {
	_exit(0);
}

// Run a batch program from a file
void runBatchFile(char* filePath) {
	FILE *file = fopen(filePath, "r");
	char line[100];

	if (file == NULL) {
		perror("File not found.\n");
		return;
	}

	while (fgets(line, 100, file) != NULL) {
		runLine(line);
	}

	fclose(file);
}

// Finds and executes command from the input line
void runLine(char line[]) {
	char* words[20];	// Holds line split into words
	char* command;		// Command to execute

	if (line[0] != '\0') {
		int numWords = parseWords(line, words);
		command = words[0];

		int runBackground = shouldRunInBackground(words, numWords);
		if (runBackground)
			words[numWords - 1] = '\0'; // Remove symbol from words

		executeCommand(command, words, runBackground);
	}
}

// Determines if command process should run in background
int shouldRunInBackground(char* words[], int numWords) {
	if (strcmp(words[numWords - 1], runBackgroundSymbol) == 0)
		return TRUE;

	return FALSE;
}

// Change the user prompt
void changePrompt(char* newPrompt) {
	prompt = newPrompt;
}

// Create an alias for a command
void createAlias(char* alias, char* command) {
	memcpy(aliasList[aliasCount][0], alias, strlen(alias));
//	memcpy(aliasList[aliasCount][1], command, strlen(command));
	aliasCount++;
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

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define FALSE	0
#define	TRUE	1

char* prompt = ">>>";
char* exitCommand = "exit";
char* batchCommand = "batch";
char* promptCommand = "prompt";
char* aliasCommand = "alias";
char* runBackgroundSymbol = "&";

void runShell();
int parseWords(char line[], char* words[]);
pid_t executeCommand(char* command, char* options[]);
void terminateProcess();

int main() {
	runShell();

	return 0;
}

void runShell() {
	char line[256];
	char* words[20];
	char* command;

	pid_t childPID;
	
	while (1) {
		printf("%s ", prompt);

		int runBackground = FALSE;

		int i = 0;
		int c;
		while ((c = getchar()) != '\n')
			line[i++] = c;
		line[i] = '\0';

		if (line[0] != '\0') {
			int numWords = parseWords(line, words);
			command = words[0];
			if (strcmp(words[numWords - 1], runBackgroundSymbol) == 0) {
				runBackground = TRUE;
				words[numWords - 1] = '\0';
			}

			childPID = executeCommand(command, words);

			signal(2, terminateProcess);
			
			if (!runBackground) {
				int status;
				waitpid(childPID, &status, 0);
			}
		}
	}
}

pid_t executeCommand(char* command, char* options[]) {
	pid_t childPID = -1;
	
	if (strcmp(command, exitCommand) == 0) {
		_exit(0);
	} else if (strcmp(command, batchCommand) == 0) {

	} else if (strcmp(command, promptCommand) == 0) {
		prompt = options[1];
	} else if (strcmp(command, aliasCommand) == 0) {

	} else {
		childPID = fork();
		if (childPID == -1) {
			perror("Fork");
			_exit(1);
		}

		if (childPID == 0) {
			execvp(command, options);
			perror("Execute");
			_exit(1);
		}
	}

	return childPID;
}

void terminateProcess() {
	printf("Sup");
}

int parseWords(char line[], char* words[]) {
	int currentWord = 0;
	char* word;

	word = strtok(line, " ");
	while (word != NULL) {
		word[strlen(word)] = '\0';
		words[currentWord] = word;
		currentWord++;
		word = strtok(NULL, " ");
	}

	words[currentWord] = NULL;

	return currentWord;
}

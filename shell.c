#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define FALSE	0
#define	TRUE	1

char* prompt = ">>> ";

void runShell();
void parseWords(char line[], char* words[]);

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
		printf("%s", prompt);

		int i = 0;
		int c;
		while ((c = getchar()) != '\n')
			line[i++] = c;
		line[i] = '\0';

		parseWords(line, words);
		command = words[0];

		childPID = fork();
		if (childPID == -1) {
			perror("Fork failed");
			_exit(1);
		}

		if (childPID == 0) {
			execvp(command, words);
			perror("Execute failed");
			_exit(1);
		}

		int status;
		waitpid(childPID, &status, 0);
	}
}

void parseWords(char line[], char* words[]) {
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
}

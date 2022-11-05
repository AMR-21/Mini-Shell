
/*
 * CS354: Shell project
 *
 * Template file.
 * You will need to add more code here to execute the command table.
 *
 * NOTE: You are responsible for fixing any bugs this code may have!
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include "y.tab.h"
#include <time.h>
#include <iostream>
using namespace std;

#include "command.h"

extern int yyparse(void);

SimpleCommand::SimpleCommand()
{
	// Creat available space for 5 arguments
	_numberOfAvailableArguments = 5;
	_numberOfArguments = 0;
	_arguments = (char **)malloc(_numberOfAvailableArguments * sizeof(char *));
}

void SimpleCommand::insertArgument(char *argument)
{
	FILE *c = fopen("cmd.txt", "a+");
	int i = 0;
	if (Command::_currentCommand.pipeIN)
	{
		fprintf(c, " | ");
		Command::_currentCommand.pipeIN = 0;
	}

	while (argument[i])
	{
		if (argument[i] == '>' ||
				argument[i] == '<' ||
				argument[i] == '&' ||
				argument[i] == '|')
		{
			fprintf(c, " ");
			fprintf(c, "%c", argument[i]);
			if (argument[i + 1] == '>' && argument[i] != '&')
			{
				i++;
				fprintf(c, "%c", argument[i]);
			}
			fprintf(c, " ");
		}
		else
		{
			fprintf(c, "%c", argument[i]);
		}
		i++;
	}

	fprintf(c, " ");
	fclose(c);

	if (_numberOfAvailableArguments == _numberOfArguments + 1)
	{
		// Double the available space
		_numberOfAvailableArguments *= 2;
		_arguments = (char **)realloc(_arguments,
																	_numberOfAvailableArguments * sizeof(char *));
	}

	_arguments[_numberOfArguments] = argument;

	// Add NULL argument at the end
	_arguments[_numberOfArguments + 1] = NULL;

	_numberOfArguments++;
}

void Command::saveIO()
{
	FILE *c = fopen("cmd.txt", "a+");
	int i = 0;

	if (Command::_currentCommand._inputFile)
	{
		fprintf(c, " < ");
		fprintf(c, "%s", Command::_currentCommand._inputFile);
	}

	if (Command::_currentCommand._errFile)
		fprintf(c, " & ");

	if (Command::_currentCommand._outFile)
	{
		if (Command::_currentCommand._outOverwrite)
		{
			fprintf(c, " > ");
		}
		else
			fprintf(c, " >> ");

		fprintf(c, "%s", Command::_currentCommand._outFile);
	}

	if (Command::_currentCommand._background)
		fprintf(c, " & ");

	fprintf(c, " ");
	fclose(c);
}

Command::Command()
{
	// Create available space for one simple command
	_numberOfAvailableSimpleCommands = 1;
	_simpleCommands = (SimpleCommand **)
			malloc(_numberOfSimpleCommands * sizeof(SimpleCommand *));

	_numberOfSimpleCommands = 0;
	_outFile = 0;
	_inputFile = 0;
	_errFile = 0;
	_outOverwrite = 0;
	_background = 0;
	_pipe = 0;
	vef = 0;
	pipeIN = 0;
}

void Command::insertSimpleCommand(SimpleCommand *simpleCommand)
{
	if (_numberOfAvailableSimpleCommands == _numberOfSimpleCommands)
	{
		_numberOfAvailableSimpleCommands *= 2;
		_simpleCommands = (SimpleCommand **)realloc(_simpleCommands,
																								_numberOfAvailableSimpleCommands * sizeof(SimpleCommand *));
	}

	_simpleCommands[_numberOfSimpleCommands] = simpleCommand;
	_numberOfSimpleCommands++;
}

void Command::clear()
{
	for (int i = 0; i < _numberOfSimpleCommands; i++)
	{
		for (int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++)
		{
			free(_simpleCommands[i]->_arguments[j]);
		}

		free(_simpleCommands[i]->_arguments);
		free(_simpleCommands[i]);
	}

	if (_errFile && _errFile != _outFile)
	{
		free(_errFile);
	}

	if (_outFile)
	{
		free(_outFile);
	}

	if (_inputFile)
	{
		free(_inputFile);
	}

	_numberOfSimpleCommands = 0;
	_outFile = 0;
	_inputFile = 0;
	_errFile = 0;
	_outOverwrite = 0;
	_background = 0;
	_pipe = 0;
}

void Command::print()
{
	printf("\n\n");
	printf("              COMMAND TABLE                \n");
	printf("\n");
	printf("  #   Simple Commands\n");
	printf("  --- ----------------------------------------------------------\n");

	for (int i = 0; i < _numberOfSimpleCommands; i++)
	{
		printf("  %-3d ", i);
		for (int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++)
		{
			printf("\"%s\" \t", _simpleCommands[i]->_arguments[j]);
		}
	}

	printf("\n\n");
	printf("  Output       Input        Error        Background\n");
	printf("  ------------ ------------ ------------ ------------\n");
	printf("  %-12s %-12s %-12s %-12s\n", _outFile ? _outFile : "default",
				 _inputFile ? _inputFile : "default", _errFile ? _errFile : "default",
				 _background ? "YES" : "NO");
	printf("\n\n");
}

void Command::execute()
{

	// Don't do anything if there are no simple commands
	if (_numberOfSimpleCommands == 0)
	{
		prompt();
		return;
	}

	// Print contents of Command data structure
	print();

	// Add execution here
	// For every simple command fork a new process
	// Setup i/o redirection
	// and call exec
	int currentIn = 0, defaultin, defaultout, defaulterr, outFl, inFl, errFl, errorFlag = 0;
	int fd[2];
	pid_t pid;

	defaultin = dup(0);
	defaultout = dup(1);
	defaulterr = dup(2);

	// Open the input file
	if (_inputFile)
	{
		// open the input file
		inFl = open(_inputFile, O_RDONLY, 0444);

		// Verification
		if (inFl < 0)
		{
			perror("\033[0;31mError\nOpening input file");
			errorFlag = 1;
		}

		// Redirect the input file to given input file
	}

	// Open/Create the output file
	if (_outFile)
	{
		// open the output file
		// Check the writing mode
		if (_currentCommand._outOverwrite)
		{
			// Overwrite >
			outFl = open(_outFile, O_CREAT | O_WRONLY | O_TRUNC, 0666);
		}
		else
		{
			// Append >>
			outFl = open(_outFile, O_CREAT | O_WRONLY | O_APPEND, 0666);
		}

		// Verification
		if (outFl < 0)
		{
			perror("\033[0;31mError\nOpening output file");
			errorFlag = 1;
		}

		// // Redirect the output file to given input file
		// dup2(outFl, 1);
		// close(outFl);
	}

	// Open/Create the output file
	if (_errFile)
	{
		// open the output file
		// Check the writing mode
		if (_currentCommand._outOverwrite)
		{
			// Overwrite >
			errFl = open(_outFile, O_CREAT | O_WRONLY | O_TRUNC, 0666);
		}
		else
		{
			// Append >>
			errFl = open(_outFile, O_CREAT | O_WRONLY | O_APPEND, 0666);
		}

		// Verification
		if (errFl < 0)
		{
			perror("\033[0;31mError\nOpening error output file");
			errorFlag = 1;
		}

		// // Redirect the output file to given input file
		// dup2(outFl, 1);
		// close(outFl);
	}

	if (!errorFlag)
		for (int i = 0; i < _numberOfSimpleCommands; i++)
		{
			if (!strcmp(_simpleCommands[i]->_arguments[0], "cd"))
			{
				if (!_simpleCommands[i]->_arguments[1])
				{
					chdir(getenv("HOME"));
				}
				else if (_simpleCommands[i]->_arguments[1][0] == '~')
				{
					chdir(getenv("HOME"));
					string s = _simpleCommands[i]->_arguments[1];
					string dest = s.substr(2, s.length() - 2);
					chdir(dest.c_str());
				}
				else if (_simpleCommands[i]->_arguments[1][0] == '\'')
				{
					string s = _simpleCommands[i]->_arguments[1];
					string dest = s.substr(1, s.length() - 2);
					chdir(dest.c_str());
				}
				else
				{
					chdir(_simpleCommands[i]->_arguments[1]);
				}
				continue;
			}

			if (!strcmp(_simpleCommands[i]->_arguments[0], "exit"))
			{
				printf("\n\nGOOD BYE!\n\n");
				exit(0);
			}

			if (pipe(fd) < 0)
			{
				perror("\033[0;31mError\nFailed to create pipe");
				break;
			}

			if ((pid = fork()) == -1)
			{
				perror("\033[0;31mError\nFailed to create process");
				break;
			}

			else if (!pid)
			{
				if (_inputFile)
				{
					dup2(inFl, 0);
					close(inFl);
					_inputFile = 0;
				}
				else
				{
					// redirection to current input - stdin initial
					dup2(currentIn, 0);
					// Close input of pipe after redirection
					close(fd[0]);
				}

				// Not last one
				if (i + 1 != _numberOfSimpleCommands)
				{
					// Redirection to output of the pipe
					dup2(fd[1], 1);
				} // Last one
				else if (_outFile)
				{
					// Redirection to the output file
					dup2(outFl, 1);
					close(outFl);

					if (_errFile)
					{
						dup2(errFl, 2);
						close(errFl);
					}
				}

				execvp(_simpleCommands[i]->_arguments[0], _simpleCommands[i]->_arguments);

				// exec() is not suppose to return, something went wrong
				perror("\033[0;31mError\nInvalid command");
				break;
			}

			// Check background flag
			if (!_currentCommand._background)
			{
				waitpid(pid, 0, 0);
			}

			// Close output pipe file
			close(fd[1]);
			currentIn = fd[0];
		}

	// Redirection to stdin, stdout, and stderr
	dup2(defaultin, 0);
	dup2(defaultout, 1);
	dup2(defaultout, 2);

	// Clear to prepare for next command
	clear();

	// Restore white color in case
	printf("\033[0m\n");

	// Print new prompt
	prompt();
}

// Shell implementation

void Command::prompt()
{
	char curr[100];
	printf("%s/", getcwd(curr, 100));
	printf("~:myshell$ ");
	fflush(stdout);
	vef = 0;
}

Command Command::_currentCommand;
SimpleCommand *Command::_currentSimpleCommand;

void exit_handler(int signo)
{
	printf("\n\nYou can't exit the program with ctrl+c..if you want to exit type exit\n\n");
	Command::_currentCommand.prompt();
	fflush(stdout);
}

void child_death_handler(int signo)
{
	FILE *log = fopen("log", "a+");
	time_t rawtime;
	struct tm *timeinfo;
	time(&rawtime);
	timeinfo = localtime(&rawtime);

	fprintf(log, "Child terminated at %s\n", asctime(timeinfo));
	fclose(log);
}

int main()
{
	Command::_currentCommand.prompt();
	signal(SIGINT, exit_handler);
	signal(SIGCHLD, child_death_handler);
	yyparse();

	return 0;
}

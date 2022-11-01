
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

#include "command.h"

SimpleCommand::SimpleCommand()
{
	// Creat available space for 5 arguments
	_numberOfAvailableArguments = 5;
	_numberOfArguments = 0;
	_arguments = (char **)malloc(_numberOfAvailableArguments * sizeof(char *));
}

void SimpleCommand::insertArgument(char *argument)
{
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

	if (_outFile)
	{
		free(_outFile);
	}

	if (_inputFile)
	{
		free(_inputFile);
	}

	if (_errFile)
	{
		free(_errFile);
	}

	_numberOfSimpleCommands = 0;
	_outFile = 0;
	_inputFile = 0;
	_errFile = 0;
	_outOverwrite = 0;
	_background = 0;
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
	int ioFlag, defaultin, defaultout, defaulterr, outFl, inFl;

	for (int i = 0; i < _numberOfSimpleCommands; i++)
	{
		ioFlag = 0;
		// Check for possible I/0 redirection

		if (_currentCommand._inputFile ||
				_currentCommand._outFile ||
				_currentCommand._errFile)
		{
			ioFlag = 1;

			// Save default descriptors values
			defaultin = dup(0);
			defaultout = dup(1);
			defaulterr = dup(2);

			// Set the input file
			if (_currentCommand._inputFile)
			{
				// open the input file
				inFl = open(_inputFile, O_RDONLY, 0444);

				// Verification
				if (inFl < 0)
				{
					perror("\033[0;31mError\nOpening input file");
					break;
				}

				// Redirect the input file to given input file
				dup2(inFl, 0);
				close(inFl);
			}

			// Set the output file
			if (_currentCommand._outFile)
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
					break;
				}

				// Redirect the output file to given input file
				dup2(outFl, 1);
				close(outFl);
			}
		}

		// Create process
		pid_t pid = fork();

		// Catch failure
		if (pid == -1)
		{
			perror("\033[0;31mError\nCommand: fork\n");
			break;
		}

		// Command execution in child process
		if (pid == 0)
		{

			if (ioFlag)
			{
				// close file descriptors that are not needed
				close(defaultin);
				close(defaultout);
				close(defaulterr);
			}

			// Child
			execvp(_simpleCommands[i]->_arguments[0], _simpleCommands[i]->_arguments);

			// exec() is not suppose to return, something went wrong
			perror("\033[0;31mError\nInvalid command\n");
			break;
		}

		if (ioFlag)
		{
			// Restore input, output, and error defaults
			dup2(defaultin, 0);
			dup2(defaultout, 1);
			dup2(defaulterr, 2);

			// Close file descriptors that are not needed
			if (_currentCommand._outFile)
				close(outFl);
			if (_currentCommand._inputFile)
				close(inFl);
			close(defaultin);
			close(defaultout);
			close(defaulterr);
		}

		// Check background flag
		if (!_currentCommand._background)
		{
			waitpid(pid, 0, 0);
		}
	}

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
	printf("myshell>");
	fflush(stdout);
}

Command Command::_currentCommand;
SimpleCommand *Command::_currentSimpleCommand;

int yyparse(void);

int main()
{
	Command::_currentCommand.prompt();
	yyparse();
	return 0;
}


#ifndef command_h
#define command_h

// Command Data Structure
struct SimpleCommand
{
	// Available space for arguments currently preallocated
	int _numberOfAvailableArguments;

	// Number of arguments
	int _numberOfArguments;
	char **_arguments;

	SimpleCommand();
	void insertArgument(char *argument);
};

struct Command
{
	int _numberOfAvailableSimpleCommands;
	int _numberOfSimpleCommands;
	SimpleCommand **_simpleCommands;
	char *_outFile;
	char *_inputFile;
	char *_errFile;
	int _outOverwrite;
	int _background;
	int _pipe;

	void prompt();
	void print();
	void execute();
	void clear();
	void verify(SimpleCommand *scmd);

	Command();
	void insertSimpleCommand(SimpleCommand *simpleCommand);

	static Command _currentCommand;
	static Command _tempCommand;
	static SimpleCommand *_currentSimpleCommand;
};

#endif

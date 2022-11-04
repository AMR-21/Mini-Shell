
/*
 * CS-413 Spring 98
 * shell.y: parser for shell
 *
 * This parser compiles the following grammar:
 *
 *	cmd [arg]* [> filename]
 *
 * you must extend it to understand the complete shell grammar
 *
 */

%token	<string_val> WORD

%token 	NOTOKEN WRITE PIPE APPEND OPEN BACK NEWLINE 

%union	{
		char   *string_val;
	}

%{
extern "C" 
{
	int yylex();
	void yyerror (char const *s);
}
#define yylex yylex
#include <stdio.h>
#include "command.h"
%}

%define parse.error verbose

%%

goal:	
	commands
	;

commands: 
	command
	| commands command 
	;

command: simple_command
        ;

simple_command:	
	command_and_args iomodifier_opt NEWLINE {
		printf("   Yacc: Execute command\n");
		Command::_currentCommand.execute();
	}
	| NEWLINE 
	| error NEWLINE { yyerrok; }
	;

command_and_args:
	command_word arg_list {
		Command::_currentCommand.
			insertSimpleCommand( Command::_currentSimpleCommand );
	}
	;

arg_list:
	arg_list argument
	| /* can be empty */
	;

argument:
	WORD {
               printf("   Yacc: insert argument \"%s\"\n", $1);

	       Command::_currentSimpleCommand->insertArgument( $1 );\
	}
	| command_word
	;
	
command_word:
	WORD {
               printf("   Yacc: insert command \"%s\"\n", $1);
	       
	       Command::_currentSimpleCommand = new SimpleCommand();
	       Command::_currentSimpleCommand->insertArgument( $1 );
	}
	| PIPE WORD {
		Command::_currentCommand.
			insertSimpleCommand( Command::_currentSimpleCommand );

			printf("   Yacc: insert piped command \"%s\"\n", $2);
	       
	       Command::_currentSimpleCommand = new SimpleCommand();
	       Command::_currentSimpleCommand->insertArgument( $2 );
				 Command::_currentCommand._pipe ++;
	}
	;

iomodifier_opt:
	WRITE WORD { 
		// Output to file ls > out
		printf("   Yacc: insert output \"%s\"\n", $2);
		Command::_currentCommand._outFile = $2;
		Command::_currentCommand._outOverwrite = 1;
	}
	| APPEND WORD { 
		// Output to file ls >> out
		printf("   Yacc: insert output \"%s\"\n", $2);
		Command::_currentCommand._outFile = $2;
	}
	| OPEN WORD { 
		// input file ls < out
		printf("   Yacc: insert input \"%s\"\n", $2);
		Command::_currentCommand._inputFile = $2;
	}
	| WRITE WORD OPEN WORD { 
		// cat > out < in
		printf("   Yacc: insert output \"%s\"\n", $2);
		Command::_currentCommand._outFile = $2;
		printf("   Yacc: insert input \"%s\"\n", $4);
		Command::_currentCommand._inputFile = $4;
		Command::_currentCommand._outOverwrite = 1;
	}
	| OPEN WORD WRITE WORD {
		// cat < in > out
		printf("   Yacc: insert output \"%s\"\n", $4);
		Command::_currentCommand._outFile = $4;
		printf("   Yacc: insert input \"%s\"\n", $2);
		Command::_currentCommand._inputFile = $2;
		Command::_currentCommand._outOverwrite = 1;
	}
	| OPEN WORD APPEND WORD {
		// cat < in >> out
		printf("   Yacc: insert output \"%s\"\n", $4);
		Command::_currentCommand._outFile = $4;
		printf("   Yacc: insert input \"%s\"\n", $2);
		Command::_currentCommand._inputFile = $2;
	}
	| APPEND WORD OPEN WORD {
		// cat >> out < in
		printf("   Yacc: insert output \"%s\"\n", $2);
		Command::_currentCommand._outFile = $2;
		printf("   Yacc: insert input \"%s\"\n", $4);
		Command::_currentCommand._inputFile = $4;
	}
	| BACK {
		// httpd &
		printf("   Yacc: activate background mode \n");
		Command::_currentCommand._background = 1;
	}
	| BACK APPEND WORD  {
		// ls /tt  & >> out2 
		printf("   Yacc: activate background mode \n");
		Command::_currentCommand._background = 1;
		printf("   Yacc: insert output \"%s\"\n", $3);
		Command::_currentCommand._outFile = $3;
	}
	| BACK WRITE WORD  {
		// ls /tt & > out2 
		printf("   Yacc: activate background mode \n");
		Command::_currentCommand._background = 1;
		printf("   Yacc: insert output \"%s\"\n", $3);
		Command::_currentCommand._outFile = $3;
		Command::_currentCommand._outOverwrite = 1;
	}
	| OPEN WORD WRITE WORD BACK {
		// cat < in > out2 &
		printf("   Yacc: activate background mode \n");
		Command::_currentCommand._background = 1;
		printf("   Yacc: insert output \"%s\"\n", $4);
		Command::_currentCommand._outFile = $4;
		printf("   Yacc: insert input \"%s\"\n", $2);
		Command::_currentCommand._inputFile = $2;
		Command::_currentCommand._outOverwrite = 1;
	}
	| OPEN WORD APPEND WORD BACK {
		// cat < in >> out &
		printf("   Yacc: activate background mode \n");
		Command::_currentCommand._background = 1;
		printf("   Yacc: insert output \"%s\"\n", $4);
		Command::_currentCommand._outFile = $4;
		printf("   Yacc: insert input \"%s\"\n", $2);
		Command::_currentCommand._inputFile = $2;
	}
	| /* can be empty */ 
	;

%%

void
yyerror(const char * s)
{
	fprintf(stderr,"%s", s);
}

#if 0
main()
{
	yyparse();
}
#endif

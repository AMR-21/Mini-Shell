
/*
 *
 * CS-413 Spring 98
 * shell.l: lexical analyzer for shell
 * You have to extend it.
 *
 */

%{

#include <string.h>
#include "y.tab.h"

int yylex();

%}

%%




\n 	{
		return NEWLINE;
	}

[ \t] 	{
		/* Discard spaces and tabs */
     	}	

"'"[^\n]*"'" {
		yylval.string_val = strdup(yytext);
		return SRC;
}

">" 	{
		return WRITE;
	}

">>" 	{
		return APPEND;
}

"<" 	{
		return OPEN;
}

"&"		{
		return BACK;
}

"|"		{
		return PIPE;
}

[^ \t\n][^ \t\n]*  {
		/* Assume that file names have only alpha chars */
		yylval.string_val = strdup(yytext);
		return WORD;
	}


.	{
		/* Invalid character in input */
		return NOTOKEN;
	}

%%




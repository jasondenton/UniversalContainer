%option outfile="contract_lexer.cpp"
%option c++
%option noyywrap
%option prefix="contract"
%{
/*
 * UniversalContainer library.
 * Copyright Jason Denton, 2008,2010.
 * Made available under the new BSD license, as described in LICENSE
 *
 * Send comments and bug reports to jason.denton@gmail.com
 * http://www.greatpanic.com/code.html
 */

#include "uccontract.h"
#include "contract_support.h"
#include "contract_parser.tab.hh"

static int line_number = 0;
%}

DIGIT [0-9]
BOOLEAN "true"|"false"
INTEGER [+-]?{DIGIT}+
EXPONENT [eE]{INTEGER}
FRACTION "."{DIGIT}+
REAL {INTEGER}({FRACTION}{EXPONENT}?)?
ECHAR "\\"[nrtfb\\\"]
PCHAR [\[\]\\<>(){}+ =/!?@#:$%^&*\,\_\.\-'\x80-\xff]
ACHAR [a-zA-Z]
DCHAR [0-9]
CHARACTER {ECHAR}|{PCHAR}|{ACHAR}|{DCHAR}
STRING "\""{CHARACTER}*"\""
IDENTIFIER {ACHAR}({ACHAR}|DIGIT)*
NEWLINE "\n"
IMPLY "==>"
STRINGT "string"
INTEGERT "integer"
REALT "real"
CHARACTERT "character"
BOOLEANT "boolean"
ARRAYTYPE "#type"
ARRAYSIZE "#size"
EXISTS "#exists"
OPENPAREN "("
CLOSEPAREN ")"
OPENBRACE "{"
CLOSEBRACE "}"
OPENBRACKET "["
CLOSEBRACKET "]"
COLON ":"
QMARK "?"
OPENGROUP "#group"
CLOSEGROUP "#endgroup"

%%

{STRING} return JAD::ContractParser::token::STRING;
{INTEGER} return JAD::ContractParser::token::INTEGER;
{REAL} return JAD::ContractParser::token::REAL;
{BOOLEAN} return JAD::ContractParser::token::BOOLEAN;
{IMPLY} return JAD::ContractParser::token::IMPLY;
{STRINGT} return JAD::ContractParser::token::STRINGT;
{REALT} return JAD::ContractParser::token::REALT;
{BOOLEANT} return JAD::ContractParser::token::BOOLEANT;
{CHARACTERT} return JAD::ContractParser::token::CHARACTERT;
{INTEGERT} return JAD::ContractParser::token::INTEGERT;
{OPENPAREN} return JAD::ContractParser::token::OPENPAREN;
{CLOSEPAREN} return JAD::ContractParser::token::CLOSEPAREN;
{OPENBRACE} return JAD::ContractParser::token::OPENBRACE;
{CLOSEBRACE} return JAD::ContractParser::token::CLOSEBRACE;
{OPENBRACKET} return JAD::ContractParser::token::OPENBRACKET;
{CLOSEBRACKET} return JAD::ContractParser::token::CLOSEBRACKET;
{COLON} return JAD::ContractParser::token::COLON;
{QMARK} return JAD::ContractParser::token::QMARK;
{ARRAYTYPE} return JAD::ContractParser::token::ARRAYTYPE;
{EXISTS} return JAD::ContractParser::token::EXISTS;
{ARRAYSIZE} return JAD::ContractParser::token::ARRAYSIZE;
{IDENTIFIER} return JAD::ContractParser::token::IDENTIFIER;
{OPENGROUP} return JAD::ContractParser::token::OPENGROUP;
{CLOSEGROUP} return JAD::ContractParser::token::CLOSEGROUP;
{NEWLINE} {line_number++;}
[\t \,] //eat whitespace and commas
%%

namespace JAD {

  ContractLexer::ContractLexer(Buffer* buf) : contractFlexLexer(0,0)
  {
    buffer = buf;
  }
 
  int ContractLexer::LexerInput(char* data, int max_size)
  {
    int sent = (int) buffer->copy_out(data,(size_t) max_size);
    return sent;
  }

  int ContractLexer::get_symbol(void* vl)
  {
    ContractParser::semantic_type* value = (ContractParser::semantic_type *) vl;
    int symtype = yylex();

    switch (symtype) {
      case ContractParser::token::STRING:
        value->text = strdup(yytext+1);
        value->text[strlen(value->text)-1] = '\0';
        break;
      case ContractParser::token::IDENTIFIER:
        value->text = strdup(yytext);
        break;
      case ContractParser::token::INTEGER:
        value->integer = strtol(yytext,NULL,0);
        break;
      case ContractParser::token::REAL:
        value->real = atof(yytext);
        break;
      case ContractParser::token::CHARACTER:
        value->character = yytext[0];
        break;
      case ContractParser::token::BOOLEAN:
        value->boolean = !strncmp(yytext,"true",5);
        break;
    }
    line_num = line_number;
    return symtype;
  } 
} //end namespace

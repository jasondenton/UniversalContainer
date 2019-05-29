%skeleton "lalr1.cc"
%defines
%define api.namespace {JAD}
%define api.parser.class {ContractParser}
%parse-param {ContractDriver& driver}
%require "3.4"

%{
#include <string>
#include <FlexLexer.h>
#include "buffer.h"
#include "uccontract.h"
#include "contract_support.h"

%}

%union {
  long integer;
  double real;
  bool boolean;
  char* text;
  char character;
}

%token <text> STRING
%token <text> IDENTIFIER
%token <real> REAL
%token <character> CHARACTER
%token <integer> INTEGER
%token <boolean> BOOLEAN
%token IMPLY
%token STRINGT
%token REALT
%token BOOLEANT
%token CHARACTERT
%token INTEGERT
%token OPENPAREN
%token CLOSEPAREN
%token OPENBRACKET
%token CLOSEBRACKET
%token OPENBRACE
%token CLOSEBRACE
%token OPENANGLE
%token CLOSEANGLE
%token BOOLEANLIT
%token COLON
%token QMARK
%token FIELDSEP
%token ARRAYTYPE
%token EXISTS
%token ARRAYSIZE
%token OPENGROUP
%token CLOSEGROUP

%destructor { free($$);} <text>

%{
#undef yylex
#define yylex driver.lexer->get_symbol
%}

%%

seq: named_contract
| seq named_contract

named_contract: IDENTIFIER IMPLY contract {driver.stk.top()->add_to_library($1);}

contract: basic_contract
| tagged_contract

basic_contract: map_contract 
| integer_contract 
| real_contract 
| character_contract 
| string_contract 
| boolean_contract 
| user_contract
| array_contract
| group_contract

tagged_contract: OPENANGLE IDENTIFIER CLOSEANGLE basic_contract { driver.stk.top()->set_tag($2);}


integer_contract: INTEGERT  { driver.stk.push(new UCSchema(false, 0, false, 0)); }
| INTEGERT OPENPAREN INTEGER COLON INTEGER CLOSEPAREN { driver.stk.push(new UCSchema(true, $3, true, $5)); }
| INTEGERT OPENPAREN INTEGER COLON CLOSEPAREN { driver.stk.push(new UCSchema(true, $3, false, 0)); }
| INTEGERT OPENPAREN COLON INTEGER CLOSEPAREN { driver.stk.push(new UCSchema(false, 0, true, $4)); }

character_contract: CHARACTERT { driver.stk.push(new UCSchema('\0','\0',false,false)); }
| CHARACTERT OPENPAREN CHARACTER COLON CHARACTER CLOSEPAREN { driver.stk.push(new UCSchema($3,$5,true,true)); }
| CHARACTERT OPENPAREN CHARACTER COLON CLOSEPAREN { driver.stk.push(new UCSchema($3,'\0',true,false)); }
| CHARACTERT OPENPAREN COLON CHARACTER CLOSEPAREN { driver.stk.push(new UCSchema('\0',$4,false,true)); }

real_contract: REALT { driver.stk.push(new UCSchema(false, false, 0.0, 0.0)); }
| REALT OPENPAREN REAL COLON REAL CLOSEPAREN { driver.stk.push(new UCSchema(true, true, $3, $5)); }
| REALT OPENPAREN REAL COLON CLOSEPAREN { driver.stk.push(new UCSchema(true, false, $3, 0.0)); }
| REALT OPENPAREN COLON REAL CLOSEPAREN { driver.stk.push(new UCSchema(false, true, 0.0, $4)); }

string_contract: STRINGT { driver.stk.push(new UCSchema((char*)NULL));}
| STRINGT OPENPAREN STRING CLOSEPAREN { driver.stk.push(new UCSchema($3)); free($3); }

boolean_contract: BOOLEANT { driver.stk.push(new UCSchema(-1)); }
| BOOLEANT OPENPAREN BOOLEAN CLOSEPAREN { driver.stk.push(new UCSchema($3)); }

map_contract: map_open CLOSEBRACE 
| map_open map_field_list CLOSEBRACE

map_open: OPENBRACE { driver.stk.push(new UCSchema(uc_Map, NULL)); }

map_field_list: map_item
| map_item map_field_list

map_item: STRING COLON contract {
  driver.tmpc = driver.stk.top();
  driver.stk.pop();
  driver.stk.top()->map_required_field($1, driver.tmpc); 
  free($1);
}
| STRING QMARK contract { 
  driver.tmpc = driver.stk.top();
  driver.stk.pop();
  driver.stk.top()->map_optional_field($1, driver.tmpc); 
  free($1);
}

array_contract: array_open CLOSEBRACKET
| array_open array_list CLOSEBRACKET 

array_open: OPENBRACKET { driver.stk.push(new UCSchema(uc_Array, NULL)); }

array_list : array_item
| array_item array_list

array_item: ARRAYTYPE COLON contract { 
  driver.tmpc = driver.stk.top();
  driver.stk.pop();
  driver.stk.top()->array_type(driver.tmpc); 
}
| EXISTS COLON contract { 
  driver.tmpc = driver.stk.top();
  driver.stk.pop();
  driver.stk.top()->array_exists(driver.tmpc); 
}
| ARRAYSIZE COLON integer_contract { 
  driver.tmpc = driver.stk.top();
  driver.stk.pop();
  driver.stk.top()->array_size(driver.tmpc); 
}
| INTEGER COLON contract { 
  driver.tmpc = driver.stk.top();
  driver.stk.pop();
  driver.stk.top()->array_element(driver.tmpc, $1); 
}

user_contract: IDENTIFIER { driver.stk.push(new UCSchema(uc_Unknown, $1)); free($1); }

group_contract: group_start CLOSEGROUP
| group_start group_list CLOSEGROUP

group_start: OPENGROUP { driver.stk.push(new UCSchema(uc_Group, NULL)); }

group_list: group_item
| group_item group_list

group_item : contract {
  driver.tmpc = driver.stk.top();
  driver.stk.pop();
  driver.stk.top()->add_alternate(driver.tmpc);  
}


%%
namespace JAD {

  void ContractParser::error(const std::string& error)
  { 
    UniversalContainer uce = ucexception(uce_Invalid_Contract);
    uce["parser error"] = error;
    uce["schema_line_number"] = driver.lexer->line_num;
  } 

  ContractDriver::ContractDriver(Buffer* buf)
  {
    lexer = new ContractLexer(buf);
    parser = new ContractParser(*this);
    tmpc = NULL;
  }

  ContractDriver::~ContractDriver(void)
  {
    delete lexer;
    delete parser;
  }

  /* static */ void UCSchema::add_contract_to_library(Buffer* buf)
  {
    ContractDriver cd(buf);
    cd.parser->parse();
  }

  /* static */ void UCSchema::add_contract_to_library(char* str)
  {
    Buffer* buf = new Buffer(str);
    ContractDriver cd(buf);
    cd.parser->parse();
    delete buf;
  }
}

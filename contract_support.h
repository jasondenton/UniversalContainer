#ifndef _CONTRACT_SUPPORT_H_
#define _CONTRACT_SUPPORT_H_

#include <stack>

#pragma clang diagnostic ignored "-Woverloaded-virtual"

#if ! defined(yyFlexLexerOnce)
#undef yyFlexLexer
#define yyFlexLexer contractFlexLexer // the trick with prefix; no namespace here :(
#include <FlexLexer.h>
#endif

namespace JAD {
	class ContractLexer : public yyFlexLexer
	{
	  Buffer* buffer;

	  virtual int LexerInput(char*, int);
	public:
	  ContractLexer(Buffer*);
	  int get_symbol(void*);
    int line_num;
	};

	class ContractParser;
  
  struct ContractDriver
  {
    ContractLexer* lexer;
    ContractParser* parser;
    UCSchema* tmpc;
    std::stack<UCSchema*> stk;
    std::string contract_name;

    ContractDriver(Buffer*);
    virtual ~ContractDriver(void);
  };
}
#endif

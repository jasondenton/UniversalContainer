###### Set these variables to control options to the bulid
DEBUG=YES
INSTALLROOT=/opt #libraries will live in $INSTALLROOT/lib, headers in $INSTALLROOT/include

#On MacOS, default bison is too old. Install a recent build instead.
LEX=/usr/local/opt/flex/bin/flex
YACC=/usr/local/opt/bison/bin/bison
COPTFLAGS=-I/usr/local/Cellar/flex/2.6.4/include

#COPTFLAGS = -Wno-format-security
SQLITE=YES #include SQLite

MYSQL=NO #include MySQL support
MYSQL_CFLAGS=-I/usr/local/mysql/include -D_MYSQL_ENABLED_
MYSQL_LIBS=-L/usr/local/mysql/lib -lmysqlclient -lpthread

CURL=YES #include CURL support
CURL_CFLAGS=
CURL_LIBS=-lcurl


#Debugging/Optimization
ifeq ($(DEBUG),YES)
DFLAGS=-g
COPTFLAGS += -DYYDEBUG
else
DFLAGS=-O3
endif

#Optional Library Defines
ifeq ($(MYSQL),YES)
COPTFLAGS += $(MYSQL_CFLAGS)
LOPTFLAGS += $(MYSQL_LIBS)
endif

#Optional Library Defines
ifeq ($(CURL),YES)
COPTFLAGS += $(CURL_CFLAGS)
LOPTFLAGS += $(CURL_LIBS)
endif

#Optional Library Defines
ifeq ($(SQLITE),YES)
COPTFLAGS += $(SQLITE_CFLAGS)
LOPTFLAGS += $(SQLITE_LIBS)
endif

#CFLAGS += -I/opt/share/bison 
#YACC = /opt/bin/yacc
#BISON = /opt/bin/bison

#Compilier defines
CFLAGS += -Wall -std=c++11 -fpic $(COPTFLAGS) $(DFLAGS)
CXXFLAGS = $(CFLAGS)
#LINKXX = $(CXX) $(DFLAGS) $(LOPTFLAGS)

#STATICLIB=ar -r


##### No user changes should be needed below this line

#optional files to build
#controled by options above

OPT_FILES=

#Optional Library Defines
ifeq ($(MYSQL),YES)
OPT_FILES += ucmysql.o
endif

#Optional Library Defines
ifeq ($(CURL),YES)
OPT_FILES += buffer_curl.o uc_curl.o
endif

#Optional Library Defines
ifeq ($(MYSQL),YES)
OPT_FILES += ucsqlite.o
endif

BASEFILES = ucontainer.o buffer.o buffer_util.o ucoder_ini.o ucoder_bin.o \
string_util.o uc_web.o ucio.o ucoder_json.o ucschema.o \
ucdb.o buffer_adapter.o file_adapter.o uc_options.o \
contract_parser.tab.o contract_lexer.o

libuc.a : $(BASEFILES) $(OPT_FILES)
	$(RM) libuc.a
	$(AR) -c -q $@ $^

ucoder_json.cpp : json_parser.lex
	$(LEX) json_parser.lex

contract_parser.tab.cc contract_parser.tab.hh : contract_parser.yy contract_support.h
	$(YACC) contract_parser.yy

contract_lexer.cpp : contract_lexer.lex contract_parser.tab.hh contract_support.h
	$(LEX) contract_lexer.lex
	
buffer.o : buffer.h
buffer_util.o : buffer.h
buffer_curl.o : buffer.h
ucontainer.o : ucontainer.h stl_util.h
ucschema.o : ucontainer.h ucschema.h
ucio.o :  ucontainer.h stl_util.h buffer.h ucio.h
ucoder_ini.o : ucontainer.h buffer.h
ucoder_bin.o : ucontainer.h buffer.h
ucoder_json.o : ucontainer.h buffer.h
uc_web.o : ucontainer.h stl_util.h buffer.h ucio.h uc_web.h
ucsqlite.o : ucdb.h ucsqlite.h
ucmysql.o : ucdb.h ucmysql.h
example.o : ucontainer.h ucio.h
buffer_adapter.o : buffer_adapter.h buffer.h ucontainer.h

install:
	install -m644 -o root -g wheel *.h $(INSTALLROOT)/include
	install -m644 -o root -g wheel *.a $(INSTALLROOT)/lib

uninstall:
	$(RM) $(INSTALLROOT)/include/buffer.h
	$(RM) $(INSTALLROOT)/include/stl_util.h
	$(RM) $(INSTALLROOT)/include/string_util.h 
	$(RM) $(INSTALLROOT)/include/uc_web.h 
	$(RM) $(INSTALLROOT)/include/ucio.h 
	$(RM) $(INSTALLROOT)/include/ucdb.h 
	$(RM) $(INSTALLROOT)/include/ucmysql.h
	$(RM) $(INSTALLROOT)/include/ucontainer.h 
	$(RM) ($INSTALLROOT)/include/uc_schema.h
	$(RM) $(INSTALLROOT)/include/ucsqlite.h 
	$(RM) $(INSTALLROOT)/include/univcont.h        
	$(RM) $(INSTALLROOT)/lib/libuc.a

clean :
	$(RM) *.o
	$(RM) *.a
	$(RM) *.hh
	$(RM) *.gcno
	$(RM) ucoder_json.cpp
	$(RM) contract_lexer.h
	$(RM) contract_lexer.cpp
	$(RM) contract_parser.tab.*
	$(RM) examples/example1
	$(RM) examples/example_schema
	rm -rf /examples/*.dSYM

/*
 * UniversalContainer library.
 * Copyright Jason Denton, 2008,2010,2012.
 * Made available under the new BSD license, as described in LICENSE
 *
 * Send comments and bug reports to jason.denton@gmail.com
 * http://www.greatpanic.com/code.html
 */

#ifndef __UCCONTAINER_H_
#define __UCCONTAINER_H_
#include <regex.h>
#include <map>
#include <vector>

#include "ucontainer.h"
#include "buffer.h"

namespace JAD {

  class UCSchema;
  typedef std::map<std::string,UCSchema*> ContractMap;
  typedef std::vector<UCSchema*> ContractVector;
  typedef std::map<int, UCSchema*> IdxMap;

  class UCSchema {
    static ContractMap __typelibrary;

    UniversalContainerType data_type;

    union {
      struct {
        bool has_lower;
        bool has_upper;
        long low;
        long high;
      } int_pair;
      struct {
        bool has_lower;
        bool has_upper;
        double low;
        double high;
      } real_pair;
      regex_t* regex;
      struct {
        ContractMap* require_map;
        ContractMap* optional_map;
      } map_constraints;
      struct {
        UCSchema* size;
        UCSchema* forall;
        IdxMap* elem;
        ContractVector* exists;
      } array_constraints;
      int bool_value;
      std::string* user_type; 
      ContractVector* group;
    } constraints;

    std::string tag;
    
    unsigned compare_map(const UniversalContainer&, UniversalContainer&) const;
    unsigned compare_array(const UniversalContainer&, UniversalContainer&) const;
    unsigned compare_group(const UniversalContainer&, UniversalContainer&) const;
  
    static std::vector<const char*> error_messages(unsigned result);

    unsigned check(const UniversalContainer&) const;
    unsigned check(const UniversalContainer&, UniversalContainer&) const;
    void check_and_throw(const UniversalContainer&, unsigned = 0XFFFFFFFF) const;
    void check_and_throw(const UniversalContainer&, UniversalContainer&, unsigned = 0XFFFFFFFF) const;
     
  public:
    UCSchema(void);
    UCSchema(bool, long, bool, long);
    UCSchema(char, char, bool, bool);
    UCSchema(bool, bool, double, double);
    UCSchema(char*);
    UCSchema(int);
    UCSchema(UniversalContainerType, char*);
    ~UCSchema(void);
    void add_to_library(const char*);

    static unsigned compare(std::string const&, const UniversalContainer&);
    static unsigned compare(std::string const&, const UniversalContainer&, 
      UniversalContainer&);
    static void compare_and_throw(std::string const&, const UniversalContainer&, 
      unsigned = 0XFFFFFFFF);
    static void compare_and_throw(std::string const&, const UniversalContainer&, 
      UniversalContainer&, unsigned = 0XFFFFFFFF);
  
    static void add_contract_to_library(char*);
    static void add_contract_to_library(Buffer*);

    void map_required_field(char* field, UCSchema* c);
    void map_optional_field(char* field, UCSchema* c);
    void array_type(UCSchema*);
    void array_exists(UCSchema*);
    void array_size(UCSchema*);
    void array_element(UCSchema*, int);
    void set_tag(char*);
    void add_alternate(UCSchema*);
  };

  bool load_contracts_file(const char*);

} //end namespace

#define ucc_IMPROPER_TYPE                     0x00000001
#define ucc_CONSTRAINT_VIOLATION              0x00000002
#define ucc_EXTRA_MAP_ELEMENT                 0x00000004
#define ucc_MISSING_REQUIRED_MAP_ELEMENT      0x00000008
#define ucc_MISSING_REQUIRED_ARRAY_ELEMENT    0x00000010
#define ucc_STRING_DOES_NOT_MATCH             0x00000020
#define ucc_WRONG_BOOLEAN_VALUE               0x00000040
#define ucc_NO_SUCH_TYPE                      0x00000080
#define ucc_SYMBOL_DOES_NOT_MATCH            0x00000100

#endif






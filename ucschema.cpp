/*
 * UniversalContainer library.
 * Copyright Jason Denton, 2008,2010,2012.
 * Made available under the new BSD license, as described in LICENSE
 *
 * Send comments and bug reports to jason.denton@gmail.com
 * http://www.greatpanic.com/code.html
 */

#include <cstring>
#include <string>
#include <map>

#include "ucschema.h"

namespace JAD {
  ContractMap UCSchema::__typelibrary;
  extern std::map<UniversalContainerType, UniversalContainerTypeAdapter*> UniversalContainerAdapter;
  extern std::map<std::string, UniversalContainerType> UniversalContainerAdapterNames;

  unsigned UCSchema::compare_map(const UniversalContainer& uc, 
    UniversalContainer& symbol_table) const
  {
    if (data_type != uc_Map || uc.get_type() != uc_Map) return 0;
    
    unsigned result = 0;
    unsigned req_count = 0;
    
    UniversalMap::iterator it = uc.map_begin();
    UniversalMap::iterator eit = uc.map_end(); 
    
    ContractMap* required = constraints.map_constraints.require_map;
    ContractMap* optional = constraints.map_constraints.optional_map;
    
    for (; it != eit; it++) {
      if (it->first[0] == '#') continue; //skip # keys
      if (required && required->count(it->first) > 0) {
	     req_count++;
	     result |= required->operator[](it->first)->check(it->second, symbol_table);
      }
      else if (optional && optional->count(it->first) > 0) {
	     result |= optional->operator[](it->first)->check(it->second, symbol_table);
      }
      else {
	     result |= ucc_EXTRA_MAP_ELEMENT;
      }
    }
    if (req_count != required->size()) result |= ucc_MISSING_REQUIRED_MAP_ELEMENT;
    
    return result;
  }
  
  unsigned UCSchema::compare_array(const UniversalContainer& uc, 
    UniversalContainer& symbol_table) const
  {
    if (data_type != uc_Array || uc.get_type() != uc_Array) return 0;
    unsigned result = 0;
    UniversalContainer tmp = (long int) uc.size();
    UniversalArray::iterator it;
    UniversalArray::iterator eit = uc.vector_end();
    
    if (constraints.array_constraints.size) 
      result |= constraints.array_constraints.size->check(tmp, symbol_table);
 
    if (constraints.array_constraints.forall) {
      it = uc.vector_begin();
      for (;it < eit; it++)
	     result |= constraints.array_constraints.forall->check(*it, symbol_table);
    }
    
    if (constraints.array_constraints.exists) {
      ContractVector::iterator exists_list_it = constraints.array_constraints.exists->begin();
      ContractVector::iterator exists_list_eit = constraints.array_constraints.exists->end();
      
      for (; exists_list_it < exists_list_eit; exists_list_it++) {	
        it = uc.vector_begin();
        bool found = false;
        for (;it < eit && !found; it++)
          found = (*exists_list_it)->check(*it, symbol_table) == 0;
        if (!found)
          result |= ucc_MISSING_REQUIRED_ARRAY_ELEMENT;
      }
    }
    return result;
  }

  unsigned UCSchema::check(const UniversalContainer& uc) const
  {
    UniversalContainer symbol_table;
    return check(uc, symbol_table);
  }

  unsigned UCSchema::compare_group(const UniversalContainer& uc, 
    UniversalContainer& symbol_table) const
  {
    UniversalContainer stable;
    unsigned result = 0;
    for (int i = 0; !result && i < constraints.group->size(); i++) {
      stable = symbol_table.clone();
      result = (*(constraints.group))[i]->check(uc, stable);
    }
    if (result) return ucc_IMPROPER_TYPE;
    symbol_table = stable;
    return 0;
  }

  unsigned UCSchema::check(const UniversalContainer& uc, 
    UniversalContainer& symbol_table) const
  {
    int tmp_int;
    double tmp_dbl;
    
    if (!tag.empty()) {
      if (symbol_table.exists(tag)) {
        if (!(symbol_table[tag] == uc)) {
          return ucc_SYMBOL_DOES_NOT_MATCH;
        }
      }
      else symbol_table[tag] = uc;
    }

    if (data_type == uc_Group) return compare_group(uc, symbol_table);

    if (data_type == uc_UserType) {       
      if (constraints.user_type->empty()) return ucc_IMPROPER_TYPE;
    
      if (__typelibrary.count(*(constraints.user_type)) < 1) return ucc_NO_SUCH_TYPE;
      return __typelibrary[*(constraints.user_type)]->check(uc, symbol_table);
    }

    if (data_type != uc.get_type() && 
      !((data_type == uc_String || data_type == uc_WString) &&
        (uc.get_type() == uc_String || uc.get_type() == uc_WString)))
        return ucc_IMPROPER_TYPE;
 
    switch(data_type) {
      case uc_Integer :
      case uc_Character :
        tmp_int = uc;
        if ((constraints.int_pair.has_lower && tmp_int < constraints.int_pair.low) || 
  	  (constraints.int_pair.has_upper && tmp_int > constraints.int_pair.high))
  	    return ucc_CONSTRAINT_VIOLATION;
        break;
      case uc_Real :
        tmp_dbl = uc;
        if ((constraints.real_pair.has_lower && tmp_dbl < constraints.real_pair.low) ||
  	    (constraints.real_pair.has_upper && tmp_dbl > constraints.real_pair.high))
  	    return ucc_CONSTRAINT_VIOLATION;
        break; 
      case uc_String :
        if (constraints.regex && regexec(constraints.regex,uc.c_str(),0,NULL,0))
  	     return ucc_STRING_DOES_NOT_MATCH;
        break;
      case uc_Boolean :
        if (constraints.bool_value == -1) break;
        tmp_int = bool(uc);
        if (tmp_int != constraints.bool_value) return ucc_WRONG_BOOLEAN_VALUE;
        break;
      case uc_Map :
        return compare_map(uc, symbol_table);
        break;
      case uc_Array :
        return compare_array(uc, symbol_table);
        break;
    }
    return 0;
  }
  
  UCSchema::UCSchema(void)
  {
    data_type = uc_Unknown;
    constraints.regex = NULL;
  }

  UCSchema::~UCSchema(void)
  {
    switch(data_type) {
    case(uc_String): 
      if (constraints.regex) {
        regfree(constraints.regex);
        delete constraints.regex;
      }
      break;
    case(uc_Map):
      if (constraints.map_constraints.require_map) delete constraints.map_constraints.require_map;
      if (constraints.map_constraints.optional_map) delete constraints.map_constraints.optional_map;
      break;
    case(uc_Array):
      if (constraints.array_constraints.size) delete constraints.array_constraints.size;
      if (constraints.array_constraints.forall) delete constraints.array_constraints.forall;
      if (constraints.array_constraints.exists) delete constraints.array_constraints.exists;
      break;
    case(uc_UserType):
      if (constraints.user_type) delete constraints.user_type;
      break;
    case(uc_Group):
      if (constraints.group) delete constraints.group;
      break;
    }
  }
 
  static const char* ucc_IMPROPER_TYPE_str = "Element has wrong type.";
  static const char* ucc_CONSTRAINT_VIOLATION_str = "A constraint was violated.";
  static const char* ucc_EXTRA_MAP_ELEMENT_str = "An map element not specified in the contract is present.";
  static const char* ucc_MISSING_REQUIRED_MAP_ELEMENT_str = "A required map element is missing.";
  static const char* ucc_MISSING_REQUIRED_ARRAY_ELEMENT_str = "An array element that must exist is not present.";
  static const char* ucc_STRING_DOES_NOT_MATCH_str = "A string does not match its given regex expression.";
  static const char* ucc_WRONG_BOOLEAN_VALUE_str = "A boolean does not have the required value.";
  static const char* ucc_NO_SUCH_TYPE_str = "No such type registered with library.";
  static const char* ucc_SYMBOL_DOES_NOT_MATCH_str = "Symbol does not match previous instance.";
  
  std::vector<const char*> UCSchema::error_messages(unsigned result)
  {
    std::vector<const char*> mesg;
    
    if (result & ucc_IMPROPER_TYPE) mesg.push_back(ucc_IMPROPER_TYPE_str); 
    if (result & ucc_CONSTRAINT_VIOLATION) mesg.push_back(ucc_CONSTRAINT_VIOLATION_str);
    if (result & ucc_EXTRA_MAP_ELEMENT) mesg.push_back(ucc_EXTRA_MAP_ELEMENT_str);
    if (result & ucc_WRONG_BOOLEAN_VALUE) mesg.push_back(ucc_WRONG_BOOLEAN_VALUE_str);
    if (result & ucc_MISSING_REQUIRED_MAP_ELEMENT) 
      mesg.push_back(ucc_MISSING_REQUIRED_MAP_ELEMENT_str); 
    if (result & ucc_MISSING_REQUIRED_ARRAY_ELEMENT)
      mesg.push_back(ucc_MISSING_REQUIRED_ARRAY_ELEMENT_str);  
    if (result & ucc_STRING_DOES_NOT_MATCH) 
      mesg.push_back(ucc_STRING_DOES_NOT_MATCH_str);
    if (result & ucc_NO_SUCH_TYPE) 
      mesg.push_back(ucc_NO_SUCH_TYPE_str);
    if (result & ucc_SYMBOL_DOES_NOT_MATCH) 
      mesg.push_back(ucc_SYMBOL_DOES_NOT_MATCH_str);
   return mesg;
  }

  void UCSchema::check_and_throw(const UniversalContainer& uc, unsigned mask) const
  {
    UniversalContainer symbol_table;
    check_and_throw(uc, symbol_table, mask);
  }

  void UCSchema::check_and_throw(const UniversalContainer& uc, UniversalContainer& symbol_table,
    unsigned mask) const
  {
    unsigned result = check(uc, symbol_table) & mask;
    if (result) {
      std::vector<const char*> msgs = error_messages(result);
      UniversalContainer uce = ucexception(uce_ContractViolation);
      for (size_t i = 0; i < msgs.size(); i++)
        uce["violations"][i] = msgs[i];
      uce["compare_result"] = int(result);
      throw uce;
    }
  }

  void UCSchema::add_to_library(const char* name)
  {
      if (__typelibrary.count(name) > 0) {
        delete __typelibrary[name];
      }   
      __typelibrary[name]= this;
  } 

 UCSchema::UCSchema(bool lower, long l, bool upper, long u)
  {
    data_type = uc_Integer;
    if (lower) {
      constraints.int_pair.has_lower = true;
      constraints.int_pair.low = l;
    } else {
      constraints.int_pair.has_lower = false;
    }

    if (upper) {
      constraints.int_pair.has_upper = true;
      constraints.int_pair.high = u;
    } else {
      constraints.int_pair.has_upper = false;
    }
  }

  UCSchema::UCSchema(char l, char u, bool lower, bool upper)
  {
    data_type = uc_Character;
    if (lower) {
      constraints.int_pair.has_lower = true;
      constraints.int_pair.low = l;
    } else {
      constraints.int_pair.has_lower = false;
    }

    if (upper) {
      constraints.int_pair.has_upper = true;
      constraints.int_pair.high = u;
    } else {
      constraints.int_pair.has_upper = false;
    }
  }

  UCSchema::UCSchema(bool lower, bool upper, double l, double u)
  {
    data_type = uc_Real;
    if (lower) {
      constraints.real_pair.has_lower = true;
      constraints.real_pair.low = l;
    } else {
      constraints.real_pair.has_lower = false;
    }

    if (upper) {
      constraints.real_pair.has_upper = true;
      constraints.real_pair.high = u;
    } else {
      constraints.real_pair.has_upper = false;
    }
  }

  UCSchema::UCSchema(char* regex)
  {
    data_type = uc_String;
    if (regex) {
      constraints.regex = new regex_t;
      regcomp(constraints.regex,regex, REG_EXTENDED | REG_NOSUB | REG_ENHANCED );
    }
  }

  UCSchema::UCSchema(int bv)
  {
    constraints.bool_value = bv;
    data_type = uc_Boolean;
  }

  UCSchema::UCSchema(UniversalContainerType dt, char* data)
  {
    if (dt == uc_Map) {
      data_type = uc_Map;
      constraints.map_constraints.require_map = new ContractMap;
      constraints.map_constraints.optional_map = new ContractMap;      
    } else if (dt == uc_Array) {
      data_type = uc_Array;
      constraints.array_constraints.size = NULL;
      constraints.array_constraints.forall = NULL;
      constraints.array_constraints.exists = new ContractVector; 
      constraints.array_constraints.elem = new IdxMap;    
    } else if (dt == uc_Group) {
      data_type = uc_Group;
      constraints.group = new ContractVector;
    } else {
      data_type = uc_UserType;
      constraints.user_type = new std::string(data);
    } 
  }

  void UCSchema::map_required_field(char* field, UCSchema* c)
  {
    if (data_type != uc_Map) throw ucexception(uce_Non_Map_as_Map);
    (*(constraints.map_constraints.require_map))[field] = c;
  }

  void UCSchema::map_optional_field(char* field, UCSchema* c)
  {
    if (data_type != uc_Map) throw ucexception(uce_Non_Map_as_Map);
    (*(constraints.map_constraints.optional_map))[field] = c;
  }

  void UCSchema::array_type(UCSchema* c)
  {
    if (data_type != uc_Array) throw ucexception(uce_Non_Array_as_Array);
    if (constraints.array_constraints.forall) delete constraints.array_constraints.forall;
    constraints.array_constraints.forall = c;
  }

  void UCSchema::array_exists(UCSchema* c)
  {
    if (data_type != uc_Array) throw ucexception(uce_Non_Array_as_Array);
    constraints.array_constraints.exists->push_back(c);
  }

  void UCSchema::array_size(UCSchema* c)
  {
    if (data_type != uc_Array) throw ucexception(uce_Non_Array_as_Array);
    if (constraints.array_constraints.size) delete constraints.array_constraints.size;
    constraints.array_constraints.size = c;
  }

  void UCSchema::array_element(UCSchema* c, int idx)
  {
     if (data_type != uc_Array) throw ucexception(uce_Non_Array_as_Array);
     (*(constraints.array_constraints.elem))[idx] = c;   
  }

  void UCSchema::add_alternate(UCSchema* c)
  {
     if (data_type != uc_Group) throw ucexception(uce_Invalid_Contract);
     constraints.group->push_back(c);  
  }

  void UCSchema::set_tag(char* t)
  {
    tag = std::string(t);
  }

  /* static */ unsigned UCSchema::compare(std::string const& type, 
    const UniversalContainer& uc)
  {
    UniversalContainer symbol_table;
    return compare(type, uc, symbol_table);
  }

  /* static */ void UCSchema::compare_and_throw(std::string const& type, 
    const UniversalContainer& uc, unsigned mask)
  {
    UniversalContainer symbol_table;
    return compare_and_throw(type, uc, symbol_table, mask);
  }

  /* static */ unsigned UCSchema::compare(std::string const& type, 
    const UniversalContainer& uc, UniversalContainer& symbol_table)
  {
    if (__typelibrary.count(type) < 1) return ucc_NO_SUCH_TYPE;
    return __typelibrary[type]->check(uc, symbol_table); 
  }

  /* static */ void UCSchema::compare_and_throw(std::string const& type, 
    const UniversalContainer& uc, UniversalContainer& symbol_table, unsigned mask)
  {
    if (__typelibrary.count(type) < 1) {
      UniversalContainer uce = ucexception(uce_ContractViolation);
      uce["no such type"][0] = type;
      uce["compare_result"] = int(ucc_NO_SUCH_TYPE);
      uce["violations"][0] = ucc_NO_SUCH_TYPE_str;
      throw uce;
    }
    __typelibrary[type]->check_and_throw(uc, symbol_table, mask); 
  }

  bool load_contracts_file(const char* filename)
  {
    Buffer* schema = read_to_buffer(filename);
    if (!schema) return false;
    UCSchema::add_contract_to_library(schema);
    delete schema;
    return true;
  }
} //end namespace

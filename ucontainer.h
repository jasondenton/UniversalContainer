/*
 * UniversalContainer library.
 * Copyright Jason Denton, 2008,2010.
 * Made available under the new BSD license, as described in LICENSE
 *
 * Send comments and bug reports to jason.denton@gmail.com
 * http://www.greatpanic.com/code.html
 */

#ifndef _UCONTAINER_HEADER_
#define _UCONTAINER_HEADER_

#include <string>
#include <map>
#include <vector>

#ifndef NDEBUG
#define ucexception(C) UniversalContainer::construct_exception(C,__FUNCTION__,__FILE__,__LINE__,NULL)
#else
#define ucexception(C) UniversalContainer::construct_exception(C,NULL,NULL,0,NULL)
#endif

#ifndef NDEBUG
#define ucdataexception(C,D) UniversalContainer::construct_exception(C,__FUNCTION__,__FILE__,__LINE__,&D)
#else
#define ucdataexception(C,D) UniversalContainer::construct_exception(C,NULL,NULL,0,&D)
#endif

namespace JAD {

  typedef char UniversalContainerType;

  const UniversalContainerType uc_Null       =  0;
  const UniversalContainerType uc_Integer    =  1;
  const UniversalContainerType uc_Boolean    =  2;
  const UniversalContainerType uc_Character  =  3;
  const UniversalContainerType uc_String     =  4;
  const UniversalContainerType uc_WString    =  5;
  const UniversalContainerType uc_Real       =  6;
  const UniversalContainerType uc_Map        = 10;
  const UniversalContainerType uc_Array      = 11;
  const UniversalContainerType uc_Reference  = 12;
  const UniversalContainerType uc_UserType   = 13; //for use in contracts
  const UniversalContainerType uc_Group      = 14; //for use in contracts
  const UniversalContainerType uc_Buffer     = 15; //for use in CL parser 
  const UniversalContainerType uc_FileInName = 16; //for use in CL parser
  const UniversalContainerType uc_FileOutName= 17; //for use in CL parser
  const UniversalContainerType uc_JSON       = 18; //for use in CL parser
  const UniversalContainerType uc_LastType   = 18; 
  const UniversalContainerType uc_Unknown    = -1;

  extern const char* UnviersalContainerTypeName[];

  /*
   * Used internally by UniversalContainer. Almost always you will
   * want to wrap these in a UniversalContainer rather than using these types.
   */
 
  class UniversalContainer;
  typedef std::vector<UniversalContainer> UniversalArray;
  typedef std::map<std::string, UniversalContainer> UniversalMap;

  struct UniversalContainerTypeAdapter {
    virtual std::string to_string(void*);
    virtual void* clone(void*) = 0;
    virtual void on_delete(void*) = 0;
    virtual UniversalContainer pack(void*);
    virtual ~UniversalContainerTypeAdapter(void);
  };

  class UniversalContainer
  {
  protected :
    
    //member variables
    UniversalContainerType type;
    bool dirty;
    unsigned* refcount;
    
    union {
      double real;
      bool tf;
      char chr;
      long num;
      std::string* str;
      std::wstring* wstr;
      UniversalArray* ray;
      UniversalMap* map;
      void* reference;
    } data;
    
    //internal setter methods
    //used by constructors and assignment operators
    inline void set_value_integer(long);
    inline void set_value_char(char);
    inline void set_value_double(double);
    inline void set_value_bool(bool);
    inline void set_value_string(const std::string&);
    inline void set_value_wstring(const std::wstring&);
    inline void set_value_cstr(const char*);
    
    //internal conversion methods
    //used by casting and equality testing operators
    inline int convert_int(void) const;
    inline double convert_double(void) const;
    inline char convert_char(void) const;
    inline bool convert_bool(void) const;
    inline long convert_long(void) const;
    inline std::string convert_string(void) const;
    inline std::wstring convert_wstring(void) const;
    
    //internal designates. These routines are generally
    //exposed through multiple interfaces, which are
    //more user friendly
    void duplicate(const UniversalContainer&);
    UniversalContainer& map_brackets(const std::string&);
    
    //utility functions
    static std::string convert_wstring_to_string(const std::wstring* w);
    static std::wstring convert_string_to_wstring(const std::string* s);
    static double convert_string_to_double(const std::string* s);
    static double convert_wstring_to_double(const std::wstring* s);
    static long convert_string_to_long(const std::string* s);
    static long convert_wstring_to_long(const std::wstring* s);
   
    bool remove_refcount(void);
    
  private:
      void cleanup(void);
      void is_integer(std::string const&, long&, bool&) const;

  public: 
    //constructors
    UniversalContainer(void);
    UniversalContainer(int);
    UniversalContainer(long);
    UniversalContainer(double);
    UniversalContainer(bool);
    UniversalContainer(char);
    UniversalContainer(const std::string);
    UniversalContainer(const std::wstring);
    UniversalContainer(char*);
    UniversalContainer(const UniversalContainer&);
    UniversalContainer(UniversalContainerType, void*);
    void string_interpret(const std::string s);
    
    //destructor
    ~UniversalContainer(void);
    
    //cast operators
    operator int(void) const;
    operator double(void) const;
    operator char(void) const;
    operator bool(void) const;
    operator long(void) const;
    operator std::string(void) const;
    operator std::string*(void) const;
    operator std::wstring(void) const;
    operator std::wstring*(void) const;
    const char* c_str(void) const;
    
    //container access
    UniversalContainer& operator[](int);
    UniversalContainer& operator[](size_t);
    UniversalContainer& operator[](const std::string&);
    UniversalContainer& operator[](const std::wstring&);
    UniversalContainer& operator[](char*);
    UniversalContainer& operator[](const char*);

    //assignment operators
    UniversalContainer& operator=(long);
    UniversalContainer& operator=(int);
    UniversalContainer& operator=(double);
    UniversalContainer& operator=(bool);
    UniversalContainer& operator=(char);
    UniversalContainer& operator=(const std::string&);
    UniversalContainer& operator=(const std::wstring&);
    UniversalContainer& operator=(const char*);
    UniversalContainer& operator=(const UniversalContainer&);
    void* cast_or_throw(UniversalContainerType) const;

    //logical operators
    bool operator==(const UniversalContainer&) const;
    bool operator==(int) const;
    bool operator==(long) const;
    bool operator==(char) const;
    bool operator==(double) const;
    bool operator==(std::string) const;
    bool operator==(std::wstring) const;
    bool operator==(const char*) const;
    bool operator==(bool) const;
    
    //support operations
    UniversalContainerType get_type(void) const;
    UniversalContainer clone(void) const;
    UniversalContainer partial_copy(const char**);

    bool remove(const std::string key);
    void clear(void);
    bool exists(std::string const& key) const;
    size_t size(void) const;
    size_t length(void) const;
    bool is_dirty(void) const;
    void clean(void);
    UniversalContainer& added_element(void);
    std::vector<std::string> keys_for_map(void) const;
    std::string to_string(void) const;
    
    //allow the user to container type
    void init_map(void);
    void init_array(void);

    //Support for maps, vectors, and iterators
    UniversalMap::iterator map_begin(void) const; 
    UniversalMap::iterator map_end(void) const;
    UniversalArray::iterator vector_begin(void) const;
    UniversalArray::iterator vector_end(void) const;
    UniversalMap* get_map(void) const;
    UniversalArray* get_vector(void) const;
    
    static UniversalContainerType register_new_type(UniversalContainerTypeAdapter*);

    static UniversalContainer construct_exception(const int, const char* = NULL,
						const char* = NULL, int = 0, const UniversalContainer* = NULL);
  };

  struct CmdLineOption {
    const char* lng;
    char shrt;
    const char* desc;
    const char* def;
    UniversalContainerType ptype;
  };

  UniversalContainer parse_command_line(int, char**, const CmdLineOption*, const char*);
  
} //end namespace

/* Defines for exception error codes */
#define KNOWN_EXCEPTIONS 18

#define uce_Unknown 0
#define uce_TypeMismatch_Write 1
#define uce_TypeMismatch_Read 2
#define uce_Collection_as_Scalar 3
#define uce_Scalar_as_Collection 4
#define uce_Non_Map_as_Map 5
#define uce_Non_Array_as_Array 6
#define uce_Array_Subscript_Out_of_Bounds 7 
#define uce_Deserialization_Error 8
#define uce_Serialization_Error 9
#define uce_DB_Connection 10
#define uce_Unknown_mime_type 11
#define uce_Communication_Error 12
#define uce_ContractViolation 13
#define uce_IOError 14
#define uce_Unregistered_Type 15
#define uce_Invalid_Contract 16
#define uce_Symbol_Does_Not_Match 17

//C++17 depreciates the register keyword; flex doesn't know this yet.
#define register 
#endif

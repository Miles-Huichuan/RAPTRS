// definition-tree2.hxx - Curtis Olson

#pragma once

#include <stdint.h>
#include <vector>
#include <variant>
#include <map>
#include <iostream>
#include <string>

using std::map;
using std::string;
using std::vector;

// minimal types for logging -- log as this type.  put these in the
// global name space otherwise the notation becomes crushing.
enum log_tag_t {
    LOG_NONE,
    LOG_BOOL, LOG_INT8, LOG_UINT8,
    LOG_INT16, LOG_UINT16,
    LOG_INT32, LOG_UINT32,
    LOG_INT64, LOG_UINT64, LOG_LONG,
    LOG_FLOAT, LOG_DOUBLE
};
    
class Element {

 private:

    // supported types
    enum { BOOL, INT, LONG, FLOAT, DOUBLE } tag;

    union {
        bool b;
        int i;
        long l;
        float f;
        double d;
    };

 public:
    
    string description;
    log_tag_t datalog{LOG_NONE};
    log_tag_t telemetry{LOG_NONE};
    
    Element() {}
    ~Element() {}

    void setBool( bool val ) { b = val; tag = BOOL; }
    void setInt( int val ) { i = val; tag = INT; }
    void setLong( long val ) { l = val; tag = LONG; }
    void setFloat( float val ) { f = val; tag = FLOAT; }
    void setDouble( double val ) { d = val; tag = DOUBLE; }

    bool getBool() {
        switch(tag) {
            case BOOL: return b;
            case INT: return i;
            case LONG: return l;
            case FLOAT: return f;
            case DOUBLE: return d;
            default: return false;
        }
    }
    int getInt() {
        switch(tag) {
            case BOOL: return b;
            case INT: return i;
            case LONG: return l;
            case FLOAT: return f;
            case DOUBLE: return d;
            default: return 0;
        }
    }
    long getLong() {
        switch(tag) {
            case BOOL: return b;
            case INT: return i;
            case LONG: return l;
            case FLOAT: return f;
            case DOUBLE: return d;
            default: return 0;
        }
    }
    float getFloat() {
        switch(tag) {
            case BOOL: return b;
            case INT: return i;
            case LONG: return l;
            case FLOAT: return f;
            case DOUBLE: return d;
            default: return 0.0;
        }
    }
    double getDouble() {
        switch(tag) {
            case BOOL: return b;
            case INT: return i;
            case LONG: return l;
            case FLOAT: return f;
            case DOUBLE: return d;
            default: return 0.0;
        }
    }

    log_tag_t getLoggingType() {
        return datalog;
    }

    log_tag_t getTelemetryType() {
        return telemetry;
    }
};
    
typedef map<string, Element *> def_tree_t ;

class DefinitionTree2 {
    
 public:
    

    DefinitionTree2() {}
    ~DefinitionTree2() {}

    Element *initElement(string name, string desc,
                         log_tag_t datalog,
                         log_tag_t telemetry);
    Element *makeAlias(string orig_name, string alias_name);
    Element *getElement(string name, bool create=true);

    void GetKeys(string Name, vector<string> *KeysPtr);
    size_t Size(std::string Name);

    void Erase(string name);
    
 private:
    
    def_tree_t data;
};

// reference a global instance of the deftree
extern DefinitionTree2 deftree;

class DefinitionTreeOld {
  public:
    // variable definition
    struct VariableDefinition {
      std::variant<uint64_t*,uint32_t*,uint16_t*,uint8_t*,int64_t*,int32_t*,int16_t*,int8_t*,float*, double*> Value;
      std::string Description;
      bool Datalog;
      bool Telemetry;
    };
    void DefineMember(std::string Name,struct VariableDefinition &VariableDefinitionRef);
    void InitMember(std::string Name);
    void InitMember(std::string Name,std::variant<uint64_t*,uint32_t*,uint16_t*,uint8_t*,int64_t*,int32_t*,int16_t*,int8_t*,float*, double*> Value,std::string Description,bool Datalog,bool Telemetry);
    void SetValuePtr(std::string Name,std::variant<uint64_t*,uint32_t*,uint16_t*,uint8_t*,int64_t*,int32_t*,int16_t*,int8_t*,float*, double*> Value);
    void SetDescription(std::string Name,std::string Description);
    void SetDatalog(std::string Name,bool Datalog);
    void SetTelemetry(std::string Name,bool Telemetry);
    /* Gets pointer to value for a definition tree member */
    template <typename T> T GetValuePtr(std::string Name) {
      if(auto val = std::get_if<T>(&Data_[Name].Value)) {
        return *val;
      } else {
        return NULL;
      }
    }
    std::string GetDescription(std::string Name);
    bool GetDatalog(std::string Name);
    bool GetTelemetry(std::string Name);
    void GetMember(std::string Name,struct VariableDefinition *VariableDefinitionPtr);
    size_t Size(std::string Name);
    void GetKeys(std::string Name,std::vector<std::string> *KeysPtr);
    void PrettyPrint(std::string Prefix);
    void Erase(std::string Name);
    void Clear();
  private:
    std::map<std::string,VariableDefinition> Data_;
    std::string GetType(const struct VariableDefinition *VariableDefinitionPtr);
    std::string GetValue(const struct VariableDefinition *VariableDefinitionPtr);
};

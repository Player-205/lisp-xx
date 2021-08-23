#pragma once
#include <numeric>
#include <string>
#include <variant>
#include <vector>
#include <memory>
#include <variant>
#include "matching.hpp"

enum class KeyWord 
{
    Div,
    Times,
    Minus,
    Plus,
    Mod,
    Concat,
    GT,
    GE,
    LT,
    LE,
    Eq,
    NoEq,
    Quote,
    TypeOf,
    Cons,
    Car,
    Cdr,
    Cond,
    Print,
    Read,
    Eval
};

inline std::string keyword_to_string(KeyWord word)
{
    switch (word) {
        case KeyWord::Plus:   return "+"     ;
        case KeyWord::Minus:  return "-"     ;
        case KeyWord::Times:  return "*"     ;
        case KeyWord::Div:    return "/"     ;
        case KeyWord::Mod:    return "mod"   ;   
        case KeyWord::Concat: return "++"    ;     
        case KeyWord::GT:     return ">"     ; 
        case KeyWord::GE:     return ">="    ; 
        case KeyWord::LT:     return "<"     ; 
        case KeyWord::LE:     return "<="    ; 
        case KeyWord::Eq:     return "="     ; 
        case KeyWord::NoEq:   return "!="    ;   
        case KeyWord::Quote:  return "quote" ;     
        case KeyWord::TypeOf: return "typeof";     
        case KeyWord::Cons:   return "cons"  ;   
        case KeyWord::Car:    return "car"   ;   
        case KeyWord::Cdr:    return "cdr"   ;   
        case KeyWord::Cond:   return "cond"  ;   
        case KeyWord::Print:  return "print" ;     
        case KeyWord::Read:   return "read"  ;   
        case KeyWord::Eval:   return "eval"  ;   
    }
    throw "unreachable";
}

struct Symbol { std::string value; };

struct LispValue;

using list_t = std::vector<LispValue>;

using value_t = std::variant<int64_t, double, bool, std::string, Symbol, KeyWord, list_t>;

std::string list_to_string(list_t list);


inline std::string to_string_with_escape_chars(std::string const &str)
{
    std::string result;
    result.reserve(str.length() + 2);
    result += "\"";
    for (char ch : str) {
        switch (ch) {
            case '\0' : result += "\\0" ; break;
            case '\b' : result += "\\b" ; break;
            case '\n' : result += "\\n" ; break;
            case '\t' : result += "\\t" ; break;
            case '\a' : result += "\\a" ; break;
            case '\f' : result += "\\f" ; break;
            case '\r' : result += "\\r" ; break;
            case '\v' : result += "\\v" ; break;
            case '\"' : result += "\\\""; break;
            case '\\' : result += "\\\\"; break;
            default: result += ch; break;
        }
    }
    result += "\"";
    return result; 
}

struct LispValue : public value_t
{
    using value_t::value_t;
    operator std::string()
    {
        return match(static_cast<value_t>(*this),
            [](list_t x)              { return list_to_string(x);                 },
            [](Symbol x)              { return x.value;                           },
            [](KeyWord x)             { return keyword_to_string(x);              },
            [](std::string const & x) { return to_string_with_escape_chars(x);    },
            [](bool x)                { return std::string{x ? "true" : "false"}; },
            [](auto x)                { return std::to_string(x);                 }
        );
    }
};




inline std::string list_to_string(list_t list)
{
    std::string result;
    result.reserve(500);
    result += "(";
    if(list.size())
      result += std::accumulate(++list.crbegin(),
          list.crend(),
          std::string(list.back()),
          [](auto acc, auto val){
              return acc + " " + std::string(val);
          }
      );
    result += ")";
    return result;
}
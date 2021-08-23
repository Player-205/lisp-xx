#pragma once

#include <iostream>
#include <stack>
#include <string>
#include <vector>
#include <algorithm>

#include "AST.hpp"

bool is_keyword(std::string const & str, auto && action)
{
  if(str == "true"  ) { action(true);            return true; }
  if(str == "false" ) { action(false);           return true; }
  if(str == "+"     ) { action(KeyWord::Plus  ); return true; }
  if(str == "-"     ) { action(KeyWord::Minus ); return true; }
  if(str == "*"     ) { action(KeyWord::Times ); return true; }
  if(str == "/"     ) { action(KeyWord::Div   ); return true; }
  if(str == "mod"   ) { action(KeyWord::Mod   ); return true; }
  if(str == "++"    ) { action(KeyWord::Concat); return true; }
  if(str == ">"     ) { action(KeyWord::GT    ); return true; }
  if(str == ">="    ) { action(KeyWord::GE    ); return true; }
  if(str == "<"     ) { action(KeyWord::LT    ); return true; }
  if(str == "<="    ) { action(KeyWord::LE    ); return true; }
  if(str == "="     ) { action(KeyWord::Eq    ); return true; }
  if(str == "!="    ) { action(KeyWord::NoEq  ); return true; }
  if(str == "quote" ) { action(KeyWord::Quote ); return true; }
  if(str == "typeof") { action(KeyWord::TypeOf); return true; }
  if(str == "cons"  ) { action(KeyWord::Cons  ); return true; }
  if(str == "car"   ) { action(KeyWord::Car   ); return true; }
  if(str == "cdr"   ) { action(KeyWord::Cdr   ); return true; }
  if(str == "cond"  ) { action(KeyWord::Cond  ); return true; }
  if(str == "print" ) { action(KeyWord::Print ); return true; }
  if(str == "read"  ) { action(KeyWord::Read  ); return true; }
  if(str == "eval"  ) { action(KeyWord::Eval  ); return true; }
  return false;
}




bool is_number(std::string const &str, auto &&action) {
  bool was_dot = false;

  for (size_t i = str[0] == '-'; i < str.size(); ++i) {
    if (!was_dot && str[i] == '.') {
      was_dot = true;
      continue;
    }
    if (!std::isdigit(str[i])) {
      return false;
    }
  }
  if (was_dot)
   action(std::stod(str));
  else
   action(std::stoll(str));
  return true;
}

void parse_raw_string(std::string input, size_t& i, size_t& line, size_t& pos ,auto && next, auto && action)
{
  std::string str;
  for(;i < input.length() && input[i] != '"'; next())
    if(input[i] == '\\')
    {
      switch(input[i + 1]){
        case '0': str += '\0'; break;  case 'a' : str += '\a'; break;
        case 'b': str += '\b'; break;  case 'f' : str += '\f'; break;
        case 'n': str += '\n'; break;  case 'r' : str += '\r'; break;
        case 't': str += '\t'; break;  case 'v' : str += '\v'; break;
        case '"': str += '\"'; break;  case '\\': str += '\\'; break;
        default: throw std::runtime_error(
          std::string{"Invalid escape sequence at line "} 
                                + std::to_string(line)
                                + std::string{" at position "}
                                + std::to_string(pos) + std::string{"."});
      }
      next();
    }
    else str += input[i];
  action(str);
}

inline LispValue parse_lisp(std::string const & input)
{

  bool quoted = false;
  std::stack<list_t> lists_stack;
  lists_stack.push({});
  size_t 
     line = 0
   , pos = 1
   , i = 0;
  
  auto next = [&i, &pos](){ ++i; ++pos; };
  for(; i < input.length(); next())
  {
    auto end_list = [&]() {
      auto end_list_internal = [&](){
        auto val = lists_stack.top();
        lists_stack.pop();
        std::reverse(val.begin(), val.end());
        lists_stack.top().emplace_back(val);
      };
      end_list_internal();
      if(quoted) { 
        end_list_internal();
        quoted = false;
      }
    };
    auto action = [&](auto && val) {
      
      if(lists_stack.empty()) throw std::runtime_error("invalid syntax");
      lists_stack.top().emplace_back(val);
      if(quoted) 
      {
        end_list();
        quoted = false;
      }
    };
    auto new_list = [&](){
        lists_stack.push({});
    };
    switch(input[i])
    {
      case '\n':
      ++line;
      pos = 1;
      case ' ':
      case '\r':
      continue;
      case '(':
      {
        new_list();
        break;
      }
      case ')': 
      {
        end_list();
        break;
      }
      case '\'':
      {
        new_list();
        action(KeyWord::Quote);
        quoted = true;
        break;
      }
      case '"':
      {
        ++i;
        parse_raw_string(input, i, line, pos, next, action);
        break;
      }

      default:
      {
        std::string str;
        for(size_t j = i +1; 
             j <= input.length() 
          && input[j] != '(' 
          && input[j] != ')' 
          && input[j] != ' ' 
          && input[j] != '\t' 
          && input[j] != '\n' 
          && input[j] != '"' ; ++j, next())
        {
          str +=input[i];
        }
        str +=input[i];
        if(str.empty()) break;
        if(is_number(str, action)) break;
        if(is_keyword(str, action))break; // bool is also keyword
        action(Symbol{str});
      }
    }
  }
  if(lists_stack.empty()) throw std::runtime_error("Parse error");
  list_t l = lists_stack.top();
  if(l.size() == 1) {return l[0];}
  return LispValue{l};
}

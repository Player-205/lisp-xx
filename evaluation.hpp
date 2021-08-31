#pragma once

#include <iostream>
#include <stack>
#include <csetjmp>
#include "AST.hpp"
#include "parsing.hpp"

class Evaluator
{
public:
    LispValue eval(LispValue const & val)
    {
      try{
        return eval_body(val);
      }
      catch(...) {
        while(!stack.empty()) stack.pop();
        throw;
      }
    }
private:
    LispValue eval_body(LispValue const & val)
    {
      stack.push(Frame{val});
    start:
      
      switch (stack.top().val.index())
      { 
        case 4: //Symbol 
        stack.top().result = stack.top().val;  // while we don't do env
        break;
        case 6: // list_t
        stack.top().list = std::get<6>(stack.top().val);
        if(stack.top().list.empty())
        {
          stack.top().result = stack.top().val;  // we can't evaluate empty list
          break;
        }

        #define EVALUATE_ELEMENT_INTO(elem, res)       \
          {                                            \
            stack.push({elem});                        \
            if(!setjmp(stack.top().addr))  goto start; \
            auto ret = stack.top().result;             \
            stack.pop();                               \
            res = ret;                                 \
          }
        #define EVALUATE_ELEMENT(elem) EVALUATE_ELEMENT_INTO(elem, elem)
        
        #define STRICT /*evaluate all elements of list except head*/ \
          stack.top().index = stack.top().list.size() - 1;           \
          while (--stack.top().index + 1)                            \
            EVALUATE_ELEMENT(stack.top().list[stack.top().index])
            
         
        EVALUATE_ELEMENT(stack.top().list.back());  // evaluate head of list

        switch (stack.top().list.back().index()){
          case 5: // KeyWord
          {
            auto kw = std::get<KeyWord>(stack.top().list.back());
            switch(kw){
              case KeyWord::Plus:   STRICT
              break;
              case KeyWord::Minus:  STRICT
              break;
              case KeyWord::Times:  STRICT
              break;
              case KeyWord::Div:    STRICT
              break;
              case KeyWord::Mod:    STRICT
              break;
              case KeyWord::Concat: STRICT
              break;
              case KeyWord::GT:     STRICT
              break;
              case KeyWord::GE:     STRICT
              break;
              case KeyWord::LT:     STRICT
              break;
              case KeyWord::LE:     STRICT 
              break;
              case KeyWord::Eq:     STRICT 
              break;
              case KeyWord::NoEq:   STRICT 
              break;
              case KeyWord::Quote:  
              {
                auto args = stack.top().list;
                args.pop_back();
                if(args.size() > 0)
                  stack.top().result = args.back();
                else 
                  throw std::runtime_error("quote: missing an argument");
                args.clear();
              }
              break;
              case KeyWord::TypeOf: STRICT
              {
                switch((++stack.top().list.rbegin()) -> index())
                {
                  case 0: stack.top().result = "int64_t";     break;
                  case 1: stack.top().result = "double";      break;
                  case 2: stack.top().result = "bool";        break;
                  case 3: stack.top().result = "std::string"; break;
                  case 4: stack.top().result = "Symbol";      break;
                  case 5: stack.top().result = "KeyWord";     break;
                  case 6: stack.top().result = "list_t";      break;
                }
              }
              break;
              case KeyWord::Cons: STRICT
              {
                auto args = stack.top().list;
                args.pop_back();
                list_t list;
                if(args.front().index() == 6) // list
                {
                  list = std::get<list_t>(args.front());
                  list.reserve(args.size() - 1);
                  for(auto i = ++args.begin(); i != args.end(); ++i)
                  {
                    list.emplace_back(*i);
                  }
                } else
                {
                  list.reserve(args.size());
                  for(auto && i : args){
                    list.emplace_back(std::forward<LispValue>(i));
                  }
                }
                stack.top().result = list;
                args.clear();
              }
              break;
              case KeyWord::Car:    STRICT
              {
                auto args = stack.top().list;
                args.pop_back();
                if(args.back().index() == 6) // list
                {
                  auto list = std::get<list_t>(args.back());
                  if(list.size() == 0){
                    throw std::runtime_error("car: empty list");
                  }
                  stack.top().result = list.back();
                }
                else{
                  throw std::runtime_error("car: argument not a list");
                }
                args.clear();
              }
              break;
              case KeyWord::Cdr:    STRICT
              { 
                auto args = stack.top().list;
                args.pop_back();
                if(args.back().index() == 6) // list
                {
                  auto list = std::get<list_t>(args.back());
                  if(list.size() != 0)
                    list.pop_back();
                  stack.top().result = list;
                }
                else{
                  throw std::runtime_error("cdr: argument not a list");
                }
                args.clear();
              }
              break;
              case KeyWord::Cond: 
              break;
              case KeyWord::Print:  STRICT
              {
                auto args = stack.top().list;
                args.pop_back();
                for(auto i =args.rbegin(); i != args.rend(); ++i)
                {
                  if(i->index() == 3)
                    std::cout << std::get<std::string>(*i);
                  else std::cout << static_cast<std::string>(*i);
                }
                std::cout << std::flush;
                args.clear();
                stack.top().result = list_t{};
              }
              break;
              case KeyWord::Read:   STRICT
              {
                auto args = stack.top().list;
                args.pop_back();
                for(auto i =args.rbegin(); i != args.rend(); ++i)
                {
                  if(i->index() == 3)
                    std::cout << std::get<std::string>(*i);
                  else std::cout << static_cast<std::string>(*i);
                }
                std::cout << std::flush;
                args.clear();
                std::string line;
                std::getline(std::cin, line);

                stack.top().result = parse_lisp(line);
              }
              break;
              case KeyWord::Eval:   STRICT
              {
                auto elem = *(++stack.top().list.rbegin());
                EVALUATE_ELEMENT_INTO(elem, stack.top().result);
              }
              break;
            }
          }
          break;
          default: STRICT // evaluate all other elements
          stack.top().result = stack.top().list.front();
          break;
        }
        break;
        default: // int64_t, double, bool, std::string, KeyWord
        stack.top().result = stack.top().val; 
        break;
      }


      if(stack.size() == 1)
      {
        auto ret = stack.top().result;
        stack.pop();
        return ret;
      }
      else  std::longjmp(stack.top().addr, 1);
    }

  class Frame
  {
    public:
    LispValue const & val;
    LispValue result;
    std::jmp_buf addr;
    list_t list;
    size_t index;
  };

  std::stack<Frame> stack;
};






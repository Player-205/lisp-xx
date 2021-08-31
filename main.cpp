
#include <iostream>

#include <type_traits>

#include "AST.hpp"
#include "parsing.hpp"
#include "evaluation.hpp"


template<class... Ts>
value_t node(Ts&&... args) {
  return {std::forward<Ts>(args)...};
}


int main()
{
    std::string code{""};
    auto brackets = 0;
    std::string line;
    Evaluator evaluator;
    while(true){
      printf("[%d]>> ", brackets);
      std::getline(std::cin,line);
      for(const char ch : line){
        if(ch == '(') ++brackets;
        if(ch == ')') --brackets;
      }
      code +=  line + '\n';
      if (brackets == 0){
        LispValue lisp = parse_lisp(code);

        std::cout << static_cast<std::string>(evaluator.eval(lisp)) << std::endl;
        code = "";
      }
      if(brackets < 0){
        printf("You have %d extra brackets in yor code", brackets);
        brackets = 0;
        code = "";
      }
    }
    std::cout << std::is_convertible_v<LispValue, value_t> << std::endl;
}


#include <iostream>
#include <string>
#include <variant>
#include <memory>
#include <utility>
#include <type_traits>

// helper type for the visitor #4
template<class... Ts>
struct visitor : Ts...
{
    using Ts::operator()...;
};
// explicit deduction guide (not needed as of C++20)
template<class... Ts> visitor(Ts...) -> visitor<Ts...>;

template<typename T, typename ...Ts>
auto match(T&& value, Ts&&... args) -> decltype(auto) 
{
    return std::visit(visitor { std::forward<Ts>(args)... } , std::forward<T>(value));
}
struct List;

using listptr_t = std::shared_ptr<List>;

enum class KeyWord 
{
  Plus,
  Minus,
  Times,
  Div
};

std::string keyword_to_string(KeyWord word)
{
  switch (word) {
    case KeyWord::Plus:  return "+"; break;
    case KeyWord::Minus: return "-"; break;
    case KeyWord::Times: return "*"; break;
    case KeyWord::Div:   return "/"; break;
  }
}

struct Symbol { std::string value; };

using value_t = std::variant<int, double, bool, std::string, Symbol, KeyWord, listptr_t>;

std::string list_to_string(listptr_t list);


std::string to_string_with_escape_chars(std::string str)
{
  std::string result;
  result.reserve(str.length() + 2);
  result += "\"";
  for (char ch : str) 
  {
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


struct LispValue
{

    LispValue(value_t const & value) : value{std::move(value)} {}
    LispValue(value_t&& value) : value{std::move(value)} {}

    operator std::string()
    {
        return match(value, 
            [](listptr_t x)           { return list_to_string(x);                 },
            [](Symbol x)              { return x.value;                           },
            [](KeyWord x)             { return keyword_to_string(x);              },
            [](std::string const & x) { return to_string_with_escape_chars(x);    },
            [](bool x)                { return std::string{x ? "true" : "false"}; },
            [](auto x)                { return std::to_string(x);                 }
        );
    }
    
    value_t value;
};

struct List
{
    LispValue car;
    listptr_t cdr;
};

std::string list_to_string(listptr_t list)
{
  std::string result;
  result.reserve(500);
  result += "(";
  while(true){
    result += static_cast<std::string>(list->car);
    if((list = list->cdr)) result += " ";
    else break;
  }
  return result += ")";
}





listptr_t node(value_t value, listptr_t tail){
  return std::make_shared<List>(LispValue{value}, tail);
}

int main()
{
    LispValue l{
      node(
        node("qwe\n\rt\a\by"
        , nullptr
        )
        , node(Symbol{"symbol"}
        , node(node(node(node(node(4,nullptr),nullptr),nullptr),nullptr)
        , node(4.3
        , node(15
        , nullptr))))
      )
    };
    
    std::cout << static_cast<std::string>(l) << '\n';
}

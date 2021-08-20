
#include <iostream>
#include <string>
#include <variant>
#include <memory>
#include <utility>
#include <type_traits>
#include <stack>
#include <vector>
#include <optional>
#include <functional>
#include <algorithm>
#include <cctype>

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
          case KeyWord::Plus:  return "+";
          case KeyWord::Minus: return "-";
          case KeyWord::Times: return "*";
          case KeyWord::Div:   return "/";
    }
    throw "unreachable";
}
std::optional<KeyWord> is_keyword(std::string const & str)
{
  if(str == "+") return KeyWord::Plus;
  if(str == "-") return KeyWord::Minus;
  if(str == "*") return KeyWord::Times;
  if(str == "/") return KeyWord::Div;
  return std::nullopt;
}

struct Symbol { std::string value; };

struct LispValue;

using list_t = std::shared_ptr<std::vector<LispValue>>;

using value_t = std::variant<int64_t, double, bool, std::string, Symbol, KeyWord, list_t>;

std::string list_to_string(list_t list);


std::string to_string_with_escape_chars(std::string const &str)
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


struct LispValue
{

    LispValue(value_t const & value) : value{std::move(value)} {}
    LispValue(value_t&& value) : value{std::move(value)} {}

    operator std::string()
    {
        return match(value, 
            [](list_t x)                { return list_to_string(x);                 },
            [](Symbol x)                { return x.value;                           },
            [](KeyWord x)               { return keyword_to_string(x);              },
            [](std::string const & x)   { return to_string_with_escape_chars(x);    },
            [](bool x)                  { return std::string{x ? "true" : "false"}; },
            [](auto x)                  { return std::to_string(x);                 }
        );
    }
    
    value_t value;
};

std::string list_to_string(list_t list)
{
    if (list->size() == 0) return "( )";
    std::string result;
    result.reserve(500);
    result += "(";
    for(size_t i = 0; i < list->size() - 1; ++i) result += static_cast<std::string>((*list)[i]) += " ";
    result += static_cast<std::string>((*list)[list->size() - 1]) += ")";
    return result;
}



std::optional<int64_t> is_int(std::string const & str)
{
  if(std::all_of(str.begin(), str.end(),
  [](const char ch) {return std::isdigit(static_cast<unsigned char>(ch));})){
    return std::stoll(str);
  }
  return std::nullopt;
}


std::optional<double> is_float(std::string const & str)
{
  bool was_dot = false;
  for(const char c : str){
    if(!was_dot && c == '.'){
      was_dot = true;
      continue;
    }
    if(!std::isdigit(static_cast<unsigned char>(c))){
      return std::nullopt;
    }
  }
  return std::stod(str);
}

LispValue parse_string(std::string const & input)
{

  std::optional<value_t> obj;
  std::stack<list_t> lists_stack;
  for(size_t i = 0; i < input.length(); ++i)
  {
    auto action = obj
      ? std::function{[&lists_stack](value_t val){
          if(!lists_stack.empty())
              lists_stack.top()->emplace_back(val);
          else throw std::runtime_error("invalid syntax");
        }}
      : std::function{[&obj](value_t val){ obj = val;}};
    switch(input[i])
    {
      case ' ':
      case '\n':
      case '\r':
      continue;
      case '(':
      {
        
        value_t list {std::make_shared<std::vector<LispValue>>()};
        action(list);
        lists_stack.push(std::get<list_t>(list));
        break;
      }
      case ')': 
      {
        lists_stack.pop();
        break;
      }

      case '"':
      {
        std::string str;
        size_t j = i + 1;
        for(;j < input.length() && input[j] != '"'; ++j)
          if(input[j] == '\\')
          {
            switch(input[j + 1]){
              case '0': str += '\0'; break;  case 'a' : str += '\a'; break;
              case 'b': str += '\b'; break;  case 'f' : str += '\f'; break;
              case 'n': str += '\n'; break;  case 'r' : str += '\r'; break;
              case 't': str += '\t'; break;  case 'v' : str += '\v'; break;
              case '"': str += '\"'; break;  case '\\': str += '\\'; break;
            }
            ++j;
          }
          else str += input[j];
        i = j;
        action(str);
        break;
      }

      default:
      {
        std::string str;
        for(;i < input.length() 
          && input[i] != '(' 
          && input[i] != ')' 
          && input[i] != ' ' 
          && input[i] != '\r' 
          && input[i] != '\n' 
          && input[i] != '"' ; ++i)
        {
          str +=input[i];
        } 
        --i;
        if(str.empty()) break;
        if(auto keyword = is_keyword(str))
        {
          action(keyword.value());
          break;
        }
        if(auto integer = is_int(str))
        {
          action(integer.value());
          break;
        }
        if(auto floating = is_float(str)){
          action(floating.value());
          break;
        }
        if(str == "true"){
          action(true);
          break;
        }
        if(str == "false"){
          action(false);
          break;
        }
        action(Symbol{str});
        break;
      }
    }
  }
  if(!obj) throw std::runtime_error("kek");
  return  obj.value();
}

template<typename ...Arg>
list_t list(Arg && ... args )
{
  return { LispValue{std::forward<Arg>(args)}...};
}

int main()
{
    std::string str{"(10 2 4 (5 symbol true) \"string\" 5.6)"};
    std::cout << str << '\n';
    LispValue l = parse_string(str);
    std::cout << static_cast<std::string>(l) << '\n';
}

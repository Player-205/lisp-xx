
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
#include <numeric>

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
bool is_keyword(std::string const & str, auto && action)
{
  if(str == "true")  { action(true);           return true; }
  if(str == "false") { action(false);          return true; }
  if(str == "+")     { action(KeyWord::Plus);  return true; }
  if(str == "-")     { action(KeyWord::Minus); return true; }
  if(str == "*")     { action(KeyWord::Times); return true; }
  if(str == "/")     { action(KeyWord::Div);   return true; }
  return false;
}

struct Symbol { std::string value; };

struct LispValue;

using list_t = std::vector<LispValue>;
using listptr_t = std::shared_ptr<list_t>;

using value_t = std::variant<int64_t, double, bool, std::string, Symbol, KeyWord, listptr_t>;

std::string list_to_string(listptr_t list);


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

std::string list_to_string(listptr_t list)
{
    if (!list) return "";
    std::string result;
    result.reserve(500);
    result += "(";
    if(list->size())
      result += std::accumulate(++list->begin(),
          list->end(),
          std::string(list->front()),
          [](auto acc, auto val){
              return acc + " " + std::string(val);
          }
      );
    result += ")";
    return result;
}





bool is_number(std::string const & str, auto && action) {
  bool was_dot = false;

  for(const char c : str) {
    if(!was_dot && c == '.') {
      was_dot = true;
      continue;
    }
    if(!std::isdigit(static_cast<unsigned char>(c))) {
      return false;
    }
  }
  if(was_dot) action(std::stod(str));
  else action(std::stoll(str));
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

LispValue parse_lisp(std::string const & input)
{

  std::optional<value_t> obj;
  std::stack<listptr_t> lists_stack;
  size_t 
     line = 0
   , pos = 1
   , i = 0;
  auto next = [&i, &pos](){ ++i; ++pos; };
  for(; i < input.length(); next())
  {
    
    auto action = [&](auto && val) {
      if(obj) {
        if(lists_stack.empty()) throw std::runtime_error("invalid syntax");
        lists_stack.top()->emplace_back(val);
      } else {
        obj = val;
      }
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
        
        value_t list {std::make_shared<list_t>()};
        action(list);
        lists_stack.push(std::get<listptr_t>(list));
        break;
      }
      case ')': 
      {
        lists_stack.pop();
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
          && input[j] != '\r' 
          && input[j] != '\n' 
          && input[j] != '"' ; ++j, next())
        {
          str +=input[i];
        }
        str +=input[i];
        if(str.empty()) break;
        if(is_keyword(str, action))break; // bool is also keyword
        if(is_number(str, action)) break;
        action(Symbol{str});
      }
    }
  }
  if(!obj) throw std::runtime_error("Parse error");
  return  obj.value();
}


template<class... Ts>
value_t node(Ts&&... args) {
  list_t a;
  a.reserve(sizeof...(args));
  (a.push_back(args),...);
  return std::make_shared<list_t>(a);
}


int main()
{
    std::string str{"(10 2 4 (5 symbol true) \"string\" 5.6)"};
    std::cout << str << '\n';
    LispValue l = parse_lisp(str);
    std::cout << static_cast<std::string>(l) << '\n' << std::flush;
}

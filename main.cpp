
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

std::string list_to_string(listptr_t list);

struct LispValue
{
    using value_t = std::variant<int, float, bool, std::string, listptr_t>;

    LispValue(value_t const & value) : value{std::move(value)} {}
    LispValue(value_t&& value) : value{std::move(value)} {}

    operator std::string()
    {
        return match(value, 
            [](listptr_t x) { return list_to_string(x); },
            [](std::string const & x) { return x; },
            [](bool x) { return std::string{x ? "true" : "false"}; },
            [](auto x) { return std::to_string(x); }
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
  result += "( ";
  do{
    result += static_cast<std::string>(list->car);
    result += " ";
  }
  while((list = list->cdr));
  return result += ")";
}

int main()
{
    listptr_t l
    {
      new List
      {
        .car{listptr_t{new List
      {
        .car{"symbol"}, 
        .cdr{}
      }}}, 
        .cdr{listptr_t{new List
      {
        .car{"symbol"}, 
        .cdr{listptr_t{new List
      {
        .car{true}, 
        .cdr{}
      }}}
      }}}
      }
    };
    
    std::cout << static_cast<std::string>(LispValue{l}) << '\n';
}
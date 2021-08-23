#pragma once

#include <variant>
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
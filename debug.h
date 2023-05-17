/***************************************************************************
 * File: debug.h
 * Author: 朱昱綸 (B11115022@mail.ntust.edu.tw)
 *         張庭嘉 (B11115027@mail.ntust.edu.tw)
 *         顏毓德 (B11115038@mail.ntust.edu.tw)
 *         黃雋祐 (B11115048@mail.ntust.edu.tw)
 * Create Date: May 4, 2023
 * Update Date: May 11, 2023
 * Description: Functions and macros used for debugging. (C++20)
***************************************************************************/
// Include guard
#pragma once

// Necessary headers
#include <iostream>
#include <source_location> // C++20
#include <tuple>

// Concepts to check if a type is printable, iterable, tuple-like, or struct-like
template<class T> concept printable = requires(const T &t) { std::cout << t; };
template<class T> concept iterable = !printable<T> && requires(const T &t) { t.begin(); t.end(); };
template<class T> concept tuplelike = !printable<T> && requires(const T &t) { std::tuple_size<T>::value; };
template<class T> concept structlike = !printable<T> && !iterable<T> && !tuplelike<T> && std::is_aggregate_v<T>;

// Structs used for type erasure
struct Any { template<class T> operator T() {} };
struct Ubiq { template <class T> operator T(); };
template <size_t> using ubiq_t = Ubiq;

// Intent: Returns the arity (number of members) of a struct-like type.
// Pre: None
// Post: The number of members in the struct-like type.
template<structlike T> constexpr size_t arity(const T &t, auto ...count) {
  if constexpr (requires { T{ count... }; })
    return arity(t, count..., Any{});
  return sizeof...(count) - 1;
}

// Intent: Counts the number of Ubiqs in a T object using sizeof...
// Pre: None
// Post: sz contains the number of Ubiqs in the T object.
template <typename T, typename... Ubiqs>
constexpr auto count_r(size_t& sz, int) -> std::void_t<decltype(T{Ubiqs{}...})> {
  sz = sizeof...(Ubiqs);
}

// Intent: Calls count_r with Ubiqs expanded in pack form
// Pre: None
// Post: None
template <typename T, typename, typename... Ubiqs> constexpr auto count_r(size_t& sz, float) {
  count_r<T, Ubiqs...>(sz, 0);
}

// Intent: Counts the number of members in a T object by calling count_r
// Pre: None
// Post: The number of members in the T object.
template<typename T, size_t... Is> constexpr auto count(std::index_sequence<Is...>) {
  size_t sz;
  count_r<T, ubiq_t<Is>...>(sz, 0);
  return sz;
}

// Intent: Returns the number of members in a struct-like type.
// Pre: None
// Post: The number of members in the struct-like type.
template<typename T> constexpr auto arity() {
  return count<T>(std::make_index_sequence<sizeof(T)>{});
}

// Forward declaration of the variadic write() function
template<class T, class... More> constexpr void write(const T &t, const More &...more);

// Intent: Print struct-like types with 0 members.
// Pre: None
// Post: None
template<structlike T, std::enable_if_t<arity<T>() == 0, bool> = true>
constexpr void printstruct(const T &t) {} 

// Intent: Print struct-like types with 1 member.
// Pre: None
// Post: None
template<structlike T, std::enable_if_t<arity<T>() == 1, bool> = true>
constexpr void printstruct(const T &t) {
  const auto &[a] = t;
  write(a);
}

// Intent: Print struct-like types with 2 members.
// Pre: None
// Post: None
template<structlike T, std::enable_if_t<arity<T>() == 2, bool> = true>
constexpr void printstruct(const T &t) {
  const auto &[a, b] = t;
  write(a, ',', b);
}

// Intent: Print struct-like types with 3 members.
// Pre: None
// Post: None
template<structlike T, std::enable_if_t<arity<T>() == 3, bool> = true>
constexpr void printstruct(const T &t) {
  const auto &[a, b, c] = t;
  write(a, ',', b, ',', c);
}

// Intent: Print struct-like types with 4 members.
// Pre: None
// Post: None
template<structlike T, std::enable_if_t<arity<T>() == 4, bool> = true>
constexpr void printstruct(const T &t) {
  const auto &[a, b, c, d] = t;
  write(a, ',', b, ',', c, ',', d);
}

// Intent: Print struct-like types with 5 members.
// Pre: None
// Post: None
template<structlike T, std::enable_if_t<arity<T>() == 5, bool> = true>
constexpr void printstruct(const T &t) {
  const auto &[a, b, c, d, e] = t;
  write(a, ',', b, ',', c, ',', d, ',', e);
}

// Intent: Print struct-like types with 6 members.
// Pre: None
// Post: None
template<structlike T, std::enable_if_t<arity<T>() == 6, bool> = true>
constexpr void printstruct(const T &t) {
  const auto &[a, b, c, d, e, f] = t;
  write(a, ',', b, ',', c, ',', d, ',', e, ',', f);
}

// Intent: Print struct-like types with 7 members.
// Pre: None
// Post: None
template<structlike T, std::enable_if_t<arity<T>() == 7, bool> = true>
constexpr void printstruct(const T &t) {
  const auto &[a, b, c, d, e, f, g] = t;
  write(a, ',', b, ',', c, ',', d, ',', e, ',', f, ',', g);
}

// Intent: Print struct-like types with 8 members.
// Pre: None
// Post: None
template<structlike T, std::enable_if_t<arity<T>() == 8, bool> = true>
constexpr void printstruct(const T &t) {
  const auto &[a, b, c, d, e, f, g, h] = t;
  write(a, ',', b, ',', c, ',', d, ',', e, ',', f, ',', g, ',', h);
}

// Intent: Print struct-like types with 9 members.
// Pre: None
// Post: None
template<structlike T, std::enable_if_t<arity<T>() == 9, bool> = true>
constexpr void printstruct(const T &t) {
  const auto &[a, b, c, d, e, f, g, h, i] = t;
  write(a, ',', b, ',', c, ',', d, ',', e, ',', f, ',', g, ',', h, ',', i);
}

// Intent: Print struct-like types with 10 members.
// Pre: None
// Post: None
template<structlike T, std::enable_if_t<arity<T>() == 10, bool> = true>
constexpr void printstruct(const T &t) {
  const auto &[a, b, c, d, e, f, g, h, i, j] = t;
  write(a, ',', b, ',', c, ',', d, ',', e, ',', f, ',', g, ',', h, ',', i, ',', j);
}

// Intent: Print struct-like types with 11 members.
// Pre: None
// Post: None
template<structlike T, std::enable_if_t<arity<T>() == 11, bool> = true>
constexpr void printstruct(const T &t) {
  const auto &[a, b, c, d, e, f, g, h, i, j, k] = t;
  write(a, ',', b, ',', c, ',', d, ',', e, ',', f, ',', g, ',', h, ',', i, ',', j, ',', k);
}

// Intent: Print struct-like types with 12 members.
// Pre: None
// Post: None
template<structlike T, std::enable_if_t<arity<T>() == 12, bool> = true>
constexpr void printstruct(const T &t) {
  const auto &[a, b, c, d, e, f, g, h, i, j, k, l] = t;
  write(a, ',', b, ',', c, ',', d, ',', e, ',', f, ',', g, ',', h, ',', i, ',', j, ',', k, ',', l);
}

// Intent: Print struct-like types with 13 members.
// Pre: None
// Post: None
template<structlike T, std::enable_if_t<arity<T>() == 13, bool> = true>
constexpr void printstruct(const T &t) {
  const auto &[a, b, c, d, e, f, g, h, i, j, k, l, m] = t;
  write(a, ',', b, ',', c, ',', d, ',', e, ',', f, ',', g, ',', h, ',', i, ',', j, ',', k, ',', l, ',', m);
}

// Intent: Print struct-like types with 14 members.
// Pre: None
// Post: None
template<structlike T, std::enable_if_t<arity<T>() == 14, bool> = true>
constexpr void printstruct(const T &t) {
  const auto &[a, b, c, d, e, f, g, h, i, j, k, l, m, n] = t;
  write(a, ',', b, ',', c, ',', d, ',', e, ',', f, ',', g, ',', h, ',', i, ',', j, ',', k, ',', l, ',', m, ',', n);
}

// Intent: Print struct-like types with 15 members.
// Pre: None
// Post: None
template<structlike T, std::enable_if_t<arity<T>() == 15, bool> = true>
constexpr void printstruct(const T &t) {
  const auto &[a, b, c, d, e, f, g, h, i, j, k, l, m, n, o] = t;
  write(a, ',', b, ',', c, ',', d, ',', e, ',', f, ',', g, ',', h, ',', i, ',', j, ',', k, ',', l, ',', m, ',', n, ',', o);
}

// Intent: Print struct-like types with 16 members.
// Pre: None
// Post: None
template<structlike T, std::enable_if_t<arity<T>() == 16, bool> = true>
constexpr void printstruct(const T &t) {
  const auto &[a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p] = t;
  write(a, ',', b, ',', c, ',', d, ',', e, ',', f, ',', g, ',', h, ',', i, ',', j, ',', k, ',', l, ',', m, ',', n, ',', o, ',', p);
}

// Intent: Print struct-like types with 17 members.
// Pre: None
// Post: None
template<structlike T, std::enable_if_t<arity<T>() == 17, bool> = true>
constexpr void printstruct(const T &t) {
  const auto &[a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q] = t;
  write(a, ',', b, ',', c, ',', d, ',', e, ',', f, ',', g, ',', h, ',', i, ',', j, ',', k, ',', l, ',', m, ',', n, ',', o, ',', p, ',', q);
}

// Intent: Print struct-like types with 18 members.
// Pre: None
// Post: None
template<structlike T, std::enable_if_t<arity<T>() == 18, bool> = true>
constexpr void printstruct(const T &t) {
  const auto &[a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r] = t;
  write(a, ',', b, ',', c, ',', d, ',', e, ',', f, ',', g, ',', h, ',', i, ',', j, ',', k, ',', l, ',', m, ',', n, ',', o, ',', p, ',', q, ',', r);
}

// Intent: Print struct-like types with 19 members.
// Pre: None
// Post: None
template<structlike T, std::enable_if_t<arity<T>() == 19, bool> = true>
constexpr void printstruct(const T &t) {
  const auto &[a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s] = t;
  write(a, ',', b, ',', c, ',', d, ',', e, ',', f, ',', g, ',', h, ',', i, ',', j, ',', k, ',', l, ',', m, ',', n, ',', o, ',', p, ',', q, ',', r, ',', s);
}

// Intent: Print struct-like types with 20 members.
// Pre: None
// Post: None
template<structlike T, std::enable_if_t<arity<T>() == 20, bool> = true>
constexpr void printstruct(const T &obj) {
  const auto &[a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t] = obj;
  write(a, ',', b, ',', c, ',', d, ',', e, ',', f, ',', g, ',', h, ',', i, ',', j, ',', k, ',', l, ',', m, ',', n, ',', o, ',', p, ',', q, ',', r, ',', s, ',', t);
}

// If you are using clang and want to see even more details about a struct,
// just use __builtin_dump_struct() instead of this one.

// Intent: Write a given set of arguments to std::cout.
// Pre: 1. Arguments can be of almost all types, but only "printable", "iterable",
//         "tuple-like" and "struct-like" types will be printed normally.
//      2. The struct printing feature doesn't currently work in MSVC.
//         More specifically, the arity counting magic does not work there.
// Post: Print each of the arguments based on their properties
//         "printable"   : printed by calling operator<<()
//         "iterable"    : printed as values enclosed in square brackets [...]
//         "tuple-like"  : printed as values enclosed in parentheses (...)
//         "struct-like" : printed as values enclosed in curly brackets {...}
//       For any other data types, print "<unprintable type: {typename}>"
template<class T, class... More> constexpr void write(const T &t, const More &...more) {
  if constexpr (printable<T>) {
    std::cout << t;
  } else if constexpr (iterable<T>) {
    std::cout << '[';
    for (auto it = t.begin(); it != t.end(); ++it) {
      write(*it);
      if (std::next(it) != t.end())
        std::cout << ',';
    }
    std::cout << ']';
  } else if constexpr (tuplelike<T>) {
    std::apply(
      [](const auto &...args) {
        std::cout << '(';
        std::size_t n = 0;
        ((write(args), std::cout << (++n != sizeof...(args) ? "," : "")), ...);
        std::cout << ')';
      }, t
    );
  } else if constexpr (structlike<T>) {
    std::cout << '{';
    printstruct(t);
    std::cout << '}';
  } else {
    std::cout << "<unprintable type: " + type(t) + ">";
  }
  if constexpr (sizeof...(more))
    write(more...);
}

// Intent: Do nothing (overloaded to support using the writeln macro without any arguments)
// Pre: None
// Post: None
constexpr void write() {}

// Intent: Do nothing (overloaded to support using the println macro without any arguments)
// Pre: None
// Post: None
constexpr void print() {}

// Intent: Same as write() but each argument is seperated by " "
// Pre: Same as with write()
// Post: Same as with write()
template<class T, class... More> constexpr void print(const T &t, const More &...more) {
  write(t);
  if constexpr (sizeof...(more)) {
    std::cout << " ";
    print(more...);
  }
}

// Intent: Get the unmangled type name of any object as std::string, unlike typeid().name()
// Pre: Only clang(clang++), gcc(g++) and msvc are currently supported.
// Post: None
template<class T> constexpr std::string type(const T& obj) {
#if defined(__clang__)
  std::string res = std::source_location::current().function_name();
  res.erase(0, 33);
  res.erase(res.end() - 1);
#elif defined(__GNUC__)
  std::string res = std::source_location::current().function_name();
  res.erase(0, 47);
  res.erase(res.find(';'));
#elif defined(_MSC_VER)
  std::string res = __FUNCSIG__;
  res.erase(0, 101);
  res.erase(res.find('(') - 1);
#else
  #error unsupported compiler sorry
#endif
  return res;
}

// These macros will print a new line "\r\n" after printing the results

// whatis(expr) prints the typename of expr, the expression and the result
// for example:
//   whatis(vector{1, 2}); ==> "std::vector<int> vector{1, 2} = [1,2]\r\n"
//   whatis(var); ==> "{type(var)} var = {contents of variable var}\r\n"
#define whatis(...) do { std::cout << type(__VA_ARGS__) << " "#__VA_ARGS__" = "; write(__VA_ARGS__); std::cout << "\r\n"; } while (0)

// where() prints the source location of its call site
// for example: "main.cpp(404:9) int main(): \r\n"
#define where_(loc) do { std::cout << loc.file_name() << "(" << loc.line() << ":" << loc.column() << ") " << loc.function_name() << ": \r\n"; } while (0)
#define where() where_(std::source_location::current())

// writeln(...) calls write(...) and prints "\r\n"
// println(...) calls print(...) and prints "\r\n"
#define writeln(...) do { write(__VA_ARGS__); std::cout << "\r\n"; } while (0)
#define println(...) do { print(__VA_ARGS__); std::cout << "\r\n"; } while (0)


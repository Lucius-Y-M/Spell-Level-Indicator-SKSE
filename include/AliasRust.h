#pragma once

#include "../src/PCH.h"

#include <utility>			// std::pair
#include <string>			// std::string, std::string_view
#include <vector>
#include <type_traits>
#include <variant>			// std::variant
#include <unordered_map>
#include <unordered_set>



/* type aliases based on Rust names */

using u8 = uint8_t;
using i8 = int8_t;
using u16 = uint16_t;
using i16 = int16_t;
using u32 = uint32_t;
using i32 = int32_t;
using usize = size_t;

using f32 = float;
using f64 = double;


using CString = const char *;
using String = std::string;
using Str = std::string_view;

// template <typename T>
// using Join = std::view::join<T>;

template<typename T, usize N>
using Array = std::array<T, N>;

template<typename T>
using Vec = std::vector<T>;

template<typename K, typename V>
using HashMap = std::unordered_map<K, V>;

template<typename T>
using HashSet = std::unordered_set<T>;


template<typename T>
using Option = std::optional<T>;
using NONE = std::nullopt_t;
static constexpr inline auto None = std::nullopt;



template<typename Ok, typename Err>
using Result = std::variant<Ok, Err>;
template<typename K>
using Ok = Result<K, Str>;



template <typename A, typename B>
using Pair = std::pair<A, B>;

template <typename A, typename B, typename C>
using Tup3 = std::tuple<A, B, C>;
template <typename A, typename B, typename C, typename D>
using Tup4 = std::tuple<A, B, C, D>;

template<typename ... R>
using Tuple = std::tuple<R...>;




/*
	C++ enums cannot have their own data types
	so have to use this
*/
template<typename ... T>
using RustEnum = std::variant<T...>;


// template<typename T, typename R>
// using IsSameV = std::is_same_v<T, R>;
template<typename T, typename R>
using IsSame = std::is_same<T, R>;


/*
	this is an imitation of Rust's MATCH statement for enums

	but this cannot short-circuit and early return
	due to the scope of the lambda function "capturing" any return statement
*/

template <typename... Ts>
struct Overloaded : Ts... {
    using Ts::operator()...;
};
template<class... Ts> Overloaded(Ts...) -> Overloaded<Ts...>;


template<typename Item, typename... Visitor>
static void Match(Item && value, Visitor && ... visitors) {
	std::visit(Overloaded(visitors), value);
}
template<typename Item, typename... Visitor>
static void Visit(Item && value, Visitor && ... visitors) {
	std::visit(Overloaded(visitors), value);
}



/* allow compile time check of str arrays */
template<usize N>
static constexpr bool AllOfArrayNotEmpty(Array<Str, N> array) {
	for (auto str : array) {
		if (str.empty()) { return false; }
	}
	return true;
}





/*
	A convenient function for making sure something is contained in an std::array

	(special impls for CString and Str)
*/

template<usize N>
[[maybe_unused]]
static bool ArrayContains(Array<Str, N> array, Str str) {
	for (const auto & item: array) {

		/* similar to why C Str must use "strcmp" */
		if (str.compare(item) == 0) {
			return true;
		}
	}
	return false;
}

template<usize N>
[[maybe_unused]]
static bool ArrayContains(Array<CString, N> array, CString str) {
	for (const auto & item: array) {

		/* similar to why C Str must use "strcmp" */
		if (strcmp(item, str) == 0) {
			return true;
		}
	}
	return false;
}

template<typename T, usize N>
[[maybe_unused]]
static bool ArrayContains(Array<T, N> array, T query) {
	for (const auto & item: array) {

		/* similar to why C Str must use "strcmp" */
		if (item == query) {
			return true;
		}
	}
	return false;
}






/* this is an impl of Rust's std::array::from_fn */

template<typename T, usize N>
[[maybe_unused]]
static Array<T, N> from_fn(T lambda(usize)) {
	Array<T, N> array;
	for (usize i = 0; i < N; i++) {
		array[i] = lambda(i);
	}
	return array;
}
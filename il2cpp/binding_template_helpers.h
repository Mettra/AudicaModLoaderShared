#pragma once

class MethodInvocationContext;

template<bool B>
struct ThisCallSpecializeTypes {
	template<typename Ret, typename... Args>
	using Fn = std::function<Ret(const MethodInvocationContext& ctx, ThisPtr ths, Args...)>;
};

template<>
struct ThisCallSpecializeTypes<false> {

	template<typename Ret, typename... Args>
	using Fn = std::function<Ret(const MethodInvocationContext& ctx, Args...)>;
};


template<typename T>
struct ReturnTypeSpecialization {
	using type = T;
};

template<typename T>
struct ReturnTypeSpecialization<std::optional<T>> {
	using type = T;
};


template<typename T>
struct is_valid_function_type {
	static const bool value = false;
};

template<typename T>
struct is_optional {
	static const bool value = false;
};

template<typename T>
struct is_optional<std::optional<T>> {
	static const bool value = true;
};

template<typename Ret>
struct is_valid_return_type {
	static const bool value = std::is_same_v<Ret, void> || is_optional<Ret>::value;
};

template<typename... Args>
constexpr bool checkThisPtr() {
	if constexpr(sizeof...(Args) < 2) {
		return false;
	}
	else {
		using tuple_type = std::tuple<Args...>;
		return std::is_same_v<std::tuple_element_t<1, tuple_type>, ThisPtr>;
	}
}

template<typename Ret, typename... Args>
struct is_valid_function_type<std::function<Ret(Args...)>> {
	using tuple_type = std::tuple<Args...>;
	static const bool hasContext = std::is_same_v<std::tuple_element_t<0, tuple_type>, const MethodInvocationContext&>;
	static const bool hasThisPtr = checkThisPtr<Args...>();
	static const bool hasValidReturn = is_valid_return_type<Ret>::value;
};
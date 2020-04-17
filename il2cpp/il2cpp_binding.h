#pragma once
#include <functional>
#include <optional>
#include <algorithm>
#include <memory>
#include "functional_type.h"

#include "semver.h"
#include "il2cpp_types.h"
#include "binding_template_helpers.h"

#include <cstddef>

const static semver BindingVersion = { 2, 1, 0 };
class il2cpp_context;
using u8 = unsigned char;

#define API_BREAK_OFFSET_MESSAGE(_Type, _Member) "The offset of " #_Type "::" #_Member " has changed! This will cause an API break. If this is intented, update this assert and increment the MAJOR number in the BindingVersion semver"
#define ENFORCE_TYPE_OFFSET(_Type, _Member, _Offset) static_assert(offsetof(_Type, _Member) == _Offset, API_BREAK_OFFSET_MESSAGE(_Type, _Member))

struct MethodInvocationStorage {
	template<size_t... I, typename... Args>
	void setArgs(std::index_sequence<I...>, std::tuple<Args*...> &&args) {
		(setArg(I, *std::get<I>(args)), ...);
	}

	~MethodInvocationStorage() {
		free(mReturnData);
		free(mArgs);
		free(mArgOffset);
	}

	template<typename Ret, typename... Args>
	void initialize(std::tuple<Args*...> &&args) {
		uint32_t retSize = 0;
		if constexpr (!std::is_same_v<Ret, void>) {
			mReturnData = (u8*)malloc(sizeof(Ret));
		}

		if constexpr (sizeof...(Args) > 0) {
			mArgs = (u8*)malloc((sizeof(Args) + ...));
			mNumArgs = sizeof...(Args);

			using OffsetType = std::decay_t<decltype(*mArgOffset)>;

			std::vector<OffsetType> offsets = { 0, sizeof(Args)..., };
			for (size_t i = 2; i < offsets.size(); ++i) {
				offsets[i] += offsets[i - 1];
			}
			offsets.resize(offsets.size() - 1);

			mArgOffset = (OffsetType*)malloc(offsets.size() * sizeof(OffsetType));
			std::memcpy(mArgOffset, offsets.data(), offsets.size() * sizeof(OffsetType));
		}

		setArgs(std::index_sequence_for<Args...>{}, std::move(args));
	}

	template<typename T>
	struct ReturnSpecialize {
		static const T& getReturn(const MethodInvocationStorage& st) {
			return *(T*)st.mReturnData;
		}
	};

	template<>
	struct ReturnSpecialize<void> {
		static void getReturn(const MethodInvocationStorage&) {}
	};

	template<typename T>
	auto getReturn() const {
		return ReturnSpecialize<T>::getReturn(*this);
	}

	template<typename T>
	void setReturn(T&& value) {
		*(T*)mReturnData = std::move(value);
	}

	template<typename T>
	const T& getArg(uint32_t idx) const {
		if (idx < 0 || idx >= mNumArgs) { throw std::out_of_range("Attempt to access argument that does not exist!"); }
		return *(T*)(mArgs + mArgOffset[idx]);
	}

	template<typename T>
	void setArg(uint32_t idx, T value) {
		if (idx < 0 || idx >= mNumArgs) { throw std::out_of_range("Attempt to access argument that does not exist!"); }
		*(T*)(mArgs + mArgOffset[idx]) = value;
	}

	uint8_t *mReturnData = nullptr;
	uint8_t* mArgs = nullptr;
	uint32_t *mArgOffset = nullptr;
	uint32_t mNumArgs = 0;
};
ENFORCE_TYPE_OFFSET(MethodInvocationStorage, mReturnData, 0);
ENFORCE_TYPE_OFFSET(MethodInvocationStorage, mArgs, 8);
ENFORCE_TYPE_OFFSET(MethodInvocationStorage, mArgOffset, 16);
ENFORCE_TYPE_OFFSET(MethodInvocationStorage, mNumArgs, 24);

class MethodInvocationContext {
public:
	MethodInvocationContext(const il2cpp_context &ctx, std::unique_ptr<MethodInvocationStorage> &&storage)
		: mCtx(&ctx), mStorage(std::move(storage)) {
	}

	const il2cpp_context &getGlobalContext() const {
		return *mCtx;
	}

	template<typename T>
	auto getReturn() const {
		return mStorage->getReturn<T>();
	}

	template<typename T>
	void setReturn(T&& value) {
		return mStorage->setReturn(std::move(value));
	}

	template<typename T>
	auto getArg(uint32_t idx) const {
		return mStorage->getArg<T>(idx);
	}

	template<typename T>
	void setArg(uint32_t idx, T&& value) {
		return mStorage->setArg(idx, std::move(value));
	}

	void stopExecution() const {
		mStopExecution = true;
	}

	bool didStopExecution() const {
		return mStopExecution;
	}

private:
	const il2cpp_context *mCtx;
	std::unique_ptr<MethodInvocationStorage> mStorage;
	mutable bool mStopExecution = false;

	void _enforceSize() {
		ENFORCE_TYPE_OFFSET(MethodInvocationContext, mCtx, 0);
		ENFORCE_TYPE_OFFSET(MethodInvocationContext, mStorage, 8);
		ENFORCE_TYPE_OFFSET(MethodInvocationContext, mStopExecution, 16);
	}
};

struct MethodHookNode {
	MethodHookNode *next;
	InvokeTime invokeTime = InvokeTime::Before;
	int priority = 0;
	void *data;
};
ENFORCE_TYPE_OFFSET(MethodHookNode, next, 0);
ENFORCE_TYPE_OFFSET(MethodHookNode, invokeTime, 8);
ENFORCE_TYPE_OFFSET(MethodHookNode, priority, 12);
ENFORCE_TYPE_OFFSET(MethodHookNode, data, 16);

template<bool isThisCall, typename FnRet, typename... Args>
struct MethodHook {
	using Fn = ThisCallSpecializeTypes<isThisCall>::Fn<FnRet, Args...>;
	using Ret = typename ReturnTypeSpecialization<FnRet>::type;

	struct Node {
		Fn fn;
	};
	ENFORCE_TYPE_OFFSET(Node, fn, 0);

	static MethodHookNode *getNewNode(Fn &&fn, InvokeTime invokeTime, int priority = 0) {
		Node *nodeData = new Node();
		nodeData->fn = std::move(fn);

		MethodHookNode *node = new MethodHookNode();
		node->priority = priority;
		node->invokeTime = invokeTime;
		node->data = nodeData;

		return node;
	}

private:
	template<size_t... I>
	static void _invokeNodeFunction(MethodInvocationContext &ctx, std::optional<ThisPtr> ths, Node *node, std::index_sequence<I...>) {
		if constexpr (isThisCall) {
			if constexpr (std::is_same_v<Ret, void>) {
				node->fn(ctx, *ths, ctx.getArg<Args>(I)...);
			}
			else {
				auto v = node->fn(ctx, *ths, ctx.getArg<Args>(I)...);
				if (v) {
					ctx.setReturn(v.value());
				}
			}
		}
		else {
			if constexpr (std::is_same_v<Ret, void>) {
				node->fn(ctx, ctx.getArg<Args>(I)...);
			}
			else {
				auto v = node->fn(ctx, ctx.getArg<Args>(I)...);
				if (v) {
					ctx.setReturn(v.value());
				}
			}
		}
	}

	template<size_t... I>
	static void _invokeOriginalFunction(MethodInvocationContext &ctx, void *ths, void *originalFn, std::index_sequence<I...>) {
		if constexpr (isThisCall) {
			auto fn = static_cast<Ret(*)(void*, Args...)>(originalFn);

			if constexpr (std::is_same_v<Ret, void>) {
				fn(ths, ctx.getArg<Args>(I)...);
			}
			else {
				auto ret = fn(ths, ctx.getArg<Args>(I)...);
				ctx.setReturn(std::move(ret));
			}
		}
		else {
			auto fn = static_cast<Ret(*)(Args...)>(originalFn);
			if constexpr (std::is_same_v<Ret, void>) {
				fn(ctx.getArg<Args>(I)...);
			}
			else {
				auto ret = fn(ctx.getArg<Args>(I)...);
				ctx.setReturn(std::move(ret));
			}
		}
	}

public:
	static void invokeNodeFunction(MethodInvocationContext &ctx, std::optional<ThisPtr> ths, void *nodeData) {
		Node *node = static_cast<Node *>(nodeData);
		_invokeNodeFunction(ctx, ths, node, std::index_sequence_for<Args...>{});
	}

	static void invokeOriginalFunction(MethodInvocationContext &ctx, void *ths, void *originalFn) {
		_invokeOriginalFunction(ctx, ths, originalFn, std::index_sequence_for<Args...>{});
	}
};

struct FunctionChainInvoker {
	static const il2cpp_context *&getContext() {
		static const il2cpp_context *ctx = nullptr;
		return ctx;
	}

	template<bool isThisCall, typename Ret, typename... Args>
	static __declspec(noinline) Ret invoke(std::optional<void *> ths, std::tuple<Args*...> &&argBuffer) {
		auto methodStorage = std::make_unique<MethodInvocationStorage>();
		methodStorage->initialize<Ret, Args...>(std::move(argBuffer));

		MethodInvocationContext methodCtx(*getContext(), std::move(methodStorage));

		getContext()->getBinding().InvokeFunctionChain(methodCtx, ths);

		return methodCtx.getReturn<Ret>();
	}
};

template<bool isThisCall, typename Ret, typename... Args>
__declspec(noinline) Ret __thiscall invokeMemberFunction(void *ths, Args... args) {
	auto argTuple = std::tuple<Args*...>(&args...);
	return FunctionChainInvoker::invoke<isThisCall, Ret, Args...>(ths, std::move(argTuple));
}

template<bool isThisCall, typename Ret, typename... Args>
static __declspec(noinline) Ret invokeStaticFunction(Args... args) {
	auto argTuple = std::tuple<Args*...>(&args...);
	return FunctionChainInvoker::invoke<isThisCall, Ret, Args...>(std::nullopt, std::move(argTuple));
}

class il2cpp_binding {
public:
	struct HookCall {
		void *originalFn = nullptr;
		void *invokeFn = nullptr;
		MethodHookNode *node = nullptr;
		il2cppapi::Class *klass = nullptr;

		uint64_t id;
		void *uniqueFn = nullptr;

		void(*invokeNodeFunction)(MethodInvocationContext &ctx, std::optional<ThisPtr> ths, void *node) = nullptr;
		void(*invokeOriginalFunction)(MethodInvocationContext &ctx, void *ths, void *originalFn) = nullptr;
		semver hookVersion = BindingVersion;
	};
	ENFORCE_TYPE_OFFSET(HookCall, originalFn, 0);
	ENFORCE_TYPE_OFFSET(HookCall, invokeFn, 8);
	ENFORCE_TYPE_OFFSET(HookCall, node, 16);
	ENFORCE_TYPE_OFFSET(HookCall, klass, 24);
	ENFORCE_TYPE_OFFSET(HookCall, id, 32);
	ENFORCE_TYPE_OFFSET(HookCall, uniqueFn, 40);
	ENFORCE_TYPE_OFFSET(HookCall, invokeNodeFunction, 48);
	ENFORCE_TYPE_OFFSET(HookCall, invokeOriginalFunction, 56);
	ENFORCE_TYPE_OFFSET(HookCall, hookVersion, 64);

	//Explicit 
	template<typename Ret, typename... Args>
	void bindClassFunction(const char *namespaceName, const char *className, const char *methodName, InvokeTime invokeTime, int priority, std::function<Ret(const MethodInvocationContext& ctx, ThisPtr ths, Args...)> &&callback) {
		static_assert(is_valid_return_type<Ret>::value, "Invalid function signature! Your function must either return `void`, or `std::optional<T>`");
		_bindClassFunction(namespaceName, className, methodName, invokeTime, priority, std::move(callback));
	}

	template<typename Ret, typename... Args>
	void bindStaticFunction(const char *namespaceName, const char *className, const char *methodName, InvokeTime invokeTime, int priority, std::function<Ret(const MethodInvocationContext& ctx, Args...)> &&callback) {
		static_assert(is_valid_return_type<Ret>::value, "Invalid function signature! Your function must either return `void`, or `std::optional<T>`");
		_bindStaticFunction(namespaceName, className, methodName, invokeTime, priority, std::move(callback));
	}

	//Passthrough + function signature check
	template<typename Fn>
	void bindClassFunction(const char *namespaceName, const char *className, const char *methodName, InvokeTime invokeTime, int priority, Fn &&callback) {
		functional_type_t<Fn> fn = [callback = std::move(callback)](auto&&... args)
		{
			return callback(std::forward<decltype(args)>(args)...);
		};

		using TypeCheck = is_valid_function_type<decltype(fn)>;
		static_assert(TypeCheck::hasContext, "Invalid function signature! Make sure your function starts with `const MethodInvocationContext& ctx`");
		static_assert(TypeCheck::hasThisPtr, "Invalid function signature! Make sure your function's second parameter is `ThisPtr ths`");
		static_assert(TypeCheck::hasValidReturn, "Invalid function signature! Your function must either return `void`, or `std::optional<T>`");

		_bindClassFunction(namespaceName, className, methodName, invokeTime, priority, std::move(fn));
	}

	template<typename Fn>
	void bindStaticFunction(const char *namespaceName, const char *className, const char *methodName, InvokeTime invokeTime, int priority, Fn &&callback) {
		functional_type_t<Fn> fn = [callback = std::move(callback)](auto&&... args)
		{
			return callback(std::forward<decltype(args)>(args)...);
		};

		using TypeCheck = is_valid_function_type<decltype(fn)>;
		static_assert(TypeCheck::hasContext, "Invalid function signature! Make sure your function starts with `const MethodInvocationContext& ctx`");
		static_assert(TypeCheck::hasValidReturn, "Invalid function signature! Your function must either return `void`, or `std::optional<T>`");

		_bindStaticFunction(namespaceName, className, methodName, invokeTime, priority, std::move(fn));
	}

	//Default priority binding, where priority = 0
	template<typename Fn>
	void bindClassFunction(const char *namespaceName, const char *className, const char *methodName, InvokeTime invokeTime, Fn &&fn) {
		bindClassFunction(namespaceName, className, methodName, invokeTime, 0, std::move(fn));
	}

	template<typename Fn>
	void bindStaticFunction(const char *namespaceName, const char *className, const char *methodName, InvokeTime invokeTime, Fn &&fn) {
		bindStaticFunction(namespaceName, className, methodName, invokeTime, 0, std::move(fn));
	}

	////////////
public:
	void(*InvokeFunctionChain)(MethodInvocationContext &ctx, std::optional<void *> ths);
protected:
	const il2cpp_context& (*GetIL2CPPContext)(const il2cpp_binding &bnd);
	void(*AddHookCall)(il2cpp_binding &bnd, const char *namespaceName, const char *className, const char *methodName, size_t numArgs, HookCall &&call);
	////////////


private:
	void _ensureSize() {
		ENFORCE_TYPE_OFFSET(il2cpp_binding, InvokeFunctionChain, 0);
		ENFORCE_TYPE_OFFSET(il2cpp_binding, GetIL2CPPContext, 8);
		ENFORCE_TYPE_OFFSET(il2cpp_binding, AddHookCall, 16);
	}

private:
	template<typename Ret, typename... Args>
	void _bindClassFunction(const char *namespaceName, const char *className, const char *methodName, InvokeTime invokeTime, int priority, std::function<Ret(const MethodInvocationContext& ctx, ThisPtr ths, Args...)> &&callback) {
		MethodHookNode *node = MethodHook<true, Ret, Args...>::getNewNode(std::move(callback), invokeTime, priority);
		_bindFunction<true, Ret, Args...>(namespaceName, className, methodName, node);
	}

	template<typename Ret, typename... Args>
	void _bindStaticFunction(const char *namespaceName, const char *className, const char *methodName, InvokeTime invokeTime, int priority, std::function<Ret(const MethodInvocationContext& ctx, Args...)> &&callback) {
		MethodHookNode *node = MethodHook<false, Ret, Args...>::getNewNode(std::move(callback), invokeTime, priority);
		_bindFunction<false, Ret, Args...>(namespaceName, className, methodName, node);
	}

	template<bool isThisCall, typename Ret, typename... Args>
	void _bindFunction(const char *namespaceName, const char *className, const char *methodName, MethodHookNode *node) {
		using MethodHookType = typename MethodHook<isThisCall, Ret, Args...>;
		FunctionChainInvoker::getContext() = &GetIL2CPPContext(*this);

		HookCall call;
		call.node = node;
		call.invokeNodeFunction = &MethodHookType::invokeNodeFunction;
		call.invokeOriginalFunction = &MethodHookType::invokeOriginalFunction;

		if constexpr (isThisCall) {
			auto invokeMemberFn = &invokeMemberFunction<isThisCall, typename ReturnTypeSpecialization<Ret>::type, Args...>;
			call.invokeFn = *(void **)&invokeMemberFn;
		}
		else {
			auto invokeStaticFn = &invokeStaticFunction<isThisCall, typename ReturnTypeSpecialization<Ret>::type, Args...>;
			call.invokeFn = *(void **)&invokeStaticFn;
		}

		AddHookCall(*this, namespaceName, className, methodName, sizeof...(Args), std::move(call));
	}
};

struct ModDeclaration {
	semver bindingVersion;
	const char *modName;
};
ENFORCE_TYPE_OFFSET(ModDeclaration, bindingVersion, 0);
ENFORCE_TYPE_OFFSET(ModDeclaration, modName, 16);
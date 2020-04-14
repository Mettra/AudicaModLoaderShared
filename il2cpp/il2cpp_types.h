#pragma once
#include <type_traits>
#include <unordered_map>
#include <variant>

enum class InvokeTime {
    Before,
    After
};

namespace internal {
    struct Il2CppType {};
    struct Il2CppClass {};

    struct FieldInfo {};
	struct PropertyInfo{};

    struct MethodInfo {
        void *methodPtr;
    };

    struct Il2CppImage {};
    struct Il2CppAssembly {};
    struct Il2CppDomain {};

    struct Il2CppObject {
        void *ptr;

		operator bool() {
			return ptr != nullptr;
		}
    };

	struct Il2CppString {
		void *strPtr;

		operator bool() {
			return strPtr != nullptr;
		}
	};
	using Il2CppChar = wchar_t;
};

template<typename T>
struct function_traits {
};

template<typename Ret, typename... Args>
struct function_traits<Ret(Args...)> {
	using PtrType = Ret(__thiscall *)(internal::Il2CppObject, Args...);
	static const int numArgs = sizeof...(Args);
};

class il2cpp_context;

namespace il2cppapi {
	struct Class;

	template<typename T>
	class Field {
	public:
		Field(const il2cpp_context &ctx, internal::Il2CppObject obj, std::variant<const internal::FieldInfo *, const internal::PropertyInfo *> fieldValue) : ctx(ctx), obj(obj), fieldValue(fieldValue) {}

		T get() {
			T value;
			if (obj) {
				if (std::holds_alternative< const internal::FieldInfo *>(fieldValue)) {
					ctx.getValueFromField(obj, std::get<const internal::FieldInfo *>(fieldValue), &value);
				}
				else {
					auto method = ctx.getPropertyGetter(std::get<const internal::PropertyInfo *>(fieldValue));
					if (method) {
						value = static_cast<T(*)(internal::Il2CppObject)>(method->methodPtr)(obj);
					}
				}
			}
			else {
				ctx.getValueFromStaticField(std::get<const internal::FieldInfo *>(fieldValue), &value);
			}
			return value;
		}

		void set(const T &rhs) {
			if (obj) {
				if (std::holds_alternative< const internal::FieldInfo *>(fieldValue)) {
					ctx.setValueFromField(obj, std::get<const internal::FieldInfo *>(fieldValue), &rhs);
				}
				else {
					auto method = ctx.getPropertySetter(std::get<const internal::PropertyInfo *>(fieldValue));
					if (method) {
						static_cast<void(*)(internal::Il2CppObject, const T*)>(method->methodPtr)(obj, &rhs);
					}
				}
			}
			else {
				ctx.setValueFromStaticField(std::get<const internal::FieldInfo *>(fieldValue), &rhs);
			}
		}

		operator T() {
			return get();
		}

		Field& operator=(const T &rhs) {
			set(rhs);
			return *this;
		}

		operator const internal::FieldInfo *() {
			return std::get<const internal::FieldInfo *>(fieldValue);
		}

		operator const internal::PropertyInfo *() {
			return std::get<const internal::PropertyInfo *>(fieldValue);
		}

		Class *getClass() {
			if (std::holds_alternative< const internal::FieldInfo *>(fieldValue)) {
				return ctx.getClassFromField(std::get<const internal::FieldInfo *>(fieldValue));
			}
			else {
				return nullptr;
			}
		}

	private:
		const il2cpp_context &ctx;
		internal::Il2CppObject obj;
		std::variant<const internal::FieldInfo *, const internal::PropertyInfo *> fieldValue;
	};

    struct Class {
        Class(const il2cpp_context& ctx, internal::Il2CppClass *klass) : ctx(ctx), klass(klass) {}

        template<typename Fn>
        typename function_traits<Fn>::PtrType method(const char *methodName) const {
			auto fn = mGetMethod(this, methodName, function_traits<Fn>::numArgs);
			return static_cast<typename function_traits<Fn>::PtrType>(fn);
        }

		template<typename T>
		Field<T> field(internal::Il2CppObject obj, const char *fieldName) const {
			auto internalField = ctx.getClassFieldInfo(klass, fieldName, false);
			if (internalField != nullptr) {
				return Field<T>(ctx, obj, internalField);
			}

			//It might be a property, so try that
			auto internalProperty = ctx.getClassPropertyInfo(klass, fieldName, false);
			if (internalProperty != nullptr) {
				return Field<T>(ctx, obj, internalProperty);
			}

			printf("ERROR: Cannot find field/property %s for class!", fieldName);
			const internal::FieldInfo *null = nullptr;
			return Field<T>(ctx, obj, null);
		}

		template<typename T>
		Field<T> static_field(const char *fieldName) const {
			auto internalField = ctx.getClassFieldInfo(klass, fieldName);
			return Field<T>(ctx, internal::Il2CppObject{ nullptr }, internalField);
		}

		operator internal::Il2CppClass*() {
			return klass;
		}

    protected:
        const il2cpp_context& ctx;
        internal::Il2CppClass *klass;
		const void * (*mGetMethod)(const Class *, const char *, uint32_t);
    };

    struct Object {
		Object(internal::Il2CppObject obj, Class *klass) : ptr(obj.ptr), klass(klass) {}

        void *ptr;
        Class *klass;

        template<typename Fn>
        typename function_traits<Fn>::PtrType method(const char *methodName) const {
            return klass->method<Fn>(methodName);
        }

		template<typename T>
		Field<T> field(const char *fieldName) const {
			return klass->field<T>(internal::Il2CppObject{ ptr }, fieldName);
		}

		template<typename T>
		Field<T> static_field(const char *fieldName) const {
			return klass->static_field<T>(fieldName);
		}

        operator internal::Il2CppObject() {
            return internal::Il2CppObject{ ptr };
        }
    };

	template<typename T>
	struct Array {
		Array(void *arrayStart) : arrayStart(arrayStart) {}

		void *arrayStart;
		uint32_t stride;

		T operator[](int32_t idx) {
			return *(reinterpret_cast<T *>(reinterpret_cast<uint8_t *>(arrayStart) + 0x20 + sizeof(T) * idx));
		}
	};
}

using ThisPtr = il2cppapi::Object;
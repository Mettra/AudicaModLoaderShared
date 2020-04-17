#include "il2cpp_context.h"


#include "il2cpp_binding.h"
using namespace internal;

il2cpp_binding &il2cpp_context::getBinding() const {
	return mGetBinding();
}

il2cppapi::Class* il2cpp_context::getClass(const char *namespaceName, const char *className) const {
	return mGetClass(namespaceName, className);
}

il2cppapi::Class* il2cpp_context::getClassFromField(const internal::FieldInfo* field) const {
	return mGetClassFromField(field);
}

il2cppapi::Class* il2cpp_context::getClassFromObject(internal::Il2CppObject obj) const {
	return mGetClassFromObject(obj);
}

void il2cpp_context::getValueFromField(internal::Il2CppObject obj, const internal::FieldInfo * field, void * value) const {
	il2cpp_field_get_value(obj, field, value);
}

void il2cpp_context::setValueFromField(internal::Il2CppObject obj, const internal::FieldInfo * field, const void * value) const {
	il2cpp_field_set_value(obj, field, value);
}

void il2cpp_context::getValueFromStaticField(const internal::FieldInfo *field, void * value) const {
	il2cpp_field_static_get_value(field, value);
}

void il2cpp_context::setValueFromStaticField(const internal::FieldInfo *field, const void * value) const {
	il2cpp_field_static_set_value(field, value);
}

const MethodInfo *il2cpp_context::getClassMethod(Il2CppClass* klass, const char *methodName, int argsCount) const {
	auto method = il2cpp_class_get_method_from_name(klass, methodName, argsCount);
	if (method == nullptr) {
		printf("ERROR: getClassMethod: Could not find method %s with args %d on class!\n", methodName, argsCount);
	}

	return method;
}

const FieldInfo *il2cpp_context::getClassFieldInfo(Il2CppClass* klass, const char *fieldName, bool error) const {
	auto field = il2cpp_class_get_field_from_name(klass, fieldName);
	if (error && field == nullptr) {
		printf("ERROR: getClassFieldInfo: Could not find field %s on class!\n", fieldName);
	}

	return field;
}

const internal::PropertyInfo *il2cpp_context::getClassPropertyInfo(internal::Il2CppClass* klass, const char *propName, bool error) const {
	auto prop = il2cpp_class_get_property_from_name(klass, propName);
	if (error && prop == nullptr) {
		printf("ERROR: getClassFieldInfo: Could not find property %s on class!\n", propName);
	}

	return prop;
}

const internal::MethodInfo *il2cpp_context::getPropertyGetter(const internal::PropertyInfo *propertyInfo) const {
	auto method = il2cpp_property_get_get_method(propertyInfo);
	if (method == nullptr) {
		printf("ERROR: getClassMethod: Could not find get method for property!\n");
	}

	return method;
}

const internal::MethodInfo *il2cpp_context::getPropertySetter(const internal::PropertyInfo *propertyInfo) const {
	auto method = il2cpp_property_get_set_method(propertyInfo);
	if (method == nullptr) {
		printf("ERROR: getClassMethod: Could not find set method for property!\n");
	}

	return method;
}

int32_t il2cpp_context::getStringLength(const internal::Il2CppString str) const {
	if (str.strPtr == nullptr) return 0;

	return il2cpp_string_length(str);
}

const internal::Il2CppChar* il2cpp_context::getStringChars(const internal::Il2CppString str) const {
	if (str.strPtr == nullptr) return nullptr;
	return il2cpp_string_chars(str);
}

internal::Il2CppString il2cpp_context::newString(const char *str) const {
	return il2cpp_string_new_len(str, (uint32_t)strlen(str));
}

std::wstring il2cpp_context::getCString(const internal::Il2CppString str) const {
	return std::wstring(getStringChars(str), getStringLength(str));
}

uint32_t il2cpp_context::getArrayLength(internal::Il2CppObject arr) const {
	return il2cpp_array_length(arr);
}

uint32_t il2cpp_context::getArrayByteLength(internal::Il2CppObject arr) const {
	return il2cpp_array_get_byte_length(arr);
}

uint32_t il2cpp_context::getArrayStride(internal::Il2CppObject arr) const {
	uint32_t elements = getArrayLength(arr);
	if (elements == 0) {
		return 0;
	}

	return getArrayByteLength(arr) / elements;
}
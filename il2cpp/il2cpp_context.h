#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <memory>
#include <string>

#include "il2cpp_types.h"

class il2cpp_binding;

//Main il2cpp class, holds all functions to interact with the C il2cpp api
//Also owns any auxilary il2cpp_ classes, like binding
class il2cpp_context {
public:
	il2cpp_binding &getBinding() const;

	il2cppapi::Class* getClass(const char *namespaceName, const char *className) const;
	il2cppapi::Class* getClassFromField(const internal::FieldInfo* field) const;

	const internal::MethodInfo *getClassMethod(internal::Il2CppClass* klass, const char *methodName, int argsCount) const;
	const internal::FieldInfo *getClassFieldInfo(internal::Il2CppClass* klass, const char *fieldName, bool error = true) const;
	const internal::PropertyInfo *getClassPropertyInfo(internal::Il2CppClass* klass, const char *propName, bool error = true) const;

	void getValueFromField(internal::Il2CppObject obj, const internal::FieldInfo* field, void *value) const;
	void setValueFromField(internal::Il2CppObject obj, const internal::FieldInfo* field, const void *value) const;

	const internal::MethodInfo *getPropertyGetter(const internal::PropertyInfo *propertyInfo) const;
	const internal::MethodInfo *getPropertySetter(const internal::PropertyInfo *propertyInfo) const;

	void getValueFromStaticField(const internal::FieldInfo* field, void *value) const;
	void setValueFromStaticField(const internal::FieldInfo* field, const void *value) const;

	int32_t getStringLength(const internal::Il2CppString str) const;
	const internal::Il2CppChar* getStringChars(const internal::Il2CppString str) const;
	internal::Il2CppString newString(const char *str) const;

	uint32_t getArrayLength(internal::Il2CppObject arr) const;
	uint32_t getArrayByteLength(internal::Il2CppObject arr) const;
	uint32_t getArrayStride(internal::Il2CppObject arr) const;

protected:
	internal::FieldInfo* (*il2cpp_class_get_field_from_name)(internal::Il2CppClass* klass, const char* name);
	void(*il2cpp_field_get_value)(internal::Il2CppObject obj, const internal::FieldInfo* field, void* value);
	void(*il2cpp_field_set_value)(internal::Il2CppObject obj, const internal::FieldInfo* field, const void *value);
	void(*il2cpp_field_static_get_value)(const internal::FieldInfo * field, void *value);
	void(*il2cpp_field_static_set_value)(const internal::FieldInfo * field, const void *value);
	const internal::MethodInfo* (*il2cpp_class_get_method_from_name)(internal::Il2CppClass* klass, const char* name, int argsCount);
	internal::Il2CppClass* (*il2cpp_class_from_name)(const internal::Il2CppImage* image, const char* namespaze, const char* name);
	const internal::Il2CppAssembly** (*il2cpp_domain_get_assemblies)(const internal::Il2CppDomain* domain, size_t* size);
	internal::Il2CppDomain* (*il2cpp_domain_get)();
	const internal::Il2CppImage* (*il2cpp_assembly_get_image)(const internal::Il2CppAssembly* assembly);
	const internal::Il2CppType* (*il2cpp_field_get_type)(const internal::FieldInfo * field);
	internal::Il2CppClass* (*il2cpp_class_from_type)(const internal::Il2CppType * type);
	const char* (*il2cpp_type_get_name)(const internal::Il2CppType * type);
	const internal::PropertyInfo* (*il2cpp_class_get_property_from_name)(internal::Il2CppClass * klass, const char *name);
	const internal::MethodInfo* (*il2cpp_property_get_get_method)(const internal::PropertyInfo * prop);
	const internal::MethodInfo* (*il2cpp_property_get_set_method)(const internal::PropertyInfo * prop);
	const int32_t(*il2cpp_string_length)(const internal::Il2CppString str);
	const internal::Il2CppChar* (*il2cpp_string_chars)(const internal::Il2CppString str);
	internal::Il2CppString(*il2cpp_string_new_len)(const char* str, uint32_t length);
	uint32_t(*il2cpp_array_length)(internal::Il2CppObject arr);
	uint32_t(*il2cpp_array_get_byte_length)(internal::Il2CppObject arr);

	il2cpp_binding &(*mGetBinding)();
	il2cppapi::Class*(*mGetClass)(const char *, const char *);
	il2cppapi::Class*(*mGetClassFromField)(const internal::FieldInfo* field);
};
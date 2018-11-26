#pragma once

#include "Core/Ref.h"
#include "Core/Class/IRuntimeClass.h"
#include "Core/Containers/AlignedVector.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IRuntimeDispatch;

class T_DLLCLASS RuntimeClass : public IRuntimeClass
{
public:
	T_NO_COPY_CLASS(RuntimeClass);

	RuntimeClass()
	{
	}

	/*! \name Constants */
	/*! \{ */

	void addConstant(const char* const name, const Any& value);

	/*! \} */

	virtual const IRuntimeDispatch* getConstructorDispatch() const T_OVERRIDE T_FINAL;

	virtual uint32_t getConstantCount() const T_OVERRIDE T_FINAL;

	virtual std::string getConstantName(uint32_t constId) const T_OVERRIDE T_FINAL;

	virtual Any getConstantValue(uint32_t constId) const T_OVERRIDE T_FINAL;

	virtual uint32_t getMethodCount() const T_OVERRIDE T_FINAL;

	virtual std::string getMethodName(uint32_t methodId) const T_OVERRIDE T_FINAL;

	virtual const IRuntimeDispatch* getMethodDispatch(uint32_t methodId) const T_OVERRIDE T_FINAL;

	virtual uint32_t getStaticMethodCount() const T_OVERRIDE T_FINAL;

	virtual std::string getStaticMethodName(uint32_t methodId) const T_OVERRIDE T_FINAL;

	virtual const IRuntimeDispatch* getStaticMethodDispatch(uint32_t methodId) const T_OVERRIDE T_FINAL;

	virtual uint32_t getPropertiesCount() const T_OVERRIDE T_FINAL;

	virtual std::string getPropertyName(uint32_t propertyId) const T_OVERRIDE T_FINAL;

	const IRuntimeDispatch* getPropertyGetDispatch(uint32_t propertyId) const T_OVERRIDE T_FINAL;

	const IRuntimeDispatch* getPropertySetDispatch(uint32_t propertyId) const T_OVERRIDE T_FINAL;

	virtual const IRuntimeDispatch* getOperatorDispatch(OperatorType op) const T_OVERRIDE T_FINAL;

	virtual const IRuntimeDispatch* getUnknownDispatch() const T_OVERRIDE T_FINAL;

protected:
	struct ConstInfo
	{
		std::string name;
		Any value;
	};

	struct MethodInfo
	{
		std::string name;
		uint32_t argc;
		Ref< IRuntimeDispatch > dispatch;
	};

	struct PropertyInfo
	{
		std::string name;
		Ref< IRuntimeDispatch > setter;
		Ref< IRuntimeDispatch > getter;
	};

	uint32_t m_constructorArgc;
	Ref< IRuntimeDispatch > m_constructor;
	AlignedVector< ConstInfo > m_consts;
	AlignedVector< MethodInfo > m_methods;
	AlignedVector< MethodInfo > m_staticMethods;
	AlignedVector< PropertyInfo > m_properties;
	Ref< IRuntimeDispatch > m_operators[OptCount];
	Ref< IRuntimeDispatch > m_unknown;

	void addConstructor(uint32_t argc, IRuntimeDispatch* constructor);

	void addMethod(const char* const methodName, uint32_t argc, IRuntimeDispatch* method);

	void addStaticMethod(const char* const methodName, uint32_t argc, IRuntimeDispatch* method);

	void addProperty(const char* const propertyName, const std::wstring& signature, IRuntimeDispatch* setter, IRuntimeDispatch* getter);
};

}
/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_IRuntimeClass_H
#define traktor_IRuntimeClass_H

#include "Core/Object.h"
#include "Core/Class/Any.h"

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

/*! \brief Runtime class definition.
 * \ingroup Core
 */
class T_DLLCLASS IRuntimeClass : public Object
{
	T_RTTI_CLASS;

public:
	/*! \brief Operator types. */
	enum OperatorType
	{
		OptAdd = 0,
		OptSubtract = 1,
		OptMultiply = 2,
		OptDivide = 3,
		OptCount = 4
	};

	/*! \brief Get exported native type. */
	virtual const TypeInfo& getExportType() const = 0;

	/*! \brief Get constructor dispatch. */
	virtual const IRuntimeDispatch* getConstructorDispatch() const = 0;

	/*! \brief Get number of constants. */
	virtual uint32_t getConstantCount() const = 0;

	/*! \brief Get constant. */
	virtual std::string getConstantName(uint32_t constId) const = 0;

	/*! \brief Get constant value. */
	virtual Any getConstantValue(uint32_t constId) const = 0;

	/*! \brief Get exported method count. */
	virtual uint32_t getMethodCount() const = 0;

	/*! \brief Get name of exported method. */
	virtual std::string getMethodName(uint32_t methodId) const = 0;

	/*! \brief Get dispatcher of exported method. */
	virtual const IRuntimeDispatch* getMethodDispatch(uint32_t methodId) const = 0;

	/*! \brief Get exported static method count. */
	virtual uint32_t getStaticMethodCount() const = 0;

	/*! \brief Get name of exported static method. */
	virtual std::string getStaticMethodName(uint32_t methodId) const = 0;

	/*! \brief Get dispatcher of exported static method. */
	virtual const IRuntimeDispatch* getStaticMethodDispatch(uint32_t methodId) const = 0;

	/*! \brief Get exported properties count. */
	virtual uint32_t getPropertiesCount() const = 0;

	/*! \brief Get name of exported property. */
	virtual std::string getPropertyName(uint32_t propertyId) const = 0;

	/*! \brief Get "get" dispatcher of exported property. */
	virtual const IRuntimeDispatch* getPropertyGetDispatch(uint32_t propertyId) const = 0;

	/*! \brief Get "set" dispatcher of exported property. */
	virtual const IRuntimeDispatch* getPropertySetDispatch(uint32_t propertyId) const = 0;

	/*! \brief Get math operator. */
	virtual const IRuntimeDispatch* getOperatorDispatch(OperatorType op) const = 0;

	/*! \brief Invoke unknown method. */
	virtual const IRuntimeDispatch* getUnknownDispatch() const = 0;
};

/*! \brief
 * \ingroup Core
 */
Ref< ITypedObject > T_DLLCLASS createRuntimeClassInstance(const IRuntimeClass* runtimeClass, ITypedObject* self, uint32_t argc, const Any* argv);

/*! \brief
 * \ingroup Core
 */
const IRuntimeDispatch T_DLLCLASS * findRuntimeClassMethod(const IRuntimeClass* runtimeClass, const std::string& methodName);

/*! \brief
 * \ingroup Core
 */
std::string T_DLLCLASS findRuntimeClassMethodName(const IRuntimeClass* runtimeClass, const IRuntimeDispatch* methodDispatch);

/*! \brief
 * \ingroup Core
 */
std::string T_DLLCLASS findRuntimeClassPropertyName(const IRuntimeClass* runtimeClass, const IRuntimeDispatch* propertyDispatch);

}

#endif	// traktor_IRuntimeClass_H

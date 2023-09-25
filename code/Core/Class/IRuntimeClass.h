/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

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

/*! Runtime class definition.
 * \ingroup Core
 */
class T_DLLCLASS IRuntimeClass : public Object
{
	T_RTTI_CLASS;

public:
	/*! Operator types. */
	enum class Operator
	{
		Add = 0,
		Subtract = 1,
		Multiply = 2,
		Divide = 3,
		Count = 4
	};

	/*! Get exported native type. */
	virtual const TypeInfo& getExportType() const = 0;

	/*! Get constructor dispatch. */
	virtual const IRuntimeDispatch* getConstructorDispatch() const = 0;

	/*! Get number of constants. */
	virtual uint32_t getConstantCount() const = 0;

	/*! Get constant. */
	virtual std::string getConstantName(uint32_t constId) const = 0;

	/*! Get constant value. */
	virtual Any getConstantValue(uint32_t constId) const = 0;

	/*! Get exported method count. */
	virtual uint32_t getMethodCount() const = 0;

	/*! Get name of exported method. */
	virtual std::string getMethodName(uint32_t methodId) const = 0;

	/*! Get dispatcher of exported method. */
	virtual const IRuntimeDispatch* getMethodDispatch(uint32_t methodId) const = 0;

	/*! Get exported static method count. */
	virtual uint32_t getStaticMethodCount() const = 0;

	/*! Get name of exported static method. */
	virtual std::string getStaticMethodName(uint32_t methodId) const = 0;

	/*! Get dispatcher of exported static method. */
	virtual const IRuntimeDispatch* getStaticMethodDispatch(uint32_t methodId) const = 0;

	/*! Get exported properties count. */
	virtual uint32_t getPropertiesCount() const = 0;

	/*! Get name of exported property. */
	virtual std::string getPropertyName(uint32_t propertyId) const = 0;

	/*! Get "get" dispatcher of exported property. */
	virtual const IRuntimeDispatch* getPropertyGetDispatch(uint32_t propertyId) const = 0;

	/*! Get "set" dispatcher of exported property. */
	virtual const IRuntimeDispatch* getPropertySetDispatch(uint32_t propertyId) const = 0;

	/*! Get math operator. */
	virtual const IRuntimeDispatch* getOperatorDispatch(Operator op) const = 0;
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


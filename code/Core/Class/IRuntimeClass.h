#ifndef traktor_IRuntimeClass_H
#define traktor_IRuntimeClass_H

#include <map>
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

/*! \brief Runtime class definition.
 * \ingroup Core
 */
class T_DLLCLASS IRuntimeClass : public Object
{
	T_RTTI_CLASS;

public:
	typedef std::map< std::string, Any > prototype_t;

	/*! \brief Get exported native type. */
	virtual const TypeInfo& getExportType() const = 0;

	/*! \brief Have constructor. */
	virtual bool haveConstructor() const = 0;

	/*! \brief Have unknown method. */
	virtual bool haveUnknown() const = 0;

	/*! \brief Construct new object. */
	virtual Ref< ITypedObject > construct(ITypedObject* self, uint32_t argc, const Any* argv, const prototype_t& proto = prototype_t()) const = 0;

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

	/*! \brief Invoke exported method. */
	virtual Any invoke(ITypedObject* object, uint32_t methodId, uint32_t argc, const Any* argv) const = 0;

	/*! \brief Get exported method count. */
	virtual uint32_t getStaticMethodCount() const = 0;

	/*! \brief Get name of exported method. */
	virtual std::string getStaticMethodName(uint32_t methodId) const = 0;

	/*! \brief Invoke exported static method. */
	virtual Any invokeStatic(uint32_t methodId, uint32_t argc, const Any* argv) const = 0;

	/*! \brief Invoke unknown method. */
	virtual Any invokeUnknown(ITypedObject* object, const std::string& methodName, uint32_t argc, const Any* argv) const = 0;

	/*! \brief Invoke math operator. */
	virtual Any invokeOperator(ITypedObject* object, uint8_t operation, const Any& arg) const = 0;
};

/*! \brief
 * \ingroup Core
 */
uint32_t T_DLLCLASS findRuntimeClassMethodId(const IRuntimeClass* runtimeClass, const std::string& methodName);

}

#endif	// traktor_IRuntimeClass_H

#ifndef traktor_script_IScriptClass_H
#define traktor_script_IScriptClass_H

#include "Core/Object.h"
#include "Script/Any.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCRIPT_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace script
	{

class IScriptContext;

/*! \brief Script class definition.
 * \ingroup Script
 *
 * This class is used to describe native
 * classes which are intended to be used
 * from scripts.
 */
class T_DLLCLASS IScriptClass : public Object
{
	T_RTTI_CLASS;

public:
	struct InvokeParam
	{
		ITypedObject* object; //< Current object.
	};

	/*! \brief Get exported native type. */
	virtual const TypeInfo& getExportType() const = 0;

	/*! \brief Have constructor. */
	virtual bool haveConstructor() const = 0;

	/*! \brief Have unknown method. */
	virtual bool haveUnknown() const = 0;

	/*! \brief Construct new object. */
	virtual Ref< ITypedObject > construct(const InvokeParam& param, uint32_t argc, const Any* argv) const = 0;

	/*! \brief Get exported method count. */
	virtual uint32_t getMethodCount() const = 0;

	/*! \brief Get name of exported method. */
	virtual std::string getMethodName(uint32_t methodId) const = 0;

	/*! \brief Invoke exported method. */
	virtual Any invoke(const InvokeParam& param, uint32_t methodId, uint32_t argc, const Any* argv) const = 0;

	/*! \brief Get exported method count. */
	virtual uint32_t getStaticMethodCount() const = 0;

	/*! \brief Get name of exported method. */
	virtual std::string getStaticMethodName(uint32_t methodId) const = 0;

	/*! \brief Invoke exported static method. */
	virtual Any invokeStatic(uint32_t methodId, uint32_t argc, const Any* argv) const = 0;

	/*! \brief Invoke unknown method. */
	virtual Any invokeUnknown(const InvokeParam& param, const std::string& methodName, uint32_t argc, const Any* argv) const = 0;

	/*! \brief Invoke math operator. */
	virtual Any invokeOperator(const InvokeParam& param, uint8_t operation, const Any& arg) const = 0;
};

	}
}

#endif	// traktor_script_IScriptClass_H

#pragma once

#include "Core/Class/CastAny.h"
#include "Core/Class/IRuntimeDelegate.h"
#include "Spark/Action/ActionValue.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace spark
	{

/*! \brief Cast AS value to Any.
 * \ingroup Spark
 */
Any T_DLLCLASS castActionToAny(const ActionValue& value);

/*! \brief Cast Any value to AS.
 * \ingroup Spark
 */
ActionValue T_DLLCLASS castAnyToAction(const Any& value);

	}

/*! \brief AS value cast rule.
 * \ingroup Spark
 */
template < >
struct CastAny < spark::ActionValue, false >
{
	static OutputStream& typeName(OutputStream& ss)
	{
		return ss << L"spark::ActionValue";
	}

	static bool accept(const Any& value)
	{
		return true;
	}

	static Any set(const spark::ActionValue& value)
	{
		return spark::castActionToAny(value);
	}

	static spark::ActionValue get(const Any& value)
	{
		return spark::castAnyToAction(value);
	}
};

/*! \brief AS value cast rule.
 * \ingroup Spark
 */
template < >
struct CastAny < const spark::ActionValue&, false >
{
	static OutputStream& typeName(OutputStream& ss)
	{
		return ss << L"const spark::ActionValue&";
	}

	static bool accept(const Any& value)
	{
		return true;
	}

	static Any set(const spark::ActionValue& value)
	{
		return spark::castActionToAny(value);
	}

	static spark::ActionValue get(const Any& value)
	{
		return spark::castAnyToAction(value);
	}
};

}


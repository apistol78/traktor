#pragma once

#include "Spark/Action/ActionObjectRelay.h"

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

/*! \brief ActionScript number wrapper.
 * \ingroup Spark
 */
class T_DLLCLASS Number : public ActionObjectRelay
{
	T_RTTI_CLASS;

public:
	Number(float value);

	void set(float value) { m_value = value; }

	float get() const { return m_value; }

private:
	float m_value;
};

	}
}


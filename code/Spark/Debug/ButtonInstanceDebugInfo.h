#pragma once

#include "Spark/Debug/InstanceDebugInfo.h"

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

class ButtonInstance;

/*! \brief
 * \ingroup Spark
 */
class T_DLLCLASS ButtonInstanceDebugInfo : public InstanceDebugInfo
{
	T_RTTI_CLASS;

public:
	ButtonInstanceDebugInfo();

	ButtonInstanceDebugInfo(const ButtonInstance* instance);

	virtual void serialize(ISerializer& s) override final;
};

	}
}

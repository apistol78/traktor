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

class TextInstance;

/*!
 * \ingroup Spark
 */
class T_DLLCLASS TextInstanceDebugInfo : public InstanceDebugInfo
{
	T_RTTI_CLASS;

public:
	TextInstanceDebugInfo() = default;

	explicit TextInstanceDebugInfo(const TextInstance* instance);

	virtual void serialize(ISerializer& s) override final;
};

	}
}

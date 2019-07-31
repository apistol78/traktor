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

class MorphShapeInstance;

/*! \brief
 * \ingroup Spark
 */
class T_DLLCLASS MorphShapeInstanceDebugInfo : public InstanceDebugInfo
{
	T_RTTI_CLASS;

public:
	MorphShapeInstanceDebugInfo();

	MorphShapeInstanceDebugInfo(const MorphShapeInstance* instance, bool mask, bool clipped);

	bool getMask() const { return m_mask; }

	bool getClipped() const { return m_clipped; }

	virtual void serialize(ISerializer& s) override final;

private:
	bool m_mask;
	bool m_clipped;
};

	}
}

#pragma once

#include "Core/Serialization/ISerializable.h"

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

class T_DLLCLASS CaptureControl : public ISerializable
{
	T_RTTI_CLASS;

public:
	CaptureControl();

	CaptureControl(int32_t frameCount);

	int32_t getFrameCount() const { return m_frameCount; }

	virtual void serialize(ISerializer& s) override final;

private:
	int32_t m_frameCount;
};

	}
}


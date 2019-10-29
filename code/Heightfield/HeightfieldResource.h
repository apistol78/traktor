#pragma once

#include <string>
#include "Core/Math/Vector4.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_HEIGHTFIELD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace hf
	{

/*!
 * \ingroup Heightfield
 */
class T_DLLCLASS HeightfieldResource : public ISerializable
{
	T_RTTI_CLASS;

public:
	HeightfieldResource();

	virtual void serialize(ISerializer& s) override final;

	const Vector4& getWorldExtent() const { return m_worldExtent; }

private:
	friend class HeightfieldPipeline;

	Vector4 m_worldExtent;
};

	}
}


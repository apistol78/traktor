#ifndef traktor_hf_HeightfieldAsset_H
#define traktor_hf_HeightfieldAsset_H

#include "Core/Math/Vector4.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_HEIGHTFIELD_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace hf
	{

class T_DLLCLASS HeightfieldAsset : public ISerializable
{
	T_RTTI_CLASS;

public:
	HeightfieldAsset();

	HeightfieldAsset(const Vector4& worldExtent);

	virtual bool serialize(ISerializer& s);

	const Vector4& getWorldExtent() const { return m_worldExtent; }

private:
	Vector4 m_worldExtent;
};

	}
}

#endif	// traktor_hf_HeightfieldAsset_H

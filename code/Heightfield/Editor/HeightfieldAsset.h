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

	virtual void serialize(ISerializer& s);

	const Vector4& getWorldExtent() const { return m_worldExtent; }

	float getVistaDistance() const { return m_vistaDistance; }

private:
	Vector4 m_worldExtent;
	float m_vistaDistance;
};

	}
}

#endif	// traktor_hf_HeightfieldAsset_H

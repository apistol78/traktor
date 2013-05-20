#ifndef traktor_hf_HeightfieldTextureAsset_H
#define traktor_hf_HeightfieldTextureAsset_H

#include "Core/Serialization/ISerializable.h"
#include "Resource/Id.h"

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

class Heightfield;

class T_DLLCLASS HeightfieldTextureAsset : public ISerializable
{
	T_RTTI_CLASS;

public:
	enum OutputType
	{
		OtHeights,
		OtNormals,
		OtCuts,
		OtUnwrap
	};

	resource::Id< Heightfield > m_heightfield;
	OutputType m_output;
	float m_scale;

	HeightfieldTextureAsset();

	virtual void serialize(ISerializer& s);
};

	}
}

#endif	// traktor_hf_HeightfieldTextureAsset_H

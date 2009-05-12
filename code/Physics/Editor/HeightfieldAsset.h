#ifndef traktor_physics_HeightfieldAsset_H
#define traktor_physics_HeightfieldAsset_H

#include "Editor/Asset.h"
#include "Core/Math/Vector4.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace physics
	{

class T_DLLCLASS HeightfieldAsset : public editor::Asset
{
	T_RTTI_CLASS(HeightfieldAsset)

public:
	HeightfieldAsset();

	virtual const Type* getOutputType() const;

	virtual bool serialize(Serializer& s);

private:
	friend class HeightfieldPipeline;

	Vector4 m_worldExtent;
};

	}
}

#endif	// traktor_physics_HeightfieldAsset_H

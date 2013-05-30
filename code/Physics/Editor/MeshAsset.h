#ifndef traktor_physics_MeshAsset_H
#define traktor_physics_MeshAsset_H

#include "Editor/Asset.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace physics
	{

class T_DLLCLASS MeshAsset : public editor::Asset
{
	T_RTTI_CLASS;

public:
	MeshAsset();

	virtual void serialize(ISerializer& s);

	void setCalculateConvexHull(bool calculateConvexHull) { m_calculateConvexHull = calculateConvexHull; }

	bool getCalculateConvexHull() const { return m_calculateConvexHull; }

	void setMargin(float margin) { m_margin = margin; }

	float getMargin() const { return m_margin; }

private:
	friend class MeshPipeline;

	bool m_calculateConvexHull;
	float m_margin;
};

	}
}

#endif	// traktor_physics_MeshAsset_H

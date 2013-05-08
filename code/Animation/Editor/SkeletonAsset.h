#ifndef traktor_animation_SkeletonAsset_H
#define traktor_animation_SkeletonAsset_H

#include "Core/Math/Vector4.h"
#include "Editor/Asset.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace animation
	{

class T_DLLCLASS SkeletonAsset : public editor::Asset
{
	T_RTTI_CLASS;

public:
	SkeletonAsset();

	virtual void serialize(ISerializer& s);

	const Vector4& getOffset() const { return m_offset; }

	float getRadius() const { return m_radius; }

	bool getInvertX() const { return m_invertX; }

	bool getInvertZ() const { return m_invertZ; }

private:
	Vector4 m_offset;
	float m_radius;
	bool m_invertX;
	bool m_invertZ;
};

	}
}

#endif	// traktor_animation_SkeletonAsset_H

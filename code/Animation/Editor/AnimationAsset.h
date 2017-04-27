/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_animation_AnimationAsset_H
#define traktor_animation_AnimationAsset_H

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

/*! \brief
 * \ingroup Animation
 */
class T_DLLCLASS AnimationAsset : public editor::Asset
{
	T_RTTI_CLASS;

public:
	AnimationAsset();

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

	const Vector4& getOffset() const { return m_offset; }

	bool getInvertX() const { return m_invertX; }

	bool getInvertZ() const { return m_invertZ; }

	bool shouldAutoCenterKeyPoses() const { return m_autoCenterKeyPoses; }

private:
	Vector4 m_offset;
	bool m_invertX;
	bool m_invertZ;
	bool m_autoCenterKeyPoses;
};

	}
}

#endif	// traktor_animation_AnimationAsset_H

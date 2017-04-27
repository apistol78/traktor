/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_scene_TransformChain_H
#define traktor_scene_TransformChain_H

#include "Core/Object.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Matrix44.h"
#include "Core/Math/Vector2.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace scene
	{

class T_DLLCLASS TransformChain : public Object
{
	T_RTTI_CLASS;

public:
	TransformChain();

	void pushProjection(const Matrix44& projection);

	void pushView(const Matrix44& view);

	void pushWorld(const Matrix44& world);

	void popWorld();

	void popView();

	void popProjection();

	Vector4 viewToClip(const Vector4& vp) const;

	Vector4 worldToView(const Vector4& wp) const;

	Vector4 worldToClip(const Vector4& wp) const;

	Vector4 objectToWorld(const Vector4& op) const;

	Vector4 objectToView(const Vector4& op) const;

	Vector4 objectToClip(const Vector4& op) const;

	Vector4 clipToView(const Vector4& cp) const;

	bool clipToScreen(const Vector4& cp, Vector2& outScreen) const;

	bool viewToScreen(const Vector4& vp, Vector2& outScreen) const;

	bool worldToScreen(const Vector4& wp, Vector2& outScreen) const;

	bool objectToScreen(const Vector4& op, Vector2& outScreen) const;

	const Matrix44& getProjection() const { return m_projection.back(); }

	const Matrix44& getView() const { return m_view.back(); }

	const Matrix44& getWorld() const { return m_world.back(); }	

private:
	AlignedVector< Matrix44 > m_projection;
	AlignedVector< Matrix44 > m_view;
	AlignedVector< Matrix44 > m_world;
};

	}
}

#endif	// traktor_scene_TransformChain_H

/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Scene/Editor/TransformChain.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.TransformChain", TransformChain, Object)

TransformChain::TransformChain()
{
	m_projection.push_back(Matrix44::identity());
	m_view.push_back(Matrix44::identity());
	m_world.push_back(Matrix44::identity());
}

void TransformChain::pushProjection(const Matrix44& projection)
{
	m_projection.push_back(projection);
}

void TransformChain::pushView(const Matrix44& view)
{
	m_view.push_back(view);
}

void TransformChain::pushWorld(const Matrix44& world)
{
	m_world.push_back(world);
}

void TransformChain::popWorld()
{
	T_ASSERT (m_world.size() > 1);
	m_world.pop_back();
}

void TransformChain::popView()
{
	T_ASSERT (m_view.size() > 1);
	m_view.pop_back();
}

void TransformChain::popProjection()
{
	T_ASSERT (m_projection.size() > 1);
	m_projection.pop_back();
}

Vector4 TransformChain::viewToClip(const Vector4& vp) const
{
	return m_projection.back() * vp;
}

Vector4 TransformChain::worldToView(const Vector4& wp) const
{
	return m_view.back() * wp;
}

Vector4 TransformChain::worldToClip(const Vector4& wp) const
{
	return m_projection.back() * m_view.back() * wp;
}

Vector4 TransformChain::objectToWorld(const Vector4& op) const
{
	return m_world.back() * op;
}

Vector4 TransformChain::objectToView(const Vector4& op) const
{
	return m_view.back() * m_world.back() * op;
}

Vector4 TransformChain::objectToClip(const Vector4& op) const
{
	return m_projection.back() * m_view.back() * m_world.back() * op;
}

Vector4 TransformChain::clipToView(const Vector4& cp) const
{
	return m_projection.back().inverse() * cp;
}

bool TransformChain::clipToScreen(const Vector4& cp, Vector2& outScreen) const
{
	Scalar iw = Scalar(1.0f) / cp.w();
	outScreen.x = cp.x() * iw;
	outScreen.y = cp.y() * iw;
	return true;
}

bool TransformChain::viewToScreen(const Vector4& vp, Vector2& outScreen) const
{
	return clipToScreen(viewToClip(vp), outScreen);
}

bool TransformChain::worldToScreen(const Vector4& wp, Vector2& outScreen) const
{
	return clipToScreen(worldToClip(wp), outScreen);
}

bool TransformChain::objectToScreen(const Vector4& op, Vector2& outScreen) const
{
	return clipToScreen(objectToClip(op), outScreen);
}

	}
}

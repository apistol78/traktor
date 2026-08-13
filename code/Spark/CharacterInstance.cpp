/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Spark/CharacterInstance.h"

#include "Spark/Context.h"
#include "Spark/Swf/SwfTypes.h"
#include "Spark/Types.h"

namespace traktor::spark
{
namespace
{

bool equal(const Matrix33& a, const Matrix33& b)
{
	return a.e11 == b.e11 && a.e12 == b.e12 && a.e13 == b.e13 &&
		a.e21 == b.e21 && a.e22 == b.e22 && a.e23 == b.e23 &&
		a.e31 == b.e31 && a.e32 == b.e32 && a.e33 == b.e33;
}

bool equal(const ColorTransform& a, const ColorTransform& b)
{
	return a.mul == b.mul && a.add == b.add;
}

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.CharacterInstance", CharacterInstance, Object)

std::atomic< int32_t > CharacterInstance::ms_instanceCount(0);

CharacterInstance::CharacterInstance(
	Context* context,
	Dictionary* dictionary,
	CharacterInstance* parent)
	: m_context(context)
	, m_dictionary(dictionary)
	, m_parent(parent)
	, m_filterColor(0.0f, 0.0f, 0.0f, 0.0f)
	, m_filter(0)
	, m_blendMode(0)
	, m_visible(true)
	, m_enabled(true)
	, m_wireOutline(false)
{
	ms_instanceCount++;

	m_cxform.mul = Color4f(1.0f, 1.0f, 1.0f, 1.0f);
	m_cxform.add = Color4f(0.0f, 0.0f, 0.0f, 0.0f);

	m_transform = Matrix33::identity();
}

CharacterInstance::~CharacterInstance()
{
	destroy();
	ms_instanceCount--;
}

int32_t CharacterInstance::getInstanceCount()
{
	return ms_instanceCount;
}

void CharacterInstance::destroy()
{
	if (m_context)
	{
		if (m_context->getFocus() == this)
			m_context->setFocus(nullptr);
		if (m_context->getPressed() == this)
			m_context->setPressed(nullptr);
		if (m_context->getRolledOver() == this)
			m_context->setRolledOver(nullptr);
	}

	m_context = nullptr;
	m_dictionary = nullptr;
	m_parent = nullptr;
	m_cacheObject = nullptr;
	m_userObject = nullptr;

	m_eventSetFocus.removeAll();
	m_eventKillFocus.removeAll();
}

void CharacterInstance::setParent(CharacterInstance* parent)
{
	m_parent = parent;
}

void CharacterInstance::setName(const std::string& name)
{
	m_name = name;
}

const std::string& CharacterInstance::getName() const
{
	return m_name;
}

void CharacterInstance::setCacheObject(IRefCount* cacheObject) const
{
	m_cacheObject = cacheObject;
}

void CharacterInstance::setUserObject(IRefCount* userObject)
{
	m_userObject = userObject;
}

void CharacterInstance::clearCacheObject()
{
	m_cacheObject = nullptr;
}

void CharacterInstance::invalidateCache()
{
	for (CharacterInstance* i = this; i != nullptr; i = i->m_parent)
		i->m_cacheVersion++;
}

void CharacterInstance::invalidateParentCache()
{
	for (CharacterInstance* i = m_parent; i != nullptr; i = i->m_parent)
		i->m_cacheVersion++;
}

std::string CharacterInstance::getTarget() const
{
	return m_parent ? (m_parent->getTarget() + "/" + getName()) : "";
}

void CharacterInstance::setColorTransform(const ColorTransform& cxform)
{
	if (equal(cxform, m_cxform))
		return;

	invalidateParentCache();
	m_cxform = cxform;
}

ColorTransform CharacterInstance::getFullColorTransform() const
{
	if (m_parent)
		return m_parent->getFullColorTransform() * m_cxform;
	else
		return m_cxform;
}

void CharacterInstance::setAlpha(float alpha)
{
	if (m_cxform.mul.getAlpha() == Scalar(alpha))
		return;

	invalidateParentCache();
	m_cxform.mul.setAlpha(Scalar(alpha));
}

float CharacterInstance::getAlpha() const
{
	return m_cxform.mul.getAlpha();
}

void CharacterInstance::setTransform(const Matrix33& transform)
{
	if (equal(transform, m_transform))
		return;

	invalidateParentCache();
	m_transform = transform;
}

Matrix33 CharacterInstance::getFullTransform() const
{
	if (m_parent)
		return m_parent->getFullTransform() * m_transform;
	else
		return m_transform;
}

Vector2 CharacterInstance::transformInto(const CharacterInstance* other, const Vector2& pnt) const
{
	const Vector2 glb = getFullTransform() * pnt;
	return other->getFullTransform().inverse() * glb;
}

void CharacterInstance::setFilter(uint8_t filter)
{
	if (filter != m_filter)
		invalidateCache();
	m_filter = filter;
}

void CharacterInstance::setFilterColor(const Color4f& filterColor)
{
	if (filterColor != m_filterColor)
		invalidateCache();
	m_filterColor = filterColor;
}

void CharacterInstance::setBlendMode(uint8_t blendMode)
{
	if (blendMode != m_blendMode)
		invalidateCache();
	m_blendMode = blendMode;
}

void CharacterInstance::setVisible(bool visible)
{
	if (visible != m_visible)
		invalidateCache();
	m_visible = visible;
}

void CharacterInstance::setEnabled(bool enabled)
{
	m_enabled = enabled;
}

void CharacterInstance::setFocus()
{
	getContext()->setFocus(this);
}

bool CharacterInstance::haveFocus() const
{
	return bool(getContext()->getFocus() == this);
}

void CharacterInstance::setWireOutline(bool wireOutline)
{
	m_wireOutline = wireOutline;
}

void CharacterInstance::eventFrame()
{
}

void CharacterInstance::eventKey(wchar_t unicode)
{
}

void CharacterInstance::eventKeyDown(int keyCode)
{
}

void CharacterInstance::eventKeyUp(int keyCode)
{
}

void CharacterInstance::eventMouseDown(int x, int y, int button)
{
}

void CharacterInstance::eventMouseUp(int x, int y, int button)
{
}

void CharacterInstance::eventMouseMove(int x, int y, int button)
{
}

void CharacterInstance::eventSetFocus()
{
	m_eventSetFocus.issue();
}

void CharacterInstance::eventKillFocus()
{
	m_eventKillFocus.issue();
}

}

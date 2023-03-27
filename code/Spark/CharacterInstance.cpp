/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Spark/CharacterInstance.h"
#include "Spark/Context.h"
#include "Spark/Types.h"
#include "Spark/Swf/SwfTypes.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.CharacterInstance", CharacterInstance, Object)

std::atomic< int32_t > CharacterInstance::ms_instanceCount(0);

CharacterInstance::CharacterInstance(
	Context* context,
	Dictionary* dictionary,
	CharacterInstance* parent
)
:	m_context(context)
,	m_dictionary(dictionary)
,	m_parent(parent)
,	m_filterColor(0.0f, 0.0f, 0.0f, 0.0f)
,	m_filter(0)
,	m_blendMode(0)
,	m_visible(true)
,	m_enabled(true)
,	m_wireOutline(false)
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
	m_parent = nullptr;

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

void CharacterInstance::setCacheObject(IRefCount* cacheObject)
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

std::string CharacterInstance::getTarget() const
{
	return m_parent ? (m_parent->getTarget() + "/" + getName()) : "";
}

void CharacterInstance::setColorTransform(const ColorTransform& cxform)
{
	clearCacheObject();
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
	clearCacheObject();
	m_cxform.mul.setAlpha(Scalar(alpha));
}

float CharacterInstance::getAlpha() const
{
	return m_cxform.mul.getAlpha();
}

void CharacterInstance::setTransform(const Matrix33& transform)
{
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
	m_filter = filter;
}

void CharacterInstance::setFilterColor(const Color4f& filterColor)
{
	m_filterColor = filterColor;
}

void CharacterInstance::setBlendMode(uint8_t blendMode)
{
	m_blendMode = blendMode;
}

void CharacterInstance::setVisible(bool visible)
{
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
}

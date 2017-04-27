/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Class/Boxes.h"
#include "Core/Class/IRuntimeClass.h"
#include "Core/Misc/TString.h"
#include "Spark/Sprite.h"
#include "Spark/Script/ScriptComponent.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.ScriptComponent", ScriptComponent, IComponent)

ScriptComponent::ScriptComponent(Sprite* owner, const resource::Proxy< IRuntimeClass >& clazz)
:	m_owner(owner)
,	m_class(clazz)
,	m_methodUpdate(~0U)
,	m_methodEventKey(~0U)
,	m_methodEventKeyDown(~0U)
,	m_methodEventKeyUp(~0U)
,	m_methodEventMouseDown(~0U)
,	m_methodEventMouseUp(~0U)
,	m_methodEventMousePress(~0U)
,	m_methodEventMouseRelease(~0U)
,	m_methodEventMouseMove(~0U)
,	m_methodEventMouseEnter(~0U)
,	m_methodEventMouseLeave(~0U)
,	m_methodEventMouseWheel(~0U)
,	m_methodEventViewResize(~0U)
{
	IRuntimeClass::prototype_t proto;

	// Place all existing child characters in prototype to be accessible from constructor.
	RefArray< Character > characters;
	m_owner->getCharacters(characters);
	for (RefArray< Character >::const_iterator i = characters.begin(); i != characters.end(); ++i)
	{
		if (!(*i)->getName().empty())
			proto[wstombs((*i)->getName())] = Any::fromObject(*i);
	}

	// Invoke script class constructor.
	m_object = m_class->construct(m_owner, 0, 0, proto);

	m_methodUpdate = findRuntimeClassMethodId(m_class, "update");
	m_methodEventKey = findRuntimeClassMethodId(m_class, "eventKey");
	m_methodEventKeyDown = findRuntimeClassMethodId(m_class, "eventKeyDown");
	m_methodEventKeyUp = findRuntimeClassMethodId(m_class, "eventKeyUp");
	m_methodEventMouseDown = findRuntimeClassMethodId(m_class, "eventMouseDown");
	m_methodEventMouseUp = findRuntimeClassMethodId(m_class, "eventMouseUp");
	m_methodEventMousePress = findRuntimeClassMethodId(m_class, "eventMousePress");
	m_methodEventMouseRelease = findRuntimeClassMethodId(m_class, "eventMouseRelease");
	m_methodEventMouseMove = findRuntimeClassMethodId(m_class, "eventMouseMove");
	m_methodEventMouseEnter = findRuntimeClassMethodId(m_class, "eventMouseEnter");
	m_methodEventMouseLeave = findRuntimeClassMethodId(m_class, "eventMouseLeave");
	m_methodEventMouseWheel = findRuntimeClassMethodId(m_class, "eventMouseWheel");
	m_methodEventViewResize = findRuntimeClassMethodId(m_class, "eventViewResize");

	m_class.consume();
}

void ScriptComponent::update()
{
	if (m_class.changed())
	{
		m_object = m_class->construct(m_owner, 0, 0);

		m_methodUpdate = findRuntimeClassMethodId(m_class, "update");
		m_methodEventKey = findRuntimeClassMethodId(m_class, "eventKey");
		m_methodEventKeyDown = findRuntimeClassMethodId(m_class, "eventKeyDown");
		m_methodEventKeyUp = findRuntimeClassMethodId(m_class, "eventKeyUp");
		m_methodEventMouseDown = findRuntimeClassMethodId(m_class, "eventMouseDown");
		m_methodEventMouseUp = findRuntimeClassMethodId(m_class, "eventMouseUp");
		m_methodEventMouseMove = findRuntimeClassMethodId(m_class, "eventMouseMove");
		m_methodEventMouseWheel = findRuntimeClassMethodId(m_class, "eventMouseWheel");
		m_methodEventViewResize = findRuntimeClassMethodId(m_class, "eventViewResize");

		m_class.consume();
	}

	if (m_class && m_object && m_methodUpdate != ~0U)
		m_class->invoke(m_object, m_methodUpdate, 0, 0);
}

void ScriptComponent::eventKey(wchar_t unicode)
{
	if (m_class && m_object && m_methodEventKey != ~0U)
	{
		Any argv[] =
		{
			Any::fromInt32(unicode)
		};
		m_class->invoke(m_object, m_methodEventKey, sizeof_array(argv), argv);
	}
}

void ScriptComponent::eventKeyDown(int32_t keyCode)
{
	if (m_class && m_object && m_methodEventKeyDown != ~0U)
	{
		Any argv[] =
		{
			Any::fromInt32(keyCode)
		};
		m_class->invoke(m_object, m_methodEventKeyDown, sizeof_array(argv), argv);
	}
}

void ScriptComponent::eventKeyUp(int32_t keyCode)
{
	if (m_class && m_object && m_methodEventKeyUp != ~0U)
	{
		Any argv[] =
		{
			Any::fromInt32(keyCode)
		};
		m_class->invoke(m_object, m_methodEventKeyUp, sizeof_array(argv), argv);
	}
}

void ScriptComponent::eventMouseDown(const Vector2& position, int32_t button)
{
	if (m_class && m_object && m_methodEventMouseDown != ~0U)
	{
		Any argv[] =
		{
			CastAny< Vector2 >::set(position),
			Any::fromInt32(button)
		};
		m_class->invoke(m_object, m_methodEventMouseDown, sizeof_array(argv), argv);
	}
}

void ScriptComponent::eventMouseUp(const Vector2& position, int32_t button)
{
	if (m_class && m_object && m_methodEventMouseUp != ~0U)
	{
		Any argv[] =
		{
			CastAny< Vector2 >::set(position),
			Any::fromInt32(button)
		};
		m_class->invoke(m_object, m_methodEventMouseUp, sizeof_array(argv), argv);
	}
}

void ScriptComponent::eventMousePress(const Vector2& position, int32_t button)
{
	if (m_class && m_object && m_methodEventMousePress != ~0U)
	{
		Any argv[] =
		{
			CastAny< Vector2 >::set(position),
			Any::fromInt32(button)
		};
		m_class->invoke(m_object, m_methodEventMousePress, sizeof_array(argv), argv);
	}
}

void ScriptComponent::eventMouseRelease(const Vector2& position, int32_t button)
{
	if (m_class && m_object && m_methodEventMouseRelease != ~0U)
	{
		Any argv[] =
		{
			CastAny< Vector2 >::set(position),
			Any::fromInt32(button)
		};
		m_class->invoke(m_object, m_methodEventMouseRelease, sizeof_array(argv), argv);
	}
}

void ScriptComponent::eventMouseMove(const Vector2& position, int32_t button)
{
	if (m_class && m_object && m_methodEventMouseMove != ~0U)
	{
		Any argv[] =
		{
			CastAny< Vector2 >::set(position),
			Any::fromInt32(button)
		};
		m_class->invoke(m_object, m_methodEventMouseMove, sizeof_array(argv), argv);
	}
}

void ScriptComponent::eventMouseEnter(const Vector2& position, int32_t button)
{
	if (m_class && m_object && m_methodEventMouseEnter != ~0U)
	{
		Any argv[] =
		{
			CastAny< Vector2 >::set(position),
			Any::fromInt32(button)
		};
		m_class->invoke(m_object, m_methodEventMouseEnter, sizeof_array(argv), argv);
	}
}

void ScriptComponent::eventMouseLeave(const Vector2& position, int32_t button)
{
	if (m_class && m_object && m_methodEventMouseLeave != ~0U)
	{
		Any argv[] =
		{
			CastAny< Vector2 >::set(position),
			Any::fromInt32(button)
		};
		m_class->invoke(m_object, m_methodEventMouseLeave, sizeof_array(argv), argv);
	}
}

void ScriptComponent::eventMouseWheel(const Vector2& position, int32_t delta)
{
	if (m_class && m_object && m_methodEventMouseWheel != ~0U)
	{
		Any argv[] =
		{
			CastAny< Vector2 >::set(position),
			Any::fromInt32(delta)
		};
		m_class->invoke(m_object, m_methodEventMouseWheel, sizeof_array(argv), argv);
	}
}

void ScriptComponent::eventViewResize(int32_t width, int32_t height)
{
	if (m_class && m_object && m_methodEventViewResize != ~0U)
	{
		Any argv[] =
		{
			Any::fromInt32(width),
			Any::fromInt32(height)
		};
		m_class->invoke(m_object, m_methodEventViewResize, sizeof_array(argv), argv);
	}
}

	}
}

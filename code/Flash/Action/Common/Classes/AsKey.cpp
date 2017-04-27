/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <algorithm>
#include <cstring>
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFrame.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Common/Classes/AsKey.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsKey", AsKey, ActionObject)

AsKey::AsKey(ActionContext* context)
:	ActionObject(context)
,	m_lastKeyCode(0)
{
	std::memset(m_keyState, 0, sizeof(m_keyState));

	setMember("BACKSPACE", ActionValue(int32_t(AkBackspace)));
	setMember("CAPSLOCK", ActionValue(int32_t(AkCapsLock)));
	setMember("CONTROL", ActionValue(int32_t(AkControl)));
	setMember("DELETEKEY", ActionValue(int32_t(AkDeleteKey)));
	setMember("DOWN", ActionValue(int32_t(AkDown)));
	setMember("END", ActionValue(int32_t(AkEnd)));
	setMember("ENTER", ActionValue(int32_t(AkEnter)));
	setMember("ESCAPE", ActionValue(int32_t(AkEscape)));
	setMember("HOME", ActionValue(int32_t(AkHome)));
	setMember("INSERT", ActionValue(int32_t(AkInsert)));
	setMember("LEFT", ActionValue(int32_t(AkLeft)));
	setMember("PGDN", ActionValue(int32_t(AkPgDn)));
	setMember("PGUP", ActionValue(int32_t(AkPgUp)));
	setMember("RIGHT", ActionValue(int32_t(AkRight)));
	setMember("SHIFT", ActionValue(int32_t(AkShift)));
	setMember("SPACE", ActionValue(int32_t(AkSpace)));
	setMember("TAB", ActionValue(int32_t(AkTab)));
	setMember("UP", ActionValue(int32_t(AkUp)));
	setMember("getAscii", ActionValue(createNativeFunction(context, this, &AsKey::Key_getAscii)));
	setMember("getCode", ActionValue(createNativeFunction(context, this, &AsKey::Key_getCode)));
	setMember("isAccessible", ActionValue(createNativeFunction(context, this, &AsKey::Key_isAccessible)));
	setMember("isDown", ActionValue(createNativeFunction(context, this, &AsKey::Key_isDown)));
	setMember("isToggled", ActionValue(createNativeFunction(context, this, &AsKey::Key_isToggled)));
}

void AsKey::eventKeyDown(int keyCode)
{
	m_keyState[keyCode] = true;
	m_lastKeyCode = keyCode;

	ActionValue broadcastMessageValue;
	getMember("broadcastMessage", broadcastMessageValue);

	Ref< ActionFunction > broadcastMessageFn = broadcastMessageValue.getObject< ActionFunction >();
	if (broadcastMessageFn)
	{
		ActionValueArray args(getContext()->getPool(), 1);
		args[0] = ActionValue("onKeyDown");
		broadcastMessageFn->call(this, args);
	}
}

void AsKey::eventKeyUp(int keyCode)
{
	m_keyState[keyCode] = false;

	ActionValue broadcastMessageValue;
	getMember("broadcastMessage", broadcastMessageValue);

	Ref< ActionFunction > broadcastMessageFn = broadcastMessageValue.getObject< ActionFunction >();
	if (broadcastMessageFn)
	{
		ActionValueArray args(getContext()->getPool(), 1);
		args[0] = ActionValue("onKeyUp");
		broadcastMessageFn->call(this, args);
	}
}

void AsKey::Key_getAscii(CallArgs& ca)
{
	ca.ret = ActionValue(m_lastKeyCode);
}

void AsKey::Key_getCode(CallArgs& ca)
{
	ca.ret = ActionValue(m_lastKeyCode);
}

void AsKey::Key_isAccessible(CallArgs& ca)
{
	ca.ret = ActionValue(false);
}

void AsKey::Key_isDown(CallArgs& ca)
{
	int32_t keyCode = ca.args[0].getInteger();
	ca.ret = ActionValue((keyCode >= 0 && keyCode < 256) ? m_keyState[keyCode] : false);
}

void AsKey::Key_isToggled(CallArgs& ca)
{
	ca.ret = ActionValue(false);
}

	}
}

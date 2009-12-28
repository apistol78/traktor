#include "Flash/Action/Avm1/ActionGlobal.h"
#include "Flash/Action/Avm1/Classes/AsObject.h"
#include "Flash/Action/Avm1/Classes/AsAccessibility.h"
#include "Flash/Action/Avm1/Classes/AsArray.h"
#include "Flash/Action/Avm1/Classes/AsBoolean.h"
#include "Flash/Action/Avm1/Classes/AsButton.h"
#include "Flash/Action/Avm1/Classes/AsDate.h"
#include "Flash/Action/Avm1/Classes/AsError.h"
#include "Flash/Action/Avm1/Classes/AsFunction.h"
#include "Flash/Action/Avm1/Classes/AsI18N.h"
#include "Flash/Action/Avm1/Classes/AsKey.h"
#include "Flash/Action/Avm1/Classes/AsLoadVars.h"
#include "Flash/Action/Avm1/Classes/AsLocalConnection.h"
#include "Flash/Action/Avm1/Classes/AsMath.h"
#include "Flash/Action/Avm1/Classes/AsMovieClip.h"
#include "Flash/Action/Avm1/Classes/AsMovieClipLoader.h"
#include "Flash/Action/Avm1/Classes/AsMouse.h"
#include "Flash/Action/Avm1/Classes/AsNumber.h"
#include "Flash/Action/Avm1/Classes/AsPoint.h"
#include "Flash/Action/Avm1/Classes/AsRectangle.h"
#include "Flash/Action/Avm1/Classes/AsSecurity.h"
#include "Flash/Action/Avm1/Classes/AsSound.h"
#include "Flash/Action/Avm1/Classes/AsStage.h"
#include "Flash/Action/Avm1/Classes/AsString.h"
#include "Flash/Action/Avm1/Classes/AsSystem.h"
#include "Flash/Action/Avm1/Classes/AsTextField.h"
#include "Flash/Action/Avm1/Classes/AsXML.h"
#include "Flash/Action/Avm1/Classes/AsXMLNode.h"
#include "Flash/Action/Avm1/ActionFunctionNative.h"
#include "Core/Io/StringOutputStream.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.ActionGlobal", ActionGlobal, ActionObject)

ActionGlobal::ActionGlobal()
:	ActionObject(AsObject::getInstance())
{
	setMember(L"ASSetPropFlags", createNativeFunctionValue(this, &ActionGlobal::Global_ASSetPropFlags));
	setMember(L"escape", createNativeFunctionValue(this, &ActionGlobal::Global_escape));

	// Create prototypes.
	setMember(L"Object", ActionValue::fromObject(AsObject::getInstance()));
	setMember(L"Accessibility", ActionValue::fromObject(AsAccessibility::getInstance()));
	setMember(L"Array", ActionValue::fromObject(AsArray::getInstance()));
	setMember(L"Boolean", ActionValue::fromObject(AsBoolean::getInstance()));
	setMember(L"Button", ActionValue::fromObject(AsButton::getInstance()));
	setMember(L"Date", ActionValue::fromObject(AsDate::getInstance()));
	setMember(L"Error", ActionValue::fromObject(AsError::getInstance()));
	setMember(L"Function", ActionValue::fromObject(AsFunction::getInstance()));
	setMember(L"I18N", ActionValue::fromObject(AsI18N::getInstance()));
	setMember(L"Key", ActionValue::fromObject(AsKey::createInstance()));
	setMember(L"LoadVars", ActionValue::fromObject(AsLoadVars::getInstance()));
	setMember(L"LocalConnection", ActionValue::fromObject(AsLocalConnection::getInstance()));
	setMember(L"Math", ActionValue::fromObject(AsMath::getInstance()));
	setMember(L"Mouse", ActionValue::fromObject(AsMouse::createInstance()));
	setMember(L"MovieClip", ActionValue::fromObject(AsMovieClip::getInstance()));
	setMember(L"MovieClipLoader", ActionValue::fromObject(AsMovieClipLoader::getInstance()));
	setMember(L"Number", ActionValue::fromObject(AsMath::getInstance()));
	setMember(L"Security", ActionValue::fromObject(AsSecurity::getInstance()));
	setMember(L"Sound", ActionValue::fromObject(AsSound::getInstance()));
	setMember(L"Stage", ActionValue::fromObject(AsStage::getInstance()));
	setMember(L"String", ActionValue::fromObject(AsString::getInstance()));
	setMember(L"System", ActionValue::fromObject(AsSystem::getInstance()));
	setMember(L"TextField", ActionValue::fromObject(AsTextField::getInstance()));
	setMember(L"XML", ActionValue::fromObject(AsXML::getInstance()));
	setMember(L"XMLNode", ActionValue::fromObject(AsXMLNode::getInstance()));

	// flash.
	Ref< ActionObject > flash = new ActionObject(AsObject::getInstance());
	{
		//// flash.external.
		//Ref< ActionObject > external = new ActionObject(AsObject::getInstance());
		//{
		//	external->setMember(L"ExternalInterface", AsExternalInterface::getInstance());
		//}
		//flash->setMember(L"external", external);

		// flash.geom.
		Ref< ActionObject > geom = new ActionObject(AsObject::getInstance());
		{
			//geom->setMember(L"Matrix", AsMatrix::getInstance());
			geom->setMember(L"Point", ActionValue::fromObject(AsPoint::getInstance()));
			geom->setMember(L"Rectangle", ActionValue::fromObject(AsRectangle::getInstance()));
		}
		flash->setMember(L"geom", ActionValue::fromObject(geom));
	}
	setMember(L"flash", ActionValue::fromObject(flash));
}

void ActionGlobal::Global_ASSetPropFlags(CallArgs& ca)
{
	Ref< ActionObject > object = ca.args[0].getObject();
	uint32_t flags = uint32_t(ca.args[2].getNumberSafe());

	// Read-only; protect classes from being modified.
	if (flags & 4)
		object->setReadOnly();
}

void ActionGlobal::Global_escape(CallArgs& ca)
{
	std::wstring text = ca.args[0].getStringSafe();
	StringOutputStream ss;

	for (size_t i = 0; i < text.length(); ++i)
	{
		wchar_t ch = text[i];
		if (
			(ch >= '0' && ch <= '9') ||
			(ch >= 'a' && ch <= 'z') ||
			(ch >= 'A' && ch <= 'Z') ||
			ch == '.' || ch == ',' || ch == '-' || ch == '_' || ch == '~'
		)
			ss.put(ch);
		else
		{
			const wchar_t c_hex[] = { L"0123456789ABCDEF" };
			ss.put('%');
			ss.put(c_hex[(ch) >> 4]);
			ss.put(c_hex[(ch) & 15]);
		}
	}

	ca.ret = ActionValue(ss.str());
}

	}
}

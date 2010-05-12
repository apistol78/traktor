#include "Core/Io/StringOutputStream.h"
#include "Flash/Action/ActionFunctionNative.h"
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
#include "Flash/Action/Avm1/Classes/AsSecurity.h"
#include "Flash/Action/Avm1/Classes/AsSound.h"
#include "Flash/Action/Avm1/Classes/AsStage.h"
#include "Flash/Action/Avm1/Classes/AsString.h"
#include "Flash/Action/Avm1/Classes/AsSystem.h"
#include "Flash/Action/Avm1/Classes/AsTextField.h"
#include "Flash/Action/Avm1/Classes/AsXML.h"
#include "Flash/Action/Avm1/Classes/AsXMLNode.h"

// flash.geom
#include "Flash/Action/Avm1/Classes/As_flash_geom_ColorTransform.h"
#include "Flash/Action/Avm1/Classes/As_flash_geom_Point.h"
#include "Flash/Action/Avm1/Classes/As_flash_geom_Rectangle.h"
#include "Flash/Action/Avm1/Classes/As_flash_geom_Transform.h"

// mx.transitions
#include "Flash/Action/Avm1/Classes/As_mx_transitions_Tween.h"

// mx.transitions.easing
#include "Flash/Action/Avm1/Classes/As_mx_transitions_easing_Back.h"
#include "Flash/Action/Avm1/Classes/As_mx_transitions_easing_Bounce.h"
#include "Flash/Action/Avm1/Classes/As_mx_transitions_easing_Elastic.h"
#include "Flash/Action/Avm1/Classes/As_mx_transitions_easing_None.h"
#include "Flash/Action/Avm1/Classes/As_mx_transitions_easing_Regular.h"
#include "Flash/Action/Avm1/Classes/As_mx_transitions_easing_Strong.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.ActionGlobal", ActionGlobal, ActionObject)

ActionGlobal::ActionGlobal()
:	ActionObject(AsObject::getInstance())
{
	setMember(L"ASSetPropFlags", ActionValue(createNativeFunction(this, &ActionGlobal::Global_ASSetPropFlags)));
	setMember(L"escape", ActionValue(createNativeFunction(this, &ActionGlobal::Global_escape)));

	// Create prototypes.
	setMember(L"Object", ActionValue(AsObject::getInstance()));
	setMember(L"Accessibility", ActionValue(AsAccessibility::getInstance()));
	setMember(L"Array", ActionValue(AsArray::getInstance()));
	setMember(L"Boolean", ActionValue(AsBoolean::getInstance()));
	setMember(L"Button", ActionValue(AsButton::getInstance()));
	setMember(L"Date", ActionValue(AsDate::getInstance()));
	setMember(L"Error", ActionValue(AsError::getInstance()));
	setMember(L"Function", ActionValue(AsFunction::getInstance()));
	setMember(L"I18N", ActionValue(AsI18N::getInstance()));
	setMember(L"Key", ActionValue(AsKey::createInstance()));
	setMember(L"LoadVars", ActionValue(AsLoadVars::getInstance()));
	setMember(L"LocalConnection", ActionValue(AsLocalConnection::getInstance()));
	setMember(L"Math", ActionValue(AsMath::getInstance()));
	setMember(L"Mouse", ActionValue(AsMouse::createInstance()));
	setMember(L"MovieClip", ActionValue(AsMovieClip::getInstance()));
	setMember(L"MovieClipLoader", ActionValue(AsMovieClipLoader::getInstance()));
	setMember(L"Number", ActionValue(AsMath::getInstance()));
	setMember(L"Security", ActionValue(AsSecurity::getInstance()));
	setMember(L"Sound", ActionValue(AsSound::getInstance()));
	setMember(L"Stage", ActionValue(AsStage::getInstance()));
	setMember(L"String", ActionValue(AsString::getInstance()));
	setMember(L"System", ActionValue(AsSystem::getInstance()));
	setMember(L"TextField", ActionValue(AsTextField::getInstance()));
	setMember(L"XML", ActionValue(AsXML::getInstance()));
	setMember(L"XMLNode", ActionValue(AsXMLNode::getInstance()));

	// flash.
	Ref< ActionObject > flash = new ActionObject(AsObject::getInstance());
	{
		// flash.geom.
		Ref< ActionObject > geom = new ActionObject(AsObject::getInstance());
		{
			geom->setMember(L"ColorTransform", ActionValue(As_flash_geom_ColorTransform::getInstance()));
			geom->setMember(L"Point", ActionValue(As_flash_geom_Point::getInstance()));
			geom->setMember(L"Rectangle", ActionValue(As_flash_geom_Rectangle::getInstance()));
			geom->setMember(L"Transform", ActionValue(As_flash_geom_Transform::getInstance()));
		}
		flash->setMember(L"geom", ActionValue(geom));
	}
	setMember(L"flash", ActionValue(flash));

	// mx.
	Ref< ActionObject > mx = new ActionObject(AsObject::getInstance());
	{
		// mx.transitions.
		Ref< ActionObject > transitions = new ActionObject(AsObject::getInstance());
		if (transitions)
		{
			transitions->setMember(L"Tween", ActionValue(As_mx_transitions_Tween::getInstance()));

			// mx.transitions.easing
			Ref< ActionObject > easing = new ActionObject(AsObject::getInstance());
			if (easing)
			{
				easing->setMember(L"Back", ActionValue(As_mx_transitions_easing_Back::getInstance()));
				easing->setMember(L"Bounce", ActionValue(As_mx_transitions_easing_Bounce::getInstance()));
				easing->setMember(L"Elastic", ActionValue(As_mx_transitions_easing_Elastic::getInstance()));
				easing->setMember(L"None", ActionValue(As_mx_transitions_easing_None::getInstance()));
				easing->setMember(L"Regular", ActionValue(As_mx_transitions_easing_Regular::getInstance()));
				easing->setMember(L"String", ActionValue(As_mx_transitions_easing_Strong::getInstance()));

				transitions->setMember(L"easing", ActionValue(easing));
			}
		}
		mx->setMember(L"transitions", ActionValue(transitions));
	}
	setMember(L"mx", ActionValue(mx));
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

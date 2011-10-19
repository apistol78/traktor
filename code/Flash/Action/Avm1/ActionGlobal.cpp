#include <limits>
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
#include "Flash/Action/Avm1/Classes/AsTextFormat.h"
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

ActionGlobal::ActionGlobal(ActionContext* context)
{
	setMember("ASSetPropFlags", ActionValue(createNativeFunction(context, this, &ActionGlobal::Global_ASSetPropFlags)));
	setMember("escape", ActionValue(createNativeFunction(context, this, &ActionGlobal::Global_escape)));
	setMember("isNaN", ActionValue(createNativeFunction(context, this, &ActionGlobal::Global_isNaN)));

	// Create prototypes.
	setMember("Object", ActionValue(new AsObject(context)));
	setMember("Accessibility", ActionValue(new AsAccessibility(context)));
	setMember("Array", ActionValue(new AsArray(context)));
	setMember("Boolean", ActionValue(new AsBoolean(context)));
	setMember("Button", ActionValue(new AsButton(context)));
	setMember("Date", ActionValue(new AsDate(context)));
	setMember("Error", ActionValue(new AsError(context)));
	setMember("Function", ActionValue(new AsFunction(context)));
	setMember("I18N", ActionValue(new AsI18N(context)));
	setMember("Key", ActionValue(new AsKey(context)));
	setMember("LoadVars", ActionValue(new AsLoadVars(context)));
	setMember("LocalConnection", ActionValue(new AsLocalConnection(context)));
	setMember("Math", ActionValue(new AsMath(context)));
	setMember("Mouse", ActionValue(new AsMouse(context)));
	setMember("MovieClip", ActionValue(new AsMovieClip(context)));
	setMember("MovieClipLoader", ActionValue(new AsMovieClipLoader(context)));
	setMember("Number", ActionValue(new AsNumber(context)));
	setMember("Security", ActionValue(new AsSecurity(context)));
	setMember("Sound", ActionValue(new AsSound(context)));
	setMember("Stage", ActionValue(new AsStage(context)));
	setMember("String", ActionValue(new AsString(context)));
	setMember("System", ActionValue(new AsSystem(context)));
	setMember("TextField", ActionValue(new AsTextField(context)));
	setMember("TextFormat", ActionValue(new AsTextFormat(context)));
	setMember("XM", ActionValue(new AsXML(context)));
	setMember("XMLNode", ActionValue(new AsXMLNode(context)));

	// flash.
	Ref< ActionObject > flash = new ActionObject();
	{
		// flash.geom.
		Ref< ActionObject > geom = new ActionObject();
		{
			geom->setMember("ColorTransform", ActionValue(new As_flash_geom_ColorTransform(context)));
			geom->setMember("Point", ActionValue(new As_flash_geom_Point(context)));
			geom->setMember("Rectangle", ActionValue(new As_flash_geom_Rectangle(context)));
			geom->setMember("Transform", ActionValue(new As_flash_geom_Transform(context)));
		}
		flash->setMember("geom", ActionValue(geom));
	}
	setMember("flash", ActionValue(flash));

	// mx.
	Ref< ActionObject > mx = new ActionObject();
	{
		// mx.transitions.
		Ref< ActionObject > transitions = new ActionObject();
		if (transitions)
		{
			transitions->setMember("Tween", ActionValue(new As_mx_transitions_Tween(context)));

			// mx.transitions.easing
			Ref< ActionObject > easing = new ActionObject();
			if (easing)
			{
				easing->setMember("Back", ActionValue(new As_mx_transitions_easing_Back(context)));
				easing->setMember("Bounce", ActionValue(new As_mx_transitions_easing_Bounce(context)));
				easing->setMember("Elastic", ActionValue(new As_mx_transitions_easing_Elastic(context)));
				easing->setMember("None", ActionValue(new As_mx_transitions_easing_None(context)));
				easing->setMember("Regular", ActionValue(new As_mx_transitions_easing_Regular(context)));
				easing->setMember("String", ActionValue(new As_mx_transitions_easing_Strong(context)));

				transitions->setMember("easing", ActionValue(easing));
			}
		}
		mx->setMember("transitions", ActionValue(transitions));
	}
	setMember("mx", ActionValue(mx));
}

void ActionGlobal::Global_ASSetPropFlags(CallArgs& ca)
{
	Ref< ActionObject > object = ca.args[0].getObject();
	uint32_t flags = uint32_t(ca.args[2].getNumber());

	// Read-only; protect classes from being modified.
	if (flags & 4)
		object->setReadOnly();
}

void ActionGlobal::Global_escape(CallArgs& ca)
{
	std::wstring text = ca.args[0].getWideString();
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

void ActionGlobal::Global_isNaN(CallArgs& ca)
{
	bool nan = true;
	if (!ca.args.empty() && ca.args[0].isNumeric())
	{
		avm_number_t n = ca.args[0].getNumber();
		if (
			std::numeric_limits< avm_number_t >::quiet_NaN() == n ||
			std::numeric_limits< avm_number_t >::signaling_NaN() == n
		)
			nan = true;
		else
			nan = false;
	}
	ca.ret = ActionValue(nan);
}

	}
}

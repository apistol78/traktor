#define T_USE_NATIVE_TWEEN 1
#define T_USE_NATIVE_TWEEN_EASE	1

#include <limits>
#include "Core/Io/StringOutputStream.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/ActionGlobal.h"
#include "Flash/Action/Common/Classes/AsObject.h"
#include "Flash/Action/Common/Classes/AsAccessibility.h"
#include "Flash/Action/Common/Classes/AsArray.h"
#include "Flash/Action/Common/Classes/AsAsBroadcaster.h"
#include "Flash/Action/Common/Classes/AsBoolean.h"
#include "Flash/Action/Common/Classes/AsButton.h"
#include "Flash/Action/Common/Classes/AsDate.h"
#include "Flash/Action/Common/Classes/AsError.h"
#include "Flash/Action/Common/Classes/AsFunction.h"
#include "Flash/Action/Common/Classes/AsKey.h"
#include "Flash/Action/Common/Classes/AsLoadVars.h"
#include "Flash/Action/Common/Classes/AsLocalConnection.h"
#include "Flash/Action/Common/Classes/AsMath.h"
#include "Flash/Action/Common/Classes/AsMovieClip.h"
#include "Flash/Action/Common/Classes/AsMovieClipLoader.h"
#include "Flash/Action/Common/Classes/AsMouse.h"
#include "Flash/Action/Common/Classes/AsNumber.h"
#include "Flash/Action/Common/Classes/AsSecurity.h"
#include "Flash/Action/Common/Classes/AsSelection.h"
#include "Flash/Action/Common/Classes/AsStage.h"
#include "Flash/Action/Common/Classes/AsString.h"
#include "Flash/Action/Common/Classes/AsSystem.h"
#include "Flash/Action/Common/Classes/AsTextField.h"
#include "Flash/Action/Common/Classes/AsTextFormat.h"
#include "Flash/Action/Common/Classes/AsXML.h"
#include "Flash/Action/Common/Classes/AsXMLNode.h"

// flash.display
#include "Flash/Action/Common/Classes/As_flash_display_BitmapData.h"
#include "Flash/Action/Common/Classes/As_flash_display_DisplayObject.h"
#include "Flash/Action/Common/Classes/As_flash_display_DisplayObjectContainer.h"
#include "Flash/Action/Common/Classes/As_flash_display_InteractiveObject.h"
#include "Flash/Action/Common/Classes/As_flash_display_MovieClip.h"
#include "Flash/Action/Common/Classes/As_flash_display_Sprite.h"

// flash.events
#include "Flash/Action/Common/Classes/As_flash_events_EventDispatcher.h"

// flash.external
#include "Flash/Action/Common/Classes/As_flash_external_ExternalInterface.h"

// flash.filters
#include "Flash/Action/Common/Classes/As_flash_filters_BitmapFilter.h"
#include "Flash/Action/Common/Classes/As_flash_filters_BlurFilter.h"

// flash.geom
#include "Flash/Action/Common/Classes/As_flash_geom_ColorTransform.h"
#include "Flash/Action/Common/Classes/As_flash_geom_Matrix.h"
#include "Flash/Action/Common/Classes/As_flash_geom_Point.h"
#include "Flash/Action/Common/Classes/As_flash_geom_Rectangle.h"
#include "Flash/Action/Common/Classes/As_flash_geom_Transform.h"

#if T_USE_NATIVE_TWEEN
// mx.transitions
#	include "Flash/Action/Common/Classes/As_mx_transitions_Tween.h"
#endif

#if T_USE_NATIVE_TWEEN_EASE
// mx.transitions.easing
#	include "Flash/Action/Common/Classes/As_mx_transitions_easing_Back.h"
#	include "Flash/Action/Common/Classes/As_mx_transitions_easing_Bounce.h"
#	include "Flash/Action/Common/Classes/As_mx_transitions_easing_Elastic.h"
#	include "Flash/Action/Common/Classes/As_mx_transitions_easing_None.h"
#	include "Flash/Action/Common/Classes/As_mx_transitions_easing_Regular.h"
#	include "Flash/Action/Common/Classes/As_mx_transitions_easing_Strong.h"
#endif

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.ActionGlobal", ActionGlobal, ActionObject)

ActionGlobal::ActionGlobal(ActionContext* context)
:	ActionObject(context)
{
	setMember("ASSetPropFlags", ActionValue(createNativeFunction(context, this, &ActionGlobal::Global_ASSetPropFlags)));
	setMember("escape", ActionValue(createNativeFunction(context, this, &ActionGlobal::Global_escape)));
	setMember("isNaN", ActionValue(createNativeFunction(context, this, &ActionGlobal::Global_isNaN)));
	setMember("Infinity", ActionValue(std::numeric_limits< float >::infinity()));

	// Create prototypes.
	setMember("Object", ActionValue(new AsObject(context)));
	setMember("Accessibility", ActionValue(new AsAccessibility(context)));
	setMember("Array", ActionValue(new AsArray(context)));
	setMember("Boolean", ActionValue(new AsBoolean(context)));
	setMember("Button", ActionValue(new AsButton(context)));
	setMember("Date", ActionValue(new AsDate(context)));
	setMember("Error", ActionValue(new AsError(context)));
	setMember("Function", ActionValue(new AsFunction(context)));
	setMember("LoadVars", ActionValue(new AsLoadVars(context)));
	setMember("LocalConnection", ActionValue(new AsLocalConnection(context)));
	setMember("Math", ActionValue(new AsMath(context)));
	setMember("MovieClip", ActionValue(new AsMovieClip(context)));
	setMember("MovieClipLoader", ActionValue(new AsMovieClipLoader(context)));
	setMember("Number", ActionValue(new AsNumber(context)));
	setMember("Security", ActionValue(new AsSecurity(context)));
	setMember("String", ActionValue(new AsString(context)));
	setMember("System", ActionValue(new AsSystem(context)));
	setMember("TextField", ActionValue(new AsTextField(context)));
	setMember("TextFormat", ActionValue(new AsTextFormat(context)));
	setMember("XML", ActionValue(new AsXML(context)));
	setMember("XMLNode", ActionValue(new AsXMLNode(context)));

	// flash.
	Ref< ActionObject > flash = new ActionObject(context);
	{
		// flash.display.
		Ref< ActionObject > display = new ActionObject(context);
		{
			display->setMember("BitmapData", ActionValue(new As_flash_display_BitmapData(context)));
			display->setMember("DisplayObject", ActionValue(new As_flash_display_DisplayObject(context)));
			display->setMember("DisplayObjectContainer", ActionValue(new As_flash_display_DisplayObjectContainer(context)));
			display->setMember("InteractiveObject", ActionValue(new As_flash_display_InteractiveObject(context)));
			display->setMember("MovieClip", ActionValue(new As_flash_display_MovieClip(context)));
			display->setMember("Sprite", ActionValue(new As_flash_display_Sprite(context)));
		}
		flash->setMember("display", ActionValue(display));

		// flash.events.
		Ref< ActionObject > events = new ActionObject(context);
		{
			events->setMember("EventDispatcher", ActionValue(new As_flash_events_EventDispatcher(context)));
		}
		flash->setMember("events", ActionValue(events));

		// flash.external.
		Ref< ActionObject > external = new ActionObject(context);
		{
			external->setMember("ExternalInterface", ActionValue(new As_flash_external_ExternalInterface(context)));
		}
		flash->setMember("external", ActionValue(external));

		// flash.filters.
		Ref< ActionObject > filters = new ActionObject(context);
		{
			filters->setMember("BitmapFilter", ActionValue(new As_flash_filters_BitmapFilter(context)));
			filters->setMember("BlurFilter", ActionValue(new As_flash_filters_BlurFilter(context)));
		}
		flash->setMember("filters", ActionValue(filters));

		// flash.geom.
		Ref< ActionObject > geom = new ActionObject(context);
		{
			geom->setMember("ColorTransform", ActionValue(new As_flash_geom_ColorTransform(context)));
			geom->setMember("Matrix", ActionValue(new As_flash_geom_Matrix(context)));
			geom->setMember("Point", ActionValue(new As_flash_geom_Point(context)));
			geom->setMember("Rectangle", ActionValue(new As_flash_geom_Rectangle(context)));
			geom->setMember("Transform", ActionValue(new As_flash_geom_Transform(context)));
		}
		flash->setMember("geom", ActionValue(geom));
	}
	setMember("flash", ActionValue(flash));

#if T_USE_NATIVE_TWEEN || T_USE_NATIVE_TWEEN_EASE
	// mx.
	Ref< ActionObject > mx = new ActionObject(context);
	{
		// mx.transitions.
		Ref< ActionObject > transitions = new ActionObject(context);
		if (transitions)
		{
#	if T_USE_NATIVE_TWEEN
			transitions->setMember("Tween", ActionValue(new As_mx_transitions_Tween(context)));
#	endif

#	if T_USE_NATIVE_TWEEN_EASE
			// mx.transitions.easing
			Ref< ActionObject > easing = new ActionObject(context);
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
#	endif
		}
		mx->setMember("transitions", ActionValue(transitions));
	}
	setMember("mx", ActionValue(mx));
#endif

	// Initialize broadcaster and subject instances.
	Ref< AsAsBroadcaster > broadcaster = new AsAsBroadcaster(context);

	Ref< AsKey > key = new AsKey(context);
	broadcaster->initializeSubject(key, false);
	Ref< AsMouse > mouse = new AsMouse(context);
	broadcaster->initializeSubject(mouse, false);
	Ref< AsSelection > selection = new AsSelection(context);
	broadcaster->initializeSubject(selection, false);
	Ref< AsStage > stage = new AsStage(context);
	broadcaster->initializeSubject(stage, false);

	setMember("AsBroadcaster", ActionValue(broadcaster));
	setMember("Key", ActionValue(key));
	setMember("Mouse", ActionValue(mouse));
	setMember("Selection", ActionValue(selection));
	setMember("Stage", ActionValue(stage));
}

void ActionGlobal::Global_ASSetPropFlags(CallArgs& ca)
{
	Ref< ActionObject > object = ca.args[0].getObject();
	uint32_t flags = uint32_t(ca.args[2].getInteger());

	// Read-only; protect classes from being modified.
	if (object != 0 && flags & 4)
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
		float n = ca.args[0].getFloat();
		if (n != n)
			nan = true;
		else
			nan = false;
	}
	ca.ret = ActionValue(nan);
}

	}
}

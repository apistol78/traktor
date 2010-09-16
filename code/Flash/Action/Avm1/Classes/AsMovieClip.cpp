#include "Core/Io/FileSystem.h"
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Flash/FlashSpriteInstance.h"
#include "Flash/FlashSprite.h"
#include "Flash/FlashMovie.h"
#include "Flash/FlashMovieFactory.h"
#include "Flash/FlashFrame.h"
#include "Flash/SwfReader.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Classes/Transform.h"
#include "Flash/Action/Avm1/Classes/AsMovieClip.h"

namespace traktor
{
	namespace flash
	{
		namespace
		{

float polarAngle(float x, float y)
{
	if (x == 0.0f && y == 0.0f)
		return 0.0f;
	
	float r = sqrtf(x * x + y * y);

	if (x > 0.0f)
		return asinf(y / r);
	else
		return -asinf(y / r) + PI;
}

void decomposeTransform(const Matrix33& transform, Vector2& outTranslate, Vector2& outScale, float& outRotation)
{
	outTranslate.x = transform.e13;
	outTranslate.y = transform.e23;

	outScale.x = Vector2(transform.e11, transform.e12).length();
	outScale.y = Vector2(transform.e21, transform.e22).length();

	outRotation = polarAngle(transform.e11, transform.e12);
}

Matrix33 composeTransform(const Vector2& translate_, const Vector2& scale_, float rotate_)
{
	return
		translate(translate_.x, translate_.y) *
		scale(scale_.x, scale_.y) *
		rotate(rotate_);
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsMovieClip", AsMovieClip, ActionClass)

AsMovieClip::AsMovieClip()
:	ActionClass(L"MovieClip")
{
	Ref< ActionObject > prototype = new ActionObject();

	prototype->setMember(L"attachAudio", ActionValue(createNativeFunction(this, &AsMovieClip::MovieClip_attachAudio)));
	prototype->setMember(L"attachBitmap", ActionValue(createNativeFunction(this, &AsMovieClip::MovieClip_attachBitmap)));
	prototype->setMember(L"attachMovie", ActionValue(createNativeFunction(this, &AsMovieClip::MovieClip_attachMovie)));
	prototype->setMember(L"beginBitmapFill", ActionValue(createNativeFunction(this, &AsMovieClip::MovieClip_beginBitmapFill)));
	prototype->setMember(L"beginFill", ActionValue(createNativeFunction(this, &AsMovieClip::MovieClip_beginFill)));
	prototype->setMember(L"beginGradientFill", ActionValue(createNativeFunction(this, &AsMovieClip::MovieClip_beginGradientFill)));
	prototype->setMember(L"clear", ActionValue(createNativeFunction(this, &AsMovieClip::MovieClip_clear)));
	prototype->setMember(L"createEmptyMovieClip", ActionValue(createNativeFunction(this, &AsMovieClip::MovieClip_createEmptyMovieClip)));
	prototype->setMember(L"createTextField", ActionValue(createNativeFunction(this, &AsMovieClip::MovieClip_createTextField)));
	prototype->setMember(L"curveTo", ActionValue(createNativeFunction(this, &AsMovieClip::MovieClip_curveTo)));
	prototype->setMember(L"duplicateMovieClip", ActionValue(createNativeFunction(this, &AsMovieClip::MovieClip_duplicateMovieClip)));
	prototype->setMember(L"endFill", ActionValue(createNativeFunction(this, &AsMovieClip::MovieClip_endFill)));
	prototype->setMember(L"getBounds", ActionValue(createNativeFunction(this, &AsMovieClip::MovieClip_getBounds)));
	prototype->setMember(L"getBytesLoaded", ActionValue(createNativeFunction(this, &AsMovieClip::MovieClip_getBytesLoaded)));
	prototype->setMember(L"getBytesTotal", ActionValue(createNativeFunction(this, &AsMovieClip::MovieClip_getBytesTotal)));
	prototype->setMember(L"getDepth", ActionValue(createNativeFunction(this, &AsMovieClip::MovieClip_getDepth)));
	prototype->setMember(L"getInstanceAtDepth", ActionValue(createNativeFunction(this, &AsMovieClip::MovieClip_getInstanceAtDepth)));
	prototype->setMember(L"getNextHighestDepth", ActionValue(createNativeFunction(this, &AsMovieClip::MovieClip_getNextHighestDepth)));
	prototype->setMember(L"getRect", ActionValue(createNativeFunction(this, &AsMovieClip::MovieClip_getRect)));
	prototype->setMember(L"getSWFVersion", ActionValue(createNativeFunction(this, &AsMovieClip::MovieClip_getSWFVersion)));
	prototype->setMember(L"getTextSnapshot", ActionValue(createNativeFunction(this, &AsMovieClip::MovieClip_getTextSnapshot)));
	prototype->setMember(L"getURL", ActionValue(createNativeFunction(this, &AsMovieClip::MovieClip_getURL)));
	prototype->setMember(L"globalToLocal", ActionValue(createNativeFunction(this, &AsMovieClip::MovieClip_globalToLocal)));
	prototype->setMember(L"gotoAndPlay", ActionValue(createNativeFunction(this, &AsMovieClip::MovieClip_gotoAndPlay)));
	prototype->setMember(L"gotoAndStop", ActionValue(createNativeFunction(this, &AsMovieClip::MovieClip_gotoAndStop)));
	prototype->setMember(L"hitTest", ActionValue(createNativeFunction(this, &AsMovieClip::MovieClip_hitTest)));
	prototype->setMember(L"lineGradientStyle", ActionValue(createNativeFunction(this, &AsMovieClip::MovieClip_lineGradientStyle)));
	prototype->setMember(L"lineStyle", ActionValue(createNativeFunction(this, &AsMovieClip::MovieClip_lineStyle)));
	prototype->setMember(L"lineTo", ActionValue(createNativeFunction(this, &AsMovieClip::MovieClip_lineTo)));
	prototype->setMember(L"loadMovie", ActionValue(createNativeFunction(this, &AsMovieClip::MovieClip_loadMovie)));
	prototype->setMember(L"loadVariables", ActionValue(createNativeFunction(this, &AsMovieClip::MovieClip_loadVariables)));
	prototype->setMember(L"localToGlobal", ActionValue(createNativeFunction(this, &AsMovieClip::MovieClip_localToGlobal)));
	prototype->setMember(L"moveTo", ActionValue(createNativeFunction(this, &AsMovieClip::MovieClip_moveTo)));
	prototype->setMember(L"nextFrame", ActionValue(createNativeFunction(this, &AsMovieClip::MovieClip_nextFrame)));
	prototype->setMember(L"play", ActionValue(createNativeFunction(this, &AsMovieClip::MovieClip_play)));
	prototype->setMember(L"prevFrame", ActionValue(createNativeFunction(this, &AsMovieClip::MovieClip_prevFrame)));
	prototype->setMember(L"removeMovieClip", ActionValue(createNativeFunction(this, &AsMovieClip::MovieClip_removeMovieClip)));
	prototype->setMember(L"setMask", ActionValue(createNativeFunction(this, &AsMovieClip::MovieClip_setMask)));
	prototype->setMember(L"startDrag", ActionValue(createNativeFunction(this, &AsMovieClip::MovieClip_startDrag)));
	prototype->setMember(L"stop", ActionValue(createNativeFunction(this, &AsMovieClip::MovieClip_stop)));
	prototype->setMember(L"stopDrag", ActionValue(createNativeFunction(this, &AsMovieClip::MovieClip_stopDrag)));
	prototype->setMember(L"swapDepths", ActionValue(createNativeFunction(this, &AsMovieClip::MovieClip_swapDepths)));
	prototype->setMember(L"unloadMovie", ActionValue(createNativeFunction(this, &AsMovieClip::MovieClip_unloadMovie)));

	prototype->addProperty(L"_alpha", createNativeFunction(this, &AsMovieClip::MovieClip_get_alpha), createNativeFunction(this, &AsMovieClip::MovieClip_set_alpha));
	prototype->addProperty(L"blendMode", createNativeFunction(this, &AsMovieClip::MovieClip_get_blendMode), createNativeFunction(this, &AsMovieClip::MovieClip_set_blendMode));
	prototype->addProperty(L"cacheAsBitmap", createNativeFunction(this, &AsMovieClip::MovieClip_get_cacheAsBitmap), createNativeFunction(this, &AsMovieClip::MovieClip_set_cacheAsBitmap));
	prototype->addProperty(L"_currentframe", createNativeFunction(this, &AsMovieClip::MovieClip_get_currentframe), createNativeFunction(this, &AsMovieClip::MovieClip_set_currentframe));
	prototype->addProperty(L"_droptarget", createNativeFunction(this, &AsMovieClip::MovieClip_get_droptarget), createNativeFunction(this, &AsMovieClip::MovieClip_set_droptarget));
	prototype->addProperty(L"enabled", createNativeFunction(this, &AsMovieClip::MovieClip_get_enabled), createNativeFunction(this, &AsMovieClip::MovieClip_set_enabled));
	prototype->addProperty(L"filters", createNativeFunction(this, &AsMovieClip::MovieClip_get_filters), createNativeFunction(this, &AsMovieClip::MovieClip_set_filters));
	prototype->addProperty(L"focusEnabled", createNativeFunction(this, &AsMovieClip::MovieClip_get_focusEnabled), createNativeFunction(this, &AsMovieClip::MovieClip_set_focusEnabled));
	prototype->addProperty(L"_focusrect", createNativeFunction(this, &AsMovieClip::MovieClip_get_focusrect), createNativeFunction(this, &AsMovieClip::MovieClip_set_focusrect));
	prototype->addProperty(L"forceSmoothing", createNativeFunction(this, &AsMovieClip::MovieClip_get_forceSmoothing), createNativeFunction(this, &AsMovieClip::MovieClip_set_forceSmoothing));
	prototype->addProperty(L"_framesloaded", createNativeFunction(this, &AsMovieClip::MovieClip_get_framesloaded), createNativeFunction(this, &AsMovieClip::MovieClip_set_framesloaded));
	prototype->addProperty(L"_height", createNativeFunction(this, &AsMovieClip::MovieClip_get_height), createNativeFunction(this, &AsMovieClip::MovieClip_set_height));
	prototype->addProperty(L"_highquality", createNativeFunction(this, &AsMovieClip::MovieClip_get_highquality), createNativeFunction(this, &AsMovieClip::MovieClip_set_highquality));
	prototype->addProperty(L"hitArea", createNativeFunction(this, &AsMovieClip::MovieClip_get_hitArea), createNativeFunction(this, &AsMovieClip::MovieClip_set_hitArea));
	prototype->addProperty(L"_lockroot", createNativeFunction(this, &AsMovieClip::MovieClip_get_lockroot), createNativeFunction(this, &AsMovieClip::MovieClip_set_lockroot));
	prototype->addProperty(L"menu", createNativeFunction(this, &AsMovieClip::MovieClip_get_menu), createNativeFunction(this, &AsMovieClip::MovieClip_set_menu));
	prototype->addProperty(L"_name", createNativeFunction(this, &AsMovieClip::MovieClip_get_name), createNativeFunction(this, &AsMovieClip::MovieClip_set_name));
	prototype->addProperty(L"opaqueBackground", createNativeFunction(this, &AsMovieClip::MovieClip_get_opaqueBackground), createNativeFunction(this, &AsMovieClip::MovieClip_set_opaqueBackground));
	prototype->addProperty(L"_parent", createNativeFunction(this, &AsMovieClip::MovieClip_get_parent), createNativeFunction(this, &AsMovieClip::MovieClip_set_parent));
	prototype->addProperty(L"_quality", createNativeFunction(this, &AsMovieClip::MovieClip_get_quality), createNativeFunction(this, &AsMovieClip::MovieClip_set_quality));
	prototype->addProperty(L"_rotation", createNativeFunction(this, &AsMovieClip::MovieClip_get_rotation), createNativeFunction(this, &AsMovieClip::MovieClip_set_rotation));
	prototype->addProperty(L"scale9Grid", createNativeFunction(this, &AsMovieClip::MovieClip_get_scale9Grid), createNativeFunction(this, &AsMovieClip::MovieClip_set_scale9Grid));
	prototype->addProperty(L"scrollRect", createNativeFunction(this, &AsMovieClip::MovieClip_get_scrollRect), createNativeFunction(this, &AsMovieClip::MovieClip_set_scrollRect));
	prototype->addProperty(L"_soundbuftime", createNativeFunction(this, &AsMovieClip::MovieClip_get_soundbuftime), createNativeFunction(this, &AsMovieClip::MovieClip_set_soundbuftime));
	prototype->addProperty(L"tabChildren", createNativeFunction(this, &AsMovieClip::MovieClip_get_tabChildren), createNativeFunction(this, &AsMovieClip::MovieClip_set_tabChildren));
	prototype->addProperty(L"tabEnabled", createNativeFunction(this, &AsMovieClip::MovieClip_get_tabEnabled), createNativeFunction(this, &AsMovieClip::MovieClip_set_tabEnabled));
	prototype->addProperty(L"tabIndex", createNativeFunction(this, &AsMovieClip::MovieClip_get_tabIndex), createNativeFunction(this, &AsMovieClip::MovieClip_set_tabIndex));
	prototype->addProperty(L"_target", createNativeFunction(this, &AsMovieClip::MovieClip_get_target), createNativeFunction(this, &AsMovieClip::MovieClip_set_target));
	prototype->addProperty(L"_totalframes", createNativeFunction(this, &AsMovieClip::MovieClip_get_totalframes), createNativeFunction(this, &AsMovieClip::MovieClip_set_totalframes));
	prototype->addProperty(L"trackAsMenu", createNativeFunction(this, &AsMovieClip::MovieClip_get_trackAsMenu), createNativeFunction(this, &AsMovieClip::MovieClip_set_trackAsMenu));
	prototype->addProperty(L"transform", createNativeFunction(this, &AsMovieClip::MovieClip_get_transform), 0);
	prototype->addProperty(L"_url", createNativeFunction(this, &AsMovieClip::MovieClip_get_url), createNativeFunction(this, &AsMovieClip::MovieClip_set_url));
	prototype->addProperty(L"useHandCursor", createNativeFunction(this, &AsMovieClip::MovieClip_get_useHandCursor), createNativeFunction(this, &AsMovieClip::MovieClip_set_useHandCursor));
	prototype->addProperty(L"_visible", createNativeFunction(this, &AsMovieClip::MovieClip_get_visible), createNativeFunction(this, &AsMovieClip::MovieClip_set_visible));
	prototype->addProperty(L"_width", createNativeFunction(this, &AsMovieClip::MovieClip_get_width), createNativeFunction(this, &AsMovieClip::MovieClip_set_width));
	prototype->addProperty(L"_x", createNativeFunction(this, &AsMovieClip::MovieClip_get_x), createNativeFunction(this, &AsMovieClip::MovieClip_set_x));
	prototype->addProperty(L"_xmouse", createNativeFunction(this, &AsMovieClip::MovieClip_get_xmouse), createNativeFunction(this, &AsMovieClip::MovieClip_set_xmouse));
	prototype->addProperty(L"_xscale", createNativeFunction(this, &AsMovieClip::MovieClip_get_xscale), createNativeFunction(this, &AsMovieClip::MovieClip_set_xscale));
	prototype->addProperty(L"_y", createNativeFunction(this, &AsMovieClip::MovieClip_get_y), createNativeFunction(this, &AsMovieClip::MovieClip_set_y));
	prototype->addProperty(L"_ymouse", createNativeFunction(this, &AsMovieClip::MovieClip_get_ymouse), createNativeFunction(this, &AsMovieClip::MovieClip_set_ymouse));
	prototype->addProperty(L"_yscale", createNativeFunction(this, &AsMovieClip::MovieClip_get_yscale), createNativeFunction(this, &AsMovieClip::MovieClip_set_yscale));

	prototype->setReadOnly();

	setMember(L"prototype", ActionValue(prototype));
}

ActionValue AsMovieClip::construct(ActionContext* context, const ActionValueArray& args)
{
	// Create a fake sprite character for this instance.
	Ref< FlashSprite > sprite = new FlashSprite(0, 0);
	sprite->addFrame(new FlashFrame());
	return ActionValue(sprite->createInstance(context, 0));
}

void AsMovieClip::MovieClip_attachAudio(CallArgs& ca)
{
}

void AsMovieClip::MovieClip_attachBitmap(CallArgs& ca)
{
}

void AsMovieClip::MovieClip_attachMovie(CallArgs& ca)
{
	FlashSpriteInstance* movieClipInstance = checked_type_cast< FlashSpriteInstance*, false >(ca.self);
	if (ca.args.size() < 3)
	{
		log::error << L"MovieClip.attachMovie, incorrect number of arguments" << Endl;
		return;
	}

	std::wstring attachClipName = ca.args[0].getStringSafe();
	std::wstring attachClipNewName = ca.args[1].getStringSafe();
	int32_t depth = int32_t(ca.args[2].getNumberSafe());

	// Get root movie.
	const FlashMovie* movie = ca.context->getMovie();

	// Get movie clip ID from name.
	uint16_t attachClipId;
	if (!movie->getExportId(attachClipName, attachClipId))
	{
		log::error << L"MovieClip.attachMovie, no such movie clip exported (" << attachClipName << L")" << Endl;
		return;
	}

	// Get movie clip character.
	Ref< const FlashSprite > attachClip = dynamic_type_cast< const FlashSprite* >(movie->getCharacter(attachClipId));
	if (!attachClip)
	{
		log::error << L"MovieClip.attachMovie, no movie clip with id " << attachClipId << L" defined" << Endl;
		return;
	}

	// Create new instance of movie clip.
	Ref< FlashSpriteInstance > attachClipInstance = checked_type_cast< FlashSpriteInstance* >(attachClip->createInstance(ca.context, movieClipInstance));
	attachClipInstance->setName(attachClipNewName);
	
	// Add new instance to display list.
	FlashDisplayList& displayList = movieClipInstance->getDisplayList();
	displayList.showObject(depth, attachClipId, attachClipInstance, true);

	ca.ret = ActionValue(attachClipInstance.ptr());
}

void AsMovieClip::MovieClip_beginBitmapFill(CallArgs& ca)
{
}

void AsMovieClip::MovieClip_beginFill(CallArgs& ca)
{
}

void AsMovieClip::MovieClip_beginGradientFill(CallArgs& ca)
{
}

void AsMovieClip::MovieClip_clear(CallArgs& ca)
{
}

void AsMovieClip::MovieClip_createEmptyMovieClip(CallArgs& ca)
{
	FlashSpriteInstance* movieClipInstance = checked_type_cast< FlashSpriteInstance*, false >(ca.self);
	if (ca.args.size() < 2)
	{
		log::error << L"MovieClip.createEmptyMovieClip, incorrect number of arguments" << Endl;
		return;
	}

	std::wstring emptyClipName = ca.args[0].getStringSafe();
	int32_t depth = uint16_t(ca.args[1].getNumberSafe());

	// Get root movie.
	const FlashMovie* movie = ca.context->getMovie();

	// Create fake character ID.
	uint16_t emptyClipId = depth + 40000;

	// Create empty movie character with a single frame.
	Ref< FlashSprite > emptyClip = new FlashSprite(emptyClipId, 0);
	emptyClip->addFrame(new FlashFrame());

	// Create new instance of movie clip.
	Ref< FlashSpriteInstance > emptyClipInstance = checked_type_cast< FlashSpriteInstance* >(emptyClip->createInstance(ca.context, movieClipInstance));
	emptyClipInstance->setName(emptyClipName);

	// Add new instance to display list.
	FlashDisplayList& displayList = movieClipInstance->getDisplayList();
	displayList.showObject(depth, emptyClipId, emptyClipInstance, true);

	ca.ret = ActionValue(emptyClipInstance.ptr());
}

void AsMovieClip::MovieClip_createTextField(CallArgs& ca)
{
}

void AsMovieClip::MovieClip_curveTo(CallArgs& ca)
{
}

void AsMovieClip::MovieClip_duplicateMovieClip(CallArgs& ca)
{
	FlashSpriteInstance* movieClipInstance = checked_type_cast< FlashSpriteInstance*, false >(ca.self);
	Ref< FlashSpriteInstance > parentClipInstance = checked_type_cast< FlashSpriteInstance* >(movieClipInstance->getParent());

	Ref< FlashSpriteInstance > cloneClipInstance = movieClipInstance->clone();
	cloneClipInstance->setName(ca.args[0].getStringSafe());

	parentClipInstance->getDisplayList().showObject(
		int32_t(ca.args[1].getNumberSafe()),
		cloneClipInstance->getSprite()->getId(),
		cloneClipInstance,
		true
	);
}

void AsMovieClip::MovieClip_endFill(CallArgs& ca)
{
}

void AsMovieClip::MovieClip_getBounds(CallArgs& ca)
{
}

void AsMovieClip::MovieClip_getBytesLoaded(CallArgs& ca)
{
	ca.ret = ActionValue(avm_number_t(1));
}

void AsMovieClip::MovieClip_getBytesTotal(CallArgs& ca)
{
	ca.ret = ActionValue(avm_number_t(1));
}

void AsMovieClip::MovieClip_getDepth(CallArgs& ca)
{
	FlashSpriteInstance* movieClipInstance = checked_type_cast< FlashSpriteInstance*, false >(ca.self);
	Ref< FlashSpriteInstance > parentClipInstance = checked_type_cast< FlashSpriteInstance* >(movieClipInstance->getParent());
	if (parentClipInstance)
		ca.ret = ActionValue(avm_number_t(parentClipInstance->getDisplayList().getObjectDepth(movieClipInstance)));
}

void AsMovieClip::MovieClip_getInstanceAtDepth(CallArgs& ca)
{
}

void AsMovieClip::MovieClip_getNextHighestDepth(CallArgs& ca)
{
	FlashSpriteInstance* movieClipInstance = checked_type_cast< FlashSpriteInstance*, false >(ca.self);
	ca.ret = ActionValue(avm_number_t(movieClipInstance->getDisplayList().getNextHighestDepth()));
}

void AsMovieClip::MovieClip_getRect(CallArgs& ca)
{
}

void AsMovieClip::MovieClip_getSWFVersion(CallArgs& ca)
{
}

void AsMovieClip::MovieClip_getTextSnapshot(CallArgs& ca)
{
}

void AsMovieClip::MovieClip_getURL(CallArgs& ca)
{
}

void AsMovieClip::MovieClip_globalToLocal(CallArgs& ca)
{
	T_FATAL_ERROR;
}

void AsMovieClip::MovieClip_gotoAndPlay(CallArgs& ca)
{
	FlashSpriteInstance* movieClipInstance = checked_type_cast< FlashSpriteInstance*, false >(ca.self);
	if (ca.args[0].isNumeric())
		movieClipInstance->gotoFrame(uint32_t(ca.args[0].getNumber() - 1));
	else if (ca.args[0].isString())
	{
		int frame = movieClipInstance->getSprite()->findFrame(ca.args[0].getString());
		if (frame >= 0)
			movieClipInstance->gotoFrame(frame);
		else
			log::warning << L"No such frame, \"" << ca.args[0].getString() << L"\"" << Endl;
	}
	movieClipInstance->setPlaying(true);
}

void AsMovieClip::MovieClip_gotoAndStop(CallArgs& ca)
{
	FlashSpriteInstance* movieClipInstance = checked_type_cast< FlashSpriteInstance*, false >(ca.self);
	if (ca.args[0].isNumeric())
	{
		int32_t frame = int32_t(ca.args[0].getNumber());
		movieClipInstance->gotoFrame(max< int32_t >(frame - 1, 0));
	}
	else if (ca.args[0].isString())
	{
		int frame = movieClipInstance->getSprite()->findFrame(ca.args[0].getString());
		if (frame >= 0)
			movieClipInstance->gotoFrame(frame);
		else
			log::warning << L"No such frame, \"" << ca.args[0].getString() << L"\"" << Endl;
	}
	movieClipInstance->setPlaying(false);
}

void AsMovieClip::MovieClip_hitTest(CallArgs& ca)
{
	FlashSpriteInstance* movieClipInstance = checked_type_cast< FlashSpriteInstance*, false >(ca.self);

	if (ca.args.size() == 3)
	{
		avm_number_t x = ca.args[0].getNumberSafe() * 20.0f;
		avm_number_t y = ca.args[1].getNumberSafe() * 20.0f;
		bool shape = ca.args[2].getBooleanSafe();

		SwfRect bounds = movieClipInstance->getBounds();
		ca.ret = ActionValue(x >= bounds.min.x && y >= bounds.min.y && x <= bounds.max.x && y <= bounds.max.y);
	}
	else if (ca.args.size() == 1)
	{
		if (!ca.args[0].isObject())
		{
			ca.ret = ActionValue(false);
			return;
		}

		Ref< FlashSpriteInstance > shapeClipInstance = ca.args[0].getObject< FlashSpriteInstance >();
		if (!shapeClipInstance)
		{
			ca.ret = ActionValue(false);
			return;
		}

		SwfRect bounds = movieClipInstance->getBounds();
		SwfRect shapeBounds = shapeClipInstance->getBounds();

		ca.ret = ActionValue((bounds.min.x < shapeBounds.max.x) && (bounds.min.y < shapeBounds.max.y) && (bounds.max.x > shapeBounds.min.x) && (bounds.max.y > shapeBounds.min.y));
	}
}

void AsMovieClip::MovieClip_lineGradientStyle(CallArgs& ca)
{
}

void AsMovieClip::MovieClip_lineStyle(CallArgs& ca)
{
}

void AsMovieClip::MovieClip_lineTo(CallArgs& ca)
{
}

void AsMovieClip::MovieClip_loadMovie(CallArgs& ca)
{
	FlashSpriteInstance* movieClipInstance = checked_type_cast< FlashSpriteInstance*, false >(ca.self);

	std::wstring fileName = ca.args[0].getStringSafe();

	Ref< IStream > file = FileSystem::getInstance().open(fileName, File::FmRead);
	if (!file)
		return;

	Ref< SwfReader > swf = new SwfReader(file);
	Ref< FlashMovie > movie = flash::FlashMovieFactory().createMovie(swf);
	if (!movie)
		return;

	Ref< FlashSpriteInstance > externalMovieClipInstance = movie->createExternalMovieClipInstance(
		movieClipInstance
	);
}

void AsMovieClip::MovieClip_loadVariables(CallArgs& ca)
{
}

void AsMovieClip::MovieClip_localToGlobal(CallArgs& ca)
{
	T_FATAL_ERROR;
}

void AsMovieClip::MovieClip_moveTo(CallArgs& ca)
{
}

void AsMovieClip::MovieClip_nextFrame(CallArgs& ca)
{
	FlashSpriteInstance* movieClipInstance = checked_type_cast< FlashSpriteInstance*, false >(ca.self);
	movieClipInstance->gotoNext();
}

void AsMovieClip::MovieClip_play(CallArgs& ca)
{
	FlashSpriteInstance* movieClipInstance = checked_type_cast< FlashSpriteInstance*, false >(ca.self);
	movieClipInstance->setPlaying(true);
}

void AsMovieClip::MovieClip_prevFrame(CallArgs& ca)
{
	FlashSpriteInstance* movieClipInstance = checked_type_cast< FlashSpriteInstance*, false >(ca.self);
	movieClipInstance->gotoPrevious();
}

void AsMovieClip::MovieClip_removeMovieClip(CallArgs& ca)
{
	FlashSpriteInstance* movieClipInstance = checked_type_cast< FlashSpriteInstance*, false >(ca.self);
	FlashSpriteInstance* parentClipInstance = checked_type_cast< FlashSpriteInstance*, false >(movieClipInstance->getParent());
	FlashDisplayList& displayList = parentClipInstance->getDisplayList();
	displayList.removeObject(movieClipInstance);
}

void AsMovieClip::MovieClip_setMask(CallArgs& ca)
{
	FlashSpriteInstance* movieClipInstance = checked_type_cast< FlashSpriteInstance*, false >(ca.self);
	Ref< FlashSpriteInstance > maskInstance = ca.args[0].getObjectSafe< FlashSpriteInstance >();
	movieClipInstance->setMask(maskInstance);
}

void AsMovieClip::MovieClip_startDrag(CallArgs& ca)
{
}

void AsMovieClip::MovieClip_stop(CallArgs& ca)
{
	FlashSpriteInstance* movieClipInstance = checked_type_cast< FlashSpriteInstance*, false >(ca.self);
	movieClipInstance->setPlaying(false);
}

void AsMovieClip::MovieClip_stopDrag(CallArgs& ca)
{
}

void AsMovieClip::MovieClip_swapDepths(CallArgs& ca)
{
	FlashSpriteInstance* movieClipInstance = checked_type_cast< FlashSpriteInstance*, false >(ca.self);
	FlashSpriteInstance* parentClipInstance = checked_type_cast< FlashSpriteInstance*, false >(movieClipInstance->getParent());

	// Get my own current depth.
	int32_t depth = parentClipInstance->getDisplayList().getObjectDepth(movieClipInstance);

	// Get target clip depth.
	int32_t targetDepth = depth;

	if (ca.args[0].isNumeric())
		targetDepth = int32_t(ca.args[0].getNumber());
	else if (ca.args[0].isObject())
	{
		Ref< FlashSpriteInstance > targetClipInstance = ca.args[0].getObject< FlashSpriteInstance >();
		targetDepth = parentClipInstance->getDisplayList().getObjectDepth(targetClipInstance);
	}

	parentClipInstance->getDisplayList().swap(depth, targetDepth);
}

void AsMovieClip::MovieClip_unloadMovie(CallArgs& ca)
{
}

void AsMovieClip::MovieClip_get_alpha(CallArgs& ca)
{
	FlashSpriteInstance* movieClipInstance = checked_type_cast< FlashSpriteInstance*, false >(ca.self);
	const SwfCxTransform& colorTransform = movieClipInstance->getColorTransform();
	ca.ret = ActionValue(colorTransform.alpha[0] * 100.0f);
}

void AsMovieClip::MovieClip_set_alpha(CallArgs& ca)
{
	FlashSpriteInstance* movieClipInstance = checked_type_cast< FlashSpriteInstance*, false >(ca.self);
	SwfCxTransform colorTransform = movieClipInstance->getColorTransform();
	colorTransform.alpha[0] = ca.args[0].getNumberSafe() / 100.0f;
	movieClipInstance->setColorTransform(colorTransform);
}

void AsMovieClip::MovieClip_get_blendMode(CallArgs& ca)
{
}

void AsMovieClip::MovieClip_set_blendMode(CallArgs& ca)
{
}

void AsMovieClip::MovieClip_get_cacheAsBitmap(CallArgs& ca)
{
}

void AsMovieClip::MovieClip_set_cacheAsBitmap(CallArgs& ca)
{
}

void AsMovieClip::MovieClip_get_currentframe(CallArgs& ca)
{
	FlashSpriteInstance* movieClipInstance = checked_type_cast< FlashSpriteInstance*, false >(ca.self);
	ca.ret = ActionValue(avm_number_t(movieClipInstance->getCurrentFrame() + 1));
}

void AsMovieClip::MovieClip_set_currentframe(CallArgs& ca)
{
	FlashSpriteInstance* movieClipInstance = checked_type_cast< FlashSpriteInstance*, false >(ca.self);
	uint32_t frame = uint32_t(ca.args[0].getNumberSafe());
	if (frame > 0)
		movieClipInstance->gotoFrame(frame - 1);
}

void AsMovieClip::MovieClip_get_droptarget(CallArgs& ca)
{
}

void AsMovieClip::MovieClip_set_droptarget(CallArgs& ca)
{
}

void AsMovieClip::MovieClip_get_enabled(CallArgs& ca)
{
}

void AsMovieClip::MovieClip_set_enabled(CallArgs& ca)
{
}

void AsMovieClip::MovieClip_get_filters(CallArgs& ca)
{
}

void AsMovieClip::MovieClip_set_filters(CallArgs& ca)
{
}

void AsMovieClip::MovieClip_get_focusEnabled(CallArgs& ca)
{
}

void AsMovieClip::MovieClip_set_focusEnabled(CallArgs& ca)
{
}

void AsMovieClip::MovieClip_get_focusrect(CallArgs& ca)
{
}

void AsMovieClip::MovieClip_set_focusrect(CallArgs& ca)
{
}

void AsMovieClip::MovieClip_get_forceSmoothing(CallArgs& ca)
{
}

void AsMovieClip::MovieClip_set_forceSmoothing(CallArgs& ca)
{
}

void AsMovieClip::MovieClip_get_framesloaded(CallArgs& ca)
{
	FlashSpriteInstance* movieClipInstance = checked_type_cast< FlashSpriteInstance*, false >(ca.self);
	Ref< const FlashSprite > movieClip = movieClipInstance->getSprite();
	ca.ret = ActionValue(avm_number_t(movieClip->getFrameCount()));
}

void AsMovieClip::MovieClip_set_framesloaded(CallArgs& ca)
{
}

void AsMovieClip::MovieClip_get_height(CallArgs& ca)
{
	FlashSpriteInstance* movieClipInstance = checked_type_cast< FlashSpriteInstance*, false >(ca.self);
	
	SwfRect bounds = movieClipInstance->getBounds();
	float extent = (bounds.max.y - bounds.min.y) / 20.0f;

	ca.ret = ActionValue(extent);
}

void AsMovieClip::MovieClip_set_height(CallArgs& ca)
{
	FlashSpriteInstance* movieClipInstance = checked_type_cast< FlashSpriteInstance*, false >(ca.self);
	
	SwfRect bounds = movieClipInstance->getLocalBounds();
	float extent = (bounds.max.y - bounds.min.y) / 20.0f;
	if (abs(extent) <= FUZZY_EPSILON)
		return;

	Vector2 T, S; float R;
	decomposeTransform(movieClipInstance->getTransform(), T, S, R);

	S.y = float(ca.args[0].getNumberSafe()) / extent;

	movieClipInstance->setTransform(composeTransform(T, S, R));
}

void AsMovieClip::MovieClip_get_highquality(CallArgs& ca)
{
}

void AsMovieClip::MovieClip_set_highquality(CallArgs& ca)
{
}

void AsMovieClip::MovieClip_get_hitArea(CallArgs& ca)
{
}

void AsMovieClip::MovieClip_set_hitArea(CallArgs& ca)
{
}

void AsMovieClip::MovieClip_get_lockroot(CallArgs& ca)
{
}

void AsMovieClip::MovieClip_set_lockroot(CallArgs& ca)
{
}

void AsMovieClip::MovieClip_get_menu(CallArgs& ca)
{
}

void AsMovieClip::MovieClip_set_menu(CallArgs& ca)
{
}

void AsMovieClip::MovieClip_get_name(CallArgs& ca)
{
	FlashSpriteInstance* movieClipInstance = checked_type_cast< FlashSpriteInstance*, false >(ca.self);
	ca.ret = ActionValue(movieClipInstance->getName());
}

void AsMovieClip::MovieClip_set_name(CallArgs& ca)
{
	FlashSpriteInstance* movieClipInstance = checked_type_cast< FlashSpriteInstance*, false >(ca.self);
	movieClipInstance->setName(ca.args[0].getStringSafe());
}

void AsMovieClip::MovieClip_get_opaqueBackground(CallArgs& ca)
{
}

void AsMovieClip::MovieClip_set_opaqueBackground(CallArgs& ca)
{
}

void AsMovieClip::MovieClip_get_parent(CallArgs& ca)
{
	FlashSpriteInstance* movieClipInstance = checked_type_cast< FlashSpriteInstance*, false >(ca.self);
	ca.ret = ActionValue(movieClipInstance->getParent());
}

void AsMovieClip::MovieClip_set_parent(CallArgs& ca)
{
}

void AsMovieClip::MovieClip_get_quality(CallArgs& ca)
{
	ca.ret = ActionValue(L"BEST");
}

void AsMovieClip::MovieClip_set_quality(CallArgs& ca)
{
}

void AsMovieClip::MovieClip_get_rotation(CallArgs& ca)
{
	FlashSpriteInstance* movieClipInstance = checked_type_cast< FlashSpriteInstance*, false >(ca.self);

	Vector2 T, S; float R;
	decomposeTransform(movieClipInstance->getTransform(), T, S, R);

	ca.ret = ActionValue(rad2deg(R));
}

void AsMovieClip::MovieClip_set_rotation(CallArgs& ca)
{
	FlashSpriteInstance* movieClipInstance = checked_type_cast< FlashSpriteInstance*, false >(ca.self);
	float deg = float(ca.args[0].getNumberSafe());

	Vector2 T, S; float R;
	decomposeTransform(movieClipInstance->getTransform(), T, S, R);

	R = deg2rad(deg);

	movieClipInstance->setTransform(composeTransform(T, S, R));
}

void AsMovieClip::MovieClip_get_scale9Grid(CallArgs& ca)
{
}

void AsMovieClip::MovieClip_set_scale9Grid(CallArgs& ca)
{
}

void AsMovieClip::MovieClip_get_scrollRect(CallArgs& ca)
{
}

void AsMovieClip::MovieClip_set_scrollRect(CallArgs& ca)
{
}

void AsMovieClip::MovieClip_get_soundbuftime(CallArgs& ca)
{
}

void AsMovieClip::MovieClip_set_soundbuftime(CallArgs& ca)
{
}

void AsMovieClip::MovieClip_get_tabChildren(CallArgs& ca)
{
}

void AsMovieClip::MovieClip_set_tabChildren(CallArgs& ca)
{
}

void AsMovieClip::MovieClip_get_tabEnabled(CallArgs& ca)
{
}

void AsMovieClip::MovieClip_set_tabEnabled(CallArgs& ca)
{
}

void AsMovieClip::MovieClip_get_tabIndex(CallArgs& ca)
{
}

void AsMovieClip::MovieClip_set_tabIndex(CallArgs& ca)
{
}

void AsMovieClip::MovieClip_get_target(CallArgs& ca)
{
}

void AsMovieClip::MovieClip_set_target(CallArgs& ca)
{
}

void AsMovieClip::MovieClip_get_totalframes(CallArgs& ca)
{
}

void AsMovieClip::MovieClip_set_totalframes(CallArgs& ca)
{
}

void AsMovieClip::MovieClip_get_trackAsMenu(CallArgs& ca)
{
}

void AsMovieClip::MovieClip_set_trackAsMenu(CallArgs& ca)
{
}

void AsMovieClip::MovieClip_get_transform(CallArgs& ca)
{
	FlashSpriteInstance* movieClipInstance = checked_type_cast< FlashSpriteInstance*, false >(ca.self);
	ca.ret = ActionValue(new Transform(movieClipInstance));
}

void AsMovieClip::MovieClip_get_url(CallArgs& ca)
{
}

void AsMovieClip::MovieClip_set_url(CallArgs& ca)
{
}

void AsMovieClip::MovieClip_get_useHandCursor(CallArgs& ca)
{
}

void AsMovieClip::MovieClip_set_useHandCursor(CallArgs& ca)
{
}

void AsMovieClip::MovieClip_get_visible(CallArgs& ca)
{
	FlashSpriteInstance* movieClipInstance = checked_type_cast< FlashSpriteInstance*, false >(ca.self);
	ca.ret = ActionValue(movieClipInstance->isVisible());
}

void AsMovieClip::MovieClip_set_visible(CallArgs& ca)
{
	FlashSpriteInstance* movieClipInstance = checked_type_cast< FlashSpriteInstance*, false >(ca.self);
	movieClipInstance->setVisible(ca.args[0].toBoolean().getBoolean());
}

void AsMovieClip::MovieClip_get_width(CallArgs& ca)
{
	FlashSpriteInstance* movieClipInstance = checked_type_cast< FlashSpriteInstance*, false >(ca.self);

	SwfRect bounds = movieClipInstance->getBounds();
	float extent = (bounds.max.x - bounds.min.x) / 20.0f;

	ca.ret = ActionValue(extent);
}

void AsMovieClip::MovieClip_set_width(CallArgs& ca)
{
	FlashSpriteInstance* movieClipInstance = checked_type_cast< FlashSpriteInstance*, false >(ca.self);

	SwfRect bounds = movieClipInstance->getLocalBounds();
	float extent = (bounds.max.x - bounds.min.x) / 20.0f;
	if (abs(extent) <= FUZZY_EPSILON)
		return;

	Vector2 T, S; float R;
	decomposeTransform(movieClipInstance->getTransform(), T, S, R);

	S.x = float(ca.args[0].getNumberSafe()) / extent;

	movieClipInstance->setTransform(composeTransform(T, S, R));
}

void AsMovieClip::MovieClip_get_x(CallArgs& ca)
{
	FlashSpriteInstance* movieClipInstance = checked_type_cast< FlashSpriteInstance*, false >(ca.self);

	Vector2 T, S; float R;
	decomposeTransform(movieClipInstance->getTransform(), T, S, R);

	ca.ret = ActionValue(avm_number_t(T.x / 20.0));
}

void AsMovieClip::MovieClip_set_x(CallArgs& ca)
{
	FlashSpriteInstance* movieClipInstance = checked_type_cast< FlashSpriteInstance*, false >(ca.self);

	Vector2 T, S; float R;
	decomposeTransform(movieClipInstance->getTransform(), T, S, R);

	T.x = float(ca.args[0].getNumberSafe() * 20.0);

	movieClipInstance->setTransform(composeTransform(T, S, R));
}

void AsMovieClip::MovieClip_get_xmouse(CallArgs& ca)
{
	FlashSpriteInstance* movieClipInstance = checked_type_cast< FlashSpriteInstance*, false >(ca.self);
	ca.ret = ActionValue(avm_number_t(movieClipInstance->getMouseX()));
}

void AsMovieClip::MovieClip_set_xmouse(CallArgs& ca)
{
}

void AsMovieClip::MovieClip_get_xscale(CallArgs& ca)
{
	FlashSpriteInstance* movieClipInstance = checked_type_cast< FlashSpriteInstance*, false >(ca.self);
	
	Vector2 T, S; float R;
	decomposeTransform(movieClipInstance->getTransform(), T, S, R);
	
	ca.ret = ActionValue(avm_number_t(S.x * 100.0));
}

void AsMovieClip::MovieClip_set_xscale(CallArgs& ca)
{
	FlashSpriteInstance* movieClipInstance = checked_type_cast< FlashSpriteInstance*, false >(ca.self);

	Vector2 T, S; float R;
	decomposeTransform(movieClipInstance->getTransform(), T, S, R);

	S.x = float(ca.args[0].getNumberSafe() / 100.0);

	movieClipInstance->setTransform(composeTransform(T, S, R));
}

void AsMovieClip::MovieClip_get_y(CallArgs& ca)
{
	FlashSpriteInstance* movieClipInstance = checked_type_cast< FlashSpriteInstance*, false >(ca.self);

	Vector2 T, S; float R;
	decomposeTransform(movieClipInstance->getTransform(), T, S, R);

	ca.ret = ActionValue(avm_number_t(T.y / 20.0));
}

void AsMovieClip::MovieClip_set_y(CallArgs& ca)
{
	FlashSpriteInstance* movieClipInstance = checked_type_cast< FlashSpriteInstance*, false >(ca.self);

	Vector2 T, S; float R;
	decomposeTransform(movieClipInstance->getTransform(), T, S, R);

	T.y = float(ca.args[0].getNumberSafe() * 20.0);

	movieClipInstance->setTransform(composeTransform(T, S, R));
}

void AsMovieClip::MovieClip_get_ymouse(CallArgs& ca)
{
	FlashSpriteInstance* movieClipInstance = checked_type_cast< FlashSpriteInstance*, false >(ca.self);
	ca.ret = ActionValue(avm_number_t(movieClipInstance->getMouseY()));
}

void AsMovieClip::MovieClip_set_ymouse(CallArgs& ca)
{
}

void AsMovieClip::MovieClip_get_yscale(CallArgs& ca)
{
	FlashSpriteInstance* movieClipInstance = checked_type_cast< FlashSpriteInstance*, false >(ca.self);

	Vector2 T, S; float R;
	decomposeTransform(movieClipInstance->getTransform(), T, S, R);

	ca.ret = ActionValue(avm_number_t(S.y * 100.0));
}

void AsMovieClip::MovieClip_set_yscale(CallArgs& ca)
{
	FlashSpriteInstance* movieClipInstance = checked_type_cast< FlashSpriteInstance*, false >(ca.self);

	Vector2 T, S; float R;
	decomposeTransform(movieClipInstance->getTransform(), T, S, R);

	S.y = float(ca.args[0].getNumberSafe() / 100.0);

	movieClipInstance->setTransform(composeTransform(T, S, R));
}

	}
}

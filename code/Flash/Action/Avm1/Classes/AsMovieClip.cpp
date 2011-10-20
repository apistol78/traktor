#include "Core/Io/FileSystem.h"
#include "Core/Math/Const.h"
#include "Flash/FlashEdit.h"
#include "Flash/FlashEditInstance.h"
#include "Flash/FlashFrame.h"
#include "Flash/FlashMovie.h"
#include "Flash/FlashMovieFactory.h"
#include "Flash/FlashSpriteInstance.h"
#include "Flash/FlashSprite.h"
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

AsMovieClip::AsMovieClip(ActionContext* context)
:	ActionClass(context, "MovieClip")
{
	Ref< ActionObject > prototype = new ActionObject();

	prototype->setMember("attachAudio", ActionValue(createNativeFunction(context, this, &AsMovieClip::MovieClip_attachAudio)));
	prototype->setMember("attachBitmap", ActionValue(createNativeFunction(context, this, &AsMovieClip::MovieClip_attachBitmap)));
	prototype->setMember("attachMovie", ActionValue(createNativeFunction(context, this, &AsMovieClip::MovieClip_attachMovie)));
	prototype->setMember("beginBitmapFill", ActionValue(createNativeFunction(context, this, &AsMovieClip::MovieClip_beginBitmapFill)));
	prototype->setMember("beginFill", ActionValue(createNativeFunction(context, this, &AsMovieClip::MovieClip_beginFill)));
	prototype->setMember("beginGradientFill", ActionValue(createNativeFunction(context, this, &AsMovieClip::MovieClip_beginGradientFill)));
	prototype->setMember("clear", ActionValue(createNativeFunction(context, this, &AsMovieClip::MovieClip_clear)));
	prototype->setMember("createEmptyMovieClip", ActionValue(createNativeFunction(context, this, &AsMovieClip::MovieClip_createEmptyMovieClip)));
	prototype->setMember("createTextField", ActionValue(createNativeFunction(context, this, &AsMovieClip::MovieClip_createTextField)));
	prototype->setMember("curveTo", ActionValue(createNativeFunction(context, this, &AsMovieClip::MovieClip_curveTo)));
	prototype->setMember("duplicateMovieClip", ActionValue(createNativeFunction(context, this, &AsMovieClip::MovieClip_duplicateMovieClip)));
	prototype->setMember("endFill", ActionValue(createNativeFunction(context, this, &AsMovieClip::MovieClip_endFill)));
	prototype->setMember("getBounds", ActionValue(createNativeFunction(context, this, &AsMovieClip::MovieClip_getBounds)));
	prototype->setMember("getBytesLoaded", ActionValue(createNativeFunction(context, this, &AsMovieClip::MovieClip_getBytesLoaded)));
	prototype->setMember("getBytesTotal", ActionValue(createNativeFunction(context, this, &AsMovieClip::MovieClip_getBytesTotal)));
	prototype->setMember("getDepth", ActionValue(createNativeFunction(context, this, &AsMovieClip::MovieClip_getDepth)));
	prototype->setMember("getInstanceAtDepth", ActionValue(createNativeFunction(context, this, &AsMovieClip::MovieClip_getInstanceAtDepth)));
	prototype->setMember("getNextHighestDepth", ActionValue(createNativeFunction(context, this, &AsMovieClip::MovieClip_getNextHighestDepth)));
	prototype->setMember("getRect", ActionValue(createNativeFunction(context, this, &AsMovieClip::MovieClip_getRect)));
	prototype->setMember("getSWFVersion", ActionValue(createNativeFunction(context, this, &AsMovieClip::MovieClip_getSWFVersion)));
	prototype->setMember("getTextSnapshot", ActionValue(createNativeFunction(context, this, &AsMovieClip::MovieClip_getTextSnapshot)));
	prototype->setMember("getURL", ActionValue(createNativeFunction(context, this, &AsMovieClip::MovieClip_getURL)));
	prototype->setMember("globalToLocal", ActionValue(createNativeFunction(context, this, &AsMovieClip::MovieClip_globalToLocal)));
	prototype->setMember("gotoAndPlay", ActionValue(createNativeFunction(context, this, &AsMovieClip::MovieClip_gotoAndPlay)));
	prototype->setMember("gotoAndStop", ActionValue(createNativeFunction(context, this, &AsMovieClip::MovieClip_gotoAndStop)));
	prototype->setMember("hitTest", ActionValue(
		createPolymorphicFunction(
			0,
			createNativeFunction(context, this, &AsMovieClip::MovieClip_hitTest_1),
			0,
			createNativeFunction(context, this, &AsMovieClip::MovieClip_hitTest_3)
		)
	));
	prototype->setMember("lineGradientStyle", ActionValue(createNativeFunction(context, this, &AsMovieClip::MovieClip_lineGradientStyle)));
	prototype->setMember("lineStyle", ActionValue(createNativeFunction(context, this, &AsMovieClip::MovieClip_lineStyle)));
	prototype->setMember("lineTo", ActionValue(createNativeFunction(context, this, &AsMovieClip::MovieClip_lineTo)));
	prototype->setMember("loadMovie", ActionValue(createNativeFunction(context, this, &AsMovieClip::MovieClip_loadMovie)));
	prototype->setMember("loadVariables", ActionValue(createNativeFunction(context, this, &AsMovieClip::MovieClip_loadVariables)));
	prototype->setMember("localToGlobal", ActionValue(createNativeFunction(context, this, &AsMovieClip::MovieClip_localToGlobal)));
	prototype->setMember("moveTo", ActionValue(createNativeFunction(context, this, &AsMovieClip::MovieClip_moveTo)));
	prototype->setMember("nextFrame", ActionValue(createNativeFunction(context, this, &AsMovieClip::MovieClip_nextFrame)));
	prototype->setMember("play", ActionValue(createNativeFunction(context, this, &AsMovieClip::MovieClip_play)));
	prototype->setMember("prevFrame", ActionValue(createNativeFunction(context, this, &AsMovieClip::MovieClip_prevFrame)));
	prototype->setMember("removeMovieClip", ActionValue(createNativeFunction(context, this, &AsMovieClip::MovieClip_removeMovieClip)));
	prototype->setMember("setMask", ActionValue(createNativeFunction(context, this, &AsMovieClip::MovieClip_setMask)));
	prototype->setMember("startDrag", ActionValue(createNativeFunction(context, this, &AsMovieClip::MovieClip_startDrag)));
	prototype->setMember("stop", ActionValue(createNativeFunction(context, this, &AsMovieClip::MovieClip_stop)));
	prototype->setMember("stopDrag", ActionValue(createNativeFunction(context, this, &AsMovieClip::MovieClip_stopDrag)));
	prototype->setMember("swapDepths", ActionValue(createNativeFunction(context, this, &AsMovieClip::MovieClip_swapDepths)));
	prototype->setMember("unloadMovie", ActionValue(createNativeFunction(context, this, &AsMovieClip::MovieClip_unloadMovie)));

	prototype->addProperty("_alpha", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_alpha), createNativeFunction(context, this, &AsMovieClip::MovieClip_set_alpha));
	prototype->addProperty("blendMode", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_blendMode), createNativeFunction(context, this, &AsMovieClip::MovieClip_set_blendMode));
	prototype->addProperty("cacheAsBitmap", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_cacheAsBitmap), createNativeFunction(context, this, &AsMovieClip::MovieClip_set_cacheAsBitmap));
	prototype->addProperty("_currentframe", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_currentframe), createNativeFunction(context, this, &AsMovieClip::MovieClip_set_currentframe));
	prototype->addProperty("_droptarget", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_droptarget), createNativeFunction(context, this, &AsMovieClip::MovieClip_set_droptarget));
	prototype->addProperty("enabled", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_enabled), createNativeFunction(context, this, &AsMovieClip::MovieClip_set_enabled));
	prototype->addProperty("filters", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_filters), createNativeFunction(context, this, &AsMovieClip::MovieClip_set_filters));
	prototype->addProperty("focusEnabled", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_focusEnabled), createNativeFunction(context, this, &AsMovieClip::MovieClip_set_focusEnabled));
	prototype->addProperty("_focusrect", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_focusrect), createNativeFunction(context, this, &AsMovieClip::MovieClip_set_focusrect));
	prototype->addProperty("forceSmoothing", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_forceSmoothing), createNativeFunction(context, this, &AsMovieClip::MovieClip_set_forceSmoothing));
	prototype->addProperty("_framesloaded", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_framesloaded), createNativeFunction(context, this, &AsMovieClip::MovieClip_set_framesloaded));
	prototype->addProperty("_height", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_height), createNativeFunction(context, this, &AsMovieClip::MovieClip_set_height));
	prototype->addProperty("_highquality", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_highquality), createNativeFunction(context, this, &AsMovieClip::MovieClip_set_highquality));
	prototype->addProperty("hitArea", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_hitArea), createNativeFunction(context, this, &AsMovieClip::MovieClip_set_hitArea));
	prototype->addProperty("_lockroot", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_lockroot), createNativeFunction(context, this, &AsMovieClip::MovieClip_set_lockroot));
	prototype->addProperty("menu", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_menu), createNativeFunction(context, this, &AsMovieClip::MovieClip_set_menu));
	prototype->addProperty("_name", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_name), createNativeFunction(context, this, &AsMovieClip::MovieClip_set_name));
	prototype->addProperty("opaqueBackground", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_opaqueBackground), createNativeFunction(context, this, &AsMovieClip::MovieClip_set_opaqueBackground));
	prototype->addProperty("_parent", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_parent), createNativeFunction(context, this, &AsMovieClip::MovieClip_set_parent));
	prototype->addProperty("_quality", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_quality), createNativeFunction(context, this, &AsMovieClip::MovieClip_set_quality));
	prototype->addProperty("_rotation", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_rotation), createNativeFunction(context, this, &AsMovieClip::MovieClip_set_rotation));
	prototype->addProperty("scale9Grid", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_scale9Grid), createNativeFunction(context, this, &AsMovieClip::MovieClip_set_scale9Grid));
	prototype->addProperty("scrollRect", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_scrollRect), createNativeFunction(context, this, &AsMovieClip::MovieClip_set_scrollRect));
	prototype->addProperty("_soundbuftime", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_soundbuftime), createNativeFunction(context, this, &AsMovieClip::MovieClip_set_soundbuftime));
	prototype->addProperty("tabChildren", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_tabChildren), createNativeFunction(context, this, &AsMovieClip::MovieClip_set_tabChildren));
	prototype->addProperty("tabEnabled", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_tabEnabled), createNativeFunction(context, this, &AsMovieClip::MovieClip_set_tabEnabled));
	prototype->addProperty("tabIndex", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_tabIndex), createNativeFunction(context, this, &AsMovieClip::MovieClip_set_tabIndex));
	prototype->addProperty("_target", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_target), createNativeFunction(context, this, &AsMovieClip::MovieClip_set_target));
	prototype->addProperty("_totalframes", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_totalframes), createNativeFunction(context, this, &AsMovieClip::MovieClip_set_totalframes));
	prototype->addProperty("trackAsMenu", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_trackAsMenu), createNativeFunction(context, this, &AsMovieClip::MovieClip_set_trackAsMenu));
	prototype->addProperty("transform", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_transform), 0);
	prototype->addProperty("_url", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_url), createNativeFunction(context, this, &AsMovieClip::MovieClip_set_url));
	prototype->addProperty("useHandCursor", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_useHandCursor), createNativeFunction(context, this, &AsMovieClip::MovieClip_set_useHandCursor));
	prototype->addProperty("_visible", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_visible), createNativeFunction(context, this, &AsMovieClip::MovieClip_set_visible));
	prototype->addProperty("_width", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_width), createNativeFunction(context, this, &AsMovieClip::MovieClip_set_width));
	prototype->addProperty("_x", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_x), createNativeFunction(context, this, &AsMovieClip::MovieClip_set_x));
	prototype->addProperty("_xmouse", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_xmouse), createNativeFunction(context, this, &AsMovieClip::MovieClip_set_xmouse));
	prototype->addProperty("_xscale", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_xscale), createNativeFunction(context, this, &AsMovieClip::MovieClip_set_xscale));
	prototype->addProperty("_y", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_y), createNativeFunction(context, this, &AsMovieClip::MovieClip_set_y));
	prototype->addProperty("_ymouse", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_ymouse), createNativeFunction(context, this, &AsMovieClip::MovieClip_set_ymouse));
	prototype->addProperty("_yscale", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_yscale), createNativeFunction(context, this, &AsMovieClip::MovieClip_set_yscale));

	prototype->setMember("constructor", ActionValue(this));
	prototype->setMember("__coerce__", ActionValue(this));

	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

void AsMovieClip::init(ActionObject* self, const ActionValueArray& args) const
{
	Ref< FlashSprite > sprite = new FlashSprite(0, 0);
	sprite->addFrame(new FlashFrame());

	Ref< FlashCharacterInstance > spriteInstance = sprite->createInstance(getContext(), 0, "");
	T_ASSERT (spriteInstance);

	self->setRelay(spriteInstance);
}

void AsMovieClip::coerce(ActionObject* self) const
{
	Ref< FlashSprite > sprite = new FlashSprite(0, 0);
	sprite->addFrame(new FlashFrame());

	Ref< FlashCharacterInstance > spriteInstance = sprite->createInstance(getContext(), 0, "");
	T_ASSERT (spriteInstance);

	self->setRelay(spriteInstance);
}

void AsMovieClip::MovieClip_attachAudio(FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::attachAudio not implemented" << Endl;
	)
}

void AsMovieClip::MovieClip_attachBitmap(FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::attachBitmap not implemented" << Endl;
	)
}

Ref< FlashSpriteInstance > AsMovieClip::MovieClip_attachMovie(FlashSpriteInstance* self, const std::string& attachClipName, const std::string& attachClipNewName, int32_t depth) const
{
	ActionContext* context = self->getContext();
	T_ASSERT (context);

	// Get root movie.
	const FlashMovie* movie = context->getMovie();
	T_ASSERT (movie);

	// Get movie clip ID from name.
	uint16_t attachClipId;
	if (!movie->getExportId(attachClipName, attachClipId))
	{
		T_IF_VERBOSE(
			log::error << L"MovieClip.attachMovie, no such movie clip exported (" << mbstows(attachClipName) << L")" << Endl;
		)
		return 0;
	}

	// Get movie clip character.
	Ref< const FlashSprite > attachClip = dynamic_type_cast< const FlashSprite* >(movie->getCharacter(attachClipId));
	if (!attachClip)
	{
		T_IF_VERBOSE(
			log::error << L"MovieClip.attachMovie, no movie clip with id " << attachClipId << L" defined" << Endl;
		)
		return 0;
	}

	// Create new instance of movie clip.
	Ref< FlashSpriteInstance > attachClipInstance = checked_type_cast< FlashSpriteInstance* >(attachClip->createInstance(context, self, attachClipName));
	
	// Add new instance to display list.
	FlashDisplayList& displayList = self->getDisplayList();
	displayList.showObject(depth, attachClipId, attachClipInstance, true);

	return attachClipInstance;
}

void AsMovieClip::MovieClip_beginBitmapFill(FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::beginBitmapFill not implemented" << Endl;
	)
}

void AsMovieClip::MovieClip_beginFill(FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::beginFill not implemented" << Endl;
	)
}

void AsMovieClip::MovieClip_beginGradientFill(FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::beginGradientFill not implemented" << Endl;
	)
}

void AsMovieClip::MovieClip_clear(FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::clear not implemented" << Endl;
	)
}

Ref< FlashSpriteInstance > AsMovieClip::MovieClip_createEmptyMovieClip(FlashSpriteInstance* self, const std::string& emptyClipName, int32_t depth) const
{
	ActionContext* context = self->getContext();
	T_ASSERT (context);

	// Create fake character ID.
	uint16_t emptyClipId = depth + 40000;

	// Create empty movie character with a single frame.
	Ref< FlashSprite > emptyClip = new FlashSprite(emptyClipId, 0);
	emptyClip->addFrame(new FlashFrame());

	// Create new instance of movie clip.
	Ref< FlashSpriteInstance > emptyClipInstance = checked_type_cast< FlashSpriteInstance* >(emptyClip->createInstance(context, self, ""));
	emptyClipInstance->setName(emptyClipName);

	// Add new instance to display list.
	FlashDisplayList& displayList = self->getDisplayList();
	displayList.showObject(depth, emptyClipId, emptyClipInstance, true);

	return emptyClipInstance;
}

Ref< FlashEditInstance > AsMovieClip::MovieClip_createTextField(
	FlashSpriteInstance* self,
	const std::string& name,
	int32_t depth,
	avm_number_t x,
	avm_number_t y,
	avm_number_t width,
	avm_number_t height
) const
{
	ActionContext* context = self->getContext();
	T_ASSERT (context);

	SwfRect bounds =
	{
		Vector2(0.0f, 0.0f),
		Vector2(width, height)
	};
	SwfColor color = { 0, 0, 0, 0 };

	// Create edit character.
	Ref< FlashEdit > edit = new FlashEdit(
		-1,
		0,
		12,
		bounds,
		color,
		L"",
		FlashEdit::AnLeft,
		0,
		0,
		false,
		false
	);

	// Create edit character instance.
	Ref< FlashEditInstance > editInstance = checked_type_cast< FlashEditInstance*, false >(edit->createInstance(context, self, name));
	
	// Place character at given location.
	editInstance->setTransform(translate(x, y));
	
	// Show edit character instance.
	self->getDisplayList().showObject(
		depth,
		edit->getId(),
		editInstance,
		true
	);

	return editInstance;
}

void AsMovieClip::MovieClip_curveTo(FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::curveTo not implemented" << Endl;
	)
}

Ref< FlashSpriteInstance > AsMovieClip::MovieClip_duplicateMovieClip(FlashSpriteInstance* self, const std::string& name, int32_t depth) const
{
	Ref< FlashSpriteInstance > parentClipInstance = checked_type_cast< FlashSpriteInstance* >(self->getParent());

	Ref< FlashSpriteInstance > cloneClipInstance = self->clone();
	cloneClipInstance->setName(name);

	parentClipInstance->getDisplayList().showObject(
		depth,
		cloneClipInstance->getSprite()->getId(),
		cloneClipInstance,
		true
	);

	return cloneClipInstance;
}

void AsMovieClip::MovieClip_endFill(FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::endFill not implemented" << Endl;
	)
}

void AsMovieClip::MovieClip_getBounds(FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::getBounds not implemented" << Endl;
	)
}

int32_t AsMovieClip::MovieClip_getBytesLoaded(FlashSpriteInstance* self) const
{
	return 1;
}

int32_t AsMovieClip::MovieClip_getBytesTotal(FlashSpriteInstance* self) const
{
	return 1;
}

int32_t AsMovieClip::MovieClip_getDepth(FlashSpriteInstance* self) const
{
	Ref< FlashSpriteInstance > parentClipInstance = checked_type_cast< FlashSpriteInstance* >(self->getParent());
	if (parentClipInstance)
		return parentClipInstance->getDisplayList().getObjectDepth(self);
	else
		return 0;
}

Ref< FlashCharacterInstance > AsMovieClip::MovieClip_getInstanceAtDepth(FlashSpriteInstance* self, int32_t depth) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::getInstanceAtDepth not implemented" << Endl;
	)
	return 0;
}

int32_t AsMovieClip::MovieClip_getNextHighestDepth(FlashSpriteInstance* self) const
{
	return self->getDisplayList().getNextHighestDepth();
}

void AsMovieClip::MovieClip_getRect(FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::getRect not implemented" << Endl;
	)
}

int32_t AsMovieClip::MovieClip_getSWFVersion(FlashSpriteInstance* self) const
{
	return 8;
}

void AsMovieClip::MovieClip_getTextSnapshot(FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::getTextSnapshot not implemented" << Endl;
	)
}

void AsMovieClip::MovieClip_getURL(FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::getURL not implemented" << Endl;
	)
}

void AsMovieClip::MovieClip_globalToLocal(FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::globalToLocal not implemented" << Endl;
	)
}

void AsMovieClip::MovieClip_gotoAndPlay(FlashSpriteInstance* self, const ActionValue& arg0) const
{
	if (arg0.isNumeric())
		self->gotoFrame(uint32_t(arg0.getNumber() - 1));
	else if (arg0.isString())
	{
		int32_t frame = self->getSprite()->findFrame(arg0.getString());
		if (frame >= 0)
			self->gotoFrame(frame);
		else
			log::warning << L"No such frame, \"" << arg0.getWideString() << L"\"" << Endl;
	}
	self->setPlaying(true);
}

void AsMovieClip::MovieClip_gotoAndStop(FlashSpriteInstance* self, const ActionValue& arg0) const
{
	if (arg0.isNumeric())
	{
		int32_t frame = int32_t(arg0.getNumber());
		self->gotoFrame(max< int32_t >(frame - 1, 0));
	}
	else if (arg0.isString())
	{
		int32_t frame = self->getSprite()->findFrame(arg0.getString());
		if (frame >= 0)
			self->gotoFrame(frame);
		else
			log::warning << L"No such frame, \"" << arg0.getWideString() << L"\"" << Endl;
	}
	self->setPlaying(false);
}

bool AsMovieClip::MovieClip_hitTest_1(const FlashSpriteInstance* self, const FlashCharacterInstance* shape) const
{
	if (!shape)
		return false;

	SwfRect bounds = self->getBounds();
	SwfRect shapeBounds = shape->getBounds();

	return
		(bounds.min.x < shapeBounds.max.x) &&
		(bounds.min.y < shapeBounds.max.y) &&
		(bounds.max.x > shapeBounds.min.x) &&
		(bounds.max.y > shapeBounds.min.y);
}

bool AsMovieClip::MovieClip_hitTest_3(const FlashSpriteInstance* self, avm_number_t x, avm_number_t y) const
{
	SwfRect bounds = self->getBounds();
	return (x >= bounds.min.x && y >= bounds.min.y && x <= bounds.max.x && y <= bounds.max.y);
}

void AsMovieClip::MovieClip_lineGradientStyle(FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::lineGradientStyle not implemented" << Endl;
	)
}

void AsMovieClip::MovieClip_lineStyle(FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::lineStyle not implemented" << Endl;
	)
}

void AsMovieClip::MovieClip_lineTo(FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::lineTo not implemented" << Endl;
	)
}

Ref< FlashSpriteInstance > AsMovieClip::MovieClip_loadMovie(FlashSpriteInstance* self, const std::wstring& fileName) const
{
	Ref< IStream > file = FileSystem::getInstance().open(fileName, File::FmRead);
	if (!file)
		return 0;

	Ref< SwfReader > swf = new SwfReader(file);
	Ref< FlashMovie > movie = flash::FlashMovieFactory().createMovie(swf);
	if (!movie)
		return 0;

	return movie->createExternalMovieClipInstance(self);
}

void AsMovieClip::MovieClip_loadVariables(FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::loadVariables not implemented" << Endl;
	)
}

void AsMovieClip::MovieClip_localToGlobal(const FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::localToGlobal not implemented" << Endl;
	)
}

void AsMovieClip::MovieClip_moveTo(FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::moveTo not implemented" << Endl;
	)
}

void AsMovieClip::MovieClip_nextFrame(FlashSpriteInstance* self) const
{
	self->gotoNext();
}

void AsMovieClip::MovieClip_play(FlashSpriteInstance* self) const
{
	self->setPlaying(true);
}

void AsMovieClip::MovieClip_prevFrame(FlashSpriteInstance* self) const
{
	self->gotoPrevious();
}

void AsMovieClip::MovieClip_removeMovieClip(FlashSpriteInstance* self) const
{
	FlashSpriteInstance* parentClipInstance = checked_type_cast< FlashSpriteInstance*, false >(self->getParent());
	FlashDisplayList& displayList = parentClipInstance->getDisplayList();
	displayList.removeObject(self);
}

void AsMovieClip::MovieClip_setMask(FlashSpriteInstance* self, FlashSpriteInstance* mask) const
{
	self->setMask(mask);
}

void AsMovieClip::MovieClip_startDrag(FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::startDrag not implemented" << Endl;
	)
}

void AsMovieClip::MovieClip_stop(FlashSpriteInstance* self) const
{
	self->setPlaying(false);
}

void AsMovieClip::MovieClip_stopDrag(FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::stopDrag not implemented" << Endl;
	)
}

void AsMovieClip::MovieClip_swapDepths(FlashSpriteInstance* self, const ActionValue& arg0) const
{
	FlashSpriteInstance* parentClipInstance = checked_type_cast< FlashSpriteInstance*, false >(self->getParent());

	// Get my own current depth.
	int32_t depth = parentClipInstance->getDisplayList().getObjectDepth(self);

	// Get target clip depth.
	int32_t targetDepth = depth;

	if (arg0.isNumeric())
		targetDepth = int32_t(arg0.getNumber());
	else if (arg0.isObject())
	{
		Ref< FlashSpriteInstance > targetClipInstance = arg0.getObject< FlashSpriteInstance >();
		targetDepth = parentClipInstance->getDisplayList().getObjectDepth(targetClipInstance);
	}

	parentClipInstance->getDisplayList().swap(depth, targetDepth);
}

void AsMovieClip::MovieClip_unloadMovie(FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::unloadMovie not implemented" << Endl;
	)
}

avm_number_t AsMovieClip::MovieClip_get_alpha(const FlashSpriteInstance* self) const
{
	const SwfCxTransform& colorTransform = self->getColorTransform();
	return colorTransform.alpha[0] * 100.0f;
}

void AsMovieClip::MovieClip_set_alpha(FlashSpriteInstance* self, avm_number_t alpha) const
{
	SwfCxTransform colorTransform = self->getColorTransform();
	colorTransform.alpha[0] = alpha / 100.0f;
	self->setColorTransform(colorTransform);
}

void AsMovieClip::MovieClip_get_blendMode(const FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::get_blendMode not implemented" << Endl;
	)
}

void AsMovieClip::MovieClip_set_blendMode(FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::set_blendMode not implemented" << Endl;
	)
}

void AsMovieClip::MovieClip_get_cacheAsBitmap(const FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::get_cacheAsBitmap not implemented" << Endl;
	)
}

void AsMovieClip::MovieClip_set_cacheAsBitmap(FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::set_cacheAsBitmap not implemented" << Endl;
	)
}

int32_t AsMovieClip::MovieClip_get_currentframe(const FlashSpriteInstance* self) const
{
	return self->getCurrentFrame() + 1;
}

void AsMovieClip::MovieClip_set_currentframe(FlashSpriteInstance* self, int32_t frame) const
{
	if (frame > 0)
		self->gotoFrame(frame - 1);
}

void AsMovieClip::MovieClip_get_droptarget(const FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::get_droptarget not implemented" << Endl;
	)
}

void AsMovieClip::MovieClip_set_droptarget(FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::set_droptarget not implemented" << Endl;
	)
}

void AsMovieClip::MovieClip_get_enabled(const FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::get_enabled not implemented" << Endl;
	)
}

void AsMovieClip::MovieClip_set_enabled(FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::set_enabled not implemented" << Endl;
	)
}

void AsMovieClip::MovieClip_get_filters(const FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::get_filters not implemented" << Endl;
	)
}

void AsMovieClip::MovieClip_set_filters(FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::set_filters not implemented" << Endl;
	)
}

void AsMovieClip::MovieClip_get_focusEnabled(const FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::got_focusEnabled not implemented" << Endl;
	)
}

void AsMovieClip::MovieClip_set_focusEnabled(FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::set_focusEnabled not implemented" << Endl;
	)
}

void AsMovieClip::MovieClip_get_focusrect(const FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::get_focusrect not implemented" << Endl;
	)
}

void AsMovieClip::MovieClip_set_focusrect(FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::set_focusrect not implemented" << Endl;
	)
}

void AsMovieClip::MovieClip_get_forceSmoothing(const FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::get_forceSmoothing not implemented" << Endl;
	)
}

void AsMovieClip::MovieClip_set_forceSmoothing(FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::set_forceSmoothing not implemented" << Endl;
	)
}

int32_t AsMovieClip::MovieClip_get_framesloaded(const FlashSpriteInstance* self) const
{
	const FlashSprite* movieClip = self->getSprite();
	return movieClip->getFrameCount();
}

void AsMovieClip::MovieClip_set_framesloaded(FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::set_framesloaded not implemented" << Endl;
	)
}

float AsMovieClip::MovieClip_get_height(const FlashSpriteInstance* self) const
{
	SwfRect bounds = self->getBounds();
	return (bounds.max.y - bounds.min.y) / 20.0f;
}

void AsMovieClip::MovieClip_set_height(FlashSpriteInstance* self, float height) const
{
	SwfRect bounds = self->getLocalBounds();
	float extent = (bounds.max.y - bounds.min.y) / 20.0f;
	if (abs(extent) <= FUZZY_EPSILON)
		return;

	Vector2 T, S; float R;
	decomposeTransform(self->getTransform(), T, S, R);
	S.y = height / extent;
	self->setTransform(composeTransform(T, S, R));
}

void AsMovieClip::MovieClip_get_highquality(const FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::get_highquality not implemented" << Endl;
	)
}

void AsMovieClip::MovieClip_set_highquality(FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::set_highquality not implemented" << Endl;
	)
}

void AsMovieClip::MovieClip_get_hitArea(const FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::get_hitArea not implemented" << Endl;
	)
}

void AsMovieClip::MovieClip_set_hitArea(FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::set_hitArea not implemented" << Endl;
	)
}

void AsMovieClip::MovieClip_get_lockroot(const FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::get_lockroot not implemented" << Endl;
	)
}

void AsMovieClip::MovieClip_set_lockroot(FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::set_lockroot not implemented" << Endl;
	)
}

void AsMovieClip::MovieClip_get_menu(const FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::get_menu not implemented" << Endl;
	)
}

void AsMovieClip::MovieClip_set_menu(FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::set_menu not implemented" << Endl;
	)
}

std::string AsMovieClip::MovieClip_get_name(const FlashSpriteInstance* self) const
{
	return self->getName();
}

void AsMovieClip::MovieClip_set_name(FlashSpriteInstance* self, const std::string& name) const
{
	self->setName(name);
}

void AsMovieClip::MovieClip_get_opaqueBackground(const FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::get_opaqueBackground not implemented" << Endl;
	)
}

void AsMovieClip::MovieClip_set_opaqueBackground(FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::set_opaqueBackground not implemented" << Endl;
	)
}

FlashCharacterInstance* AsMovieClip::MovieClip_get_parent(FlashSpriteInstance* self) const
{
	return self->getParent();
}

void AsMovieClip::MovieClip_set_parent(FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::set_parent not implemented" << Endl;
	)
}

std::string AsMovieClip::MovieClip_get_quality(const FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::get_quality not implemented" << Endl;
	)
	return "BEST";
}

void AsMovieClip::MovieClip_set_quality(FlashSpriteInstance* self, const std::string& quality) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::set_quality not implemented" << Endl;
	)
}

float AsMovieClip::MovieClip_get_rotation(const FlashSpriteInstance* self) const
{
	Vector2 T, S; float R;
	decomposeTransform(self->getTransform(), T, S, R);
	return rad2deg(R);
}

void AsMovieClip::MovieClip_set_rotation(FlashSpriteInstance* self, float rotation) const
{
	Vector2 T, S; float R;
	decomposeTransform(self->getTransform(), T, S, R);
	R = deg2rad(rotation);
	self->setTransform(composeTransform(T, S, R));
}

void AsMovieClip::MovieClip_get_scale9Grid(const FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::get_scale9Grid not implemented" << Endl;
	)
}

void AsMovieClip::MovieClip_set_scale9Grid(FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::set_scale9Grid not implemented" << Endl;
	)
}

void AsMovieClip::MovieClip_get_scrollRect(const FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::get_scrollRect not implemented" << Endl;
	)
}

void AsMovieClip::MovieClip_set_scrollRect(FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::set_scrollRect not implemented" << Endl;
	)
}

void AsMovieClip::MovieClip_get_soundbuftime(const FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::get_soundbuftime not implemented" << Endl;
	)
}

void AsMovieClip::MovieClip_set_soundbuftime(FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::set_soundbuftime not implemented" << Endl;
	)
}

void AsMovieClip::MovieClip_get_tabChildren(const FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::get_tabChildren not implemented" << Endl;
	)
}

void AsMovieClip::MovieClip_set_tabChildren(FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::set_tabChildren not implemented" << Endl;
	)
}

void AsMovieClip::MovieClip_get_tabEnabled(const FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::get_tabEnabled not implemented" << Endl;
	)
}

void AsMovieClip::MovieClip_set_tabEnabled(FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::set_tabEnabled not implemented" << Endl;
	)
}

void AsMovieClip::MovieClip_get_tabIndex(const FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::get_tabIndex not implemented" << Endl;
	)
}

void AsMovieClip::MovieClip_set_tabIndex(FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::set_tabIndex not implemented" << Endl;
	)
}

void AsMovieClip::MovieClip_get_target(const FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::get_target not implemented" << Endl;
	)
}

void AsMovieClip::MovieClip_set_target(FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::set_target not implemented" << Endl;
	)
}

void AsMovieClip::MovieClip_get_totalframes(const FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::get_totalframes not implemented" << Endl;
	)
}

void AsMovieClip::MovieClip_set_totalframes(FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::set_totalframes not implemented" << Endl;
	)
}

void AsMovieClip::MovieClip_get_trackAsMenu(const FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::get_trackAsMenu not implemented" << Endl;
	)
}

void AsMovieClip::MovieClip_set_trackAsMenu(FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::set_trackAsMenu not implemented" << Endl;
	)
}

Ref< Transform > AsMovieClip::MovieClip_get_transform(FlashSpriteInstance* self) const
{
	return new Transform(self);
}

void AsMovieClip::MovieClip_get_url(const FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::get_url not implemented" << Endl;
	)
}

void AsMovieClip::MovieClip_set_url(FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::set_url not implemented" << Endl;
	)
}

void AsMovieClip::MovieClip_get_useHandCursor(const FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::get_useHandCursor not implemented" << Endl;
	)
}

void AsMovieClip::MovieClip_set_useHandCursor(FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::set_useHandCursor not implemented" << Endl;
	)
}

bool AsMovieClip::MovieClip_get_visible(const FlashSpriteInstance* self) const
{
	return self->isVisible();
}

void AsMovieClip::MovieClip_set_visible(FlashSpriteInstance* self, bool visible) const
{
	self->setVisible(visible);
}

float AsMovieClip::MovieClip_get_width(const FlashSpriteInstance* self) const
{
	SwfRect bounds = self->getBounds();
	return (bounds.max.x - bounds.min.x) / 20.0f;
}

void AsMovieClip::MovieClip_set_width(FlashSpriteInstance* self, float width) const
{
	SwfRect bounds = self->getLocalBounds();
	float extent = (bounds.max.x - bounds.min.x) / 20.0f;
	if (abs(extent) <= FUZZY_EPSILON)
		return;

	Vector2 T, S; float R;
	decomposeTransform(self->getTransform(), T, S, R);
	S.x = width / extent;
	self->setTransform(composeTransform(T, S, R));
}

float AsMovieClip::MovieClip_get_x(const FlashSpriteInstance* self) const
{
	Vector2 T, S; float R;
	decomposeTransform(self->getTransform(), T, S, R);
	return T.x / 20.0f;
}

void AsMovieClip::MovieClip_set_x(FlashSpriteInstance* self, float x) const
{
	Vector2 T, S; float R;
	decomposeTransform(self->getTransform(), T, S, R);
	T.x = x * 20.0f;
	self->setTransform(composeTransform(T, S, R));
}

int32_t AsMovieClip::MovieClip_get_xmouse(const FlashSpriteInstance* self) const
{
	return self->getMouseX();
}

void AsMovieClip::MovieClip_set_xmouse(FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::set_xmouse not implemented" << Endl;
	)
}

float AsMovieClip::MovieClip_get_xscale(const FlashSpriteInstance* self) const
{
	Vector2 T, S; float R;
	decomposeTransform(self->getTransform(), T, S, R);
	return S.x * 100.0f;
}

void AsMovieClip::MovieClip_set_xscale(FlashSpriteInstance* self, float x) const
{
	Vector2 T, S; float R;
	decomposeTransform(self->getTransform(), T, S, R);
	S.x = x / 100.0f;
	self->setTransform(composeTransform(T, S, R));
}

float AsMovieClip::MovieClip_get_y(const FlashSpriteInstance* self) const
{
	Vector2 T, S; float R;
	decomposeTransform(self->getTransform(), T, S, R);
	return T.y / 20.0f;
}

void AsMovieClip::MovieClip_set_y(FlashSpriteInstance* self, float y) const
{
	Vector2 T, S; float R;
	decomposeTransform(self->getTransform(), T, S, R);
	T.y = y * 20.0f;
	self->setTransform(composeTransform(T, S, R));
}

int32_t AsMovieClip::MovieClip_get_ymouse(const FlashSpriteInstance* self) const
{
	return self->getMouseY();
}

void AsMovieClip::MovieClip_set_ymouse(FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::set_ymouse not implemented" << Endl;
	)
}

float AsMovieClip::MovieClip_get_yscale(const FlashSpriteInstance* self) const
{
	Vector2 T, S; float R;
	decomposeTransform(self->getTransform(), T, S, R);
	return S.y * 100.0f;
}

void AsMovieClip::MovieClip_set_yscale(FlashSpriteInstance* self, float y) const
{
	Vector2 T, S; float R;
	decomposeTransform(self->getTransform(), T, S, R);
	S.y = y / 100.0f;
	self->setTransform(composeTransform(T, S, R));
}

	}
}

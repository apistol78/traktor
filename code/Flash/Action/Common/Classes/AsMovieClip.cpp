#include "Core/Io/FileSystem.h"
#include "Core/Math/Const.h"
#include "Core/Misc/String.h"
#include "Flash/FlashBitmapImage.h"
#include "Flash/FlashDictionary.h"
#include "Flash/FlashCanvas.h"
#include "Flash/FlashEditInstance.h"
#include "Flash/FlashFrame.h"
#include "Flash/FlashMovie.h"
#include "Flash/FlashMovieFactory.h"
#include "Flash/FlashShape.h"
#include "Flash/FlashShapeInstance.h"
#include "Flash/FlashSprite.h"
#include "Flash/FlashSpriteInstance.h"
#include "Flash/IFlashMovieLoader.h"
#include "Flash/SwfReader.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Common/Array.h"
#include "Flash/Action/Common/BitmapData.h"
#include "Flash/Action/Common/Matrix.h"
#include "Flash/Action/Common/Transform.h"
#include "Flash/Action/Common/Classes/AsMovieClip.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsMovieClip", AsMovieClip, ActionClass)

AsMovieClip::AsMovieClip(ActionContext* context)
:	ActionClass(context, "MovieClip")
{
	Ref< ActionObject > prototype = new ActionObject(context);

	prototype->setMember("attachAudio", ActionValue(createNativeFunction(context, this, &AsMovieClip::MovieClip_attachAudio)));
	// \fixme attachVideo
	prototype->setMember("attachBitmap", ActionValue(
		createPolymorphicFunction(
			context,
			0,
			0,
			createNativeFunction(context, this, &AsMovieClip::MovieClip_attachBitmap_2),
			createNativeFunction(context, this, &AsMovieClip::MovieClip_attachBitmap_3),
			createNativeFunction(context, this, &AsMovieClip::MovieClip_attachBitmap_4)
		)
	));
	prototype->setMember("attachMovie", ActionValue(
		createPolymorphicFunction(
			context,
			0,
			0,
			0,
			createNativeFunction(context, this, &AsMovieClip::MovieClip_attachMovie_3),
			createNativeFunction(context, this, &AsMovieClip::MovieClip_attachMovie_4)
		)
	));
	prototype->setMember("beginBitmapFill", ActionValue(createNativeFunction(context, this, &AsMovieClip::MovieClip_beginBitmapFill)));
	prototype->setMember("beginFill", ActionValue(
		createPolymorphicFunction(
			context,
			0,
			createNativeFunction(context, this, &AsMovieClip::MovieClip_beginFill_1),
			createNativeFunction(context, this, &AsMovieClip::MovieClip_beginFill_2)
		)
	));
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
			context,
			0,
			createNativeFunction(context, this, &AsMovieClip::MovieClip_hitTest_1),
			createNativeFunction(context, this, &AsMovieClip::MovieClip_hitTest_2)
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
	prototype->setMember("toString", ActionValue(createNativeFunction(context, this, &AsMovieClip::MovieClip_toString)));
	prototype->setMember("unloadMovie", ActionValue(createNativeFunction(context, this, &AsMovieClip::MovieClip_unloadMovie)));

	prototype->addProperty("blendMode", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_blendMode), createNativeFunction(context, this, &AsMovieClip::MovieClip_set_blendMode));
	prototype->addProperty("cacheAsBitmap", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_cacheAsBitmap), createNativeFunction(context, this, &AsMovieClip::MovieClip_set_cacheAsBitmap));
	prototype->addProperty("enabled", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_enabled), createNativeFunction(context, this, &AsMovieClip::MovieClip_set_enabled));
	prototype->addProperty("filters", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_filters), createNativeFunction(context, this, &AsMovieClip::MovieClip_set_filters));
	prototype->addProperty("forceSmoothing", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_forceSmoothing), createNativeFunction(context, this, &AsMovieClip::MovieClip_set_forceSmoothing));
	prototype->addProperty("_framesloaded", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_framesloaded), 0);
	prototype->addProperty("_lockroot", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_lockroot), createNativeFunction(context, this, &AsMovieClip::MovieClip_set_lockroot));
	prototype->addProperty("opaqueBackground", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_opaqueBackground), createNativeFunction(context, this, &AsMovieClip::MovieClip_set_opaqueBackground));
	prototype->addProperty("scale9Grid", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_scale9Grid), createNativeFunction(context, this, &AsMovieClip::MovieClip_set_scale9Grid));
	prototype->addProperty("scrollRect", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_scrollRect), createNativeFunction(context, this, &AsMovieClip::MovieClip_set_scrollRect));
	prototype->addProperty("tabIndex", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_tabIndex), createNativeFunction(context, this, &AsMovieClip::MovieClip_set_tabIndex));
	prototype->addProperty("transform", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_transform), 0);
	prototype->addProperty("useHandCursor", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_useHandCursor), createNativeFunction(context, this, &AsMovieClip::MovieClip_set_useHandCursor));

	prototype->setMember("constructor", ActionValue(this));
	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

void AsMovieClip::initialize(ActionObject* self)
{
	ActionContext* context = getContext();

	self->addProperty("focusEnabled", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_focusEnabled), createNativeFunction(context, this, &AsMovieClip::MovieClip_set_focusEnabled));
	self->addProperty("hitArea", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_hitArea), createNativeFunction(context, this, &AsMovieClip::MovieClip_set_hitArea));
	self->addProperty("menu", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_menu), createNativeFunction(context, this, &AsMovieClip::MovieClip_set_menu));
	self->addProperty("tabChildren", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_tabChildren), createNativeFunction(context, this, &AsMovieClip::MovieClip_set_tabChildren));
	self->addProperty("tabEnabled", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_tabEnabled), createNativeFunction(context, this, &AsMovieClip::MovieClip_set_tabEnabled));
	self->addProperty("trackAsMenu", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_trackAsMenu), createNativeFunction(context, this, &AsMovieClip::MovieClip_set_trackAsMenu));
	self->addProperty("_alpha", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_alpha), createNativeFunction(context, this, &AsMovieClip::MovieClip_set_alpha));
	self->addProperty("_currentframe", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_currentframe), 0);
	self->addProperty("_droptarget", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_droptarget), 0);
	self->addProperty("_focusrect", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_focusrect), createNativeFunction(context, this, &AsMovieClip::MovieClip_set_focusrect));
	self->addProperty("_height", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_height), createNativeFunction(context, this, &AsMovieClip::MovieClip_set_height));
	self->addProperty("_highquality", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_highquality), createNativeFunction(context, this, &AsMovieClip::MovieClip_set_highquality));
	self->addProperty("_name", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_name), createNativeFunction(context, this, &AsMovieClip::MovieClip_set_name));
	self->addProperty("_parent", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_parent), createNativeFunction(context, this, &AsMovieClip::MovieClip_set_parent));
	self->addProperty("_quality", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_quality), createNativeFunction(context, this, &AsMovieClip::MovieClip_set_quality));
	self->addProperty("_rotation", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_rotation), createNativeFunction(context, this, &AsMovieClip::MovieClip_set_rotation));
	self->addProperty("_soundbuftime", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_soundbuftime), createNativeFunction(context, this, &AsMovieClip::MovieClip_set_soundbuftime));
	self->addProperty("_target", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_target), 0);
	self->addProperty("_totalframes", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_totalframes), 0);
	self->addProperty("_url", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_url), 0);
	self->addProperty("_visible", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_visible), createNativeFunction(context, this, &AsMovieClip::MovieClip_set_visible));
	self->addProperty("_width", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_width), createNativeFunction(context, this, &AsMovieClip::MovieClip_set_width));
	self->addProperty("_x", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_x), createNativeFunction(context, this, &AsMovieClip::MovieClip_set_x));
	self->addProperty("_xmouse", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_xmouse), 0);
	self->addProperty("_xscale", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_xscale), createNativeFunction(context, this, &AsMovieClip::MovieClip_set_xscale));
	self->addProperty("_y", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_y), createNativeFunction(context, this, &AsMovieClip::MovieClip_set_y));
	self->addProperty("_ymouse", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_ymouse), 0);
	self->addProperty("_yscale", createNativeFunction(context, this, &AsMovieClip::MovieClip_get_yscale), createNativeFunction(context, this, &AsMovieClip::MovieClip_set_yscale));
}

void AsMovieClip::construct(ActionObject* self, const ActionValueArray& args)
{
}

ActionValue AsMovieClip::xplicit(const ActionValueArray& args)
{
	return ActionValue();
}

void AsMovieClip::MovieClip_attachAudio(FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::attachAudio not implemented" << Endl;
	)
}

void AsMovieClip::MovieClip_attachBitmap_2(FlashSpriteInstance* self, BitmapData* bmp, int32_t depth) const
{
	MovieClip_attachBitmap_4(self, bmp, depth, "auto", false);
}

void AsMovieClip::MovieClip_attachBitmap_3(FlashSpriteInstance* self, BitmapData* bmp, int32_t depth, const std::string& pixelSnapping) const
{
	MovieClip_attachBitmap_4(self, bmp, depth, pixelSnapping, false);
}

void AsMovieClip::MovieClip_attachBitmap_4(FlashSpriteInstance* self, BitmapData* bmp, int32_t depth, const std::string& pixelSnapping, bool smoothing) const
{
	ActionContext* context = self->getContext();
	T_ASSERT (context);

	// Get dictionary.
	FlashDictionary* dictionary = self->getDictionary();
	if (!dictionary)
		return;

	// Define bitmap symbol.
	uint16_t bitmapId = dictionary->addBitmap(new FlashBitmapImage(bmp->getImage()));

	// Create a quad shape.
	Ref< FlashShape > shape = new FlashShape();
	shape->create(
		bitmapId,
		bmp->getWidth() * 20,
		bmp->getHeight() * 20
	);

	// Define shape character.
	uint16_t shapeId = dictionary->addCharacter(shape);

	// Create new instance of shape.
	Ref< FlashShapeInstance > attachShapeInstance = checked_type_cast< FlashShapeInstance* >(shape->createInstance(context, dictionary, self, "", Matrix33::identity(), 0, 0));
	T_ASSERT (attachShapeInstance);

	// Add new instance to display list.
	FlashDisplayList& displayList = self->getDisplayList();
	displayList.showObject(depth, shapeId, attachShapeInstance, true);
}

Ref< FlashSpriteInstance > AsMovieClip::MovieClip_attachMovie_3(FlashSpriteInstance* self, const std::string& attachClipName, const std::string& attachClipNewName, int32_t depth) const
{
	return MovieClip_attachMovie_4(self, attachClipName, attachClipNewName, depth, 0);
}

Ref< FlashSpriteInstance > AsMovieClip::MovieClip_attachMovie_4(FlashSpriteInstance* self, const std::string& attachClipName, const std::string& attachClipNewName, int32_t depth, ActionObject* initObject) const
{
	ActionContext* context = self->getContext();
	T_ASSERT (context);

	// Get dictionary.
	FlashDictionary* dictionary = self->getDictionary();
	if (!dictionary)
		return 0;

	// Get movie clip ID from name.
	uint16_t attachClipId;
	if (!dictionary->getExportId(attachClipName, attachClipId))
	{
		T_IF_VERBOSE(
			log::error << L"MovieClip.attachMovie, no such movie clip exported (" << mbstows(attachClipName) << L")" << Endl;
		)
		return 0;
	}

	// Get movie clip character.
	Ref< const FlashSprite > attachClip = dynamic_type_cast< const FlashSprite* >(dictionary->getCharacter(attachClipId));
	if (!attachClip)
	{
		T_IF_VERBOSE(
			log::error << L"MovieClip.attachMovie, no movie clip with id " << attachClipId << L" defined" << Endl;
		)
		return 0;
	}

	// Create new instance of movie clip.
	Ref< FlashSpriteInstance > attachClipInstance = checked_type_cast< FlashSpriteInstance* >(attachClip->createInstance(
		context,
		dictionary,
		self,
		attachClipNewName,
		Matrix33::identity(),
		initObject,
		0
	));
	
	// Add new instance to display list.
	FlashDisplayList& displayList = self->getDisplayList();
	displayList.showObject(depth, attachClipId, attachClipInstance, true);
	return attachClipInstance;
}

void AsMovieClip::MovieClip_beginBitmapFill(FlashSpriteInstance* self, BitmapData* bm, const Matrix* matrix, bool repeat, bool smoothing) const
{
	Matrix33 M(
		20.0f, 0.0f, 0.0f,
		0.0f, 20.0f, 0.0f,
		0.0f, 0.0f, 1.0f
	);

	if (matrix)
		M = M * matrix->m_v;

	self->createCanvas()->beginBitmapFill(new FlashBitmapImage(bm->getImage()), M, repeat);
}

void AsMovieClip::MovieClip_beginFill_1(FlashSpriteInstance* self, uint32_t rgb) const
{
	MovieClip_beginFill_2(self, rgb, 100);
}

void AsMovieClip::MovieClip_beginFill_2(FlashSpriteInstance* self, uint32_t rgb, int32_t alpha) const
{
	uint8_t r = rgb >> 16;
	uint8_t g = (rgb >> 8) & 255;
	uint8_t b = rgb & 255;
	uint8_t a = (255 * clamp(alpha, 0, 100)) / 100;
	self->createCanvas()->beginFill(Color4f(r, g, b, a) / Scalar(255.0f));
}

bool AsMovieClip::MovieClip_beginGradientFill(FlashSpriteInstance* self, const std::string& fillType, const Array* colors, const Array* alphas, const Array* ratios, ActionObject* matrix) const
{
	FlashFillStyle::GradientType gradientType = FlashFillStyle::GtInvalid;

	if (compareIgnoreCase< std::string >(fillType, "linear") == 0)
		gradientType = FlashFillStyle::GtLinear;
	else if (compareIgnoreCase< std::string >(fillType, "radial") == 0)
		gradientType = FlashFillStyle::GtRadial;
	else
		return false;

	if (!colors || !alphas || !ratios || !matrix)
		return false;

	if (colors->length() != alphas->length() || colors->length() != ratios->length())
		return false;

	AlignedVector< FlashFillStyle::ColorRecord > colorRecords(colors->length());
	for (uint32_t i = 0; i < colors->length(); ++i)
	{
		uint32_t rgb = uint32_t((*colors)[i].getNumber());
		int32_t alpha = int32_t((*alphas)[i].getNumber());

		uint8_t r = rgb >> 16;
		uint8_t g = (rgb >> 8) & 255;
		uint8_t b = rgb & 255;
		uint8_t a = (255 * clamp(alpha, 0, 100)) / 100;

		colorRecords[i].ratio = clamp< float >((*ratios)[i].getNumber() / 255.0f, 0.0f, 1.0f);
		colorRecords[i].color = Color4f(r, g, b, a) / Scalar(255.0f);
	}

	Matrix33 gradientMatrix = Matrix33::identity();

	ActionValue matrixTypeValue;
	Matrix* mx;

	if (matrix->getMember("matrixType", matrixTypeValue))
	{
		if (compareIgnoreCase< std::string >(matrixTypeValue.getString(), "box") != 0)
			return false;

		ActionValue memberValue;
		avm_number_t x, y, w, h, r;

		matrix->getMember("x", memberValue);
		x = memberValue.getNumber();

		matrix->getMember("y", memberValue);
		y = memberValue.getNumber();

		matrix->getMember("w", memberValue);
		w = memberValue.getNumber();

		matrix->getMember("h", memberValue);
		h = memberValue.getNumber();

		matrix->getMember("r", memberValue);
		r = memberValue.getNumber();

		gradientMatrix = translate(w * 10.0f + x * 20.0f, h * 10.0f + y * 20.0f) * rotate(r) * scale(1.0f / w, 1.0f / h) * scale(20.0f, 20.0f);
	}
	else if ((mx = matrix->getRelay< Matrix >()) != 0)
	{
		gradientMatrix = mx->m_v;
	}

	self->createCanvas()->beginGradientFill(gradientType, colorRecords, gradientMatrix);
	return true;
}

void AsMovieClip::MovieClip_clear(FlashSpriteInstance* self) const
{
	self->createCanvas()->clear();
}

Ref< FlashSpriteInstance > AsMovieClip::MovieClip_createEmptyMovieClip(FlashSpriteInstance* self, const std::string& emptyClipName, int32_t depth) const
{
	return self->createEmptyMovieClip(emptyClipName, depth);
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
	return self->createTextField(name, depth, x, y, width, height);
}

void AsMovieClip::MovieClip_curveTo(FlashSpriteInstance* self, avm_number_t controlX, avm_number_t controlY, avm_number_t anchorX, avm_number_t anchorY) const
{
	self->createCanvas()->curveTo(controlX * 20.0f, controlY * 20.0f, anchorX * 20.0f, anchorY * 20.0f);
}

Ref< FlashSpriteInstance > AsMovieClip::MovieClip_duplicateMovieClip(FlashSpriteInstance* self, const std::string& name, int32_t depth) const
{
	return self->duplicateMovieClip(name, depth);
}

void AsMovieClip::MovieClip_endFill(FlashSpriteInstance* self) const
{
	self->createCanvas()->endFill();
}

Ref< ActionObject > AsMovieClip::MovieClip_getBounds(FlashSpriteInstance* self, const FlashCharacterInstance* reference) const
{
	Aabb2 bounds = self->getLocalBounds();

	if (reference)
	{
		Matrix33 transformRef = reference->getFullTransform();
		Matrix33 transformSelf = self->getFullTransform();

		Matrix33 transform = transformRef.inverse() * transformSelf;

		bounds.mn = transform * bounds.mn;
		bounds.mx = transform * bounds.mx;
	}

	Ref< ActionObject > result = new ActionObject(getContext());
	result->setMember("xMin", ActionValue(bounds.mn.x / 20.0f));
	result->setMember("yMin", ActionValue(bounds.mn.y / 20.0f));
	result->setMember("xMax", ActionValue(bounds.mx.x / 20.0f));
	result->setMember("yMax", ActionValue(bounds.mx.y / 20.0f));

	return result;
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

Ref< ActionObject > AsMovieClip::MovieClip_getRect(FlashSpriteInstance* self, const FlashCharacterInstance* reference) const
{
	Aabb2 bounds = self->getLocalBounds();

	if (reference)
	{
		Matrix33 transform = reference->getTransform();
		bounds.mn = transform * bounds.mn;
		bounds.mx = transform * bounds.mx;
	}

	Ref< ActionObject > result = new ActionObject(getContext());
	result->setMember("xMin", ActionValue(bounds.mn.x / 20.0f));
	result->setMember("yMin", ActionValue(bounds.mn.y / 20.0f));
	result->setMember("xMax", ActionValue(bounds.mx.x / 20.0f));
	result->setMember("yMax", ActionValue(bounds.mx.y / 20.0f));

	return result;
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

void AsMovieClip::MovieClip_getURL(FlashSpriteInstance* self, const std::wstring& url) const
{
	ActionContext* context = self->getContext();

	ActionValue getUrl;
	context->getGlobal()->getLocalMember("getURL", getUrl);

	Ref< ActionFunction > fn = getUrl.getObject< ActionFunction >();
	if (fn)
	{
		flash::ActionValueArray args(context->getPool(), 2);
		args[0] = ActionValue(url);
		fn->call(args);
	}
	else
	{
		T_IF_VERBOSE(
			log::warning << L"MovieClip::getURL, _global.getURL must be a function object" << Endl;
		)
	}
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
#if defined(_DEBUG)
		else
			log::warning << L"No such frame, \"" << arg0.getWideString() << L"\"" << Endl;
#endif
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
#if defined(_DEBUG)
		else
			log::warning << L"No such frame, \"" << arg0.getWideString() << L"\"" << Endl;
#endif
	}
	self->setPlaying(false);
}

bool AsMovieClip::MovieClip_hitTest_1(const FlashSpriteInstance* self, const FlashCharacterInstance* shape) const
{
	if (!shape)
		return false;

	Aabb2 bounds = self->getBounds();
	Aabb2 shapeBounds = shape->getBounds();

	return
		(bounds.mn.x < shapeBounds.mx.x) &&
		(bounds.mn.y < shapeBounds.mx.y) &&
		(bounds.mx.x > shapeBounds.mn.x) &&
		(bounds.mx.y > shapeBounds.mn.y);
}

bool AsMovieClip::MovieClip_hitTest_2(const FlashSpriteInstance* self, avm_number_t x, avm_number_t y) const
{
	// Transform coordinates into local space.
	Matrix33 Tinv = self->getFullTransform().inverse();
	Vector2 L = Tinv * Vector2(x, y);

	// Check if inside bounding box.
	Aabb2 bounds = self->getLocalBounds();
	return (L.x >= bounds.mn.x && L.y >= bounds.mn.y && L.x <= bounds.mx.x && L.y <= bounds.mx.y);
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

void AsMovieClip::MovieClip_lineTo(FlashSpriteInstance* self, avm_number_t x, avm_number_t y) const
{
	self->createCanvas()->lineTo(x * 20.0f, y * 20.0f);
}

Ref< FlashSpriteInstance > AsMovieClip::MovieClip_loadMovie(FlashSpriteInstance* self, const std::wstring& fileName) const
{
	ActionContext* cx = getContext();
	T_ASSERT (cx);

	const IFlashMovieLoader* movieLoader = cx->getMovieLoader();
	if (!movieLoader)
		return 0;

	Ref< FlashMovie > movie = movieLoader->load(fileName);
	if (!movie)
		return 0;

	return movie->createExternalMovieClipInstance(self, 0);
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

void AsMovieClip::MovieClip_moveTo(FlashSpriteInstance* self, avm_number_t x, avm_number_t y) const
{
	self->createCanvas()->moveTo(x * 20.0f, y * 20.0f);
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
	self->removeMovieClip();
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
		Ref< FlashSpriteInstance > targetClipInstance = arg0.getObject()->getRelay< FlashSpriteInstance >();
		if (targetClipInstance)
			targetDepth = parentClipInstance->getDisplayList().getObjectDepth(targetClipInstance);
	}

	parentClipInstance->getDisplayList().swap(depth, targetDepth);
}

std::string AsMovieClip::MovieClip_toString(const FlashSpriteInstance* self) const
{
	std::string target = self->getTarget();
	return "_level0" + replaceAll(target, '/', '.');
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
	return colorTransform.mul.getAlpha() * 100.0f;
}

void AsMovieClip::MovieClip_set_alpha(FlashSpriteInstance* self, avm_number_t alpha) const
{
	SwfCxTransform colorTransform = self->getColorTransform();
	colorTransform.mul.setAlpha(Scalar(alpha / 100.0f));
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

bool AsMovieClip::MovieClip_get_cacheAsBitmap(const FlashSpriteInstance* self) const
{
	return self->getCacheAsBitmap();
}

void AsMovieClip::MovieClip_set_cacheAsBitmap(FlashSpriteInstance* self, bool enabled) const
{
	self->setCacheAsBitmap(enabled);
}

int32_t AsMovieClip::MovieClip_get_currentframe(const FlashSpriteInstance* self) const
{
	return self->getCurrentFrame() + 1;
}

void AsMovieClip::MovieClip_get_droptarget(const FlashSpriteInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"MovieClip::get_droptarget not implemented" << Endl;
	)
}

bool AsMovieClip::MovieClip_get_enabled(const FlashSpriteInstance* self) const
{
	return self->isEnabled();
}

void AsMovieClip::MovieClip_set_enabled(FlashSpriteInstance* self, bool enabled) const
{
	self->setEnabled(enabled);
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
	return self->getSprite()->getFrameCount();
}

float AsMovieClip::MovieClip_get_height(const FlashSpriteInstance* self) const
{
	return self->getHeight();
}

void AsMovieClip::MovieClip_set_height(FlashSpriteInstance* self, float height) const
{
	self->setHeight(height);
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

ActionValue AsMovieClip::MovieClip_get_name(const FlashSpriteInstance* self) const
{
	std::string name = self->getName();
	return !name.empty() ? ActionValue(name) : ActionValue();
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

void AsMovieClip::MovieClip_set_opaqueBackground(FlashSpriteInstance* self, const ActionValue& value) const
{
	self->setOpaqueBackground(!value.isUndefined());
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
	return self->getRotation();
}

void AsMovieClip::MovieClip_set_rotation(FlashSpriteInstance* self, float rotation) const
{
	self->setRotation(rotation);
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

std::string AsMovieClip::MovieClip_get_target(const FlashSpriteInstance* self) const
{
	return self->getTarget();
}

uint32_t AsMovieClip::MovieClip_get_totalframes(const FlashSpriteInstance* self) const
{
	return self->getSprite()->getFrameCount();
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

std::string AsMovieClip::MovieClip_get_url(const FlashSpriteInstance* self) const
{
	return "file:///Internal.swf";
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
	return self->getWidth();
}

void AsMovieClip::MovieClip_set_width(FlashSpriteInstance* self, float width) const
{
	self->setWidth(width);
}

float AsMovieClip::MovieClip_get_x(const FlashSpriteInstance* self) const
{
	return self->getX();
}

void AsMovieClip::MovieClip_set_x(FlashSpriteInstance* self, float x) const
{
	self->setX(x);
}

int32_t AsMovieClip::MovieClip_get_xmouse(const FlashSpriteInstance* self) const
{
	return self->getMouseX();
}

float AsMovieClip::MovieClip_get_xscale(const FlashSpriteInstance* self) const
{
	return self->getXScale();
}

void AsMovieClip::MovieClip_set_xscale(FlashSpriteInstance* self, float x) const
{
	self->setXScale(x);
}

float AsMovieClip::MovieClip_get_y(const FlashSpriteInstance* self) const
{
	return self->getY();
}

void AsMovieClip::MovieClip_set_y(FlashSpriteInstance* self, float y) const
{
	self->setY(y);
}

int32_t AsMovieClip::MovieClip_get_ymouse(const FlashSpriteInstance* self) const
{
	return self->getMouseY();
}

float AsMovieClip::MovieClip_get_yscale(const FlashSpriteInstance* self) const
{
	return self->getYScale();
}

void AsMovieClip::MovieClip_set_yscale(FlashSpriteInstance* self, float y) const
{
	self->setYScale(y);
}

	}
}

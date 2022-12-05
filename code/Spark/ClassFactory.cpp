/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Core/Class/IRuntimeDelegate.h"
#include "Core/Class/Boxes/BoxedAabb2.h"
#include "Core/Class/Boxes/BoxedColor4f.h"
#include "Core/Class/Boxes/BoxedGuid.h"
#include "Core/Class/Boxes/BoxedMatrix33.h"
#include "Core/Class/Boxes/BoxedRefArray.h"
#include "Core/Io/IStream.h"
#include "Drawing/Image.h"
#include "Spark/DefaultCharacterFactory.h"
#include "Spark/BitmapImage.h"
#include "Spark/BitmapResource.h"
#include "Spark/BitmapTexture.h"
#include "Spark/Button.h"
#include "Spark/ButtonInstance.h"
#include "Spark/Canvas.h"
#include "Spark/Character.h"
#include "Spark/ClassFactory.h"
#include "Spark/Context.h"
#include "Spark/Dictionary.h"
#include "Spark/Edit.h"
#include "Spark/EditInstance.h"
#include "Spark/Event.h"
#include "Spark/Font.h"
#include "Spark/Frame.h"
#include "Spark/Key.h"
#include "Spark/Movie.h"
#include "Spark/MovieLoader.h"
#include "Spark/MoviePlayer.h"
#include "Spark/MorphShape.h"
#include "Spark/MorphShapeInstance.h"
#include "Spark/Mouse.h"
#include "Spark/Optimizer.h"
#include "Spark/Shape.h"
#include "Spark/ShapeInstance.h"
#include "Spark/Sound.h"
#include "Spark/Sprite.h"
#include "Spark/SpriteInstance.h"
#include "Spark/Stage.h"
#include "Spark/Text.h"
#include "Spark/TextFormat.h"
#include "Spark/TextInstance.h"
#include "Spark/Swf/SwfMovieFactory.h"
#include "Spark/Swf/SwfReader.h"
#include "Render/ITexture.h"

namespace traktor
{
	namespace spark
	{
		namespace
		{

void ColorTransform_setMul(ColorTransform* self, const Color4f& mul)
{
	self->mul = mul;
}

const Color4f& ColorTransform_getMul(ColorTransform* self)
{
	return self->mul;
}

void ColorTransform_setAdd(ColorTransform* self, const Color4f& add)
{
	self->add = add;
}

const Color4f& ColorTransform_getAdd(ColorTransform* self)
{
	return self->add;
}

Any Font_getBounds(Font* self, uint16_t index)
{
	const Aabb2* bounds = self->getBounds(index);
	if (bounds)
		return CastAny< Aabb2 >::set(*bounds);
	else
		return Any();
}

Any Dictionary_getExportId(Dictionary* self, const std::string& exportName)
{
	uint16_t exportId;
	if (self->getExportId(exportName, exportId))
		return Any::fromInt32(exportId);
	else
		return Any();
}

Any Dictionary_getExportName(Dictionary* self, uint16_t exportId)
{
	std::string exportName;
	if (self->getExportName(exportId, exportName))
		return Any::fromString(exportName);
	else
		return Any();
}

void CharacterInstance_setColorTransform(CharacterInstance* self, const Color4f& mul, const Color4f& add)
{
	self->setColorTransform(ColorTransform(mul, add));
}

Ref< SpriteInstance > SpriteInstance_duplicateMovieClip_1(SpriteInstance* self, const std::string& cloneName, int32_t depth)
{
	return self->duplicateMovieClip(cloneName, depth);
}

Ref< SpriteInstance > SpriteInstance_duplicateMovieClip_2(SpriteInstance* self, const std::string& cloneName, int32_t depth, SpriteInstance* intoParent)
{
	return self->duplicateMovieClip(cloneName, depth, intoParent);
}

uint32_t Shape_getPathCount(Shape* self)
{
	return uint32_t(self->getPaths().size());
}

void DisplayList_removeObject(DisplayList* self, const Any& item)
{
	if (item.isObject< CharacterInstance >())
		self->removeObject(item.getObjectUnsafe< CharacterInstance >());
	else if (item.isNumeric())
		self->removeObject(item.getInt32());
}

RefArray< CharacterInstance > DisplayList_getObjects(DisplayList* self)
{
	RefArray< CharacterInstance > characters;
	self->getObjects(characters);
	return characters;
}

DisplayList* SpriteInstance_getDisplayList(SpriteInstance* self)
{
	return &self->getDisplayList();
}

Vector2 EditInstance_measureText_1(EditInstance* self, const std::wstring& text)
{
	return self->measureText(text);
}

Vector2 EditInstance_measureText_2(EditInstance* self, const std::wstring& text, float width)
{
	return self->measureText(text, width);
}

void EditInstance_setTextFormat_1(EditInstance* self, const TextFormat* textFormat)
{
	self->setTextFormat(textFormat);
}

Ref< TextFormat > EditInstance_getTextFormat_0(EditInstance* self)
{
	return self->getTextFormat();
}

void EditInstance_setTextFormat_3(EditInstance* self, const TextFormat* textFormat, int32_t beginIndex, int32_t endIndex)
{
	self->setTextFormat(textFormat, beginIndex, endIndex);
}

Ref< TextFormat > EditInstance_getTextFormat_2(EditInstance* self, int32_t beginIndex, int32_t endIndex)
{
	return self->getTextFormat(beginIndex, endIndex);
}

Ref< TextFormat > TextFormat_ctor(float letterSpacing, int32_t align, float size)
{
	return new TextFormat(letterSpacing, (SwfTextAlignType)align, size);
}

void TextFormat_setAlign(TextFormat* self, int32_t align)
{
	self->setAlign((SwfTextAlignType)align);
}

int32_t TextFormat_getAlign(TextFormat* self)
{
	return (int32_t)self->getAlign();
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spark.ClassFactory", 0, ClassFactory, IRuntimeClassFactory)

void ClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	// Bitmap
	auto classBitmap = new AutoRuntimeClass< Bitmap >();
	classBitmap->addProperty("X", &Bitmap::getX);
	classBitmap->addProperty("Y", &Bitmap::getY);
	classBitmap->addProperty("width", &Bitmap::getWidth);
	classBitmap->addProperty("height", &Bitmap::getHeight);
	registrar->registerClass(classBitmap);

	// BitmapImage
	auto classBitmapImage = new AutoRuntimeClass< BitmapImage >();
	classBitmapImage->addConstructor< const drawing::Image* >();
	classBitmapImage->addProperty("image", &BitmapImage::getImage);
	registrar->registerClass(classBitmapImage);

	// BitmapResource
	auto classBitmapResource = new AutoRuntimeClass< BitmapResource >();
	classBitmapResource->addProperty("atlasWidth", &BitmapResource::getAtlasWidth);
	classBitmapResource->addProperty("atlasHeight", &BitmapResource::getAtlasHeight);
	classBitmapResource->addProperty("resourceId", &BitmapResource::getResourceId);
	registrar->registerClass(classBitmapResource);

	// BitmapTexture
	auto classBitmapTexture = new AutoRuntimeClass< BitmapTexture >();
	classBitmapTexture->addConstructor< render::ITexture* >();
	classBitmapTexture->addProperty("texture", &BitmapTexture::getTexture);
	registrar->registerClass(classBitmapTexture);

	// Button
	auto classButton = new AutoRuntimeClass< Button >();
	registrar->registerClass(classButton);

	// ButtonInstance
	auto classButtonInstance = new AutoRuntimeClass< ButtonInstance >();
	classButtonInstance->addProperty("eventPress", &ButtonInstance::getEventPress);
	classButtonInstance->addProperty("eventRelease", &ButtonInstance::getEventRelease);
	classButtonInstance->addProperty("eventReleaseOutside", &ButtonInstance::getEventReleaseOutside);
	classButtonInstance->addProperty("eventRollOver", &ButtonInstance::getEventRollOver);
	classButtonInstance->addProperty("eventRollOut", &ButtonInstance::getEventRollOut);
	registrar->registerClass(classButtonInstance);

	// Canvas
	auto classCanvas = new AutoRuntimeClass< Canvas >();
	classCanvas->addProperty("bounds", &Canvas::getBounds);
	classCanvas->addMethod("clear", &Canvas::clear);
	classCanvas->addMethod("beginFill", &Canvas::beginFill);
	classCanvas->addMethod("beginBitmapFill", &Canvas::beginBitmapFill);
	classCanvas->addMethod("endFill", &Canvas::endFill);
	classCanvas->addMethod("moveTo", &Canvas::moveTo);
	classCanvas->addMethod("lineTo", &Canvas::lineTo);
	classCanvas->addMethod("curveTo", &Canvas::curveTo);
	registrar->registerClass(classCanvas);

	// Character
	auto classCharacter = new AutoRuntimeClass< Character >();
	registrar->registerClass(classCharacter);

	// CharacterInstance
	auto classCharacterInstance = new AutoRuntimeClass< CharacterInstance >();
	classCharacterInstance->addConstant("SbmDefault", Any::fromInt32(SbmDefault));
	classCharacterInstance->addConstant("SbmNormal", Any::fromInt32(SbmNormal));
	classCharacterInstance->addConstant("SbmLayer", Any::fromInt32(SbmLayer));
	classCharacterInstance->addConstant("SbmMultiply", Any::fromInt32(SbmMultiply));
	classCharacterInstance->addConstant("SbmScreen", Any::fromInt32(SbmScreen));
	classCharacterInstance->addConstant("SbmLighten", Any::fromInt32(SbmLighten));
	classCharacterInstance->addConstant("SbmDarken", Any::fromInt32(SbmDarken));
	classCharacterInstance->addConstant("SbmDifference", Any::fromInt32(SbmDifference));
	classCharacterInstance->addConstant("SbmAdd", Any::fromInt32(SbmAdd));
	classCharacterInstance->addConstant("SbmSubtract", Any::fromInt32(SbmSubtract));
	classCharacterInstance->addConstant("SbmInvert", Any::fromInt32(SbmInvert));
	classCharacterInstance->addConstant("SbmAlpha", Any::fromInt32(SbmAlpha));
	classCharacterInstance->addConstant("SbmErase", Any::fromInt32(SbmErase));
	classCharacterInstance->addConstant("SbmOverlay", Any::fromInt32(SbmOverlay));
	classCharacterInstance->addConstant("SbmHardlight", Any::fromInt32(SbmHardlight));
	classCharacterInstance->addConstant("SbmOpaque", Any::fromInt32(SbmOpaque));
	classCharacterInstance->addProperty("context", &CharacterInstance::getContext);
	classCharacterInstance->addProperty("dictionary", &CharacterInstance::getDictionary);
	classCharacterInstance->addProperty("parent", &CharacterInstance::getParent);
	classCharacterInstance->addProperty("name", &CharacterInstance::setName, &CharacterInstance::getName);
	classCharacterInstance->addProperty("target", &CharacterInstance::getTarget);
	classCharacterInstance->addProperty("colorTransform", &CharacterInstance::setColorTransform, &CharacterInstance::getColorTransform);
	classCharacterInstance->addProperty("alpha", &CharacterInstance::setAlpha, &CharacterInstance::getAlpha);
	classCharacterInstance->addProperty("transform", &CharacterInstance::setTransform, &CharacterInstance::getTransform);
	classCharacterInstance->addProperty("fullTransform", &CharacterInstance::getFullTransform);
	classCharacterInstance->addProperty("filter", &CharacterInstance::setFilter, &CharacterInstance::getFilter);
	classCharacterInstance->addProperty("filterColor", &CharacterInstance::setFilterColor, &CharacterInstance::getFilterColor);
	classCharacterInstance->addProperty("blendMode", &CharacterInstance::setBlendMode, &CharacterInstance::getBlendMode);
	classCharacterInstance->addProperty("visible", &CharacterInstance::setVisible, &CharacterInstance::isVisible);
	classCharacterInstance->addProperty("enabled", &CharacterInstance::setEnabled, &CharacterInstance::isEnabled);
	classCharacterInstance->addProperty("focus", &CharacterInstance::haveFocus);
	classCharacterInstance->addProperty("wireOutline", &CharacterInstance::setWireOutline, &CharacterInstance::getWireOutline);
	classCharacterInstance->addProperty("bounds", &CharacterInstance::getBounds);
	classCharacterInstance->addProperty("eventSetFocus", &CharacterInstance::getEventSetFocus);
	classCharacterInstance->addProperty("eventKillFocus", &CharacterInstance::getEventKillFocus);
	classCharacterInstance->addStaticMethod("getInstanceCount", &CharacterInstance::getInstanceCount);
	classCharacterInstance->addMethod("destroy", &CharacterInstance::destroy);
	classCharacterInstance->addMethod("setColorTransform", &CharacterInstance::setColorTransform);
	classCharacterInstance->addMethod("setColorTransform", &CharacterInstance_setColorTransform);
	classCharacterInstance->addMethod("transformInto", &CharacterInstance::transformInto);
	classCharacterInstance->addMethod("setFocus", &CharacterInstance::setFocus);
	registrar->registerClass(classCharacterInstance);

	// ColorTransform
	auto classColorTransform = new AutoRuntimeClass< ColorTransform >();
	classColorTransform->addConstructor();
	classColorTransform->addConstructor< const Color4f&, const Color4f& >();
	classColorTransform->addProperty("mul", &ColorTransform_setMul, &ColorTransform_getMul);
	classColorTransform->addProperty("add", &ColorTransform_setAdd, &ColorTransform_getAdd);
	registrar->registerClass(classColorTransform);

	// Context
	auto classContext = new AutoRuntimeClass< Context >();
	classContext->addProperty("movie", &Context::getMovie);
	classContext->addProperty("key", &Context::getKey);
	classContext->addProperty("mouse", &Context::getMouse);
	classContext->addProperty("sound", &Context::getSound);
	classContext->addProperty("stage", &Context::getStage);
	classContext->addProperty("movieClip", &Context::getMovieClip);
	classContext->addProperty("focus", &Context::getFocus);
	classContext->addProperty("pressed", &Context::getPressed);
	classContext->addProperty("rolledOver", &Context::getRolledOver);
	registrar->registerClass(classContext);

	// DefaultCharacterFactory
	auto classDefaultCharacterFactory = new AutoRuntimeClass< DefaultCharacterFactory >();
	classDefaultCharacterFactory->addConstructor();
	registrar->registerClass(classDefaultCharacterFactory);

	// Dictionary
	auto classDictionary = new AutoRuntimeClass< Dictionary >();
	classDictionary->addMethod("addFont", &Dictionary::addFont);
	classDictionary->addMethod("addBitmap", &Dictionary::addBitmap);
	classDictionary->addMethod("addSound", &Dictionary::addSound);
	classDictionary->addMethod("addCharacter", &Dictionary::addCharacter);
	classDictionary->addMethod("getFont", &Dictionary::getFont);
	classDictionary->addMethod("getBitmap", &Dictionary::getBitmap);
	classDictionary->addMethod("getSound", &Dictionary::getSound);
	classDictionary->addMethod("getCharacter", &Dictionary::getCharacter);
	classDictionary->addMethod("getExportId", &Dictionary_getExportId);
	classDictionary->addMethod("getExportName", &Dictionary_getExportName);
	registrar->registerClass(classDictionary);

	// DisplayList
	auto classDisplayList = new AutoRuntimeClass< DisplayList >();
	classDisplayList->addProperty("nextHighestDepth", &DisplayList::getNextHighestDepth);
	classDisplayList->addProperty("objects", &DisplayList_getObjects);
	classDisplayList->addMethod("reset", &DisplayList::reset);
	classDisplayList->addMethod("showObject", &DisplayList::showObject);
	classDisplayList->addMethod("removeObject", &DisplayList_removeObject);
	classDisplayList->addMethod("getObjectDepth", &DisplayList::getObjectDepth);
	classDisplayList->addMethod("getNextHighestDepthInRange", &DisplayList::getNextHighestDepthInRange);
	classDisplayList->addMethod("swap", &DisplayList::swap);
	registrar->registerClass(classDisplayList);

	// Edit
	auto classEdit = new AutoRuntimeClass< Edit >();
	classEdit->addProperty("fontId", &Edit::getFontId);
	classEdit->addProperty("fontHeight", &Edit::getFontHeight);
	classEdit->addProperty("textBounds", &Edit::getTextBounds);
	classEdit->addProperty("textColor", &Edit::getTextColor);
	classEdit->addProperty("maxLength", &Edit::getMaxLength);
	classEdit->addProperty("initialText", &Edit::setInitialText, &Edit::getInitialText);
	classEdit->addProperty("leftMargin", &Edit::getLeftMargin);
	classEdit->addProperty("rightMargin", &Edit::getRightMargin);
	classEdit->addProperty("indent", &Edit::getIndent);
	classEdit->addProperty("leading", &Edit::getLeading);
	classEdit->addProperty("readOnly", &Edit::readOnly);
	classEdit->addProperty("wordWrap", &Edit::wordWrap);
	classEdit->addProperty("multiLine", &Edit::multiLine);
	classEdit->addProperty("password", &Edit::password);
	classEdit->addProperty("renderHtml", &Edit::renderHtml);
	registrar->registerClass(classEdit);

	// EditInstance
	auto classEditInstance = new AutoRuntimeClass< EditInstance >();
	classEditInstance->addProperty("edit", &EditInstance::getEdit);
	classEditInstance->addProperty("textBounds", &EditInstance::setTextBounds, &EditInstance::getTextBounds);
	classEditInstance->addProperty("textColor", &EditInstance::setTextColor, &EditInstance::getTextColor);
	classEditInstance->addProperty("letterSpacing", &EditInstance::setLetterSpacing, &EditInstance::getLetterSpacing);
	classEditInstance->addProperty("text", &EditInstance::getText);
	classEditInstance->addProperty("htmlText", &EditInstance::getHtmlText);
	classEditInstance->addProperty("multiLine", &EditInstance::setMultiLine, &EditInstance::getMultiLine);
	classEditInstance->addProperty("wordWrap", &EditInstance::setWordWrap, &EditInstance::getWordWrap);
	classEditInstance->addProperty("password", &EditInstance::setPassword, &EditInstance::getPassword);
	classEditInstance->addProperty("caret", &EditInstance::getCaret);
	classEditInstance->addProperty("scroll", &EditInstance::setScroll, &EditInstance::getScroll);
	classEditInstance->addProperty("maxScroll", &EditInstance::getMaxScroll);
	classEditInstance->addProperty("renderClipMask", &EditInstance::setRenderClipMask, &EditInstance::getRenderClipMask);
	classEditInstance->addProperty("position", &EditInstance::setPosition, &EditInstance::getPosition);
	classEditInstance->addProperty("X", &EditInstance::setX, &EditInstance::getX);
	classEditInstance->addProperty("Y", &EditInstance::setY, &EditInstance::getY);
	classEditInstance->addProperty("size", &EditInstance::setSize, &EditInstance::getSize);
	classEditInstance->addProperty("width", &EditInstance::setWidth, &EditInstance::getWidth);
	classEditInstance->addProperty("height", &EditInstance::setHeight, &EditInstance::getHeight);
	classEditInstance->addProperty("rotation", &EditInstance::setRotation, &EditInstance::getRotation);
	classEditInstance->addProperty("scale", &EditInstance::setScale, &EditInstance::getScale);
	classEditInstance->addProperty("xScale", &EditInstance::setXScale, &EditInstance::getXScale);
	classEditInstance->addProperty("yScale", &EditInstance::setYScale, &EditInstance::getYScale);
	classEditInstance->addProperty("textSize", &EditInstance::getTextSize);
	classEditInstance->addProperty("textWidth", &EditInstance::getTextWidth);
	classEditInstance->addProperty("textHeight", &EditInstance::getTextHeight);
	classEditInstance->addProperty("eventChanged", &EditInstance::getEventChanged);
	classEditInstance->addMethod("parseText", &EditInstance::parseText);
	classEditInstance->addMethod("parseHtml", &EditInstance::parseHtml);
	classEditInstance->addMethod("measureText", &EditInstance_measureText_1);
	classEditInstance->addMethod("measureText", &EditInstance_measureText_2);
	classEditInstance->addMethod("setTextFormat", &EditInstance_setTextFormat_1);
	classEditInstance->addMethod("getTextFormat", &EditInstance_getTextFormat_0);
	classEditInstance->addMethod("setTextFormat", &EditInstance_setTextFormat_3);
	classEditInstance->addMethod("getTextFormat", &EditInstance_getTextFormat_2);
	registrar->registerClass(classEditInstance);

	// Event
	auto classEvent = new AutoRuntimeClass< Event >();
	classEvent->addMethod("add", &Event::add);
	classEvent->addMethod("remove", &Event::remove);
	classEvent->addMethod("removeAll", &Event::removeAll);
	registrar->registerClass(classEvent);

	// FillStyle

	// Font
	auto classFont = new AutoRuntimeClass< Font >();
	classFont->addProperty("fontName", &Font::getFontName);
	classFont->addProperty("italic", &Font::isItalic);
	classFont->addProperty("bold", &Font::isBold);
	classFont->addProperty("ascent", &Font::getAscent);
	classFont->addProperty("descent", &Font::getDescent);
	classFont->addProperty("leading", &Font::getLeading);
	classFont->addProperty("maxDimension", &Font::getMaxDimension);
	classFont->addMethod("getAdvance", &Font::getAdvance);
	classFont->addMethod("getBounds", &Font_getBounds);
	classFont->addMethod("lookupKerning", &Font::lookupKerning);
	classFont->addMethod("lookupIndex", &Font::lookupIndex);
	registrar->registerClass(classFont);

	// ICharacterFactory
	auto classICharacterFactory = new AutoRuntimeClass< ICharacterFactory >();
	registrar->registerClass(classICharacterFactory);

	// IMovieLoader
	auto classIMovieLoader = new AutoRuntimeClass< IMovieLoader >();
	classIMovieLoader->addMethod("loadAsync", &IMovieLoader::loadAsync);
	classIMovieLoader->addMethod("load", &IMovieLoader::load);
	registrar->registerClass(classIMovieLoader);

	// IMovieLoader::IHandle
	Ref< AutoRuntimeClass< IMovieLoader::IHandle > > classIMovieLoader_IHandle = new AutoRuntimeClass< IMovieLoader::IHandle >();
	classIMovieLoader_IHandle->addProperty("ready", &IMovieLoader::IHandle::ready);
	classIMovieLoader_IHandle->addProperty("succeeded", &IMovieLoader::IHandle::succeeded);
	classIMovieLoader_IHandle->addProperty("failed", &IMovieLoader::IHandle::failed);
	classIMovieLoader_IHandle->addMethod("wait", &IMovieLoader::IHandle::wait);
	classIMovieLoader_IHandle->addMethod("get", &IMovieLoader::IHandle::get);
	registrar->registerClass(classIMovieLoader_IHandle);

	// Key
	auto classKey = new AutoRuntimeClass< Key >();
	classKey->addConstant("AkBackspace", Any::fromInt32(Key::AkBackspace));
	classKey->addConstant("AkCapsLock", Any::fromInt32(Key::AkCapsLock));
	classKey->addConstant("AkControl", Any::fromInt32(Key::AkControl));
	classKey->addConstant("AkDeleteKey", Any::fromInt32(Key::AkDeleteKey));
	classKey->addConstant("AkDown", Any::fromInt32(Key::AkDown));
	classKey->addConstant("AkEnd", Any::fromInt32(Key::AkEnd));
	classKey->addConstant("AkEnter", Any::fromInt32(Key::AkEnter));
	classKey->addConstant("AkEscape", Any::fromInt32(Key::AkEscape));
	classKey->addConstant("AkHome", Any::fromInt32(Key::AkHome));
	classKey->addConstant("AkInsert", Any::fromInt32(Key::AkInsert));
	classKey->addConstant("AkLeft", Any::fromInt32(Key::AkLeft));
	classKey->addConstant("AkPgDn", Any::fromInt32(Key::AkPgDn));
	classKey->addConstant("AkPgUp", Any::fromInt32(Key::AkPgUp));
	classKey->addConstant("AkRight", Any::fromInt32(Key::AkRight));
	classKey->addConstant("AkShift", Any::fromInt32(Key::AkShift));
	classKey->addConstant("AkSpace", Any::fromInt32(Key::AkSpace));
	classKey->addConstant("AkTab", Any::fromInt32(Key::AkTab));
	classKey->addConstant("AkUp", Any::fromInt32(Key::AkUp));
	classKey->addProperty("ascii", &Key::getAscii);
	classKey->addProperty("code", &Key::getCode);
	classKey->addProperty("eventKeyDown", &Key::getEventKeyDown);
	classKey->addProperty("eventKeyUp", &Key::getEventKeyUp);
	classKey->addMethod("isDown", &Key::isDown);
	registrar->registerClass(classKey);

	// LineStyle

	// MorphShape
	auto classMorphShape = new AutoRuntimeClass< MorphShape >();
	classMorphShape->addProperty("shapeBounds", &MorphShape::getShapeBounds);
	registrar->registerClass(classMorphShape);

	// MorphShapeInstance
	auto classMorphShapeInstance = new AutoRuntimeClass< MorphShapeInstance >();
	classMorphShapeInstance->addProperty("shape", &MorphShapeInstance::getShape);
	registrar->registerClass(classMorphShapeInstance);

	// Mouse
	auto classMouse = new AutoRuntimeClass< Mouse >();
	classMouse->addProperty("eventButtonDown", &Mouse::getEventButtonDown);
	classMouse->addProperty("eventButtonUp", &Mouse::getEventButtonUp);
	classMouse->addProperty("eventMouseMove", &Mouse::getEventMouseMove);
	classMouse->addProperty("eventMouseWheel", &Mouse::getEventMouseWheel);
	registrar->registerClass(classMouse);

	// Movie
	auto classMovie = new AutoRuntimeClass< Movie >();
	classMovie->addProperty("frameBounds", &Movie::getFrameBounds);
	classMovie->addProperty("movieClip", &Movie::getMovieClip);
	classMovie->addMethod("defineFont", &Movie::defineFont);
	classMovie->addMethod("defineBitmap", &Movie::defineBitmap);
	classMovie->addMethod("defineSound", &Movie::defineSound);
	classMovie->addMethod("defineCharacter", &Movie::defineCharacter);
	classMovie->addMethod("setExport", &Movie::setExport);
	classMovie->addMethod("createMovieClipInstance", &Movie::createMovieClipInstance);
	classMovie->addMethod("createExternalMovieClipInstance", &Movie::createExternalMovieClipInstance);
	classMovie->addMethod("createExternalSpriteInstance", &Movie::createExternalSpriteInstance);
	registrar->registerClass(classMovie);

	// MovieLoader
	auto classMovieLoader = new AutoRuntimeClass< MovieLoader >();
	classMovieLoader->addConstructor();
	classMovieLoader->addMethod("setCacheDirectory", &MovieLoader::setCacheDirectory);
	classMovieLoader->addMethod("setMerge", &MovieLoader::setMerge);
	classMovieLoader->addMethod("setTriangulate", &MovieLoader::setTriangulate);
	registrar->registerClass(classMovieLoader);

	// MoviePlayer
	auto classMoviePlayer = new AutoRuntimeClass< MoviePlayer >();
	classMoviePlayer->addProperty("frameCount", &MoviePlayer::getFrameCount);
	classMoviePlayer->addProperty("movieInstance", &MoviePlayer::getMovieInstance);
	classMoviePlayer->addMethod< void, uint32_t >("gotoAndPlay", &MoviePlayer::gotoAndPlay);
	classMoviePlayer->addMethod< void, uint32_t >("gotoAndStop", &MoviePlayer::gotoAndStop);
	classMoviePlayer->addMethod< bool, const std::string& >("gotoAndPlay", &MoviePlayer::gotoAndPlay);
	classMoviePlayer->addMethod< bool, const std::string& >("gotoAndStop", &MoviePlayer::gotoAndStop);
	classMoviePlayer->addMethod("postKeyDown", &MoviePlayer::postKeyDown);
	classMoviePlayer->addMethod("postKeyUp", &MoviePlayer::postKeyUp);
	classMoviePlayer->addMethod("postMouseDown", &MoviePlayer::postMouseDown);
	classMoviePlayer->addMethod("postMouseUp", &MoviePlayer::postMouseUp);
	classMoviePlayer->addMethod("postMouseMove", &MoviePlayer::postMouseMove);
	registrar->registerClass(classMoviePlayer);

	// Optimizer
	auto classOptimizer = new AutoRuntimeClass< Optimizer >();
	classOptimizer->addConstructor();
	classOptimizer->addMethod("merge", &Optimizer::merge);
	classOptimizer->addMethod("triangulate", &Optimizer::triangulate);
	registrar->registerClass(classOptimizer);

	// Shape
	auto classShape = new AutoRuntimeClass< Shape >();
	classShape->addProperty("pathCount", &Shape_getPathCount);
	registrar->registerClass(classShape);

	// ShapeInstance
	auto classShapeInstance = new AutoRuntimeClass< ShapeInstance >();
	classShapeInstance->addProperty("shape", &ShapeInstance::getShape);
	registrar->registerClass(classShapeInstance);

	// Sound
	auto classSound = new AutoRuntimeClass< Sound >();
	classSound->addProperty("channels", &Sound::getChannels);
	classSound->addProperty("sampleRate", &Sound::getSampleRate);
	classSound->addProperty("sampleCount", &Sound::getSampleCount);
	registrar->registerClass(classSound);

	// Sprite
	auto classSprite = new AutoRuntimeClass< Sprite >();
	classSprite->addProperty("frameRate", &Sprite::getFrameRate);
	classSprite->addProperty("frameCount", &Sprite::getFrameCount);
	classSprite->addMethod("addFrame", &Sprite::addFrame);
	classSprite->addMethod("getFrame", &Sprite::getFrame);
	classSprite->addMethod("findFrame", &Sprite::findFrame);
	registrar->registerClass(classSprite);

	// SpriteInstance
	auto classSpriteInstance = new AutoRuntimeClass< SpriteInstance >();
	classSpriteInstance->addProperty("sprite", &SpriteInstance::getSprite);
	classSpriteInstance->addProperty("cacheAsBitmap", &SpriteInstance::setCacheAsBitmap, &SpriteInstance::getCacheAsBitmap);
	classSpriteInstance->addProperty("currentFrame", &SpriteInstance::getCurrentFrame);
	classSpriteInstance->addProperty("playing", &SpriteInstance::getPlaying);
	classSpriteInstance->addProperty("displayList", &SpriteInstance_getDisplayList);
	classSpriteInstance->addProperty("localBounds", &SpriteInstance::getLocalBounds);
	classSpriteInstance->addProperty("visibleLocalBounds", &SpriteInstance::getVisibleLocalBounds);
	classSpriteInstance->addProperty("mask", &SpriteInstance::setMask, &SpriteInstance::getMask);
	classSpriteInstance->addProperty("canvas", &SpriteInstance::getCanvas);
	classSpriteInstance->addProperty("mouseX", &SpriteInstance::getMouseX);
	classSpriteInstance->addProperty("mouseY", &SpriteInstance::getMouseY);
	classSpriteInstance->addProperty("position", &SpriteInstance::setPosition, &SpriteInstance::getPosition);
	classSpriteInstance->addProperty("X", &SpriteInstance::setX, &SpriteInstance::getX);
	classSpriteInstance->addProperty("Y", &SpriteInstance::setY, &SpriteInstance::getY);
	classSpriteInstance->addProperty("size", &SpriteInstance::setSize, &SpriteInstance::getSize);
	classSpriteInstance->addProperty("width", &SpriteInstance::setWidth, &SpriteInstance::getWidth);
	classSpriteInstance->addProperty("height", &SpriteInstance::setHeight, &SpriteInstance::getHeight);
	classSpriteInstance->addProperty("rotation", &SpriteInstance::setRotation, &SpriteInstance::getRotation);
	classSpriteInstance->addProperty("scale", &SpriteInstance::setScale, &SpriteInstance::getScale);
	classSpriteInstance->addProperty("xScale", &SpriteInstance::setXScale, &SpriteInstance::getXScale);
	classSpriteInstance->addProperty("yScale", &SpriteInstance::setYScale, &SpriteInstance::getYScale);
	classSpriteInstance->addProperty("eventEnterFrame", &SpriteInstance::getEventEnterFrame);
	classSpriteInstance->addProperty("eventKeyDown", &SpriteInstance::getEventKeyDown);
	classSpriteInstance->addProperty("eventKeyUp", &SpriteInstance::getEventKeyUp);
	classSpriteInstance->addProperty("eventMouseDown", &SpriteInstance::getEventMouseDown);
	classSpriteInstance->addProperty("eventMouseUp", &SpriteInstance::getEventMouseUp);
	classSpriteInstance->addProperty("eventMouseMove", &SpriteInstance::getEventMouseMove);
	classSpriteInstance->addProperty("eventPress", &SpriteInstance::getEventPress);
	classSpriteInstance->addProperty("eventRelease", &SpriteInstance::getEventRelease);
	classSpriteInstance->addProperty("eventRollOver", &SpriteInstance::getEventRollOver);
	classSpriteInstance->addProperty("eventRollOut", &SpriteInstance::getEventRollOut);
	classSpriteInstance->addMethod("gotoFrame", &SpriteInstance::gotoFrame);
	classSpriteInstance->addMethod("gotoPrevious", &SpriteInstance::gotoPrevious);
	classSpriteInstance->addMethod("gotoNext", &SpriteInstance::gotoNext);
	classSpriteInstance->addMethod< void, uint32_t >("gotoAndPlay", &SpriteInstance::gotoAndPlay);
	classSpriteInstance->addMethod< void, uint32_t >("gotoAndStop", &SpriteInstance::gotoAndStop);
	classSpriteInstance->addMethod< bool, const std::string& >("gotoAndPlay", &SpriteInstance::gotoAndPlay);
	classSpriteInstance->addMethod< bool, const std::string& >("gotoAndStop", &SpriteInstance::gotoAndStop);
	classSpriteInstance->addMethod("setPlaying", &SpriteInstance::setPlaying);
	classSpriteInstance->addMethod("createEmptyMovieClip", &SpriteInstance::createEmptyMovieClip);
	classSpriteInstance->addMethod("createTextField", &SpriteInstance::createTextField);
	classSpriteInstance->addMethod("removeMovieClip", &SpriteInstance::removeMovieClip);
	classSpriteInstance->addMethod("clone", &SpriteInstance::clone);
	classSpriteInstance->addMethod("duplicateMovieClip", &SpriteInstance_duplicateMovieClip_1);
	classSpriteInstance->addMethod("duplicateMovieClip", &SpriteInstance_duplicateMovieClip_2);
	classSpriteInstance->addMethod("attachBitmap", &SpriteInstance::attachBitmap);
	classSpriteInstance->addMethod("getMember", &SpriteInstance::getMember);
	classSpriteInstance->addMethod("createCanvas", &SpriteInstance::createCanvas);
	registrar->registerClass(classSpriteInstance);

	// Stage
	auto classStage = new AutoRuntimeClass< Stage >();
	classStage->addProperty("width", &Stage::getWidth);
	classStage->addProperty("height", &Stage::getHeight);
	classStage->addProperty("align", &Stage::setAlign, &Stage::getAlign);
	classStage->addProperty("scaleMode", &Stage::setScaleMode, &Stage::getScaleMode);
	classStage->addProperty("eventResize", &Stage::getEventResize);
	registrar->registerClass(classStage);

	// SwfMovieFactory
	auto classSwfMovieFactory = new AutoRuntimeClass< SwfMovieFactory >();
	classSwfMovieFactory->addConstructor();
	classSwfMovieFactory->addMethod("createMovie", &SwfMovieFactory::createMovie);
	classSwfMovieFactory->addMethod("createMovieFromImage", &SwfMovieFactory::createMovieFromImage);
	registrar->registerClass(classSwfMovieFactory);

	// SwfReader
	auto classSwfReader = new AutoRuntimeClass< SwfReader >();
	classSwfReader->addConstructor< IStream* >();
	registrar->registerClass(classSwfReader);

	// Text
	auto classText = new AutoRuntimeClass< Text >();
	classText->addProperty("textBounds", &Text::getTextBounds);
	classText->addProperty("textMatrix", &Text::getTextMatrix);
	registrar->registerClass(classText);

	// TextFormat
	auto classTextFormat = new AutoRuntimeClass< TextFormat >();
	classTextFormat->addConstant("StaLeft", Any::fromInt32(StaLeft));
	classTextFormat->addConstant("StaRight", Any::fromInt32(StaRight));
	classTextFormat->addConstant("StaCenter", Any::fromInt32(StaCenter));
	classTextFormat->addConstant("StaJustify", Any::fromInt32(StaJustify));
	classTextFormat->addConstructor< float, int32_t, float >(&TextFormat_ctor);
	classTextFormat->addProperty("letterSpacing", &TextFormat::setLetterSpacing, &TextFormat::getLetterSpacing);
	classTextFormat->addProperty("align", &TextFormat_setAlign, &TextFormat_getAlign);
	classTextFormat->addProperty("size", &TextFormat::setSize, &TextFormat::getSize);
	registrar->registerClass(classTextFormat);

	// TextInstance
	auto classTextInstance = new AutoRuntimeClass< TextInstance >();
	classTextInstance->addProperty("text", &TextInstance::getText);
	registrar->registerClass(classTextInstance);
}

	}
}

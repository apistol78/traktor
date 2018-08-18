/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/Boxes.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Core/Io/IStream.h"
#include "Drawing/Image.h"
#include "Flash/DefaultCharacterFactory.h"
#include "Flash/BitmapImage.h"
#include "Flash/BitmapResource.h"
#include "Flash/BitmapTexture.h"
#include "Flash/Canvas.h"
#include "Flash/Cast.h"
#include "Flash/Character.h"
#include "Flash/ClassFactory.h"
#include "Flash/Dictionary.h"
#include "Flash/Edit.h"
#include "Flash/EditInstance.h"
#include "Flash/Font.h"
#include "Flash/Frame.h"
#include "Flash/Movie.h"
#include "Flash/MovieFactory.h"
#include "Flash/MovieLoader.h"
#include "Flash/MoviePlayer.h"
#include "Flash/Optimizer.h"
#include "Flash/Shape.h"
#include "Flash/ShapeInstance.h"
#include "Flash/Sound.h"
#include "Flash/Sprite.h"
#include "Flash/SpriteInstance.h"
#include "Flash/TextFormat.h"
#include "Flash/SwfReader.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFunction.h"
#include "Flash/Action/ActionValue.h"
#include "Flash/Action/Common/BitmapData.h"
#include "Render/ISimpleTexture.h"

namespace traktor
{
	namespace flash
	{
		namespace
		{

Any ActionObject_getMember(ActionObject* self, const std::string& memberName)
{
	ActionValue memberValue;
	if (self->getMember(memberName, memberValue))
		return CastAny< ActionValue >::set(memberValue);
	else
		return Any();
}

Any ActionObject_getMemberByQName(ActionObject* self, const std::string& memberName)
{
	ActionValue memberValue;
	if (self->getMemberByQName(memberName, memberValue))
		return CastAny< ActionValue >::set(memberValue);
	else
		return Any();
}

void ActionObject_setMember(ActionObject* self, const std::string& memberName, const Any& value)
{
	ActionValue memberValue = CastAny< ActionValue >::get(value);
	self->setMember(memberName, memberValue);
}

Any ActionObject_getProperty(ActionObject* self, const std::string& propertyName)
{
	Ref< ActionFunction > propertyGetFn;
	if (self->getPropertyGet(propertyName, propertyGetFn))
	{
		ActionValue propertyValue = propertyGetFn->call(self);
		return CastAny< ActionValue >::set(propertyValue);
	}
	else
		return Any();
}

void ActionObject_setProperty(ActionObject* self, const std::string& propertyName, const Any& value)
{
	Ref< ActionFunction > propertySetFn;
	if (self->getPropertySet(propertyName, propertySetFn))
	{
		ActionValueArray callArgv(self->getContext()->getPool(), 1);
		callArgv[0] = CastAny< ActionValue >::get(value);
		propertySetFn->call(self, callArgv);
	}
}

Any ActionObject_invoke(ActionObject* self, const std::string& methodName, uint32_t argc, const Any* argv)
{
	ActionValue memberValue;
	if (self->getMember(methodName, memberValue))
	{
		ActionFunction* fn = memberValue.getObject< ActionFunction >();
		if (fn)
		{
			ActionValueArray callArgv(self->getContext()->getPool(), argc);
			for (uint32_t i = 0; i < argc; ++i)
				callArgv[i] = CastAny< ActionValue >::get(argv[i]);
			ActionValue ret = fn->call(self, callArgv);
			return CastAny< ActionValue >::set(ret);
		}
	}
	return Any();
}

Any ActionObjectRelay_invoke(ActionObjectRelay* self, const std::string& methodName, uint32_t argc, const Any* argv)
{
	ActionObject* actionObject = self->getAsObject();
	if (actionObject)
	{
		if (methodName == "getMember")
		{
			ActionValue memberValue;
			if (actionObject->getMember(argv[0].getString(), memberValue))
				return CastAny< ActionValue >::set(memberValue);
		}
		else if (methodName == "getMemberByQName")
		{
			ActionValue memberValue;
			if (actionObject->getMemberByQName(argv[0].getString(), memberValue))
				return CastAny< ActionValue >::set(memberValue);
		}
		else if (methodName == "setMember")
		{
			ActionValue memberValue = CastAny< ActionValue >::get(argv[1]);
			actionObject->setMember(argv[0].getString(), memberValue);
		}
		else if (methodName == "getProperty")
		{
			Ref< ActionFunction > propertyGetFn;
			if (actionObject->getPropertyGet(argv[0].getString(), propertyGetFn))
			{
				ActionValue propertyValue = propertyGetFn->call(actionObject);
				return CastAny< ActionValue >::set(propertyValue);
			}
		}
		else if (methodName == "setProperty")
		{
			Ref< ActionFunction > propertySetFn;
			if (actionObject->getPropertySet(argv[0].getString(), propertySetFn))
			{
				ActionValueArray callArgv(actionObject->getContext()->getPool(), 1);
				callArgv[0] = CastAny< ActionValue >::get(argv[1]);
				propertySetFn->call(actionObject, callArgv);
			}
		}
		else
		{
			ActionValue memberValue;
			if (actionObject->getMember(methodName, memberValue))
			{
				ActionFunction* fn = memberValue.getObject< ActionFunction >();
				if (fn)
				{
					ActionValueArray callArgv(actionObject->getContext()->getPool(), argc);
					for (uint32_t i = 0; i < argc; ++i)
						callArgv[i] = CastAny< ActionValue >::get(argv[i]);
					ActionValue ret = fn->call(actionObject, callArgv);
					return CastAny< ActionValue >::set(ret);
				}
			}
		}
	}
	return Any();
}

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

Any CharacterInstance_invoke(CharacterInstance* self, const std::string& methodName, uint32_t argc, const Any* argv)
{
	return ActionObjectRelay_invoke(self, methodName, argc, argv);
}

Any ShapeInstance_invoke(ShapeInstance* self, const std::string& methodName, uint32_t argc, const Any* argv)
{
	return ActionObjectRelay_invoke(self, methodName, argc, argv);
}

Ref< SpriteInstance > SpriteInstance_duplicateMovieClip_1(SpriteInstance* self, const std::string& cloneName, int32_t depth)
{
	return self->duplicateMovieClip(cloneName, depth);
}

Ref< SpriteInstance > SpriteInstance_duplicateMovieClip_2(SpriteInstance* self, const std::string& cloneName, int32_t depth, SpriteInstance* intoParent)
{
	return self->duplicateMovieClip(cloneName, depth, intoParent);
}

Any SpriteInstance_invoke(SpriteInstance* self, const std::string& methodName, uint32_t argc, const Any* argv)
{
	return ActionObjectRelay_invoke(self, methodName, argc, argv);
}

Any EditInstance_invoke(EditInstance* self, const std::string& methodName, uint32_t argc, const Any* argv)
{
	return ActionObjectRelay_invoke(self, methodName, argc, argv);
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

Ref< ActionObject > ActionContext_createObject_0(ActionContext* self)
{
	return new flash::ActionObject(self);
}

Ref< ActionObject > ActionContext_createObject_N(ActionContext* self, const std::string& prototype, ActionValueArray& args)
{
	ActionValue classFunctionValue;
	self->getGlobal()->getMemberByQName(prototype, classFunctionValue);

	Ref< ActionFunction > classFunction = classFunctionValue.getObject< ActionFunction >();
	if (!classFunction)
	{
		log::error << L"Unable to create object; no such prototype \"" << mbstows(prototype) << L"\"" << Endl;
		return 0;
	}

	ActionValue classPrototypeValue;
	classFunction->getLocalMember(ActionContext::IdPrototype, classPrototypeValue);

	Ref< ActionObject > classPrototype = classPrototypeValue.getObject();
	if (!classPrototype)
	{
		log::error << L"Unable to create object; no such prototype \"" << mbstows(prototype) << L"\"" << Endl;
		return 0;
	}

	Ref< ActionObject > obj = new ActionObject(self, classPrototype);
	obj->setMember(flash::ActionContext::Id__ctor__, classFunctionValue);

	classFunction->call(obj, args);
	return obj;
}

Ref< ActionObject > ActionContext_createObject_1(ActionContext* self, const std::string& prototype)
{
	ActionValueArray args(self->getPool(), 0);
	return ActionContext_createObject_N(self, prototype, args);
}

Ref< ActionObject > ActionContext_createObject_2(ActionContext* self, const std::string& prototype, const Any& arg1)
{
	ActionValueArray args(self->getPool(), 1);
	args[0] = CastAny< ActionValue >::get(arg1);
	return ActionContext_createObject_N(self, prototype, args);
}

Ref< ActionObject > ActionContext_createObject_3(ActionContext* self, const std::string& prototype, const Any& arg1, const Any& arg2)
{
	ActionValueArray args(self->getPool(), 2);
	args[0] = CastAny< ActionValue >::get(arg1);
	args[1] = CastAny< ActionValue >::get(arg2);
	return ActionContext_createObject_N(self, prototype, args);
}

Ref< ActionObject > ActionContext_createObject_4(ActionContext* self, const std::string& prototype, const Any& arg1, const Any& arg2, const Any& arg3)
{
	ActionValueArray args(self->getPool(), 3);
	args[0] = CastAny< ActionValue >::get(arg1);
	args[1] = CastAny< ActionValue >::get(arg2);
	args[2] = CastAny< ActionValue >::get(arg3);
	return ActionContext_createObject_N(self, prototype, args);
}

Ref< ActionObject > ActionContext_createObject_5(ActionContext* self, const std::string& prototype, const Any& arg1, const Any& arg2, const Any& arg3, const Any& arg4)
{
	ActionValueArray args(self->getPool(), 4);
	args[0] = CastAny< ActionValue >::get(arg1);
	args[1] = CastAny< ActionValue >::get(arg2);
	args[2] = CastAny< ActionValue >::get(arg3);
	args[3] = CastAny< ActionValue >::get(arg3);
	return ActionContext_createObject_N(self, prototype, args);
}

Ref< ActionObject > ActionContext_createObject_6(ActionContext* self, const std::string& prototype, const Any& arg1, const Any& arg2, const Any& arg3, const Any& arg4, const Any& arg5)
{
	ActionValueArray args(self->getPool(), 5);
	args[0] = CastAny< ActionValue >::get(arg1);
	args[1] = CastAny< ActionValue >::get(arg2);
	args[2] = CastAny< ActionValue >::get(arg3);
	args[3] = CastAny< ActionValue >::get(arg4);
	args[4] = CastAny< ActionValue >::get(arg5);
	return ActionContext_createObject_N(self, prototype, args);
}

Ref< ActionObject > ActionContext_createObject_7(ActionContext* self, const std::string& prototype, const Any& arg1, const Any& arg2, const Any& arg3, const Any& arg4, const Any& arg5, const Any& arg6)
{
	ActionValueArray args(self->getPool(), 6);
	args[0] = CastAny< ActionValue >::get(arg1);
	args[1] = CastAny< ActionValue >::get(arg2);
	args[2] = CastAny< ActionValue >::get(arg3);
	args[3] = CastAny< ActionValue >::get(arg4);
	args[4] = CastAny< ActionValue >::get(arg5);
	args[5] = CastAny< ActionValue >::get(arg6);
	return ActionContext_createObject_N(self, prototype, args);
}

Ref< ActionObject > ActionContext_createObject_8(ActionContext* self, const std::string& prototype, const Any& arg1, const Any& arg2, const Any& arg3, const Any& arg4, const Any& arg5, const Any& arg6, const Any& arg7)
{
	ActionValueArray args(self->getPool(), 7);
	args[0] = CastAny< ActionValue >::get(arg1);
	args[1] = CastAny< ActionValue >::get(arg2);
	args[2] = CastAny< ActionValue >::get(arg3);
	args[3] = CastAny< ActionValue >::get(arg4);
	args[4] = CastAny< ActionValue >::get(arg5);
	args[5] = CastAny< ActionValue >::get(arg6);
	args[6] = CastAny< ActionValue >::get(arg7);
	return ActionContext_createObject_N(self, prototype, args);
}

Ref< ActionObject > ActionContext_createBitmap(ActionContext* self, drawing::Image* image)
{
	Ref< BitmapData > bitmap = new BitmapData(image);
	return bitmap->getAsObject(self);
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.flash.ClassFactory", 0, ClassFactory, IRuntimeClassFactory)

void ClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	Ref< AutoRuntimeClass< ActionObject > > classActionObject = new AutoRuntimeClass< ActionObject >();
	classActionObject->addMethod("getMember", &ActionObject_getMember);
	classActionObject->addMethod("getMemberByQName", &ActionObject_getMemberByQName);
	classActionObject->addMethod("setMember", &ActionObject_setMember);
	classActionObject->addMethod("getProperty", &ActionObject_getProperty);
	classActionObject->addMethod("setProperty", &ActionObject_setProperty);
	classActionObject->setUnknownHandler(&ActionObject_invoke);
	registrar->registerClass(classActionObject);

	Ref< AutoRuntimeClass< ActionObjectRelay > > classActionObjectRelay = new AutoRuntimeClass< ActionObjectRelay >();
	classActionObjectRelay->setUnknownHandler(&ActionObjectRelay_invoke);
	registrar->registerClass(classActionObjectRelay);

	Ref< AutoRuntimeClass< ICharacterFactory > > classICharacterFactory = new AutoRuntimeClass< ICharacterFactory >();
	registrar->registerClass(classICharacterFactory);

	Ref< AutoRuntimeClass< DefaultCharacterFactory > > classDefaultCharacterFactory = new AutoRuntimeClass< DefaultCharacterFactory >();
	classDefaultCharacterFactory->addConstructor();
	registrar->registerClass(classDefaultCharacterFactory);

	Ref< AutoRuntimeClass< IMovieLoader > > classIMovieLoader = new AutoRuntimeClass< IMovieLoader >();
	classIMovieLoader->addMethod("loadAsync", &IMovieLoader::loadAsync);
	classIMovieLoader->addMethod("load", &IMovieLoader::load);
	registrar->registerClass(classIMovieLoader);

	Ref< AutoRuntimeClass< IMovieLoader::IHandle > > classIMovieLoader_IHandle = new AutoRuntimeClass< IMovieLoader::IHandle >();
	classIMovieLoader_IHandle->addProperty("ready", &IMovieLoader::IHandle::ready);
	classIMovieLoader_IHandle->addProperty("succeeded", &IMovieLoader::IHandle::succeeded);
	classIMovieLoader_IHandle->addProperty("failed", &IMovieLoader::IHandle::failed);
	classIMovieLoader_IHandle->addMethod("wait", &IMovieLoader::IHandle::wait);
	classIMovieLoader_IHandle->addMethod("get", &IMovieLoader::IHandle::get);
	registrar->registerClass(classIMovieLoader_IHandle);

	Ref< AutoRuntimeClass< MovieLoader > > classMovieLoader = new AutoRuntimeClass< MovieLoader >();
	classMovieLoader->addConstructor();
	classMovieLoader->addMethod("setCacheDirectory", &MovieLoader::setCacheDirectory);
	classMovieLoader->addMethod("setMerge", &MovieLoader::setMerge);
	classMovieLoader->addMethod("setTriangulate", &MovieLoader::setTriangulate);
	classMovieLoader->addMethod("setIncludeAS", &MovieLoader::setIncludeAS);
	registrar->registerClass(classMovieLoader);

	Ref< AutoRuntimeClass< MovieFactory > > classMovieFactory = new AutoRuntimeClass< MovieFactory >();
	classMovieFactory->addConstructor< bool >();
	classMovieFactory->addMethod("createMovie", &MovieFactory::createMovie);
	classMovieFactory->addMethod("createMovieFromImage", &MovieFactory::createMovieFromImage);
	registrar->registerClass(classMovieFactory);

	Ref< AutoRuntimeClass< MoviePlayer > > classMoviePlayer = new AutoRuntimeClass< MoviePlayer >();
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
	classMoviePlayer->addMethod("setGlobal", &MoviePlayer::setGlobal);
	classMoviePlayer->addMethod("getGlobal", &MoviePlayer::getGlobal);
	registrar->registerClass(classMoviePlayer);

	Ref< AutoRuntimeClass< SwfReader > > classSwfReader = new AutoRuntimeClass< SwfReader >();
	classSwfReader->addConstructor< IStream* >();
	registrar->registerClass(classSwfReader);

	Ref< AutoRuntimeClass< ColorTransform > > classColorTransform = new AutoRuntimeClass< ColorTransform >();
	classColorTransform->addConstructor();
	classColorTransform->addConstructor< const Color4f&, const Color4f& >();
	classColorTransform->addProperty("mul", &ColorTransform_setMul, &ColorTransform_getMul);
	classColorTransform->addProperty("add", &ColorTransform_setAdd, &ColorTransform_getAdd);
	registrar->registerClass(classColorTransform);

	Ref< AutoRuntimeClass< Bitmap > > classBitmap = new AutoRuntimeClass< Bitmap >();
	classBitmap->addProperty("X", &Bitmap::getX);
	classBitmap->addProperty("Y", &Bitmap::getY);
	classBitmap->addProperty("width", &Bitmap::getWidth);
	classBitmap->addProperty("height", &Bitmap::getHeight);
	registrar->registerClass(classBitmap);

	Ref< AutoRuntimeClass< BitmapImage > > classBitmapImage = new AutoRuntimeClass< BitmapImage >();
	classBitmapImage->addConstructor< const drawing::Image* >();
	classBitmapImage->addProperty("image", &BitmapImage::getImage);
	registrar->registerClass(classBitmapImage);

	Ref< AutoRuntimeClass< BitmapResource > > classBitmapResource = new AutoRuntimeClass< BitmapResource >();
	classBitmapResource->addProperty("atlasWidth", &BitmapResource::getAtlasWidth);
	classBitmapResource->addProperty("atlasHeight", &BitmapResource::getAtlasHeight);
	classBitmapResource->addProperty("resourceId", &BitmapResource::getResourceId);
	registrar->registerClass(classBitmapResource);

	Ref< AutoRuntimeClass< BitmapTexture > > classBitmapTexture = new AutoRuntimeClass< BitmapTexture >();
	classBitmapTexture->addConstructor< render::ISimpleTexture* >();
	classBitmapTexture->addProperty("texture", &BitmapTexture::getTexture);
	registrar->registerClass(classBitmapTexture);

	Ref< AutoRuntimeClass< Font > > classFont = new AutoRuntimeClass< Font >();
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

	Ref< AutoRuntimeClass< Dictionary > > classDictionary = new AutoRuntimeClass< Dictionary >();
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

	Ref< AutoRuntimeClass< Character > > classCharacter = new AutoRuntimeClass< Character >();
	classCharacter->addProperty("id", &Character::getId);
	registrar->registerClass(classCharacter);

	Ref< AutoRuntimeClass< CharacterInstance > > classCharacterInstance = new AutoRuntimeClass< CharacterInstance >();
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
	classCharacterInstance->addProperty("bounds", &CharacterInstance::getBounds);
	classCharacterInstance->addStaticMethod("getInstanceCount", &CharacterInstance::getInstanceCount);
	classCharacterInstance->addMethod("destroy", &CharacterInstance::destroy);
	classCharacterInstance->addMethod("setColorTransform", &CharacterInstance::setColorTransform);
	classCharacterInstance->addMethod("setColorTransform", &CharacterInstance_setColorTransform);
	classCharacterInstance->addMethod("transformInto", &CharacterInstance::transformInto);
	classCharacterInstance->addMethod("setFocus", &CharacterInstance::setFocus);
	classCharacterInstance->setUnknownHandler(&CharacterInstance_invoke);
	registrar->registerClass(classCharacterInstance);

	Ref< AutoRuntimeClass< Shape > > classShape = new AutoRuntimeClass< Shape >();
	classShape->addProperty("pathCount", &Shape_getPathCount);
	registrar->registerClass(classShape);

	Ref< AutoRuntimeClass< ShapeInstance > > classShapeInstance = new AutoRuntimeClass< ShapeInstance >();
	classShapeInstance->addProperty("shape", &ShapeInstance::getShape);
	classShapeInstance->setUnknownHandler(&ShapeInstance_invoke);
	registrar->registerClass(classShapeInstance);

	Ref< AutoRuntimeClass< DisplayList > > classDisplayList = new AutoRuntimeClass< DisplayList >();
	classDisplayList->addProperty("nextHighestDepth", &DisplayList::getNextHighestDepth);
	classDisplayList->addProperty("objects", &DisplayList_getObjects);
	classDisplayList->addMethod("reset", &DisplayList::reset);
	classDisplayList->addMethod("showObject", &DisplayList::showObject);
	classDisplayList->addMethod("removeObject", &DisplayList_removeObject);
	classDisplayList->addMethod("getObjectDepth", &DisplayList::getObjectDepth);
	classDisplayList->addMethod("getNextHighestDepthInRange", &DisplayList::getNextHighestDepthInRange);
	classDisplayList->addMethod("swap", &DisplayList::swap);
	registrar->registerClass(classDisplayList);

	Ref< AutoRuntimeClass< Sprite > > classSprite = new AutoRuntimeClass< Sprite >();
	classSprite->addProperty("frameRate", &Sprite::getFrameRate);
	classSprite->addProperty("frameCount", &Sprite::getFrameCount);
	classSprite->addMethod("addFrame", &Sprite::addFrame);
	classSprite->addMethod("getFrame", &Sprite::getFrame);
	classSprite->addMethod("findFrame", &Sprite::findFrame);
	registrar->registerClass(classSprite);

	Ref< AutoRuntimeClass< SpriteInstance > > classSpriteInstance = new AutoRuntimeClass< SpriteInstance >();
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
	classSpriteInstance->addMethod("gotoFrame", &SpriteInstance::gotoFrame);
	classSpriteInstance->addMethod("gotoPrevious", &SpriteInstance::gotoPrevious);
	classSpriteInstance->addMethod("gotoNext", &SpriteInstance::gotoNext);
	classSpriteInstance->addMethod("setPlaying", &SpriteInstance::setPlaying);
	classSpriteInstance->addMethod("createEmptyMovieClip", &SpriteInstance::createEmptyMovieClip);
	classSpriteInstance->addMethod("createTextField", &SpriteInstance::createTextField);
	classSpriteInstance->addMethod("removeMovieClip", &SpriteInstance::removeMovieClip);
	classSpriteInstance->addMethod("clone", &SpriteInstance::clone);
	classSpriteInstance->addMethod("duplicateMovieClip", &SpriteInstance_duplicateMovieClip_1);
	classSpriteInstance->addMethod("duplicateMovieClip", &SpriteInstance_duplicateMovieClip_2);
	classSpriteInstance->addMethod("attachBitmap", &SpriteInstance::attachBitmap);
	classSpriteInstance->addMethod("createCanvas", &SpriteInstance::createCanvas);
	classSpriteInstance->setUnknownHandler(&SpriteInstance_invoke);
	registrar->registerClass(classSpriteInstance);

	Ref< AutoRuntimeClass< TextFormat > > classTextFormat = new AutoRuntimeClass< TextFormat >();
	//classTextFormat->addConstructor< float, int32_t, float >();
	classTextFormat->addProperty("letterSpacing", &TextFormat::setLetterSpacing, &TextFormat::getLetterSpacing);
	//classTextFormat->addProperty("align", &TextFormat::setAlign, &TextFormat::getAlign);
	classTextFormat->addProperty("size", &TextFormat::setSize, &TextFormat::getSize);
	registrar->registerClass(classTextFormat);

	Ref< AutoRuntimeClass< Edit > > classEdit = new AutoRuntimeClass< Edit >();
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

	Ref< AutoRuntimeClass< EditInstance > > classEditInstance = new AutoRuntimeClass< EditInstance >();
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
	classEditInstance->addMethod("parseText", &EditInstance::parseText);
	classEditInstance->addMethod("parseHtml", &EditInstance::parseHtml);
	classEditInstance->addMethod("measureText", &EditInstance_measureText_1);
	classEditInstance->addMethod("measureText", &EditInstance_measureText_2);
	classEditInstance->addMethod("setTextFormat", &EditInstance_setTextFormat_1);
	classEditInstance->addMethod("getTextFormat", &EditInstance_getTextFormat_0);
	classEditInstance->addMethod("setTextFormat", &EditInstance_setTextFormat_3);
	classEditInstance->addMethod("getTextFormat", &EditInstance_getTextFormat_2);
	classEditInstance->setUnknownHandler(&EditInstance_invoke);
	registrar->registerClass(classEditInstance);

	Ref< AutoRuntimeClass< Movie > > classMovie = new AutoRuntimeClass< Movie >();
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

	Ref< AutoRuntimeClass< ActionContext > > classActionContext = new AutoRuntimeClass< ActionContext >();
	classActionContext->addProperty("movie", &ActionContext::getMovie);
	classActionContext->addProperty("global", &ActionContext::getGlobal);
	classActionContext->addProperty("movieClip", &ActionContext::getMovieClip);
	classActionContext->addProperty("focus", &ActionContext::getFocus);
	classActionContext->addProperty("pressed", &ActionContext::getPressed);
	classActionContext->addMethod("lookupClass", &ActionContext::lookupClass);
	classActionContext->addMethod("createObject", &ActionContext_createObject_0);
	classActionContext->addMethod("createObject", &ActionContext_createObject_1);
	classActionContext->addMethod("createObject", &ActionContext_createObject_2);
	classActionContext->addMethod("createObject", &ActionContext_createObject_3);
	classActionContext->addMethod("createObject", &ActionContext_createObject_4);
	classActionContext->addMethod("createObject", &ActionContext_createObject_5);
	classActionContext->addMethod("createObject", &ActionContext_createObject_6);
	classActionContext->addMethod("createObject", &ActionContext_createObject_7);
	classActionContext->addMethod("createObject", &ActionContext_createObject_8);
	classActionContext->addMethod("createBitmap", &ActionContext_createBitmap);
	registrar->registerClass(classActionContext);

	Ref< AutoRuntimeClass< Optimizer > > classOptimizer = new AutoRuntimeClass< Optimizer >();
	classOptimizer->addConstructor();
	classOptimizer->addMethod("merge", &Optimizer::merge);
	classOptimizer->addMethod("triangulate", &Optimizer::triangulate);
	registrar->registerClass(classOptimizer);
}

	}
}

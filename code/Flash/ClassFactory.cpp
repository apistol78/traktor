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
	classIMovieLoader_IHandle->addMethod("wait", &IMovieLoader::IHandle::wait);
	classIMovieLoader_IHandle->addMethod("ready", &IMovieLoader::IHandle::ready);
	classIMovieLoader_IHandle->addMethod("succeeded", &IMovieLoader::IHandle::succeeded);
	classIMovieLoader_IHandle->addMethod("get", &IMovieLoader::IHandle::get);
	classIMovieLoader_IHandle->addMethod("failed", &IMovieLoader::IHandle::failed);
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
	classMoviePlayer->addMethod< void, uint32_t >("gotoAndPlay", &MoviePlayer::gotoAndPlay);
	classMoviePlayer->addMethod< void, uint32_t >("gotoAndStop", &MoviePlayer::gotoAndStop);
	classMoviePlayer->addMethod< bool, const std::string& >("gotoAndPlay", &MoviePlayer::gotoAndPlay);
	classMoviePlayer->addMethod< bool, const std::string& >("gotoAndStop", &MoviePlayer::gotoAndStop);
	classMoviePlayer->addMethod("getFrameCount", &MoviePlayer::getFrameCount);
	classMoviePlayer->addMethod("postKeyDown", &MoviePlayer::postKeyDown);
	classMoviePlayer->addMethod("postKeyUp", &MoviePlayer::postKeyUp);
	classMoviePlayer->addMethod("postMouseDown", &MoviePlayer::postMouseDown);
	classMoviePlayer->addMethod("postMouseUp", &MoviePlayer::postMouseUp);
	classMoviePlayer->addMethod("postMouseMove", &MoviePlayer::postMouseMove);
	classMoviePlayer->addMethod("getMovieInstance", &MoviePlayer::getMovieInstance);
	classMoviePlayer->addMethod("setGlobal", &MoviePlayer::setGlobal);
	classMoviePlayer->addMethod("getGlobal", &MoviePlayer::getGlobal);
	registrar->registerClass(classMoviePlayer);

	Ref< AutoRuntimeClass< SwfReader > > classSwfReader = new AutoRuntimeClass< SwfReader >();
	classSwfReader->addConstructor< IStream* >();
	registrar->registerClass(classSwfReader);

	Ref< AutoRuntimeClass< ColorTransform > > classColorTransform = new AutoRuntimeClass< ColorTransform >();
	classColorTransform->addConstructor();
	classColorTransform->addConstructor< const Color4f&, const Color4f& >();
	classColorTransform->addMethod("setMul", &ColorTransform_setMul);
	classColorTransform->addMethod("getMul", &ColorTransform_getMul);
	classColorTransform->addMethod("setAdd", &ColorTransform_setAdd);
	classColorTransform->addMethod("getAdd", &ColorTransform_getAdd);
	registrar->registerClass(classColorTransform);

	Ref< AutoRuntimeClass< Bitmap > > classBitmap = new AutoRuntimeClass< Bitmap >();
	classBitmap->addMethod("getX", &Bitmap::getX);
	classBitmap->addMethod("getY", &Bitmap::getY);
	classBitmap->addMethod("getWidth", &Bitmap::getWidth);
	classBitmap->addMethod("getHeight", &Bitmap::getHeight);
	registrar->registerClass(classBitmap);

	Ref< AutoRuntimeClass< BitmapImage > > classBitmapImage = new AutoRuntimeClass< BitmapImage >();
	classBitmapImage->addConstructor< const drawing::Image* >();
	classBitmapImage->addMethod("getImage", &BitmapImage::getImage);
	registrar->registerClass(classBitmapImage);

	Ref< AutoRuntimeClass< BitmapResource > > classBitmapResource = new AutoRuntimeClass< BitmapResource >();
	classBitmapResource->addMethod("getAtlasWidth", &BitmapResource::getAtlasWidth);
	classBitmapResource->addMethod("getAtlasHeight", &BitmapResource::getAtlasHeight);
	classBitmapResource->addMethod("getResourceId", &BitmapResource::getResourceId);
	registrar->registerClass(classBitmapResource);

	Ref< AutoRuntimeClass< BitmapTexture > > classBitmapTexture = new AutoRuntimeClass< BitmapTexture >();
	classBitmapTexture->addConstructor< render::ISimpleTexture* >();
	classBitmapTexture->addMethod("getTexture", &BitmapTexture::getTexture);
	registrar->registerClass(classBitmapTexture);

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
	classCharacterInstance->addStaticMethod("getInstanceCount", &CharacterInstance::getInstanceCount);
	classCharacterInstance->addMethod("destroy", &CharacterInstance::destroy);
	classCharacterInstance->addMethod("getContext", &CharacterInstance::getContext);
	classCharacterInstance->addMethod("getDictionary", &CharacterInstance::getDictionary);
	classCharacterInstance->addMethod("getParent", &CharacterInstance::getParent);
	classCharacterInstance->addMethod("setName", &CharacterInstance::setName);
	classCharacterInstance->addMethod("getName", &CharacterInstance::getName);
	classCharacterInstance->addMethod("getTarget", &CharacterInstance::getTarget);
	classCharacterInstance->addMethod("setColorTransform", &CharacterInstance::setColorTransform);
	classCharacterInstance->addMethod("setColorTransform", &CharacterInstance_setColorTransform);
	classCharacterInstance->addMethod("getColorTransform", &CharacterInstance::getColorTransform);
	classCharacterInstance->addMethod("setTransform", &CharacterInstance::setTransform);
	classCharacterInstance->addMethod("getTransform", &CharacterInstance::getTransform);
	classCharacterInstance->addMethod("getFullTransform", &CharacterInstance::getFullTransform);
	classCharacterInstance->addMethod("transformInto", &CharacterInstance::transformInto);
	classCharacterInstance->addMethod("setFilter", &CharacterInstance::setFilter);
	classCharacterInstance->addMethod("getFilter", &CharacterInstance::getFilter);
	classCharacterInstance->addMethod("setFilterColor", &CharacterInstance::setFilterColor);
	classCharacterInstance->addMethod("getFilterColor", &CharacterInstance::getFilterColor);
	classCharacterInstance->addMethod("setBlendMode", &CharacterInstance::setBlendMode);
	classCharacterInstance->addMethod("getBlendMode", &CharacterInstance::getBlendMode);
	classCharacterInstance->addMethod("setVisible", &CharacterInstance::setVisible);
	classCharacterInstance->addMethod("isVisible", &CharacterInstance::isVisible);
	classCharacterInstance->addMethod("setEnabled", &CharacterInstance::setEnabled);
	classCharacterInstance->addMethod("isEnabled", &CharacterInstance::isEnabled);
	classCharacterInstance->addMethod("setFocus", &CharacterInstance::setFocus);
	classCharacterInstance->addMethod("haveFocus", &CharacterInstance::haveFocus);
	classCharacterInstance->addMethod("getBounds", &CharacterInstance::getBounds);
	classCharacterInstance->setUnknownHandler(&CharacterInstance_invoke);
	registrar->registerClass(classCharacterInstance);

	Ref< AutoRuntimeClass< Shape > > classShape = new AutoRuntimeClass< Shape >();
	classShape->addMethod("getPathCount", &Shape_getPathCount);
	registrar->registerClass(classShape);

	Ref< AutoRuntimeClass< ShapeInstance > > classShapeInstance = new AutoRuntimeClass< ShapeInstance >();
	classShapeInstance->addMethod("getShape", &ShapeInstance::getShape);
	classShapeInstance->setUnknownHandler(&ShapeInstance_invoke);
	registrar->registerClass(classShapeInstance);

	Ref< AutoRuntimeClass< DisplayList > > classDisplayList = new AutoRuntimeClass< DisplayList >();
	classDisplayList->addMethod("reset", &DisplayList::reset);
	classDisplayList->addMethod("showObject", &DisplayList::showObject);
	classDisplayList->addMethod("removeObject", &DisplayList_removeObject);
	classDisplayList->addMethod("getObjectDepth", &DisplayList::getObjectDepth);
	classDisplayList->addMethod("getNextHighestDepth", &DisplayList::getNextHighestDepth);
	classDisplayList->addMethod("swap", &DisplayList::swap);
	classDisplayList->addMethod("getObjects", &DisplayList_getObjects);
	registrar->registerClass(classDisplayList);

	Ref< AutoRuntimeClass< Sprite > > classSprite = new AutoRuntimeClass< Sprite >();
	classSprite->addMethod("getFrameRate", &Sprite::getFrameRate);
	classSprite->addMethod("addFrame", &Sprite::addFrame);
	classSprite->addMethod("getFrameCount", &Sprite::getFrameCount);
	classSprite->addMethod("getFrame", &Sprite::getFrame);
	classSprite->addMethod("findFrame", &Sprite::findFrame);
	registrar->registerClass(classSprite);

	Ref< AutoRuntimeClass< SpriteInstance > > classSpriteInstance = new AutoRuntimeClass< SpriteInstance >();
	classSpriteInstance->addMethod("getSprite", &SpriteInstance::getSprite);
	classSpriteInstance->addMethod("setCacheAsBitmap", &SpriteInstance::setCacheAsBitmap);
	classSpriteInstance->addMethod("getCacheAsBitmap", &SpriteInstance::getCacheAsBitmap);
	classSpriteInstance->addMethod("gotoFrame", &SpriteInstance::gotoFrame);
	classSpriteInstance->addMethod("gotoPrevious", &SpriteInstance::gotoPrevious);
	classSpriteInstance->addMethod("gotoNext", &SpriteInstance::gotoNext);
	classSpriteInstance->addMethod("getCurrentFrame", &SpriteInstance::getCurrentFrame);
	classSpriteInstance->addMethod("setPlaying", &SpriteInstance::setPlaying);
	classSpriteInstance->addMethod("getPlaying", &SpriteInstance::getPlaying);
	classSpriteInstance->addMethod("getDisplayList", &SpriteInstance_getDisplayList);
	classSpriteInstance->addMethod("createEmptyMovieClip", &SpriteInstance::createEmptyMovieClip);
	classSpriteInstance->addMethod("createTextField", &SpriteInstance::createTextField);
	classSpriteInstance->addMethod("removeMovieClip", &SpriteInstance::removeMovieClip);
	classSpriteInstance->addMethod("clone", &SpriteInstance::clone);
	classSpriteInstance->addMethod("duplicateMovieClip", &SpriteInstance_duplicateMovieClip_1);
	classSpriteInstance->addMethod("duplicateMovieClip", &SpriteInstance_duplicateMovieClip_2);
	classSpriteInstance->addMethod("attachBitmap", &SpriteInstance::attachBitmap);
	classSpriteInstance->addMethod("getLocalBounds", &SpriteInstance::getLocalBounds);
	classSpriteInstance->addMethod("getVisibleLocalBounds", &SpriteInstance::getVisibleLocalBounds);
	classSpriteInstance->addMethod("setMask", &SpriteInstance::setMask);
	classSpriteInstance->addMethod("getMask", &SpriteInstance::getMask);
	classSpriteInstance->addMethod("createCanvas", &SpriteInstance::createCanvas);
	classSpriteInstance->addMethod("getCanvas", &SpriteInstance::getCanvas);
	classSpriteInstance->addMethod("getMouseX", &SpriteInstance::getMouseX);
	classSpriteInstance->addMethod("getMouseY", &SpriteInstance::getMouseY);
	classSpriteInstance->addMethod("setPosition", &SpriteInstance::setPosition);
	classSpriteInstance->addMethod("getPosition", &SpriteInstance::getPosition);
	classSpriteInstance->addMethod("setX", &SpriteInstance::setX);
	classSpriteInstance->addMethod("getX", &SpriteInstance::getX);
	classSpriteInstance->addMethod("setY", &SpriteInstance::setY);
	classSpriteInstance->addMethod("getY", &SpriteInstance::getY);
	classSpriteInstance->addMethod("setSize", &SpriteInstance::setSize);
	classSpriteInstance->addMethod("getSize", &SpriteInstance::getSize);
	classSpriteInstance->addMethod("setWidth", &SpriteInstance::setWidth);
	classSpriteInstance->addMethod("getWidth", &SpriteInstance::getWidth);
	classSpriteInstance->addMethod("setHeight", &SpriteInstance::setHeight);
	classSpriteInstance->addMethod("getHeight", &SpriteInstance::getHeight);
	classSpriteInstance->addMethod("setRotation", &SpriteInstance::setRotation);
	classSpriteInstance->addMethod("getRotation", &SpriteInstance::getRotation);
	classSpriteInstance->addMethod("setScale", &SpriteInstance::setScale);
	classSpriteInstance->addMethod("getScale", &SpriteInstance::getScale);
	classSpriteInstance->addMethod("setXScale", &SpriteInstance::setXScale);
	classSpriteInstance->addMethod("getXScale", &SpriteInstance::getXScale);
	classSpriteInstance->addMethod("setYScale", &SpriteInstance::setYScale);
	classSpriteInstance->addMethod("getYScale", &SpriteInstance::getYScale);
	classSpriteInstance->setUnknownHandler(&SpriteInstance_invoke);
	registrar->registerClass(classSpriteInstance);

	Ref< AutoRuntimeClass< TextFormat > > classTextFormat = new AutoRuntimeClass< TextFormat >();
	//classTextFormat->addConstructor< float, int32_t, float >();
	classTextFormat->addMethod("setLetterSpacing", &TextFormat::setLetterSpacing);
	classTextFormat->addMethod("getLetterSpacing", &TextFormat::getLetterSpacing);
	//classTextFormat->addMethod("setAlign", &TextFormat::setAlign);
	//classTextFormat->addMethod("getAlign", &TextFormat::getAlign);
	classTextFormat->addMethod("setSize", &TextFormat::setSize);
	classTextFormat->addMethod("getSize", &TextFormat::getSize);
	registrar->registerClass(classTextFormat);

	Ref< AutoRuntimeClass< EditInstance > > classEditInstance = new AutoRuntimeClass< EditInstance >();
	classEditInstance->addMethod("parseText", &EditInstance::parseText);
	classEditInstance->addMethod("parseHtml", &EditInstance::parseHtml);
	classEditInstance->addMethod("measureText", &EditInstance::measureText);
	classEditInstance->addMethod("setTextBounds", &EditInstance::setTextBounds);
	classEditInstance->addMethod("getTextBounds", &EditInstance::getTextBounds);
	classEditInstance->addMethod("setTextColor", &EditInstance::setTextColor);
	classEditInstance->addMethod("getTextColor", &EditInstance::getTextColor);
	classEditInstance->addMethod("setLetterSpacing", &EditInstance::setLetterSpacing);
	classEditInstance->addMethod("getLetterSpacing", &EditInstance::getLetterSpacing);
	classEditInstance->addMethod("setTextFormat", &EditInstance_setTextFormat_1);
	classEditInstance->addMethod("getTextFormat", &EditInstance_getTextFormat_0);
	classEditInstance->addMethod("setTextFormat", &EditInstance_setTextFormat_3);
	classEditInstance->addMethod("getTextFormat", &EditInstance_getTextFormat_2);
	classEditInstance->addMethod("getText", &EditInstance::getText);
	classEditInstance->addMethod("getHtmlText", &EditInstance::getHtmlText);
	classEditInstance->addMethod("setWordWrap", &EditInstance::setWordWrap);
	classEditInstance->addMethod("getWordWrap", &EditInstance::getWordWrap);
	classEditInstance->addMethod("setPassword", &EditInstance::setPassword);
	classEditInstance->addMethod("getPassword", &EditInstance::getPassword);
	classEditInstance->addMethod("getCaret", &EditInstance::getCaret);
	classEditInstance->addMethod("setScroll", &EditInstance::setScroll);
	classEditInstance->addMethod("getScroll", &EditInstance::getScroll);
	classEditInstance->addMethod("getMaxScroll", &EditInstance::getMaxScroll);
	classEditInstance->addMethod("setRenderClipMask", &EditInstance::setRenderClipMask);
	classEditInstance->addMethod("getRenderClipMask", &EditInstance::getRenderClipMask);
	classEditInstance->addMethod("setPosition", &EditInstance::setPosition);
	classEditInstance->addMethod("getPosition", &EditInstance::getPosition);
	classEditInstance->addMethod("setX", &EditInstance::setX);
	classEditInstance->addMethod("getX", &EditInstance::getX);
	classEditInstance->addMethod("setY", &EditInstance::setY);
	classEditInstance->addMethod("getY", &EditInstance::getY);
	classEditInstance->addMethod("setSize", &EditInstance::setSize);
	classEditInstance->addMethod("getSize", &EditInstance::getSize);
	classEditInstance->addMethod("setWidth", &EditInstance::setWidth);
	classEditInstance->addMethod("getWidth", &EditInstance::getWidth);
	classEditInstance->addMethod("setHeight", &EditInstance::setHeight);
	classEditInstance->addMethod("getHeight", &EditInstance::getHeight);
	classEditInstance->addMethod("setRotation", &EditInstance::setRotation);
	classEditInstance->addMethod("getRotation", &EditInstance::getRotation);
	classEditInstance->addMethod("setScale", &EditInstance::setScale);
	classEditInstance->addMethod("getScale", &EditInstance::getScale);
	classEditInstance->addMethod("setXScale", &EditInstance::setXScale);
	classEditInstance->addMethod("getXScale", &EditInstance::getXScale);
	classEditInstance->addMethod("setYScale", &EditInstance::setYScale);
	classEditInstance->addMethod("getYScale", &EditInstance::getYScale);
	classEditInstance->addMethod("getTextSize", &EditInstance::getTextSize);
	classEditInstance->addMethod("getTextWidth", &EditInstance::getTextWidth);
	classEditInstance->addMethod("getTextHeight", &EditInstance::getTextHeight);
	classEditInstance->setUnknownHandler(&EditInstance_invoke);
	registrar->registerClass(classEditInstance);

	Ref< AutoRuntimeClass< Movie > > classMovie = new AutoRuntimeClass< Movie >();
	classMovie->addMethod("defineFont", &Movie::defineFont);
	classMovie->addMethod("defineBitmap", &Movie::defineBitmap);
	classMovie->addMethod("defineSound", &Movie::defineSound);
	classMovie->addMethod("defineCharacter", &Movie::defineCharacter);
	classMovie->addMethod("setExport", &Movie::setExport);
	classMovie->addMethod("createMovieClipInstance", &Movie::createMovieClipInstance);
	classMovie->addMethod("createExternalMovieClipInstance", &Movie::createExternalMovieClipInstance);
	classMovie->addMethod("createExternalSpriteInstance", &Movie::createExternalSpriteInstance);
	classMovie->addMethod("getFrameBounds", &Movie::getFrameBounds);
	classMovie->addMethod("getMovieClip", &Movie::getMovieClip);
	registrar->registerClass(classMovie);

	Ref< AutoRuntimeClass< ActionContext > > classActionContext = new AutoRuntimeClass< ActionContext >();
	classActionContext->addMethod("lookupClass", &ActionContext::lookupClass);
	classActionContext->addMethod("getMovie", &ActionContext::getMovie);
	classActionContext->addMethod("getGlobal", &ActionContext::getGlobal);
	classActionContext->addMethod("getMovieClip", &ActionContext::getMovieClip);
	classActionContext->addMethod("getFocus", &ActionContext::getFocus);
	classActionContext->addMethod("getPressed", &ActionContext::getPressed);
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

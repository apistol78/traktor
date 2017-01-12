#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/Boxes.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Core/Io/IStream.h"
#include "Drawing/Image.h"
#include "Flash/FlashBitmapImage.h"
#include "Flash/FlashBitmapResource.h"
#include "Flash/FlashBitmapTexture.h"
#include "Flash/FlashCanvas.h"
#include "Flash/FlashCast.h"
#include "Flash/FlashCharacter.h"
#include "Flash/FlashClassFactory.h"
#include "Flash/FlashDictionary.h"
#include "Flash/FlashEditInstance.h"
#include "Flash/FlashFont.h"
#include "Flash/FlashFrame.h"
#include "Flash/FlashMovie.h"
#include "Flash/FlashMovieFactory.h"
#include "Flash/FlashMovieLoader.h"
#include "Flash/FlashMoviePlayer.h"
#include "Flash/FlashOptimizer.h"
#include "Flash/FlashShape.h"
#include "Flash/FlashShapeInstance.h"
#include "Flash/FlashSound.h"
#include "Flash/FlashSprite.h"
#include "Flash/FlashSpriteInstance.h"
#include "Flash/FlashTextFormat.h"
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

Any FlashDictionary_getExportId(FlashDictionary* self, const std::string& exportName)
{
	uint16_t exportId;
	if (self->getExportId(exportName, exportId))
		return Any::fromInteger(exportId);
	else
		return Any();
}

Any FlashDictionary_getExportName(FlashDictionary* self, uint16_t exportId)
{
	std::string exportName;
	if (self->getExportName(exportId, exportName))
		return Any::fromString(exportName);
	else
		return Any();
}

void FlashCharacterInstance_setColorTransform(FlashCharacterInstance* self, const Color4f& mul, const Color4f& add)
{
	self->setColorTransform(ColorTransform(mul, add));
}

Any FlashCharacterInstance_invoke(FlashCharacterInstance* self, const std::string& methodName, uint32_t argc, const Any* argv)
{
	return ActionObjectRelay_invoke(self, methodName, argc, argv);
}

Any FlashShapeInstance_invoke(FlashShapeInstance* self, const std::string& methodName, uint32_t argc, const Any* argv)
{
	return ActionObjectRelay_invoke(self, methodName, argc, argv);
}

Ref< FlashSpriteInstance > FlashSpriteInstance_duplicateMovieClip_1(FlashSpriteInstance* self, const std::string& cloneName, int32_t depth)
{
	return self->duplicateMovieClip(cloneName, depth);
}

Ref< FlashSpriteInstance > FlashSpriteInstance_duplicateMovieClip_2(FlashSpriteInstance* self, const std::string& cloneName, int32_t depth, FlashSpriteInstance* intoParent)
{
	return self->duplicateMovieClip(cloneName, depth, intoParent);
}

Any FlashSpriteInstance_invoke(FlashSpriteInstance* self, const std::string& methodName, uint32_t argc, const Any* argv)
{
	return ActionObjectRelay_invoke(self, methodName, argc, argv);
}

Any FlashEditInstance_invoke(FlashEditInstance* self, const std::string& methodName, uint32_t argc, const Any* argv)
{
	return ActionObjectRelay_invoke(self, methodName, argc, argv);
}

uint32_t FlashShape_getPathCount(FlashShape* self)
{
	return uint32_t(self->getPaths().size());
}

RefArray< FlashCharacterInstance > FlashDisplayList_getObjects(FlashDisplayList* self)
{
	RefArray< FlashCharacterInstance > characters;
	self->getObjects(characters);
	return characters;
}

FlashDisplayList* FlashSpriteInstance_getDisplayList(FlashSpriteInstance* self)
{
	return &self->getDisplayList();
}

void FlashEditInstance_setTextFormat_1(FlashEditInstance* self, const FlashTextFormat* textFormat)
{
	self->setTextFormat(textFormat);
}

Ref< FlashTextFormat > FlashEditInstance_getTextFormat_0(FlashEditInstance* self)
{
	return self->getTextFormat();
}

void FlashEditInstance_setTextFormat_3(FlashEditInstance* self, const FlashTextFormat* textFormat, int32_t beginIndex, int32_t endIndex)
{
	self->setTextFormat(textFormat, beginIndex, endIndex);
}

Ref< FlashTextFormat > FlashEditInstance_getTextFormat_2(FlashEditInstance* self, int32_t beginIndex, int32_t endIndex)
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

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.flash.FlashClassFactory", 0, FlashClassFactory, IRuntimeClassFactory)

void FlashClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
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

	Ref< AutoRuntimeClass< IFlashMovieLoader > > classIFlashMovieLoader = new AutoRuntimeClass< IFlashMovieLoader >();
	classIFlashMovieLoader->addMethod("loadAsync", &IFlashMovieLoader::loadAsync);
	classIFlashMovieLoader->addMethod("load", &IFlashMovieLoader::load);
	registrar->registerClass(classIFlashMovieLoader);

	Ref< AutoRuntimeClass< IFlashMovieLoader::IHandle > > classIFlashMovieLoader_IHandle = new AutoRuntimeClass< IFlashMovieLoader::IHandle >();
	classIFlashMovieLoader_IHandle->addMethod("wait", &IFlashMovieLoader::IHandle::wait);
	classIFlashMovieLoader_IHandle->addMethod("ready", &IFlashMovieLoader::IHandle::ready);
	classIFlashMovieLoader_IHandle->addMethod("succeeded", &IFlashMovieLoader::IHandle::succeeded);
	classIFlashMovieLoader_IHandle->addMethod("get", &IFlashMovieLoader::IHandle::get);
	classIFlashMovieLoader_IHandle->addMethod("failed", &IFlashMovieLoader::IHandle::failed);
	registrar->registerClass(classIFlashMovieLoader_IHandle);

	Ref< AutoRuntimeClass< FlashMovieLoader > > classFlashMovieLoader = new AutoRuntimeClass< FlashMovieLoader >();
	classFlashMovieLoader->addConstructor();
	classFlashMovieLoader->addMethod("setCacheDirectory", &FlashMovieLoader::setCacheDirectory);
	classFlashMovieLoader->addMethod("setMerge", &FlashMovieLoader::setMerge);
	classFlashMovieLoader->addMethod("setTriangulate", &FlashMovieLoader::setTriangulate);
	classFlashMovieLoader->addMethod("setIncludeAS", &FlashMovieLoader::setIncludeAS);
	registrar->registerClass(classFlashMovieLoader);

	Ref< AutoRuntimeClass< FlashMovieFactory > > classFlashMovieFactory = new AutoRuntimeClass< FlashMovieFactory >();
	classFlashMovieFactory->addConstructor< bool >();
	classFlashMovieFactory->addMethod("createMovie", &FlashMovieFactory::createMovie);
	classFlashMovieFactory->addMethod("createMovieFromImage", &FlashMovieFactory::createMovieFromImage);
	registrar->registerClass(classFlashMovieFactory);

	Ref< AutoRuntimeClass< FlashMoviePlayer > > classFlashMoviePlayer = new AutoRuntimeClass< FlashMoviePlayer >();
	classFlashMoviePlayer->addMethod< void, uint32_t >("gotoAndPlay", &FlashMoviePlayer::gotoAndPlay);
	classFlashMoviePlayer->addMethod< void, uint32_t >("gotoAndStop", &FlashMoviePlayer::gotoAndStop);
	classFlashMoviePlayer->addMethod< bool, const std::string& >("gotoAndPlay", &FlashMoviePlayer::gotoAndPlay);
	classFlashMoviePlayer->addMethod< bool, const std::string& >("gotoAndStop", &FlashMoviePlayer::gotoAndStop);
	classFlashMoviePlayer->addMethod("getFrameCount", &FlashMoviePlayer::getFrameCount);
	classFlashMoviePlayer->addMethod("postKeyDown", &FlashMoviePlayer::postKeyDown);
	classFlashMoviePlayer->addMethod("postKeyUp", &FlashMoviePlayer::postKeyUp);
	classFlashMoviePlayer->addMethod("postMouseDown", &FlashMoviePlayer::postMouseDown);
	classFlashMoviePlayer->addMethod("postMouseUp", &FlashMoviePlayer::postMouseUp);
	classFlashMoviePlayer->addMethod("postMouseMove", &FlashMoviePlayer::postMouseMove);
	classFlashMoviePlayer->addMethod("getMovieInstance", &FlashMoviePlayer::getMovieInstance);
	classFlashMoviePlayer->addMethod("setGlobal", &FlashMoviePlayer::setGlobal);
	classFlashMoviePlayer->addMethod("getGlobal", &FlashMoviePlayer::getGlobal);
	registrar->registerClass(classFlashMoviePlayer);

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

	Ref< AutoRuntimeClass< FlashBitmap > > classFlashBitmap = new AutoRuntimeClass< FlashBitmap >();
	classFlashBitmap->addMethod("getX", &FlashBitmap::getX);
	classFlashBitmap->addMethod("getY", &FlashBitmap::getY);
	classFlashBitmap->addMethod("getWidth", &FlashBitmap::getWidth);
	classFlashBitmap->addMethod("getHeight", &FlashBitmap::getHeight);
	registrar->registerClass(classFlashBitmap);

	Ref< AutoRuntimeClass< FlashBitmapImage > > classFlashBitmapImage = new AutoRuntimeClass< FlashBitmapImage >();
	classFlashBitmapImage->addConstructor< const drawing::Image* >();
	classFlashBitmapImage->addMethod("getImage", &FlashBitmapImage::getImage);
	registrar->registerClass(classFlashBitmapImage);

	Ref< AutoRuntimeClass< FlashBitmapResource > > classFlashBitmapResource = new AutoRuntimeClass< FlashBitmapResource >();
	classFlashBitmapResource->addMethod("getAtlasWidth", &FlashBitmapResource::getAtlasWidth);
	classFlashBitmapResource->addMethod("getAtlasHeight", &FlashBitmapResource::getAtlasHeight);
	classFlashBitmapResource->addMethod("getResourceId", &FlashBitmapResource::getResourceId);
	registrar->registerClass(classFlashBitmapResource);

	Ref< AutoRuntimeClass< FlashBitmapTexture > > classFlashBitmapTexture = new AutoRuntimeClass< FlashBitmapTexture >();
	classFlashBitmapTexture->addConstructor< render::ISimpleTexture* >();
	classFlashBitmapTexture->addMethod("getTexture", &FlashBitmapTexture::getTexture);
	registrar->registerClass(classFlashBitmapTexture);

	Ref< AutoRuntimeClass< FlashDictionary > > classFlashDictionary = new AutoRuntimeClass< FlashDictionary >();
	classFlashDictionary->addMethod("addFont", &FlashDictionary::addFont);
	classFlashDictionary->addMethod("addBitmap", &FlashDictionary::addBitmap);
	classFlashDictionary->addMethod("addSound", &FlashDictionary::addSound);
	classFlashDictionary->addMethod("addCharacter", &FlashDictionary::addCharacter);
	classFlashDictionary->addMethod("getFont", &FlashDictionary::getFont);
	classFlashDictionary->addMethod("getBitmap", &FlashDictionary::getBitmap);
	classFlashDictionary->addMethod("getSound", &FlashDictionary::getSound);
	classFlashDictionary->addMethod("getCharacter", &FlashDictionary::getCharacter);
	classFlashDictionary->addMethod("getExportId", &FlashDictionary_getExportId);
	classFlashDictionary->addMethod("getExportName", &FlashDictionary_getExportName);
	registrar->registerClass(classFlashDictionary);

	Ref< AutoRuntimeClass< FlashCharacterInstance > > classFlashCharacterInstance = new AutoRuntimeClass< FlashCharacterInstance >();
	classFlashCharacterInstance->addConstant("SbmDefault", Any::fromInteger(SbmDefault));
	classFlashCharacterInstance->addConstant("SbmNormal", Any::fromInteger(SbmNormal));
	classFlashCharacterInstance->addConstant("SbmLayer", Any::fromInteger(SbmLayer));
	classFlashCharacterInstance->addConstant("SbmMultiply", Any::fromInteger(SbmMultiply));
	classFlashCharacterInstance->addConstant("SbmScreen", Any::fromInteger(SbmScreen));
	classFlashCharacterInstance->addConstant("SbmLighten", Any::fromInteger(SbmLighten));
	classFlashCharacterInstance->addConstant("SbmDarken", Any::fromInteger(SbmDarken));
	classFlashCharacterInstance->addConstant("SbmDifference", Any::fromInteger(SbmDifference));
	classFlashCharacterInstance->addConstant("SbmAdd", Any::fromInteger(SbmAdd));
	classFlashCharacterInstance->addConstant("SbmSubtract", Any::fromInteger(SbmSubtract));
	classFlashCharacterInstance->addConstant("SbmInvert", Any::fromInteger(SbmInvert));
	classFlashCharacterInstance->addConstant("SbmAlpha", Any::fromInteger(SbmAlpha));
	classFlashCharacterInstance->addConstant("SbmErase", Any::fromInteger(SbmErase));
	classFlashCharacterInstance->addConstant("SbmOverlay", Any::fromInteger(SbmOverlay));
	classFlashCharacterInstance->addConstant("SbmHardlight", Any::fromInteger(SbmHardlight));
	classFlashCharacterInstance->addConstant("SbmOpaque", Any::fromInteger(SbmOpaque));
	classFlashCharacterInstance->addStaticMethod("getInstanceCount", &FlashCharacterInstance::getInstanceCount);
	classFlashCharacterInstance->addMethod("destroy", &FlashCharacterInstance::destroy);
	classFlashCharacterInstance->addMethod("getContext", &FlashCharacterInstance::getContext);
	classFlashCharacterInstance->addMethod("getDictionary", &FlashCharacterInstance::getDictionary);
	classFlashCharacterInstance->addMethod("getParent", &FlashCharacterInstance::getParent);
	classFlashCharacterInstance->addMethod("setName", &FlashCharacterInstance::setName);
	classFlashCharacterInstance->addMethod("getName", &FlashCharacterInstance::getName);
	classFlashCharacterInstance->addMethod("getTarget", &FlashCharacterInstance::getTarget);
	classFlashCharacterInstance->addMethod("setColorTransform", &FlashCharacterInstance::setColorTransform);
	classFlashCharacterInstance->addMethod("setColorTransform", &FlashCharacterInstance_setColorTransform);
	classFlashCharacterInstance->addMethod("getColorTransform", &FlashCharacterInstance::getColorTransform);
	classFlashCharacterInstance->addMethod("setTransform", &FlashCharacterInstance::setTransform);
	classFlashCharacterInstance->addMethod("getTransform", &FlashCharacterInstance::getTransform);
	classFlashCharacterInstance->addMethod("getFullTransform", &FlashCharacterInstance::getFullTransform);
	classFlashCharacterInstance->addMethod("transformInto", &FlashCharacterInstance::transformInto);
	classFlashCharacterInstance->addMethod("setFilter", &FlashCharacterInstance::setFilter);
	classFlashCharacterInstance->addMethod("getFilter", &FlashCharacterInstance::getFilter);
	classFlashCharacterInstance->addMethod("setFilterColor", &FlashCharacterInstance::setFilterColor);
	classFlashCharacterInstance->addMethod("getFilterColor", &FlashCharacterInstance::getFilterColor);
	classFlashCharacterInstance->addMethod("setBlendMode", &FlashCharacterInstance::setBlendMode);
	classFlashCharacterInstance->addMethod("getBlendMode", &FlashCharacterInstance::getBlendMode);
	classFlashCharacterInstance->addMethod("setVisible", &FlashCharacterInstance::setVisible);
	classFlashCharacterInstance->addMethod("isVisible", &FlashCharacterInstance::isVisible);
	classFlashCharacterInstance->addMethod("setEnabled", &FlashCharacterInstance::setEnabled);
	classFlashCharacterInstance->addMethod("isEnabled", &FlashCharacterInstance::isEnabled);
	classFlashCharacterInstance->addMethod("getBounds", &FlashCharacterInstance::getBounds);
	classFlashCharacterInstance->setUnknownHandler(&FlashCharacterInstance_invoke);
	registrar->registerClass(classFlashCharacterInstance);

	Ref< AutoRuntimeClass< FlashShape > > classFlashShape = new AutoRuntimeClass< FlashShape >();
	classFlashShape->addMethod("getPathCount", &FlashShape_getPathCount);
	registrar->registerClass(classFlashShape);

	Ref< AutoRuntimeClass< FlashShapeInstance > > classFlashShapeInstance = new AutoRuntimeClass< FlashShapeInstance >();
	classFlashShapeInstance->addMethod("getShape", &FlashShapeInstance::getShape);
	classFlashShapeInstance->setUnknownHandler(&FlashShapeInstance_invoke);
	registrar->registerClass(classFlashShapeInstance);

	Ref< AutoRuntimeClass< FlashDisplayList > > classFlashDisplayList = new AutoRuntimeClass< FlashDisplayList >();
	classFlashDisplayList->addMethod("reset", &FlashDisplayList::reset);
	classFlashDisplayList->addMethod("showObject", &FlashDisplayList::showObject);
	classFlashDisplayList->addMethod("removeObject", &FlashDisplayList::removeObject);
	classFlashDisplayList->addMethod("getObjectDepth", &FlashDisplayList::getObjectDepth);
	classFlashDisplayList->addMethod("getNextHighestDepth", &FlashDisplayList::getNextHighestDepth);
	classFlashDisplayList->addMethod("swap", &FlashDisplayList::swap);
	classFlashDisplayList->addMethod("getObjects", &FlashDisplayList_getObjects);
	registrar->registerClass(classFlashDisplayList);

	Ref< AutoRuntimeClass< FlashSprite > > classFlashSprite = new AutoRuntimeClass< FlashSprite >();
	classFlashSprite->addMethod("getFrameRate", &FlashSprite::getFrameRate);
	classFlashSprite->addMethod("addFrame", &FlashSprite::addFrame);
	classFlashSprite->addMethod("getFrameCount", &FlashSprite::getFrameCount);
	classFlashSprite->addMethod("getFrame", &FlashSprite::getFrame);
	classFlashSprite->addMethod("findFrame", &FlashSprite::findFrame);
	registrar->registerClass(classFlashSprite);

	Ref< AutoRuntimeClass< FlashSpriteInstance > > classFlashSpriteInstance = new AutoRuntimeClass< FlashSpriteInstance >();
	classFlashSpriteInstance->addMethod("getSprite", &FlashSpriteInstance::getSprite);
	classFlashSpriteInstance->addMethod("setCacheAsBitmap", &FlashSpriteInstance::setCacheAsBitmap);
	classFlashSpriteInstance->addMethod("getCacheAsBitmap", &FlashSpriteInstance::getCacheAsBitmap);
	classFlashSpriteInstance->addMethod("gotoFrame", &FlashSpriteInstance::gotoFrame);
	classFlashSpriteInstance->addMethod("gotoPrevious", &FlashSpriteInstance::gotoPrevious);
	classFlashSpriteInstance->addMethod("gotoNext", &FlashSpriteInstance::gotoNext);
	classFlashSpriteInstance->addMethod("getCurrentFrame", &FlashSpriteInstance::getCurrentFrame);
	classFlashSpriteInstance->addMethod("setPlaying", &FlashSpriteInstance::setPlaying);
	classFlashSpriteInstance->addMethod("getPlaying", &FlashSpriteInstance::getPlaying);
	classFlashSpriteInstance->addMethod("getDisplayList", &FlashSpriteInstance_getDisplayList);
	classFlashSpriteInstance->addMethod("createEmptyMovieClip", &FlashSpriteInstance::createEmptyMovieClip);
	classFlashSpriteInstance->addMethod("createTextField", &FlashSpriteInstance::createTextField);
	classFlashSpriteInstance->addMethod("removeMovieClip", &FlashSpriteInstance::removeMovieClip);
	classFlashSpriteInstance->addMethod("clone", &FlashSpriteInstance::clone);
	classFlashSpriteInstance->addMethod("duplicateMovieClip", &FlashSpriteInstance_duplicateMovieClip_1);
	classFlashSpriteInstance->addMethod("duplicateMovieClip", &FlashSpriteInstance_duplicateMovieClip_2);
	classFlashSpriteInstance->addMethod("attachBitmap", &FlashSpriteInstance::attachBitmap);
	classFlashSpriteInstance->addMethod("getLocalBounds", &FlashSpriteInstance::getLocalBounds);
	classFlashSpriteInstance->addMethod("getVisibleLocalBounds", &FlashSpriteInstance::getVisibleLocalBounds);
	classFlashSpriteInstance->addMethod("setMask", &FlashSpriteInstance::setMask);
	classFlashSpriteInstance->addMethod("getMask", &FlashSpriteInstance::getMask);
	classFlashSpriteInstance->addMethod("createCanvas", &FlashSpriteInstance::createCanvas);
	classFlashSpriteInstance->addMethod("getCanvas", &FlashSpriteInstance::getCanvas);
	classFlashSpriteInstance->addMethod("getMouseX", &FlashSpriteInstance::getMouseX);
	classFlashSpriteInstance->addMethod("getMouseY", &FlashSpriteInstance::getMouseY);
	classFlashSpriteInstance->addMethod("setPosition", &FlashSpriteInstance::setPosition);
	classFlashSpriteInstance->addMethod("getPosition", &FlashSpriteInstance::getPosition);
	classFlashSpriteInstance->addMethod("setX", &FlashSpriteInstance::setX);
	classFlashSpriteInstance->addMethod("getX", &FlashSpriteInstance::getX);
	classFlashSpriteInstance->addMethod("setY", &FlashSpriteInstance::setY);
	classFlashSpriteInstance->addMethod("getY", &FlashSpriteInstance::getY);
	classFlashSpriteInstance->addMethod("setSize", &FlashSpriteInstance::setSize);
	classFlashSpriteInstance->addMethod("getSize", &FlashSpriteInstance::getSize);
	classFlashSpriteInstance->addMethod("setWidth", &FlashSpriteInstance::setWidth);
	classFlashSpriteInstance->addMethod("getWidth", &FlashSpriteInstance::getWidth);
	classFlashSpriteInstance->addMethod("setHeight", &FlashSpriteInstance::setHeight);
	classFlashSpriteInstance->addMethod("getHeight", &FlashSpriteInstance::getHeight);
	classFlashSpriteInstance->addMethod("setRotation", &FlashSpriteInstance::setRotation);
	classFlashSpriteInstance->addMethod("getRotation", &FlashSpriteInstance::getRotation);
	classFlashSpriteInstance->addMethod("setScale", &FlashSpriteInstance::setScale);
	classFlashSpriteInstance->addMethod("getScale", &FlashSpriteInstance::getScale);
	classFlashSpriteInstance->addMethod("setXScale", &FlashSpriteInstance::setXScale);
	classFlashSpriteInstance->addMethod("getXScale", &FlashSpriteInstance::getXScale);
	classFlashSpriteInstance->addMethod("setYScale", &FlashSpriteInstance::setYScale);
	classFlashSpriteInstance->addMethod("getYScale", &FlashSpriteInstance::getYScale);
	classFlashSpriteInstance->setUnknownHandler(&FlashSpriteInstance_invoke);
	registrar->registerClass(classFlashSpriteInstance);

	Ref< AutoRuntimeClass< FlashTextFormat > > classFlashTextFormat = new AutoRuntimeClass< FlashTextFormat >();
	//classFlashTextFormat->addConstructor< float, int32_t, float >();
	classFlashTextFormat->addMethod("setLetterSpacing", &FlashTextFormat::setLetterSpacing);
	classFlashTextFormat->addMethod("getLetterSpacing", &FlashTextFormat::getLetterSpacing);
	//classFlashTextFormat->addMethod("setAlign", &FlashTextFormat::setAlign);
	//classFlashTextFormat->addMethod("getAlign", &FlashTextFormat::getAlign);
	classFlashTextFormat->addMethod("setSize", &FlashTextFormat::setSize);
	classFlashTextFormat->addMethod("getSize", &FlashTextFormat::getSize);
	registrar->registerClass(classFlashTextFormat);

	Ref< AutoRuntimeClass< FlashEditInstance > > classFlashEditInstance = new AutoRuntimeClass< FlashEditInstance >();
	classFlashEditInstance->addMethod("parseText", &FlashEditInstance::parseText);
	classFlashEditInstance->addMethod("parseHtml", &FlashEditInstance::parseHtml);
	classFlashEditInstance->addMethod("setTextBounds", &FlashEditInstance::setTextBounds);
	classFlashEditInstance->addMethod("getTextBounds", &FlashEditInstance::getTextBounds);
	classFlashEditInstance->addMethod("setTextColor", &FlashEditInstance::setTextColor);
	classFlashEditInstance->addMethod("getTextColor", &FlashEditInstance::getTextColor);
	classFlashEditInstance->addMethod("setLetterSpacing", &FlashEditInstance::setLetterSpacing);
	classFlashEditInstance->addMethod("getLetterSpacing", &FlashEditInstance::getLetterSpacing);
	classFlashEditInstance->addMethod("setTextFormat", &FlashEditInstance_setTextFormat_1);
	classFlashEditInstance->addMethod("getTextFormat", &FlashEditInstance_getTextFormat_0);
	classFlashEditInstance->addMethod("setTextFormat", &FlashEditInstance_setTextFormat_3);
	classFlashEditInstance->addMethod("getTextFormat", &FlashEditInstance_getTextFormat_2);
	classFlashEditInstance->addMethod("getText", &FlashEditInstance::getText);
	classFlashEditInstance->addMethod("getHtmlText", &FlashEditInstance::getHtmlText);
	classFlashEditInstance->addMethod("setPassword", &FlashEditInstance::setPassword);
	classFlashEditInstance->addMethod("getPassword", &FlashEditInstance::getPassword);
	classFlashEditInstance->addMethod("getCaret", &FlashEditInstance::getCaret);
	classFlashEditInstance->addMethod("setScroll", &FlashEditInstance::setScroll);
	classFlashEditInstance->addMethod("getScroll", &FlashEditInstance::getScroll);
	classFlashEditInstance->addMethod("getMaxScroll", &FlashEditInstance::getMaxScroll);
	classFlashEditInstance->addMethod("setPosition", &FlashEditInstance::setPosition);
	classFlashEditInstance->addMethod("getPosition", &FlashEditInstance::getPosition);
	classFlashEditInstance->addMethod("setX", &FlashEditInstance::setX);
	classFlashEditInstance->addMethod("getX", &FlashEditInstance::getX);
	classFlashEditInstance->addMethod("setY", &FlashEditInstance::setY);
	classFlashEditInstance->addMethod("getY", &FlashEditInstance::getY);
	classFlashEditInstance->addMethod("setSize", &FlashEditInstance::setSize);
	classFlashEditInstance->addMethod("getSize", &FlashEditInstance::getSize);
	classFlashEditInstance->addMethod("setWidth", &FlashEditInstance::setWidth);
	classFlashEditInstance->addMethod("getWidth", &FlashEditInstance::getWidth);
	classFlashEditInstance->addMethod("setHeight", &FlashEditInstance::setHeight);
	classFlashEditInstance->addMethod("getHeight", &FlashEditInstance::getHeight);
	classFlashEditInstance->addMethod("setRotation", &FlashEditInstance::setRotation);
	classFlashEditInstance->addMethod("getRotation", &FlashEditInstance::getRotation);
	classFlashEditInstance->addMethod("setScale", &FlashEditInstance::setScale);
	classFlashEditInstance->addMethod("getScale", &FlashEditInstance::getScale);
	classFlashEditInstance->addMethod("setXScale", &FlashEditInstance::setXScale);
	classFlashEditInstance->addMethod("getXScale", &FlashEditInstance::getXScale);
	classFlashEditInstance->addMethod("setYScale", &FlashEditInstance::setYScale);
	classFlashEditInstance->addMethod("getYScale", &FlashEditInstance::getYScale);
	classFlashEditInstance->addMethod("getTextSize", &FlashEditInstance::getTextSize);
	classFlashEditInstance->addMethod("getTextWidth", &FlashEditInstance::getTextWidth);
	classFlashEditInstance->addMethod("getTextHeight", &FlashEditInstance::getTextHeight);
	classFlashEditInstance->setUnknownHandler(&FlashEditInstance_invoke);
	registrar->registerClass(classFlashEditInstance);

	Ref< AutoRuntimeClass< FlashMovie > > classFlashMovie = new AutoRuntimeClass< FlashMovie >();
	classFlashMovie->addMethod("defineFont", &FlashMovie::defineFont);
	classFlashMovie->addMethod("defineBitmap", &FlashMovie::defineBitmap);
	classFlashMovie->addMethod("defineSound", &FlashMovie::defineSound);
	classFlashMovie->addMethod("defineCharacter", &FlashMovie::defineCharacter);
	classFlashMovie->addMethod("setExport", &FlashMovie::setExport);
	classFlashMovie->addMethod("createMovieClipInstance", &FlashMovie::createMovieClipInstance);
	classFlashMovie->addMethod("createExternalMovieClipInstance", &FlashMovie::createExternalMovieClipInstance);
	classFlashMovie->addMethod("createExternalSpriteInstance", &FlashMovie::createExternalSpriteInstance);
	classFlashMovie->addMethod("getFrameBounds", &FlashMovie::getFrameBounds);
	classFlashMovie->addMethod("getMovieClip", &FlashMovie::getMovieClip);
	registrar->registerClass(classFlashMovie);

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

	Ref< AutoRuntimeClass< FlashOptimizer > > classFlashOptimizer = new AutoRuntimeClass< FlashOptimizer >();
	classFlashOptimizer->addConstructor();
	classFlashOptimizer->addMethod("merge", &FlashOptimizer::merge);
	classFlashOptimizer->addMethod("triangulate", &FlashOptimizer::triangulate);
	registrar->registerClass(classFlashOptimizer);
}

	}
}

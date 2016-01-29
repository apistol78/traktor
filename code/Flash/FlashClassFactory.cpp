#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/Boxes.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Core/Io/IStream.h"
#include "Drawing/Image.h"
#include "Flash/FlashBitmap.h"
#include "Flash/FlashCast.h"
#include "Flash/FlashCharacter.h"
#include "Flash/FlashClassFactory.h"
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
#include "Flash/SwfReader.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFunction.h"
#include "Flash/Action/ActionValue.h"
#include "Flash/Action/Common/BitmapData.h"

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

void FlashCharacterInstance_setColorTransform(FlashCharacterInstance* self, const Color4f& mul, const Color4f& add)
{
	SwfCxTransform cxform =
	{
		{ mul.getRed()  , add.getRed()   },
		{ mul.getGreen(), add.getGreen() },
		{ mul.getBlue() , add.getBlue()  },
		{ mul.getAlpha(), add.getAlpha() }
	};
	self->setColorTransform(cxform);
}

Any FlashCharacterInstance_invoke(FlashCharacterInstance* self, const std::string& methodName, uint32_t argc, const Any* argv)
{
	return ActionObjectRelay_invoke(self, methodName, argc, argv);
}

Any FlashShapeInstance_invoke(FlashShapeInstance* self, const std::string& methodName, uint32_t argc, const Any* argv)
{
	return ActionObjectRelay_invoke(self, methodName, argc, argv);
}

Any FlashSpriteInstance_invoke(FlashSpriteInstance* self, const std::string& methodName, uint32_t argc, const Any* argv)
{
	return ActionObjectRelay_invoke(self, methodName, argc, argv);
}

uint32_t FlashShape_getPathCount(FlashShape* self)
{
	return uint32_t(self->getPaths().size());
}

RefArray< FlashCharacterInstance > FlashDisplayList_getVisibleObjects(FlashDisplayList* self)
{
	RefArray< FlashCharacterInstance > visibleCharacters;
	self->getVisibleObjects(visibleCharacters);
	return visibleCharacters;
}

FlashDisplayList* FlashSpriteInstance_getDisplayList(FlashSpriteInstance* self)
{
	return &self->getDisplayList();
}


Ref< ActionObject > ActionContext_createObject_0(ActionContext* self)
{
	return new flash::ActionObject(self);
}

Ref< ActionObject > ActionContext_createObject_1(ActionContext* self, const std::string& prototype)
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

	classFunction->call(obj);
	return obj;
}

Ref< ActionObject > ActionContext_createObject_2(ActionContext* self, const std::string& prototype, const std::vector< Any >& argv)
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

	ActionValueArray args(self->getPool(), argv.size());
	for (uint32_t i = 0; i < argv.size(); ++i)
		args[i] = CastAny< ActionValue >::get(argv[i]);

	classFunction->call(obj, args);
	return obj;
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
	classFlashMovieLoader->addMethod("setIncludeAS", &FlashMovieLoader::setIncludeAS);
	registrar->registerClass(classFlashMovieLoader);

	Ref< AutoRuntimeClass< FlashMovieFactory > > classFlashMovieFactory = new AutoRuntimeClass< FlashMovieFactory >();
	classFlashMovieFactory->addConstructor< bool >();
	classFlashMovieFactory->addMethod("createMovie", &FlashMovieFactory::createMovie);
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

	Ref< AutoRuntimeClass< FlashCharacterInstance > > classFlashCharacterInstance = new AutoRuntimeClass< FlashCharacterInstance >();
	classFlashCharacterInstance->addMethod("getContext", &FlashCharacterInstance::getContext);
	classFlashCharacterInstance->addMethod("getParent", &FlashCharacterInstance::getParent);
	classFlashCharacterInstance->addMethod("setName", &FlashCharacterInstance::setName);
	classFlashCharacterInstance->addMethod("getName", &FlashCharacterInstance::getName);
	classFlashCharacterInstance->addMethod("getTarget", &FlashCharacterInstance::getTarget);
	classFlashCharacterInstance->addMethod("setColorTransform", &FlashCharacterInstance_setColorTransform);
	classFlashCharacterInstance->addMethod("setTransform", &FlashCharacterInstance::setTransform);
	classFlashCharacterInstance->addMethod("getTransform", &FlashCharacterInstance::getTransform);
	classFlashCharacterInstance->addMethod("getFullTransform", &FlashCharacterInstance::getFullTransform);
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
	classFlashDisplayList->addMethod("getVisibleObjects", &FlashDisplayList_getVisibleObjects);
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
	classFlashSpriteInstance->addMethod("setOpaqueBackground", &FlashSpriteInstance::setOpaqueBackground);
	classFlashSpriteInstance->addMethod("getOpaqueBackground", &FlashSpriteInstance::getOpaqueBackground);
	classFlashSpriteInstance->addMethod("gotoFrame", &FlashSpriteInstance::gotoFrame);
	classFlashSpriteInstance->addMethod("gotoPrevious", &FlashSpriteInstance::gotoPrevious);
	classFlashSpriteInstance->addMethod("gotoNext", &FlashSpriteInstance::gotoNext);
	classFlashSpriteInstance->addMethod("getCurrentFrame", &FlashSpriteInstance::getCurrentFrame);
	classFlashSpriteInstance->addMethod("setPlaying", &FlashSpriteInstance::setPlaying);
	classFlashSpriteInstance->addMethod("getPlaying", &FlashSpriteInstance::getPlaying);
	classFlashSpriteInstance->addMethod("getDisplayList", &FlashSpriteInstance_getDisplayList);
	classFlashSpriteInstance->addMethod("createEmptyMovieClip", &FlashSpriteInstance::createEmptyMovieClip);
	classFlashSpriteInstance->addMethod("removeMovieClip", &FlashSpriteInstance::removeMovieClip);
	classFlashSpriteInstance->addMethod("clone", &FlashSpriteInstance::clone);
	classFlashSpriteInstance->addMethod("getLocalBounds", &FlashSpriteInstance::getLocalBounds);
	classFlashSpriteInstance->addMethod("getVisibleLocalBounds", &FlashSpriteInstance::getVisibleLocalBounds);
	classFlashSpriteInstance->addMethod("setMask", &FlashSpriteInstance::setMask);
	classFlashSpriteInstance->addMethod("getMask", &FlashSpriteInstance::getMask);
	classFlashSpriteInstance->addMethod("createCanvas", &FlashSpriteInstance::createCanvas);
	classFlashSpriteInstance->addMethod("getCanvas", &FlashSpriteInstance::getCanvas);
	classFlashSpriteInstance->addMethod("getMouseX", &FlashSpriteInstance::getMouseX);
	classFlashSpriteInstance->addMethod("getMouseY", &FlashSpriteInstance::getMouseY);
	classFlashSpriteInstance->setUnknownHandler(&FlashSpriteInstance_invoke);
	registrar->registerClass(classFlashSpriteInstance);

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
	classActionContext->addMethod("createBitmap", &ActionContext_createBitmap);
	registrar->registerClass(classActionContext);

	Ref< AutoRuntimeClass< FlashOptimizer > > classFlashOptimizer = new AutoRuntimeClass< FlashOptimizer >();
	classFlashOptimizer->addConstructor();
	classFlashOptimizer->addMethod("merge", &FlashOptimizer::merge);
	registrar->registerClass(classFlashOptimizer);
}

	}
}

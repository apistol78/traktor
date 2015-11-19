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
#include "Flash/FlashMovie.h"
#include "Flash/FlashMovieFactory.h"
#include "Flash/FlashMovieLoader.h"
#include "Flash/FlashMoviePlayer.h"
#include "Flash/FlashOptimizer.h"
#include "Flash/FlashSound.h"
#include "Flash/FlashSpriteInstance.h"
#include "Flash/SwfReader.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFunction.h"
#include "Flash/Action/ActionValue.h"

namespace traktor
{
	namespace flash
	{
		namespace
		{

ActionObject* ActionObjectRelay_getAsObject_0(ActionObjectRelay* self)
{
	return self->getAsObject();
}

ActionObject* ActionObjectRelay_getAsObject_1(ActionObjectRelay* self, ActionContext* cx)
{
	return self->getAsObject(cx);
}

		}

class ActionObjectClass : public IRuntimeClass
{
	T_RTTI_CLASS;

public:
	virtual const TypeInfo& getExportType() const T_OVERRIDE;

	virtual bool haveConstructor() const T_OVERRIDE;

	virtual bool haveUnknown() const T_OVERRIDE;

	virtual Ref< ITypedObject > construct(ITypedObject* self, uint32_t argc, const Any* argv, const prototype_t& proto) const T_OVERRIDE;

	virtual uint32_t getConstantCount() const T_OVERRIDE;

	virtual std::string getConstantName(uint32_t constId) const T_OVERRIDE;

	virtual Any getConstantValue(uint32_t constId) const T_OVERRIDE;

	virtual uint32_t getMethodCount() const T_OVERRIDE;

	virtual std::string getMethodName(uint32_t methodId) const T_OVERRIDE;

	virtual void getMethodSignature(uint32_t methodId, const wchar_t* outSignature[MaxSignatures]) const T_OVERRIDE;

	virtual Any invoke(ITypedObject* object, uint32_t methodId, uint32_t argc, const Any* argv) const T_OVERRIDE;

	virtual uint32_t getStaticMethodCount() const T_OVERRIDE;

	virtual std::string getStaticMethodName(uint32_t methodId) const T_OVERRIDE;

	virtual void getStaticMethodSignature(uint32_t methodId, const wchar_t* outSignature[MaxSignatures]) const T_OVERRIDE;

	virtual Any invokeStatic(uint32_t methodId, uint32_t argc, const Any* argv) const T_OVERRIDE;

	virtual Any invokeUnknown(ITypedObject* object, const std::string& methodName, uint32_t argc, const Any* argv) const T_OVERRIDE;

	virtual Any invokeOperator(ITypedObject* object, uint8_t operation, const Any& arg) const T_OVERRIDE;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.ActionObjectClass", ActionObjectClass, IRuntimeClass)

const TypeInfo& ActionObjectClass::getExportType() const
{
	return type_of< ActionObject >();
}

bool ActionObjectClass::haveConstructor() const
{
	return false;
}

bool ActionObjectClass::haveUnknown() const
{
	return true;
}

Ref< ITypedObject > ActionObjectClass::construct(ITypedObject* self, uint32_t argc, const Any* argv, const prototype_t& proto) const
{
	return 0;
}

uint32_t ActionObjectClass::getConstantCount() const
{
	return 0;
}

std::string ActionObjectClass::getConstantName(uint32_t constId) const
{
	return "";
}

Any ActionObjectClass::getConstantValue(uint32_t constId) const
{
	return Any();
}

uint32_t ActionObjectClass::getMethodCount() const
{
	return 5;
}

std::string ActionObjectClass::getMethodName(uint32_t methodId) const
{
	switch (methodId)
	{
	case 0:
		return "getMember";
	case 1:
		return "getMemberByQName";
	case 2:
		return "setMember";
	case 3:
		return "getProperty";
	case 4:
		return "setProperty";
	default:
		return "";
	}
}

void ActionObjectClass::getMethodSignature(uint32_t methodId, const wchar_t* outSignature[MaxSignatures]) const
{
}

Any ActionObjectClass::invoke(ITypedObject* object, uint32_t methodId, uint32_t argc, const Any* argv) const
{
	ActionObject* actionObject = checked_type_cast< ActionObject*, false >(object);
	switch (methodId)
	{
	case 0:
		{
			ActionValue memberValue;
			if (actionObject->getMember(argv[0].getString(), memberValue))
				return CastAny< ActionValue >::set(memberValue);
		}
		break;

	case 1:
		{
			ActionValue memberValue;
			if (actionObject->getMemberByQName(argv[0].getString(), memberValue))
				return CastAny< ActionValue >::set(memberValue);
		}
		break;

	case 2:
		{
			ActionValue memberValue = CastAny< ActionValue >::get(argv[1]);
			actionObject->setMember(argv[0].getString(), memberValue);
		}
		break;

	case 3:
		{
			Ref< ActionFunction > propertyGetFn;
			if (actionObject->getPropertyGet(argv[0].getString(), propertyGetFn))
			{
				ActionValue propertyValue = propertyGetFn->call(actionObject);
				return CastAny< ActionValue >::set(propertyValue);
			}
		}
		break;

	case 4:
		{
			Ref< ActionFunction > propertySetFn;
			if (actionObject->getPropertySet(argv[0].getString(), propertySetFn))
			{
				ActionValueArray callArgv(actionObject->getContext()->getPool(), 1);
				callArgv[0] = CastAny< ActionValue >::get(argv[1]);
				propertySetFn->call(actionObject, callArgv);
			}
		}
		break;

	default:
		break;
	}
	return Any();
}

uint32_t ActionObjectClass::getStaticMethodCount() const
{
	return 0;
}

std::string ActionObjectClass::getStaticMethodName(uint32_t methodId) const
{
	return "";
}

void ActionObjectClass::getStaticMethodSignature(uint32_t methodId, const wchar_t* outSignature[MaxSignatures]) const
{
}

Any ActionObjectClass::invokeStatic(uint32_t methodId, uint32_t argc, const Any* argv) const
{
	return Any();
}

Any ActionObjectClass::invokeUnknown(ITypedObject* object, const std::string& methodName, uint32_t argc, const Any* argv) const
{
	ActionObject* actionObject = checked_type_cast< ActionObject*, false >(object);
	T_ASSERT (object);

	ActionValuePool& pool = actionObject->getContext()->getPool();
	T_ANONYMOUS_VAR(ActionValuePool::Scope)(pool);

	ActionValueArray args(pool, argc);
	for (uint32_t i = 0; i < argc; ++i)
		args[i] = CastAny< ActionValue >::get(argv[i]);

	ActionValue functionValue;
	actionObject->getMember(methodName, functionValue);

	ActionFunction* fn = functionValue.getObject< ActionFunction >();
	if (fn)
	{
		ActionValue ret = fn->call(actionObject, args);
		return CastAny< ActionValue >::set(ret);
	}

	return Any();
}

Any ActionObjectClass::invokeOperator(ITypedObject* object, uint8_t operation, const Any& arg) const
{
	return Any();
}

class ActionFunctionClass : public ActionObjectClass
{
	T_RTTI_CLASS;

public:
	virtual const TypeInfo& getExportType() const T_OVERRIDE T_FINAL;

	virtual uint32_t getMethodCount() const T_OVERRIDE T_FINAL;

	virtual std::string getMethodName(uint32_t methodId) const T_OVERRIDE T_FINAL;

	virtual Any invoke(ITypedObject* object, uint32_t methodId, uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.ActionFunctionClass", ActionFunctionClass, ActionObjectClass)

const TypeInfo& ActionFunctionClass::getExportType() const
{
	return type_of< ActionFunction >();
}

uint32_t ActionFunctionClass::getMethodCount() const
{
	return 6;
}

std::string ActionFunctionClass::getMethodName(uint32_t methodId) const
{
	switch (methodId)
	{
	case 0:
		return "getMember";
	case 1:
		return "getMemberByQName";
	case 2:
		return "setMember";
	case 3:
		return "getProperty";
	case 4:
		return "setProperty";
	case 5:
		return "call";
	default:
		return "";
	}
}

Any ActionFunctionClass::invoke(ITypedObject* object, uint32_t methodId, uint32_t argc, const Any* argv) const
{
	ActionFunction* actionObject = checked_type_cast< ActionFunction*, false >(object);
	switch (methodId)
	{
	case 0:
		{
			ActionValue memberValue;
			if (actionObject->getMember(argv[0].getString(), memberValue))
				return CastAny< ActionValue >::set(memberValue);
		}
		break;

	case 1:
		{
			ActionValue memberValue;
			if (actionObject->getMemberByQName(argv[0].getString(), memberValue))
				return CastAny< ActionValue >::set(memberValue);
		}
		break;

	case 2:
		{
			ActionValue memberValue = CastAny< ActionValue >::get(argv[1]);
			actionObject->setMember(argv[0].getString(), memberValue);
		}
		break;

	case 3:
		{
			Ref< ActionFunction > propertyGetFn;
			if (actionObject->getPropertyGet(argv[0].getString(), propertyGetFn))
			{
				ActionValue propertyValue = propertyGetFn->call(actionObject);
				return CastAny< ActionValue >::set(propertyValue);
			}
		}
		break;

	case 4:
		{
			Ref< ActionFunction > propertySetFn;
			if (actionObject->getPropertySet(argv[0].getString(), propertySetFn))
			{
				ActionValueArray callArgv(actionObject->getContext()->getPool(), 1);
				callArgv[0] = CastAny< ActionValue >::get(argv[1]);
				propertySetFn->call(actionObject, callArgv);
			}
		}
		break;

	case 5:
		{
			ActionValuePool& pool = actionObject->getContext()->getPool();
			T_ANONYMOUS_VAR(ActionValuePool::Scope)(pool);

			ActionValueArray args(pool, argc - 1);
			for (uint32_t i = 0; i < argc - 1; ++i)
				args[i] = CastAny< ActionValue >::get(argv[i + 1]);

			ActionValue ret = actionObject->call(
				checked_type_cast< ActionObject*, true >(argv[0].getObject()),
				args
			);

			return CastAny< ActionValue >::set(ret);
		}
		break;

	default:
		break;
	}
	return Any();
}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.flash.FlashClassFactory", 0, FlashClassFactory, IRuntimeClassFactory)

void FlashClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
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
	registrar->registerClass(classFlashMovieLoader);

	Ref< AutoRuntimeClass< FlashMovieFactory > > classFlashMovieFactory = new AutoRuntimeClass< FlashMovieFactory >();
	classFlashMovieFactory->addConstructor();
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
	classFlashCharacterInstance->addMethod("setTransform", &FlashCharacterInstance::setTransform);
	classFlashCharacterInstance->addMethod("getTransform", &FlashCharacterInstance::getTransform);
	classFlashCharacterInstance->addMethod("getFullTransform", &FlashCharacterInstance::getFullTransform);
	classFlashCharacterInstance->addMethod("setVisible", &FlashCharacterInstance::setVisible);
	classFlashCharacterInstance->addMethod("isVisible", &FlashCharacterInstance::isVisible);
	classFlashCharacterInstance->addMethod("setEnabled", &FlashCharacterInstance::setEnabled);
	classFlashCharacterInstance->addMethod("isEnabled", &FlashCharacterInstance::isEnabled);
	classFlashCharacterInstance->addMethod("getBounds", &FlashCharacterInstance::getBounds);
	registrar->registerClass(classFlashCharacterInstance);

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
	registrar->registerClass(classFlashSpriteInstance);

	Ref< AutoRuntimeClass< FlashMovie > > classFlashMovie = new AutoRuntimeClass< FlashMovie >();
	classFlashMovie->addMethod("defineFont", &FlashMovie::defineFont);
	classFlashMovie->addMethod("defineBitmap", &FlashMovie::defineBitmap);
	classFlashMovie->addMethod("defineSound", &FlashMovie::defineSound);
	classFlashMovie->addMethod("defineCharacter", &FlashMovie::defineCharacter);
	classFlashMovie->addMethod("setExport", &FlashMovie::setExport);
	classFlashMovie->addMethod("createExternalMovieClipInstance", &FlashMovie::createExternalMovieClipInstance);
	classFlashMovie->addMethod("getFrameBounds", &FlashMovie::getFrameBounds);
	classFlashMovie->addMethod("getMovieClip", &FlashMovie::getMovieClip);
	registrar->registerClass(classFlashMovie);

	Ref< AutoRuntimeClass< ActionObjectRelay > > classActionObjectRelay = new AutoRuntimeClass< ActionObjectRelay >();
	classActionObjectRelay->addMethod("getAsObject", &ActionObjectRelay_getAsObject_0);
	classActionObjectRelay->addMethod("getAsObject", &ActionObjectRelay_getAsObject_1);
	registrar->registerClass(classActionObjectRelay);

	Ref< AutoRuntimeClass< ActionContext > > classActionContext = new AutoRuntimeClass< ActionContext >();
	classActionContext->addMethod("lookupClass", &ActionContext::lookupClass);
	classActionContext->addMethod("getMovie", &ActionContext::getMovie);
	classActionContext->addMethod("getGlobal", &ActionContext::getGlobal);
	classActionContext->addMethod("getMovieClip", &ActionContext::getMovieClip);
	classActionContext->addMethod("getFocus", &ActionContext::getFocus);
	classActionContext->addMethod("getPressed", &ActionContext::getPressed);
	registrar->registerClass(classActionContext);

	Ref< ActionObjectClass > classActionObject = new ActionObjectClass();
	registrar->registerClass(classActionObject);

	Ref< ActionFunctionClass > classActionFunction = new ActionFunctionClass();
	registrar->registerClass(classActionFunction);
	
	Ref< AutoRuntimeClass< FlashOptimizer > > classFlashOptimizer = new AutoRuntimeClass< FlashOptimizer >();
	classFlashOptimizer->addConstructor();
	classFlashOptimizer->addMethod("merge", &FlashOptimizer::merge);
	classFlashOptimizer->addMethod("rasterize", &FlashOptimizer::rasterize);
	registrar->registerClass(classFlashOptimizer);
}

	}
}

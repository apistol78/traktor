#include "Amalgam/Engine/FlashCast.h"
#include "Amalgam/Engine/FlashLayer.h"
#include "Amalgam/Engine/Classes/FlashClasses.h"
#include "Drawing/Image.h"
#include "Flash/FlashMoviePlayer.h"
#include "Flash/FlashSpriteInstance.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFunction.h"
#include "Flash/Action/ActionValue.h"
#include "Script/AutoScriptClass.h"
#include "Script/IScriptManager.h"

namespace traktor
{
	namespace amalgam
	{
		namespace
		{

Ref< flash::ActionObject > FlashLayer_createObject_0(FlashLayer* self)
{
	return self->createObject();
}

Ref< flash::ActionObject > FlashLayer_createObject_1(FlashLayer* self, uint32_t argc, const script::Any* argv)
{
	return self->createObject(argc, argv);
}

		}

class ActionObjectClass : public script::IScriptClass
{
	T_RTTI_CLASS;

public:
	virtual const TypeInfo& getExportType() const;

	virtual bool haveConstructor() const;

	virtual bool haveUnknown() const;

	virtual Ref< ITypedObject > construct(const InvokeParam& param, uint32_t argc, const script::Any* argv) const;

	virtual uint32_t getMethodCount() const;

	virtual std::string getMethodName(uint32_t methodId) const;

	virtual script::Any invoke(const InvokeParam& param, uint32_t methodId, uint32_t argc, const script::Any* argv) const;

	virtual uint32_t getStaticMethodCount() const;

	virtual std::string getStaticMethodName(uint32_t methodId) const;

	virtual script::Any invokeStatic(uint32_t methodId, uint32_t argc, const script::Any* argv) const;

	virtual script::Any invokeUnknown(const InvokeParam& param, const std::string& methodName, uint32_t argc, const script::Any* argv) const;

	virtual script::Any invokeOperator(const InvokeParam& param, uint8_t operation, const script::Any& arg) const;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.ActionObjectClass", ActionObjectClass, script::IScriptClass)

const TypeInfo& ActionObjectClass::getExportType() const
{
	return type_of< flash::ActionObject >();
}

bool ActionObjectClass::haveConstructor() const
{
	return false;
}

bool ActionObjectClass::haveUnknown() const
{
	return true;
}

Ref< ITypedObject > ActionObjectClass::construct(const InvokeParam& param, uint32_t argc, const script::Any* argv) const
{
	return 0;
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

script::Any ActionObjectClass::invoke(const InvokeParam& param, uint32_t methodId, uint32_t argc, const script::Any* argv) const
{
	flash::ActionObject* object = checked_type_cast< flash::ActionObject*, false >(param.object);
	switch (methodId)
	{
	case 0:
		{
			flash::ActionValue memberValue;
			if (object->getMember(argv[0].getString(), memberValue))
				return script::CastAny< flash::ActionValue >::set(memberValue);
		}
		break;

	case 1:
		{
			flash::ActionValue memberValue;
			if (object->getMemberByQName(argv[0].getString(), memberValue))
				return script::CastAny< flash::ActionValue >::set(memberValue);
		}
		break;

	case 2:
		{
			flash::ActionValue memberValue = script::CastAny< flash::ActionValue >::get(argv[1]);
			object->setMember(argv[0].getString(), memberValue);
		}
		break;

	case 3:
		{
			Ref< flash::ActionFunction > propertyGetFn;
			if (object->getPropertyGet(argv[0].getString(), propertyGetFn))
			{
				flash::ActionValue propertyValue = propertyGetFn->call(object);
				return script::CastAny< flash::ActionValue >::set(propertyValue);
			}
		}
		break;

	case 4:
		{
			Ref< flash::ActionFunction > propertySetFn;
			if (object->getPropertySet(argv[0].getString(), propertySetFn))
			{
				flash::ActionValueArray callArgv(object->getContext()->getPool(), 1);
				callArgv[0] = script::CastAny< flash::ActionValue >::get(argv[1]);
				propertySetFn->call(object, callArgv);
			}
		}
		break;

	default:
		break;
	}
	return script::Any();
}

uint32_t ActionObjectClass::getStaticMethodCount() const
{
	return 0;
}

std::string ActionObjectClass::getStaticMethodName(uint32_t methodId) const
{
	return "";
}

script::Any ActionObjectClass::invokeStatic(uint32_t methodId, uint32_t argc, const script::Any* argv) const
{
	return script::Any();
}

script::Any ActionObjectClass::invokeUnknown(const InvokeParam& param, const std::string& methodName, uint32_t argc, const script::Any* argv) const
{
	flash::ActionObject* object = checked_type_cast< flash::ActionObject*, false >(param.object);
	T_ASSERT (object);

	flash::ActionValuePool& pool = object->getContext()->getPool();
	T_ANONYMOUS_VAR(flash::ActionValuePool::Scope)(pool);

	flash::ActionValueArray args(pool, argc);
	for (uint32_t i = 0; i < argc; ++i)
		args[i] = script::CastAny< flash::ActionValue >::get(argv[i]);

	flash::ActionValue functionValue;
	object->getMember(methodName, functionValue);

	flash::ActionFunction* fn = functionValue.getObject< flash::ActionFunction >();
	if (fn)
	{
		flash::ActionValue ret = fn->call(object, args);
		return script::CastAny< flash::ActionValue >::set(ret);
	}

	return script::Any();
}

script::Any ActionObjectClass::invokeOperator(const InvokeParam& param, uint8_t operation, const script::Any& arg) const
{
	return script::Any();
}

class ActionFunctionClass : public ActionObjectClass
{
	T_RTTI_CLASS;

public:
	virtual const TypeInfo& getExportType() const;

	virtual uint32_t getMethodCount() const;

	virtual std::string getMethodName(uint32_t methodId) const;

	virtual script::Any invoke(const InvokeParam& param, uint32_t methodId, uint32_t argc, const script::Any* argv) const;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.ActionFunctionClass", ActionFunctionClass, ActionObjectClass)

const TypeInfo& ActionFunctionClass::getExportType() const
{
	return type_of< flash::ActionFunction >();
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

script::Any ActionFunctionClass::invoke(const InvokeParam& param, uint32_t methodId, uint32_t argc, const script::Any* argv) const
{
	flash::ActionFunction* object = checked_type_cast< flash::ActionFunction*, false >(param.object);
	switch (methodId)
	{
	case 0:
		{
			flash::ActionValue memberValue;
			if (object->getMember(argv[0].getString(), memberValue))
				return script::CastAny< flash::ActionValue >::set(memberValue);
		}
		break;

	case 1:
		{
			flash::ActionValue memberValue;
			if (object->getMemberByQName(argv[0].getString(), memberValue))
				return script::CastAny< flash::ActionValue >::set(memberValue);
		}
		break;

	case 2:
		{
			flash::ActionValue memberValue = script::CastAny< flash::ActionValue >::get(argv[1]);
			object->setMember(argv[0].getString(), memberValue);
		}
		break;

	case 3:
		{
			Ref< flash::ActionFunction > propertyGetFn;
			if (object->getPropertyGet(argv[0].getString(), propertyGetFn))
			{
				flash::ActionValue propertyValue = propertyGetFn->call(object);
				return script::CastAny< flash::ActionValue >::set(propertyValue);
			}
		}
		break;

	case 4:
		{
			Ref< flash::ActionFunction > propertySetFn;
			if (object->getPropertySet(argv[0].getString(), propertySetFn))
			{
				flash::ActionValueArray callArgv(object->getContext()->getPool(), 1);
				callArgv[0] = script::CastAny< flash::ActionValue >::get(argv[1]);
				propertySetFn->call(object, callArgv);
			}
		}
		break;

	case 5:
		{
			flash::ActionValuePool& pool = object->getContext()->getPool();
			T_ANONYMOUS_VAR(flash::ActionValuePool::Scope)(pool);

			flash::ActionValueArray args(pool, argc - 1);
			for (uint32_t i = 0; i < argc - 1; ++i)
				args[i] = script::CastAny< flash::ActionValue >::get(argv[i + 1]);

			flash::ActionValue ret = object->call(
				checked_type_cast< flash::ActionObject*, true >(argv[0].getObject()),
				args
				);

			return script::CastAny< flash::ActionValue >::set(ret);
		}
		break;

	default:
		break;
	}
	return script::Any();
}

void registerFlashClasses(script::IScriptManager* scriptManager)
{
	Ref< script::AutoScriptClass< flash::FlashMoviePlayer > > classFlashMoviePlayer = new script::AutoScriptClass< flash::FlashMoviePlayer >();
	classFlashMoviePlayer->addMethod< void, uint32_t >("gotoAndPlay", &flash::FlashMoviePlayer::gotoAndPlay);
	classFlashMoviePlayer->addMethod< void, uint32_t >("gotoAndStop", &flash::FlashMoviePlayer::gotoAndStop);
	classFlashMoviePlayer->addMethod< bool, const std::string& >("gotoAndPlay", &flash::FlashMoviePlayer::gotoAndPlay);
	classFlashMoviePlayer->addMethod< bool, const std::string& >("gotoAndStop", &flash::FlashMoviePlayer::gotoAndStop);
	classFlashMoviePlayer->addMethod("getFrameCount", &flash::FlashMoviePlayer::getFrameCount);
	classFlashMoviePlayer->addMethod("postKeyDown", &flash::FlashMoviePlayer::postKeyDown);
	classFlashMoviePlayer->addMethod("postKeyUp", &flash::FlashMoviePlayer::postKeyUp);
	classFlashMoviePlayer->addMethod("postMouseDown", &flash::FlashMoviePlayer::postMouseDown);
	classFlashMoviePlayer->addMethod("postMouseUp", &flash::FlashMoviePlayer::postMouseUp);
	classFlashMoviePlayer->addMethod("postMouseMove", &flash::FlashMoviePlayer::postMouseMove);
	classFlashMoviePlayer->addMethod("setGlobal", &flash::FlashMoviePlayer::setGlobal);
	classFlashMoviePlayer->addMethod("getGlobal", &flash::FlashMoviePlayer::getGlobal);
	scriptManager->registerClass(classFlashMoviePlayer);

	Ref< script::AutoScriptClass< FlashLayer > > classFlashLayer = new script::AutoScriptClass< FlashLayer >();
	classFlashLayer->addMethod("getMoviePlayer", &FlashLayer::getMoviePlayer);
	classFlashLayer->addMethod("getGlobal", &FlashLayer::getGlobal);
	classFlashLayer->addMethod("getRoot", &FlashLayer::getRoot);
	classFlashLayer->addMethod("createObject", &FlashLayer_createObject_0);
	classFlashLayer->addVariadicMethod("createObject", &FlashLayer_createObject_1);
	classFlashLayer->addMethod("createBitmap", &FlashLayer::createBitmap);
	classFlashLayer->addMethod("isVisible", &FlashLayer::isVisible);
	classFlashLayer->addMethod("setVisible", &FlashLayer::setVisible);
	classFlashLayer->setUnknownMethod(&FlashLayer::externalCall);
	scriptManager->registerClass(classFlashLayer);

	Ref< ActionObjectClass > classActionObject = new ActionObjectClass();
	scriptManager->registerClass(classActionObject);

	Ref< ActionFunctionClass > classActionFunction = new ActionFunctionClass();
	scriptManager->registerClass(classActionFunction);
}

	}
}

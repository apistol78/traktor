#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Drawing/Image.h"
#include "Flash/FlashCast.h"
#include "Flash/FlashClassFactory.h"
#include "Flash/FlashMoviePlayer.h"
#include "Flash/FlashSpriteInstance.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFunction.h"
#include "Flash/Action/ActionValue.h"

namespace traktor
{
	namespace flash
	{

class ActionObjectClass : public IRuntimeClass
{
	T_RTTI_CLASS;

public:
	virtual const TypeInfo& getExportType() const;

	virtual bool haveConstructor() const;

	virtual bool haveUnknown() const;

	virtual Ref< ITypedObject > construct(const InvokeParam& param, uint32_t argc, const Any* argv) const;

	virtual uint32_t getMethodCount() const;

	virtual std::string getMethodName(uint32_t methodId) const;

	virtual Any invoke(const InvokeParam& param, uint32_t methodId, uint32_t argc, const Any* argv) const;

	virtual uint32_t getStaticMethodCount() const;

	virtual std::string getStaticMethodName(uint32_t methodId) const;

	virtual Any invokeStatic(uint32_t methodId, uint32_t argc, const Any* argv) const;

	virtual Any invokeUnknown(const InvokeParam& param, const std::string& methodName, uint32_t argc, const Any* argv) const;

	virtual Any invokeOperator(const InvokeParam& param, uint8_t operation, const Any& arg) const;
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

Ref< ITypedObject > ActionObjectClass::construct(const InvokeParam& param, uint32_t argc, const Any* argv) const
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

Any ActionObjectClass::invoke(const InvokeParam& param, uint32_t methodId, uint32_t argc, const Any* argv) const
{
	ActionObject* object = checked_type_cast< ActionObject*, false >(param.object);
	switch (methodId)
	{
	case 0:
		{
			ActionValue memberValue;
			if (object->getMember(argv[0].getString(), memberValue))
				return CastAny< ActionValue >::set(memberValue);
		}
		break;

	case 1:
		{
			ActionValue memberValue;
			if (object->getMemberByQName(argv[0].getString(), memberValue))
				return CastAny< ActionValue >::set(memberValue);
		}
		break;

	case 2:
		{
			ActionValue memberValue = CastAny< ActionValue >::get(argv[1]);
			object->setMember(argv[0].getString(), memberValue);
		}
		break;

	case 3:
		{
			Ref< ActionFunction > propertyGetFn;
			if (object->getPropertyGet(argv[0].getString(), propertyGetFn))
			{
				ActionValue propertyValue = propertyGetFn->call(object);
				return CastAny< ActionValue >::set(propertyValue);
			}
		}
		break;

	case 4:
		{
			Ref< ActionFunction > propertySetFn;
			if (object->getPropertySet(argv[0].getString(), propertySetFn))
			{
				ActionValueArray callArgv(object->getContext()->getPool(), 1);
				callArgv[0] = CastAny< ActionValue >::get(argv[1]);
				propertySetFn->call(object, callArgv);
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

Any ActionObjectClass::invokeStatic(uint32_t methodId, uint32_t argc, const Any* argv) const
{
	return Any();
}

Any ActionObjectClass::invokeUnknown(const InvokeParam& param, const std::string& methodName, uint32_t argc, const Any* argv) const
{
	ActionObject* object = checked_type_cast< ActionObject*, false >(param.object);
	T_ASSERT (object);

	ActionValuePool& pool = object->getContext()->getPool();
	T_ANONYMOUS_VAR(ActionValuePool::Scope)(pool);

	ActionValueArray args(pool, argc);
	for (uint32_t i = 0; i < argc; ++i)
		args[i] = CastAny< ActionValue >::get(argv[i]);

	ActionValue functionValue;
	object->getMember(methodName, functionValue);

	ActionFunction* fn = functionValue.getObject< ActionFunction >();
	if (fn)
	{
		ActionValue ret = fn->call(object, args);
		return CastAny< ActionValue >::set(ret);
	}

	return Any();
}

Any ActionObjectClass::invokeOperator(const InvokeParam& param, uint8_t operation, const Any& arg) const
{
	return Any();
}

class ActionFunctionClass : public ActionObjectClass
{
	T_RTTI_CLASS;

public:
	virtual const TypeInfo& getExportType() const;

	virtual uint32_t getMethodCount() const;

	virtual std::string getMethodName(uint32_t methodId) const;

	virtual Any invoke(const InvokeParam& param, uint32_t methodId, uint32_t argc, const Any* argv) const;
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

Any ActionFunctionClass::invoke(const InvokeParam& param, uint32_t methodId, uint32_t argc, const Any* argv) const
{
	ActionFunction* object = checked_type_cast< ActionFunction*, false >(param.object);
	switch (methodId)
	{
	case 0:
		{
			ActionValue memberValue;
			if (object->getMember(argv[0].getString(), memberValue))
				return CastAny< ActionValue >::set(memberValue);
		}
		break;

	case 1:
		{
			ActionValue memberValue;
			if (object->getMemberByQName(argv[0].getString(), memberValue))
				return CastAny< ActionValue >::set(memberValue);
		}
		break;

	case 2:
		{
			ActionValue memberValue = CastAny< ActionValue >::get(argv[1]);
			object->setMember(argv[0].getString(), memberValue);
		}
		break;

	case 3:
		{
			Ref< ActionFunction > propertyGetFn;
			if (object->getPropertyGet(argv[0].getString(), propertyGetFn))
			{
				ActionValue propertyValue = propertyGetFn->call(object);
				return CastAny< ActionValue >::set(propertyValue);
			}
		}
		break;

	case 4:
		{
			Ref< ActionFunction > propertySetFn;
			if (object->getPropertySet(argv[0].getString(), propertySetFn))
			{
				ActionValueArray callArgv(object->getContext()->getPool(), 1);
				callArgv[0] = CastAny< ActionValue >::get(argv[1]);
				propertySetFn->call(object, callArgv);
			}
		}
		break;

	case 5:
		{
			ActionValuePool& pool = object->getContext()->getPool();
			T_ANONYMOUS_VAR(ActionValuePool::Scope)(pool);

			ActionValueArray args(pool, argc - 1);
			for (uint32_t i = 0; i < argc - 1; ++i)
				args[i] = CastAny< ActionValue >::get(argv[i + 1]);

			ActionValue ret = object->call(
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
	classFlashMoviePlayer->addMethod("setGlobal", &FlashMoviePlayer::setGlobal);
	classFlashMoviePlayer->addMethod("getGlobal", &FlashMoviePlayer::getGlobal);
	registrar->registerClass(classFlashMoviePlayer);

	Ref< ActionObjectClass > classActionObject = new ActionObjectClass();
	registrar->registerClass(classActionObject);

	Ref< ActionFunctionClass > classActionFunction = new ActionFunctionClass();
	registrar->registerClass(classActionFunction);
}

	}
}

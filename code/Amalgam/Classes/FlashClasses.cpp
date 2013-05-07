#include "Amalgam/FlashCast.h"
#include "Amalgam/FlashLayer.h"
#include "Amalgam/Classes/FlashClasses.h"
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

	virtual Ref< Object > construct(const InvokeParam& param, uint32_t argc, const script::Any* argv) const;

	virtual uint32_t getMethodCount() const;

	virtual std::string getMethodName(uint32_t methodId) const;

	virtual script::Any invoke(const InvokeParam& param, uint32_t methodId, uint32_t argc, const script::Any* argv) const;

	virtual uint32_t getStaticMethodCount() const;

	virtual std::string getStaticMethodName(uint32_t methodId) const;

	virtual script::Any invokeStatic(uint32_t methodId, uint32_t argc, const script::Any* argv) const;

	virtual script::Any invokeUnknown(const InvokeParam& param, const std::string& methodName, uint32_t argc, const script::Any* argv) const;

	virtual uint32_t getPropertyCount() const;

	virtual std::string getPropertyName(uint32_t propertyId) const;

	virtual script::Any getPropertyValue(const InvokeParam& param, uint32_t propertyId) const;

	virtual void setPropertyValue(const InvokeParam& param, uint32_t propertyId, const script::Any& value) const;
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

Ref< Object > ActionObjectClass::construct(const InvokeParam& param, uint32_t argc, const script::Any* argv) const
{
	return 0;
}

uint32_t ActionObjectClass::getMethodCount() const
{
	return 3;
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

uint32_t ActionObjectClass::getPropertyCount() const
{
	return 0;
}

std::string ActionObjectClass::getPropertyName(uint32_t propertyId) const
{
	return "";
}

script::Any ActionObjectClass::getPropertyValue(const InvokeParam& param, uint32_t propertyId) const
{
	return script::Any();
}

void ActionObjectClass::setPropertyValue(const InvokeParam& param, uint32_t propertyId, const script::Any& value) const
{
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
}

	}
}

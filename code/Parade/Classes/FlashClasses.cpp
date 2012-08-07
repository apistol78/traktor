#include "Flash/FlashMoviePlayer.h"
#include "Flash/FlashSpriteInstance.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFunction.h"
#include "Flash/Action/ActionValue.h"
#include "Parade/FlashLayer.h"
#include "Parade/Classes/FlashClasses.h"
#include "Script/AutoScriptClass.h"
#include "Script/IScriptManager.h"

namespace traktor
{
	namespace script
	{

template < >
struct CastAny < flash::ActionValue, false >
{
	static Any set(const flash::ActionValue& value)
	{
		switch (value.getType())
		{
		case flash::ActionValue::AvtBoolean:
			return Any(value.getBoolean());
		case flash::ActionValue::AvtNumber:
			return Any(float(value.getNumber()));
		case flash::ActionValue::AvtString:
			return Any(value.getWideString());
		case flash::ActionValue::AvtObject:
			return Any(value.getObject());
		default:
			return Any();
		}
	}

	static flash::ActionValue get(const Any& value)
	{
		if (value.isBoolean())
			return flash::ActionValue(value.getBoolean());
		else if (value.isInteger())
			return flash::ActionValue((flash::avm_number_t)value.getInteger());
		else if (value.isFloat())
			return flash::ActionValue(value.getFloat());
		else if (value.isString())
			return flash::ActionValue(value.getString());
		else if (value.isObject())
			return flash::ActionValue(dynamic_type_cast< flash::ActionObject* >(value.getObject()));
		else
			return flash::ActionValue();
	}
};

template < >
struct CastAny < const flash::ActionValue&, false >
{
	static Any set(const flash::ActionValue& value)
	{
		switch (value.getType())
		{
		case flash::ActionValue::AvtBoolean:
			return Any(value.getBoolean());
		case flash::ActionValue::AvtNumber:
			return Any(float(value.getNumber()));
		case flash::ActionValue::AvtString:
			return Any(value.getWideString());
		case flash::ActionValue::AvtObject:
			return Any(value.getObject());
		default:
			return Any();
		}
	}

	static flash::ActionValue get(const Any& value)
	{
		if (value.isBoolean())
			return flash::ActionValue(value.getBoolean());
		else if (value.isInteger())
			return flash::ActionValue((flash::avm_number_t)value.getInteger());
		else if (value.isFloat())
			return flash::ActionValue(value.getFloat());
		else if (value.isString())
			return flash::ActionValue(value.getString());
		else if (value.isObject())
			return flash::ActionValue(dynamic_type_cast< flash::ActionObject* >(value.getObject()));
		else
			return flash::ActionValue();
	}
};

	}

	namespace parade
	{

class ActionObjectClass : public script::IScriptClass
{
	T_RTTI_CLASS;

public:
	virtual const TypeInfo& getExportType() const;

	virtual bool haveConstructor() const;

	virtual bool haveUnknown() const;

	virtual Ref< Object > construct(const InvokeParam& param, uint32_t argc, const script::Any* argv) const;

	virtual uint32_t getMethodCount() const;

	virtual std::wstring getMethodName(uint32_t methodId) const;

	virtual script::Any invoke(const InvokeParam& param, uint32_t methodId, uint32_t argc, const script::Any* argv) const;

	virtual script::Any invokeUnknown(const InvokeParam& param, const std::wstring& methodName, uint32_t argc, const script::Any* argv) const;

	virtual uint32_t getPropertyCount() const;

	virtual std::wstring getPropertyName(uint32_t propertyId) const;

	virtual script::Any getPropertyValue(const InvokeParam& param, uint32_t propertyId) const;

	virtual void setPropertyValue(const InvokeParam& param, uint32_t propertyId, const script::Any& value) const;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.parade.ActionObjectClass", ActionObjectClass, script::IScriptClass)

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
	return 0;
}

std::wstring ActionObjectClass::getMethodName(uint32_t methodId) const
{
	return L"";
}

script::Any ActionObjectClass::invoke(const InvokeParam& param, uint32_t methodId, uint32_t argc, const script::Any* argv) const
{
	return script::Any();
}

script::Any ActionObjectClass::invokeUnknown(const InvokeParam& param, const std::wstring& methodName, uint32_t argc, const script::Any* argv) const
{
	flash::ActionObject* object = checked_type_cast< flash::ActionObject*, false >(param.object);
	flash::ActionValuePool& pool = object->getContext()->getPool();

	flash::ActionValueArray args(pool, argc);
	for (uint32_t i = 0; i < argc; ++i)
		args[i] = script::CastAny< flash::ActionValue >::get(argv[i]);

	flash::ActionValue functionValue;
	object->getMember(wstombs(methodName), functionValue);

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

std::wstring ActionObjectClass::getPropertyName(uint32_t propertyId) const
{
	return L"";
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
	classFlashMoviePlayer->addMethod< void, uint32_t >(L"gotoAndPlay", &flash::FlashMoviePlayer::gotoAndPlay);
	classFlashMoviePlayer->addMethod< void, uint32_t >(L"gotoAndStop", &flash::FlashMoviePlayer::gotoAndStop);
	classFlashMoviePlayer->addMethod< bool, const std::string& >(L"gotoAndPlay", &flash::FlashMoviePlayer::gotoAndPlay);
	classFlashMoviePlayer->addMethod< bool, const std::string& >(L"gotoAndStop", &flash::FlashMoviePlayer::gotoAndStop);
	classFlashMoviePlayer->addMethod(L"getFrameCount", &flash::FlashMoviePlayer::getFrameCount);
	classFlashMoviePlayer->addMethod(L"postKeyDown", &flash::FlashMoviePlayer::postKeyDown);
	classFlashMoviePlayer->addMethod(L"postKeyUp", &flash::FlashMoviePlayer::postKeyUp);
	classFlashMoviePlayer->addMethod(L"postMouseDown", &flash::FlashMoviePlayer::postMouseDown);
	classFlashMoviePlayer->addMethod(L"postMouseUp", &flash::FlashMoviePlayer::postMouseUp);
	classFlashMoviePlayer->addMethod(L"postMouseMove", &flash::FlashMoviePlayer::postMouseMove);
	classFlashMoviePlayer->addMethod(L"setGlobal", &flash::FlashMoviePlayer::setGlobal);
	classFlashMoviePlayer->addMethod(L"getGlobal", &flash::FlashMoviePlayer::getGlobal);
	scriptManager->registerClass(classFlashMoviePlayer);

	Ref< script::AutoScriptClass< FlashLayer > > classFlashLayer = new script::AutoScriptClass< FlashLayer >();
	classFlashLayer->addMethod(L"getMoviePlayer", &FlashLayer::getMoviePlayer);
	classFlashLayer->addMethod(L"getGlobal", &FlashLayer::getGlobal);
	classFlashLayer->addMethod(L"getRoot", &FlashLayer::getRoot);
	classFlashLayer->addMethod(L"isVisible", &FlashLayer::isVisible);
	classFlashLayer->addMethod(L"setVisible", &FlashLayer::setVisible);
	scriptManager->registerClass(classFlashLayer);

	Ref< ActionObjectClass > classActionObject = new ActionObjectClass();
	scriptManager->registerClass(classActionObject);
}

	}
}

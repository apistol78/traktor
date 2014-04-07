#include "Flash/FlashCharacterInstance.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Avm1/Classes/As_flash_external_ExternalInterface.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.As_flash_external_ExternalInterface", As_flash_external_ExternalInterface, ActionObject)

As_flash_external_ExternalInterface::As_flash_external_ExternalInterface(ActionContext* context)
:	ActionObject(context)
,	m_externalCall(0)
{
	addProperty("available", createNativeFunction(context, this, &As_flash_external_ExternalInterface::ExternalInterface_get_available), 0);

	setMember("addCallback", ActionValue(createNativeFunction(context, this, &As_flash_external_ExternalInterface::ExternalInterface_addCallback)));
	setMember("call", ActionValue(createNativeFunction(context, this, &As_flash_external_ExternalInterface::ExternalInterface_call)));
}

void As_flash_external_ExternalInterface::setExternalCall(IExternalCall* externalCall)
{
	m_externalCall = externalCall;
}

ActionValue As_flash_external_ExternalInterface::dispatchCallback(const std::string& methodName, int32_t argc, const ActionValue* argv)
{
	std::map< std::string, Callback >::const_iterator i = m_callbacks.find(methodName);
	if (i != m_callbacks.end())
	{
		ActionValueArray args(getContext()->getPool(), argc);
		for (int32_t j = 0; j < argc; ++j)
			args[j] = argv[j];

		return i->second.method->call(
			i->second.instance,
			args
		);
	}
	else
		return ActionValue();
}

void As_flash_external_ExternalInterface::trace(const IVisitor& visitor) const
{
	for (std::map< std::string, Callback >::const_iterator i = m_callbacks.begin(); i != m_callbacks.end(); ++i)
	{
		if (i->second.instance)
			visitor(i->second.instance);
		if (i->second.method)
			visitor(i->second.method);
	}
	ActionObject::trace(visitor);
}

void As_flash_external_ExternalInterface::dereference()
{
	m_callbacks.clear();
	ActionObject::dereference();
}

void As_flash_external_ExternalInterface::ExternalInterface_get_available(CallArgs& ca)
{
	ca.ret = ActionValue(true);
}

void As_flash_external_ExternalInterface::ExternalInterface_addCallback(CallArgs& ca)
{
	if (ca.args.size() < 3)
		return;

	std::string methodName = ca.args[0].getString();
	Ref< ActionObject > instance = ca.args[1].getObject();
	Ref< ActionFunction > method = ca.args[2].getObject< ActionFunction >();

	Callback& callback = m_callbacks[methodName];
	callback.instance = instance;
	callback.method = method;
}

void As_flash_external_ExternalInterface::ExternalInterface_call(CallArgs& ca)
{
	if (!m_externalCall)
		return;

	ActionValue argv[16];
	for (uint32_t i = 0; i < ca.args.size() - 1; ++i)
		argv[i] = ca.args[i + 1];

	ca.ret = m_externalCall->dispatchExternalCall(
		ca.args[0].getString(),
		ca.args.size() - 1,
		argv
	);
}

	}
}

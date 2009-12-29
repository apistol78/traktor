#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Avm1/Classes/AsXMLNode.h"
#include "Flash/Action/Avm1/Classes/AsObject.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsXMLNode", AsXMLNode, ActionClass)

Ref< AsXMLNode > AsXMLNode::getInstance()
{
	static Ref< AsXMLNode > instance = 0;
	if (!instance)
	{
		instance = new AsXMLNode();
		instance->createPrototype();
		instance->setReadOnly();
	}
	return instance;
}

AsXMLNode::AsXMLNode()
:	ActionClass(L"XMLNode")
{
}

void AsXMLNode::createPrototype()
{
	Ref< ActionObject > prototype = new ActionObject();

	prototype->setMember(L"__proto__", ActionValue::fromObject(AsObject::getInstance()));
	prototype->setMember(L"appendChild", createNativeFunctionValue(this, &AsXMLNode::XMLNode_appendChild));
	prototype->setMember(L"cloneNode", createNativeFunctionValue(this, &AsXMLNode::XMLNode_cloneNode));
	prototype->setMember(L"getNamespaceForPrefix", createNativeFunctionValue(this, &AsXMLNode::XMLNode_getNamespaceForPrefix));
	prototype->setMember(L"getPrefixForNamespace", createNativeFunctionValue(this, &AsXMLNode::XMLNode_getPrefixForNamespace));
	prototype->setMember(L"hasChildNodes", createNativeFunctionValue(this, &AsXMLNode::XMLNode_hasChildNodes));
	prototype->setMember(L"insertBefore", createNativeFunctionValue(this, &AsXMLNode::XMLNode_insertBefore));
	prototype->setMember(L"removeNode", createNativeFunctionValue(this, &AsXMLNode::XMLNode_removeNode));
	prototype->setMember(L"toString", createNativeFunctionValue(this, &AsXMLNode::XMLNode_toString));

	prototype->setReadOnly();

	setMember(L"prototype", ActionValue::fromObject(prototype));
}

ActionValue AsXMLNode::construct(ActionContext* context, const args_t& args)
{
	return ActionValue::fromObject(new ActionObject(this));
}

void AsXMLNode::XMLNode_appendChild(CallArgs& ca)
{
}

void AsXMLNode::XMLNode_cloneNode(CallArgs& ca)
{
}

void AsXMLNode::XMLNode_getNamespaceForPrefix(CallArgs& ca)
{
}

void AsXMLNode::XMLNode_getPrefixForNamespace(CallArgs& ca)
{
}

void AsXMLNode::XMLNode_hasChildNodes(CallArgs& ca)
{
}

void AsXMLNode::XMLNode_insertBefore(CallArgs& ca)
{
}

void AsXMLNode::XMLNode_removeNode(CallArgs& ca)
{
}

void AsXMLNode::XMLNode_toString(CallArgs& ca)
{
}

	}
}

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

	prototype->setMember(L"__proto__", ActionValue(AsObject::getInstance()));
	prototype->setMember(L"appendChild", ActionValue(createNativeFunction(this, &AsXMLNode::XMLNode_appendChild)));
	prototype->setMember(L"cloneNode", ActionValue(createNativeFunction(this, &AsXMLNode::XMLNode_cloneNode)));
	prototype->setMember(L"getNamespaceForPrefix", ActionValue(createNativeFunction(this, &AsXMLNode::XMLNode_getNamespaceForPrefix)));
	prototype->setMember(L"getPrefixForNamespace", ActionValue(createNativeFunction(this, &AsXMLNode::XMLNode_getPrefixForNamespace)));
	prototype->setMember(L"hasChildNodes", ActionValue(createNativeFunction(this, &AsXMLNode::XMLNode_hasChildNodes)));
	prototype->setMember(L"insertBefore", ActionValue(createNativeFunction(this, &AsXMLNode::XMLNode_insertBefore)));
	prototype->setMember(L"removeNode", ActionValue(createNativeFunction(this, &AsXMLNode::XMLNode_removeNode)));
	prototype->setMember(L"toString", ActionValue(createNativeFunction(this, &AsXMLNode::XMLNode_toString)));

	prototype->setReadOnly();

	setMember(L"prototype", ActionValue(prototype));
}

ActionValue AsXMLNode::construct(ActionContext* context, const args_t& args)
{
	return ActionValue(new ActionObject(this));
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

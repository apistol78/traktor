#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Avm1/Classes/AsXMLNode.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsXMLNode", AsXMLNode, ActionClass)

AsXMLNode::AsXMLNode()
:	ActionClass("XMLNode")
{
	Ref< ActionObject > prototype = new ActionObject();

	prototype->setMember("appendChild", ActionValue(createNativeFunction(this, &AsXMLNode::XMLNode_appendChild)));
	prototype->setMember("cloneNode", ActionValue(createNativeFunction(this, &AsXMLNode::XMLNode_cloneNode)));
	prototype->setMember("getNamespaceForPrefix", ActionValue(createNativeFunction(this, &AsXMLNode::XMLNode_getNamespaceForPrefix)));
	prototype->setMember("getPrefixForNamespace", ActionValue(createNativeFunction(this, &AsXMLNode::XMLNode_getPrefixForNamespace)));
	prototype->setMember("hasChildNodes", ActionValue(createNativeFunction(this, &AsXMLNode::XMLNode_hasChildNodes)));
	prototype->setMember("insertBefore", ActionValue(createNativeFunction(this, &AsXMLNode::XMLNode_insertBefore)));
	prototype->setMember("removeNode", ActionValue(createNativeFunction(this, &AsXMLNode::XMLNode_removeNode)));
	prototype->setMember("toString", ActionValue(createNativeFunction(this, &AsXMLNode::XMLNode_toString)));

	prototype->setMember("constructor", ActionValue(this));
	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

Ref< ActionObject > AsXMLNode::alloc(ActionContext* context)
{
	return new ActionObject("XMLNode");
}

void AsXMLNode::init(ActionContext* context, ActionObject* self, const ActionValueArray& args)
{
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

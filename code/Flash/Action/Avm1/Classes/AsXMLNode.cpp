#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Avm1/Classes/AsXMLNode.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsXMLNode", AsXMLNode, ActionClass)

AsXMLNode::AsXMLNode(ActionContext* context)
:	ActionClass(context, "XMLNode")
{
	Ref< ActionObject > prototype = new ActionObject(context);

	prototype->setMember("appendChild", ActionValue(createNativeFunction(context, this, &AsXMLNode::XMLNode_appendChild)));
	prototype->setMember("cloneNode", ActionValue(createNativeFunction(context, this, &AsXMLNode::XMLNode_cloneNode)));
	prototype->setMember("getNamespaceForPrefix", ActionValue(createNativeFunction(context, this, &AsXMLNode::XMLNode_getNamespaceForPrefix)));
	prototype->setMember("getPrefixForNamespace", ActionValue(createNativeFunction(context, this, &AsXMLNode::XMLNode_getPrefixForNamespace)));
	prototype->setMember("hasChildNodes", ActionValue(createNativeFunction(context, this, &AsXMLNode::XMLNode_hasChildNodes)));
	prototype->setMember("insertBefore", ActionValue(createNativeFunction(context, this, &AsXMLNode::XMLNode_insertBefore)));
	prototype->setMember("removeNode", ActionValue(createNativeFunction(context, this, &AsXMLNode::XMLNode_removeNode)));
	prototype->setMember("toString", ActionValue(createNativeFunction(context, this, &AsXMLNode::XMLNode_toString)));

	prototype->setMember("constructor", ActionValue(this));
	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

void AsXMLNode::init(ActionObject* self, const ActionValueArray& args)
{
}

void AsXMLNode::coerce(ActionObject* self) const
{
	T_FATAL_ERROR;
}

void AsXMLNode::XMLNode_appendChild(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"XMLNode::appendChild not implemented" << Endl;
	)
}

void AsXMLNode::XMLNode_cloneNode(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"XMLNode::cloneNode not implemented" << Endl;
	)
}

void AsXMLNode::XMLNode_getNamespaceForPrefix(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"XMLNode::getNamespaceForPrefix not implemented" << Endl;
	)
}

void AsXMLNode::XMLNode_getPrefixForNamespace(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"XMLNode::getPrefixForNamespace not implemented" << Endl;
	)
}

void AsXMLNode::XMLNode_hasChildNodes(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"XMLNode::hasChildNodes not implemented" << Endl;
	)
}

void AsXMLNode::XMLNode_insertBefore(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"XMLNode::insertBefore not implemented" << Endl;
	)
}

void AsXMLNode::XMLNode_removeNode(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"XMLNode::removeNode not implemented" << Endl;
	)
}

void AsXMLNode::XMLNode_toString(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"XMLNode::toString not implemented" << Endl;
	)
}

	}
}

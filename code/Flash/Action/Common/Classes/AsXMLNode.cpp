/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Common/Classes/AsXMLNode.h"
#include "Flash/Action/Common/Array.h"
#include "Flash/Action/Common/XMLNode.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsXMLNode", AsXMLNode, ActionClass)

AsXMLNode::AsXMLNode(ActionContext* context)
:	ActionClass(context, "XMLNode")
{
	Ref< ActionObject > prototype = new ActionObject(context);

	prototype->addProperty("attributes", createNativeFunction(context, this, &AsXMLNode::XMLNode_get_attributes), createNativeFunction(context, this, &AsXMLNode::XMLNode_set_attributes));
	prototype->addProperty("childNodes", createNativeFunction(context, this, &AsXMLNode::XMLNode_get_childNodes), 0);
	prototype->addProperty("firstChild", createNativeFunction(context, this, &AsXMLNode::XMLNode_get_firstChild), createNativeFunction(context, this, &AsXMLNode::XMLNode_set_firstChild));
	prototype->addProperty("lastChild", createNativeFunction(context, this, &AsXMLNode::XMLNode_get_lastChild), createNativeFunction(context, this, &AsXMLNode::XMLNode_set_lastChild));
	prototype->addProperty("localName", createNativeFunction(context, this, &AsXMLNode::XMLNode_get_localName), 0);
	prototype->addProperty("namespaceURI", createNativeFunction(context, this, &AsXMLNode::XMLNode_get_namespaceURI), 0);
	prototype->addProperty("nextSibling", createNativeFunction(context, this, &AsXMLNode::XMLNode_get_nextSibling), createNativeFunction(context, this, &AsXMLNode::XMLNode_set_nextSibling));
	prototype->addProperty("nodeName", createNativeFunction(context, this, &AsXMLNode::XMLNode_get_nodeName), createNativeFunction(context, this, &AsXMLNode::XMLNode_set_nodeName));
	prototype->addProperty("nodeType", createNativeFunction(context, this, &AsXMLNode::XMLNode_get_nodeType), createNativeFunction(context, this, &AsXMLNode::XMLNode_set_nodeType));
	prototype->addProperty("nodeValue", createNativeFunction(context, this, &AsXMLNode::XMLNode_get_nodeValue), createNativeFunction(context, this, &AsXMLNode::XMLNode_set_nodeValue));
	prototype->addProperty("parentNode", createNativeFunction(context, this, &AsXMLNode::XMLNode_get_parentNode), createNativeFunction(context, this, &AsXMLNode::XMLNode_set_parentNode));
	prototype->addProperty("prefix", createNativeFunction(context, this, &AsXMLNode::XMLNode_get_prefix), 0);
	prototype->addProperty("previousSibling", createNativeFunction(context, this, &AsXMLNode::XMLNode_get_previousSibling), createNativeFunction(context, this, &AsXMLNode::XMLNode_set_previousSibling));

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

void AsXMLNode::initialize(ActionObject* self)
{
}

void AsXMLNode::construct(ActionObject* self, const ActionValueArray& args)
{
	self->setRelay(new XMLNode("XMLNode", XMLNode::NtUnknown, L"", L""));
}

ActionValue AsXMLNode::xplicit(const ActionValueArray& args)
{
	return ActionValue();
}

ActionObject* AsXMLNode::XMLNode_get_attributes(XMLNode* self) const
{
	T_IF_VERBOSE(
		log::warning << L"XMLNode::attributes not implemented" << Endl;
	)
	return 0;
}

void AsXMLNode::XMLNode_set_attributes(XMLNode* self, ActionObject* attributes) const
{
	T_IF_VERBOSE(
		log::warning << L"XMLNode::attributes not implemented" << Endl;
	)
}

void AsXMLNode::XMLNode_get_childNodes(CallArgs& ca)
{
	XMLNode* self = ca.self->getRelay< XMLNode >();
	if (!self)
		return;

	Ref< Array > childNodes = new Array(16);
	for (XMLNode* child = self->getFirstChild(); child; child = child->getNextSibling())
		childNodes->push(ActionValue(child->getAsObject(ca.context)));

	ca.ret = ActionValue(childNodes->getAsObject(ca.context));
}

XMLNode* AsXMLNode::XMLNode_get_firstChild(XMLNode* self) const
{
	return self->getFirstChild();
}

void AsXMLNode::XMLNode_set_firstChild(XMLNode* self, XMLNode* firstChild) const
{
	T_IF_VERBOSE(
		log::warning << L"XMLNode::firstChild not implemented" << Endl;
	)
}

XMLNode* AsXMLNode::XMLNode_get_lastChild(XMLNode* self) const
{
	return self->getLastChild();
}

void AsXMLNode::XMLNode_set_lastChild(XMLNode* self, XMLNode* lastChild) const
{
	T_IF_VERBOSE(
		log::warning << L"XMLNode::lastChild not implemented" << Endl;
	)
}

std::wstring AsXMLNode::XMLNode_get_localName(XMLNode* self) const
{
	return self->getLocalName();
}

std::wstring AsXMLNode::XMLNode_get_namespaceURI(XMLNode* self) const
{
	return self->getNamespaceURI();
}

XMLNode* AsXMLNode::XMLNode_get_nextSibling(XMLNode* self) const
{
	return self->getNextSibling();
}

void AsXMLNode::XMLNode_set_nextSibling(XMLNode* self, XMLNode* nextSibling) const
{
	T_IF_VERBOSE(
		log::warning << L"XMLNode::nextSibling not implemented" << Endl;
	)
}

std::wstring AsXMLNode::XMLNode_get_nodeName(XMLNode* self) const
{
	return self->getNodeName();
}

void AsXMLNode::XMLNode_set_nodeName(XMLNode* self, const std::wstring& nodeName) const
{
	T_IF_VERBOSE(
		log::warning << L"XMLNode::nodeName not implemented" << Endl;
	)
}

float AsXMLNode::XMLNode_get_nodeType(XMLNode* self) const
{
	return float(self->getNodeType());
}

void AsXMLNode::XMLNode_set_nodeType(XMLNode* self, float nodeType) const
{
	T_IF_VERBOSE(
		log::warning << L"XMLNode::nodeType not implemented" << Endl;
	)
}

std::wstring AsXMLNode::XMLNode_get_nodeValue(XMLNode* self) const
{
	return self->getNodeValue();
}

void AsXMLNode::XMLNode_set_nodeValue(XMLNode* self, const std::wstring& nodeValue) const
{
	T_IF_VERBOSE(
		log::warning << L"XMLNode::nodeValue not implemented" << Endl;
	)
}

XMLNode* AsXMLNode::XMLNode_get_parentNode(XMLNode* self) const
{
	return self->getParentNode();
}

void AsXMLNode::XMLNode_set_parentNode(XMLNode* self, XMLNode* parentNode) const
{
	T_IF_VERBOSE(
		log::warning << L"XMLNode::parentNode not implemented" << Endl;
	)
}

std::wstring AsXMLNode::XMLNode_get_prefix(XMLNode* self) const
{
	return self->getPrefix();
}

XMLNode* AsXMLNode::XMLNode_get_previousSibling(XMLNode* self) const
{
	return self->getPreviousSibling();
}

void AsXMLNode::XMLNode_set_previousSibling(XMLNode* self, XMLNode* previousSibling) const
{
	T_IF_VERBOSE(
		log::warning << L"XMLNode::previousSibling not implemented" << Endl;
	)
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

bool AsXMLNode::XMLNode_hasChildNodes(XMLNode* self) const
{
	return self->getFirstChild() != 0;
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

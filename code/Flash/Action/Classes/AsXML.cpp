#include "Flash/Action/Classes/AsXML.h"
#include "Flash/Action/Classes/AsXMLNode.h"
#include "Flash/Action/ActionFunctionNative.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsXML", AsXML, ActionClass)

Ref< AsXML > AsXML::getInstance()
{
	static Ref< AsXML > instance = 0;
	if (!instance)
	{
		instance = new AsXML();
		instance->createPrototype();
		instance->setReadOnly();
	}
	return instance;
}

AsXML::AsXML()
:	ActionClass(L"XML")
{
}

void AsXML::createPrototype()
{
	Ref< ActionObject > prototype = new ActionObject();

	prototype->setMember(L"__proto__", ActionValue::fromObject(AsXMLNode::getInstance()));
	prototype->setMember(L"addRequestHeader", createNativeFunctionValue(this, &AsXML::XML_addRequestHeader));
	prototype->setMember(L"createElement", createNativeFunctionValue(this, &AsXML::XML_createElement));
	prototype->setMember(L"createTextNode", createNativeFunctionValue(this, &AsXML::XML_createTextNode));
	prototype->setMember(L"getBytesLoaded", createNativeFunctionValue(this, &AsXML::XML_getBytesLoaded));
	prototype->setMember(L"getBytesTotal", createNativeFunctionValue(this, &AsXML::XML_getBytesTotal));
	prototype->setMember(L"load", createNativeFunctionValue(this, &AsXML::XML_load));
	prototype->setMember(L"parseXML", createNativeFunctionValue(this, &AsXML::XML_parseXML));
	prototype->setMember(L"send", createNativeFunctionValue(this, &AsXML::XML_send));
	prototype->setMember(L"sendAndLoad", createNativeFunctionValue(this, &AsXML::XML_sendAndLoad));

	prototype->addProperty(L"contentType", createNativeFunction(this, &AsXML::XML_get_contentType), createNativeFunction(this, &AsXML::XML_set_contentType));
	prototype->addProperty(L"docTypeDecl", createNativeFunction(this, &AsXML::XML_get_docTypeDecl), createNativeFunction(this, &AsXML::XML_set_docTypeDecl));
	prototype->addProperty(L"idMap", createNativeFunction(this, &AsXML::XML_get_idMap), createNativeFunction(this, &AsXML::XML_set_idMap));
	prototype->addProperty(L"ignoreWhite", createNativeFunction(this, &AsXML::XML_get_ignoreWhite), createNativeFunction(this, &AsXML::XML_set_ignoreWhite));
	prototype->addProperty(L"loaded", createNativeFunction(this, &AsXML::XML_get_loaded), createNativeFunction(this, &AsXML::XML_set_loaded));
	prototype->addProperty(L"status", createNativeFunction(this, &AsXML::XML_get_status), createNativeFunction(this, &AsXML::XML_set_status));
	prototype->addProperty(L"xmlDecl", createNativeFunction(this, &AsXML::XML_get_xmlDecl), createNativeFunction(this, &AsXML::XML_set_xmlDecl));

	prototype->setReadOnly();

	setMember(L"prototype", ActionValue::fromObject(prototype));
}

ActionValue AsXML::construct(ActionContext* context, const args_t& args)
{
	return ActionValue::fromObject(new ActionObject(this));
}

void AsXML::XML_addRequestHeader(CallArgs& ca)
{
}

void AsXML::XML_createElement(CallArgs& ca)
{
}

void AsXML::XML_createTextNode(CallArgs& ca)
{
}

void AsXML::XML_getBytesLoaded(CallArgs& ca)
{
	ca.ret = ActionValue(0.0);
}

void AsXML::XML_getBytesTotal(CallArgs& ca)
{
	ca.ret = ActionValue(0.0);
}

void AsXML::XML_load(CallArgs& ca)
{
	ca.ret = ActionValue(false);
}

void AsXML::XML_parseXML(CallArgs& ca)
{
}

void AsXML::XML_send(CallArgs& ca)
{
	ca.ret = ActionValue(false);
}

void AsXML::XML_sendAndLoad(CallArgs& ca)
{
}

void AsXML::XML_get_contentType(CallArgs& ca)
{
}

void AsXML::XML_set_contentType(CallArgs& ca)
{
}

void AsXML::XML_get_docTypeDecl(CallArgs& ca)
{
}

void AsXML::XML_set_docTypeDecl(CallArgs& ca)
{
}

void AsXML::XML_get_idMap(CallArgs& ca)
{
}

void AsXML::XML_set_idMap(CallArgs& ca)
{
}

void AsXML::XML_get_ignoreWhite(CallArgs& ca)
{
}

void AsXML::XML_set_ignoreWhite(CallArgs& ca)
{
}

void AsXML::XML_get_loaded(CallArgs& ca)
{
}

void AsXML::XML_set_loaded(CallArgs& ca)
{
}

void AsXML::XML_get_status(CallArgs& ca)
{
}

void AsXML::XML_set_status(CallArgs& ca)
{
}

void AsXML::XML_get_xmlDecl(CallArgs& ca)
{
}

void AsXML::XML_set_xmlDecl(CallArgs& ca)
{
}

	}
}

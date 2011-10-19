#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Avm1/Classes/AsXML.h"
#include "Flash/Action/Avm1/Classes/AsXMLNode.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsXML", AsXML, ActionClass)

AsXML::AsXML(ActionContext* context)
:	ActionClass(context, "XML")
{
	Ref< ActionObject > prototype = new ActionObject();

	prototype->setMember("addRequestHeader", ActionValue(createNativeFunction(context, this, &AsXML::XML_addRequestHeader)));
	prototype->setMember("createElement", ActionValue(createNativeFunction(context, this, &AsXML::XML_createElement)));
	prototype->setMember("createTextNode", ActionValue(createNativeFunction(context, this, &AsXML::XML_createTextNode)));
	prototype->setMember("getBytesLoaded", ActionValue(createNativeFunction(context, this, &AsXML::XML_getBytesLoaded)));
	prototype->setMember("getBytesTotal", ActionValue(createNativeFunction(context, this, &AsXML::XML_getBytesTotal)));
	prototype->setMember("load", ActionValue(createNativeFunction(context, this, &AsXML::XML_load)));
	prototype->setMember("parseXML", ActionValue(createNativeFunction(context, this, &AsXML::XML_parseXML)));
	prototype->setMember("send", ActionValue(createNativeFunction(context, this, &AsXML::XML_send)));
	prototype->setMember("sendAndLoad", ActionValue(createNativeFunction(context, this, &AsXML::XML_sendAndLoad)));

	prototype->addProperty("contentType", createNativeFunction(context, this, &AsXML::XML_get_contentType), createNativeFunction(context, this, &AsXML::XML_set_contentType));
	prototype->addProperty("docTypeDecl", createNativeFunction(context, this, &AsXML::XML_get_docTypeDecl), createNativeFunction(context, this, &AsXML::XML_set_docTypeDecl));
	prototype->addProperty("idMap", createNativeFunction(context, this, &AsXML::XML_get_idMap), createNativeFunction(context, this, &AsXML::XML_set_idMap));
	prototype->addProperty("ignoreWhite", createNativeFunction(context, this, &AsXML::XML_get_ignoreWhite), createNativeFunction(context, this, &AsXML::XML_set_ignoreWhite));
	prototype->addProperty("loaded", createNativeFunction(context, this, &AsXML::XML_get_loaded), createNativeFunction(context, this, &AsXML::XML_set_loaded));
	prototype->addProperty("status", createNativeFunction(context, this, &AsXML::XML_get_status), createNativeFunction(context, this, &AsXML::XML_set_status));
	prototype->addProperty("xmlDecl", createNativeFunction(context, this, &AsXML::XML_get_xmlDecl), createNativeFunction(context, this, &AsXML::XML_set_xmlDecl));

	prototype->setMember("constructor", ActionValue(this));
	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

void AsXML::init(ActionObject* self, const ActionValueArray& args) const
{
}

void AsXML::coerce(ActionObject* self) const
{
	T_FATAL_ERROR;
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
	ca.ret = ActionValue(avm_number_t(0.0));
}

void AsXML::XML_getBytesTotal(CallArgs& ca)
{
	ca.ret = ActionValue(avm_number_t(0.0));
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

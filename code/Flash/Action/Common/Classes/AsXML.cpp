/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/RefArray.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Common/Classes/AsXML.h"
#include "Flash/Action/Common/XML.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsXML", AsXML, ActionClass)

AsXML::AsXML(ActionContext* context)
:	ActionClass(context, "XML")
{
	Ref< ActionObject > prototype = new ActionObject(context);

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
	prototype->setMember("__proto__", ActionValue("XMLNode"));

	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

void AsXML::initialize(ActionObject* self)
{
}

void AsXML::construct(ActionObject* self, const ActionValueArray& args)
{
	self->setRelay(new XML(getContext()));
}

ActionValue AsXML::xplicit(const ActionValueArray& args)
{
	return ActionValue();
}

void AsXML::XML_addRequestHeader(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"XML::addRequestHeader not implemented" << Endl;
	)
}

void AsXML::XML_createElement(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"XML::createElement not implemented" << Endl;
	)
}

void AsXML::XML_createTextNode(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"XML::createTextNode not implemented" << Endl;
	)
}

void AsXML::XML_getBytesLoaded(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"XML::getBytesLoaded not implemented" << Endl;
	)
}

void AsXML::XML_getBytesTotal(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"XML::getBytesTotal not implemented" << Endl;
	)
}

bool AsXML::XML_load(XML* self, const std::wstring& url) const
{
	return self->load(url);
}

void AsXML::XML_parseXML(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"XML::parseXML not implemented" << Endl;
	)
}

void AsXML::XML_send(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"XML::send not implemented" << Endl;
	)
}

void AsXML::XML_sendAndLoad(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"XML::sendAndLoad not implemented" << Endl;
	)
}

void AsXML::XML_get_contentType(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"XML::get_contentType not implemented" << Endl;
	)
}

void AsXML::XML_set_contentType(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"XML::set_contentType not implemented" << Endl;
	)
}

void AsXML::XML_get_docTypeDecl(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"XML::get_docTypeDecl not implemented" << Endl;
	)
}

void AsXML::XML_set_docTypeDecl(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"XML::set_docTypeDecl not implemented" << Endl;
	)
}

void AsXML::XML_get_idMap(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"XML::get_idMap not implemented" << Endl;
	)
}

void AsXML::XML_set_idMap(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"XML::set_idMap not implemented" << Endl;
	)
}

void AsXML::XML_get_ignoreWhite(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"XML::get_ignoreWhite not implemented" << Endl;
	)
}

void AsXML::XML_set_ignoreWhite(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"XML::set_ignoreWhite not implemented" << Endl;
	)
}

void AsXML::XML_get_loaded(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"XML::get_loaded not implemented" << Endl;
	)
}

void AsXML::XML_set_loaded(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"XML::set_loaded not implemented" << Endl;
	)
}

void AsXML::XML_get_status(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"XML::get_status not implemented" << Endl;
	)
}

void AsXML::XML_set_status(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"XML::set_status not implemented" << Endl;
	)
}

void AsXML::XML_get_xmlDecl(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"XML::get_xmlDecl not implemented" << Endl;
	)
}

void AsXML::XML_set_xmlDecl(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"XML::set_xmlDecl not implemented" << Endl;
	)
}

	}
}

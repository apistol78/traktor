#include <Core/Io/StringOutputStream.h>
#include <Core/Log/Log.h>
#include <Script/AutoScriptClass.h>
#include <Script/IScriptContext.h>
#include <Script/Js/ScriptManagerJs.h>
#include <Xml/Attribute.h>
#include <Xml/Document.h>
#include <Xml/Element.h>
#include "Transformer.h"

using namespace traktor;

T_IMPLEMENT_RTTI_CLASS(L"Transformer", Transformer, Object)

bool Transformer::create(const std::wstring& script)
{
	m_scriptManager = new script::ScriptManagerJs();

	Ref< script::AutoScriptClass< xml::Node > > xmlNodeClass = new script::AutoScriptClass< xml::Node >();
	xmlNodeClass->addMethod(L"getName", &xml::Node::getName);
	xmlNodeClass->addMethod(L"setName", &xml::Node::setName);
	xmlNodeClass->addMethod(L"getValue", &xml::Node::getValue);
	xmlNodeClass->addMethod(L"setValue", &xml::Node::setValue);
	xmlNodeClass->addMethod(L"addChild", &xml::Node::addChild);
	xmlNodeClass->addMethod(L"removeChild", &xml::Node::removeChild);
	xmlNodeClass->addMethod(L"removeAllChildren", &xml::Node::removeAllChildren);
	xmlNodeClass->addMethod(L"insertBefore", &xml::Node::insertBefore);
	xmlNodeClass->addMethod(L"insertAfter", &xml::Node::insertAfter);
	xmlNodeClass->addMethod(L"getParent", &xml::Node::getParent);
	xmlNodeClass->addMethod(L"getPreviousSibling", &xml::Node::getPreviousSibling);
	xmlNodeClass->addMethod(L"getNextSibling", &xml::Node::getNextSibling);
	xmlNodeClass->addMethod(L"getFirstChild", &xml::Node::getFirstChild);
	xmlNodeClass->addMethod(L"getLastChild", &xml::Node::getLastChild);
	m_scriptManager->registerClass(xmlNodeClass);

	Ref< script::AutoScriptClass< xml::Attribute > > xmlAttributeClass = new script::AutoScriptClass< xml::Attribute >();
	xmlAttributeClass->addConstructor< const std::wstring& >();
	xmlAttributeClass->addConstructor< const std::wstring&, const std::wstring& >();
	xmlAttributeClass->addMethod(L"getName", &xml::Attribute::getName);
	xmlAttributeClass->addMethod(L"setName", &xml::Attribute::setName);
	xmlAttributeClass->addMethod(L"getValue", &xml::Attribute::getValue);
	xmlAttributeClass->addMethod(L"setValue", &xml::Attribute::setValue);
	xmlAttributeClass->addMethod(L"getPrevious", &xml::Attribute::getPrevious);
	xmlAttributeClass->addMethod(L"getNext", &xml::Attribute::getNext);
	m_scriptManager->registerClass(xmlAttributeClass);

	Ref< script::AutoScriptClass< xml::Element > > xmlElementClass = new script::AutoScriptClass< xml::Element >();
	xmlElementClass->addConstructor< const std::wstring& >();
	xmlElementClass->addMethod(L"getSingle", &xml::Element::getSingle);
	xmlElementClass->addMethod(L"getPath", &xml::Element::getPath);
	xmlElementClass->addMethod(L"match", &xml::Element::match);
	xmlElementClass->addMethod(L"hasAttribute", &xml::Element::hasAttribute);
	xmlElementClass->addMethod(L"setAttribute", &xml::Element::setAttribute);
	xmlElementClass->addMethod(L"getFirstAttribute", &xml::Element::getFirstAttribute);
	xmlElementClass->addMethod(L"getLastAttribute", &xml::Element::getLastAttribute);
	
	Ref< xml::Attribute > (xml::Element::*mptr1)(const std::wstring&) const = &xml::Element::getAttribute;
	xmlElementClass->addMethod(L"getAttribute", mptr1);

	Ref< xml::Attribute > (xml::Element::*mptr2)(const std::wstring&, const std::wstring&) const = &xml::Element::getAttribute;
	xmlElementClass->addMethod(L"getAttribute", mptr2);

	xmlElementClass->addMethod(L"getChildElementByName", &xml::Element::getChildElementByName);
	m_scriptManager->registerClass(xmlElementClass);

	m_script = script;
	return true;
}

void Transformer::destroy()
{
	m_scriptManager = 0;
}

int32_t Transformer::transform(xml::Document* document)
{
	Ref< script::IScriptContext > context = m_scriptManager->createContext();

	if (!context->executeScript(m_script, false, 0))
	{
		log::error << L"Error when compiling script; unable to transform document" << Endl;
		return false;
	}

	if (!context->haveFunction(L"transform"))
	{
		log::error << L"No \"transform\" function defined; unable to transform document" << Endl;
		return false;
	}

	return transform(context, document->getDocumentElement());
}

int32_t Transformer::transform(script::IScriptContext* context, xml::Element* element)
{
	int32_t changes = 0;

	StringOutputStream ss1, ss2;
	element->write(ss1);

	script::Any arg(element);
	context->executeFunction(L"transform", 1, &arg);

	element->write(ss2);

	if (ss1.str() != ss2.str())
		changes++;

	for (Ref< xml::Node > child = element->getFirstChild(); child; child = child->getNextSibling())
	{
		xml::Element* childElement = dynamic_type_cast< xml::Element* >(child);
		if (childElement)
			changes += transform(context, childElement);
	}

	return changes;
}

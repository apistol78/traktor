#include <Core/Io/StringOutputStream.h>
#include <Core/Log/Log.h>
#include <Core/Math/Quaternion.h>
#include <Core/Misc/Split.h>
#include <Core/Misc/String.h>
#include <Script/AutoScriptClass.h>
#include <Script/IScriptContext.h>
#include <Script/Js/ScriptManagerJs.h>
#include <Xml/Attribute.h>
#include <Xml/Document.h>
#include <Xml/Element.h>
#include <Xml/Text.h>
#include "Transformer.h"

using namespace traktor;

namespace
{

class QuaternionHelper : public Object
{
	T_RTTI_CLASS;

public:
	QuaternionHelper(const std::wstring& desc)
	{
		std::vector< float > q;
		Split< std::wstring, float >::any(desc, L",", q, true, 4);
		m_value = Quaternion(q[0], q[1], q[2], q[3]);
	}

	float getHead() const
	{
		return m_value.toEulerAngles().x();
	}

	float getPitch() const
	{
		return m_value.toEulerAngles().y();
	}

	float getBank() const
	{
		return m_value.toEulerAngles().z();
	}

private:
	Quaternion m_value;
};

T_IMPLEMENT_RTTI_CLASS(L"QuaternionHelper", QuaternionHelper, Object)

}

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

	Ref< script::AutoScriptClass< xml::Text > > xmlTextClass = new script::AutoScriptClass< xml::Text >();
	xmlTextClass->addConstructor< const std::wstring& >();
	m_scriptManager->registerClass(xmlTextClass);

	Ref< script::AutoScriptClass< QuaternionHelper > > quaternionClass = new script::AutoScriptClass< QuaternionHelper >();
	quaternionClass->addConstructor< const std::wstring& >();
	quaternionClass->addMethod(L"getHead", &QuaternionHelper::getHead);
	quaternionClass->addMethod(L"getPitch", &QuaternionHelper::getPitch);
	quaternionClass->addMethod(L"getBank", &QuaternionHelper::getBank);
	m_scriptManager->registerClass(quaternionClass);

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

	int32_t changes = 0;
	for (int32_t pass = 0; ; ++pass)
	{
		std::wstring entry = L"transform" + toString(pass);

		if (!context->haveFunction(entry))
			break;

		changes += transform(context, document->getDocumentElement(), entry);
	}

	return changes;
}

int32_t Transformer::transform(script::IScriptContext* context, xml::Element* current, const std::wstring& entry)
{
	int32_t changes = 0;

	StringOutputStream ss1, ss2;
	current->write(ss1);

	script::Any arg[1];
	arg[0] = script::Any(current);
	context->executeFunction(entry, 1, arg);

	current->write(ss2);

	if (ss1.str() != ss2.str())
		changes++;

	RefArray< xml::Element > childElements;
	for (Ref< xml::Node > child = current->getFirstChild(); child; child = child->getNextSibling())
	{
		xml::Element* childElement = dynamic_type_cast< xml::Element* >(child);
		if (childElement)
			childElements.push_back(childElement);
	}

	for (RefArray< xml::Element >::iterator i = childElements.begin(); i != childElements.end(); ++i)
		changes += transform(context, *i, entry);

	return changes;
}

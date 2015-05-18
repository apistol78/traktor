#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/Boxes.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Core/Io/StringOutputStream.h"
#include "Xml/Attribute.h"
#include "Xml/Document.h"
#include "Xml/Element.h"
#include "Xml/Text.h"
#include "Xml/XmlClassFactory.h"

namespace traktor
{
	namespace xml
	{
		namespace
		{

std::wstring xml_Node_write(xml::Node* node)
{
	StringOutputStream ss;
	node->write(ss);
	return ss.str();
}

RefArray< xml::Element > xml_Element_get(xml::Element* element, const std::wstring& path)
{
	RefArray< xml::Element > elements;
	element->get(path, elements);
	return elements;
}

Ref< xml::Attribute > xml_Element_getAttribute_1(xml::Element* element, const std::wstring& name)
{
	return element->getAttribute(name);
}

Ref< xml::Attribute > xml_Element_getAttribute_2(xml::Element* element, const std::wstring& name, const std::wstring& defaultValue)
{
	return element->getAttribute(name, defaultValue);
}

RefArray< xml::Element > xml_Document_get(xml::Document* document, const std::wstring& path)
{
	RefArray< xml::Element > elements;
	document->get(path, elements);
	return elements;
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.xml.XmlClassFactory", 0, XmlClassFactory, IRuntimeClassFactory)

void XmlClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	Ref< AutoRuntimeClass< xml::Attribute > > classAttribute = new AutoRuntimeClass< xml::Attribute >();
	classAttribute->addConstructor< const std::wstring&, const std::wstring& >();
	classAttribute->addMethod("getName", &xml::Attribute::getName);
	classAttribute->addMethod("setName", &xml::Attribute::setName);
	classAttribute->addMethod("getValue", &xml::Attribute::getValue);
	classAttribute->addMethod("setValue", &xml::Attribute::setValue);
	classAttribute->addMethod("getPrevious", &xml::Attribute::getPrevious);
	classAttribute->addMethod("getNext", &xml::Attribute::getNext);
	registrar->registerClass(classAttribute);

	Ref< AutoRuntimeClass< xml::Node > > classNode = new AutoRuntimeClass< xml::Node >();
	classNode->addMethod("getName", &xml::Node::getName);
	classNode->addMethod("setName", &xml::Node::setName);
	classNode->addMethod("getValue", &xml::Node::getValue);
	classNode->addMethod("setValue", &xml::Node::setValue);
	classNode->addMethod("write", &xml_Node_write);
	classNode->addMethod("unlink", &xml::Node::unlink);
	classNode->addMethod("addChild", &xml::Node::addChild);
	classNode->addMethod("removeChild", &xml::Node::removeChild);
	classNode->addMethod("removeAllChildren", &xml::Node::removeAllChildren);
	classNode->addMethod("insertBefore", &xml::Node::insertBefore);
	classNode->addMethod("insertAfter", &xml::Node::insertAfter);
	classNode->addMethod("getParent", &xml::Node::getParent);
	classNode->addMethod("getPreviousSibling", &xml::Node::getPreviousSibling);
	classNode->addMethod("getNextSibling", &xml::Node::getNextSibling);
	classNode->addMethod("getFirstChild", &xml::Node::getFirstChild);
	classNode->addMethod("getLastChild", &xml::Node::getLastChild);
	registrar->registerClass(classNode);

	Ref< AutoRuntimeClass< xml::Text > > classText = new AutoRuntimeClass< xml::Text >();
	classText->addConstructor< const std::wstring& >();
	classText->addMethod("getValue", &xml::Text::getValue);
	classText->addMethod("setValue", &xml::Text::setValue);
	classText->addMethod("clone", &xml::Text::clone);
	registrar->registerClass(classText);

	Ref< AutoRuntimeClass< xml::Element > > classElement = new AutoRuntimeClass< xml::Element >();
	classElement->addConstructor< const std::wstring& >();
	classElement->addMethod("get", &xml_Element_get);
	classElement->addMethod("getSingle", &xml::Element::getSingle);
	classElement->addMethod("getPath", &xml::Element::getPath);
	classElement->addMethod("match", &xml::Element::match);
	classElement->addMethod("hasAttribute", &xml::Element::hasAttribute);
	classElement->addMethod("setAttribute", &xml::Element::setAttribute);
	classElement->addMethod("getFirstAttribute", &xml::Element::getFirstAttribute);
	classElement->addMethod("getLastAttribute", &xml::Element::getLastAttribute);
	classElement->addMethod("getAttribute", &xml_Element_getAttribute_1);
	classElement->addMethod("getAttribute", &xml_Element_getAttribute_2);
	classElement->addMethod("getChildElementByName", &xml::Element::getChildElementByName);
	classElement->addMethod("clone", &xml::Element::clone);
	registrar->registerClass(classElement);

	Ref< AutoRuntimeClass< xml::Document > > classDocument = new AutoRuntimeClass< xml::Document >();
	classDocument->addConstructor();
	classDocument->addMethod("loadFromFile", &xml::Document::loadFromFile);
	classDocument->addMethod("loadFromStream", &xml::Document::loadFromStream);
	classDocument->addMethod("loadFromText", &xml::Document::loadFromText);
	classDocument->addMethod("saveAsFile", &xml::Document::saveAsFile);
	classDocument->addMethod("saveIntoStream", &xml::Document::saveIntoStream);
	classDocument->addMethod("get", &xml_Document_get);
	classDocument->addMethod("setDocumentElement", &xml::Document::setDocumentElement);
	classDocument->addMethod("getDocumentElement", &xml::Document::getDocumentElement);
	classDocument->addMethod("clone", &xml::Document::clone);
	registrar->registerClass(classDocument);
}

	}
}

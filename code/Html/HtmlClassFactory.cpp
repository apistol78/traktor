/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Core/Io/IEncoding.h"
#include "Html/HtmlClassFactory.h"
#include "Html/Attribute.h"
#include "Html/Comment.h"
#include "Html/Document.h"
#include "Html/Element.h"
#include "Html/Text.h"

namespace traktor
{
	namespace html
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.html.HtmlClassFactory", 0, HtmlClassFactory, IRuntimeClassFactory)

void HtmlClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	auto classNode = new AutoRuntimeClass< Node >();
	classNode->addProperty("name", &Node::getName);
	classNode->addProperty("value", &Node::getValue);
	classNode->addProperty("parent", &Node::getParent);
	classNode->addProperty("previousSibling", &Node::getPreviousSibling);
	classNode->addProperty("nextSibling", &Node::getNextSibling);
	classNode->addProperty("firstChild", &Node::getFirstChild);
	classNode->addProperty("lastChild", &Node::getLastChild);
	classNode->addMethod("addChild", &Node::addChild);
	classNode->addMethod("insertBefore", &Node::insertBefore);
	classNode->addMethod("insertAfter", &Node::insertAfter);
	registrar->registerClass(classNode);

	auto classAttribute = new AutoRuntimeClass< Attribute >();
	classAttribute->addProperty("name", &Attribute::getName);
	classAttribute->addProperty("value", &Attribute::setValue, &Attribute::getValue);
	classAttribute->addProperty("previous", &Attribute::getPrevious);
	classAttribute->addProperty("next", &Attribute::getNext);
	registrar->registerClass(classAttribute);

	auto classComment = new AutoRuntimeClass< Comment >();
	registrar->registerClass(classComment);

	auto classDocument = new AutoRuntimeClass< Document >();
	classDocument->addProperty("documentElement", &Document::setDocumentElement, &Document::getDocumentElement);
	classDocument->addMethod("loadFromFile", &Document::loadFromFile);
	classDocument->addMethod("loadFromStream", &Document::loadFromStream);
	classDocument->addMethod("loadFromText", &Document::loadFromText);
	classDocument->addMethod("saveAsFile", &Document::saveAsFile);
	classDocument->addMethod("saveIntoStream", &Document::saveIntoStream);
	registrar->registerClass(classDocument);

	auto classElement = new AutoRuntimeClass< Element >();
	classElement->addProperty("firstAttribute", &Element::getFirstAttribute);
	classElement->addProperty("lastAttribute", &Element::getLastAttribute);
	classElement->addProperty("previousElementSibling", &Element::getPreviousElementSibling);
	classElement->addProperty("nextElementSibling", &Element::getNextElementSibling);
	classElement->addProperty("firstElementChild", &Element::getFirstElementChild);
	classElement->addProperty("lastElementChild", &Element::getLastElementChild);
	classElement->addMethod("hasAttribute", &Element::hasAttribute);
	classElement->addMethod("setAttribute", &Element::setAttribute);
	classElement->addMethod("getAttribute", &Element::getAttribute);
	registrar->registerClass(classElement);

	auto classText = new AutoRuntimeClass< Text >();
	registrar->registerClass(classText);
}

	}
}

/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/RefArray.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/FileOutputStream.h"
#include "Core/Io/MemoryStream.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Misc/String.h"
#include "Xml/Document.h"
#include "Xml/Element.h"
#include "Xml/Text.h"
#include "Xml/XmlPullParser.h"

namespace traktor::xml
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.xml.Document", Document, Object)

bool Document::loadFromFile(const Path& fileName)
{
	Ref< IStream > file = FileSystem::getInstance().open(fileName, File::FmRead);
	bool result = false;

	if (file != nullptr)
	{
		result = loadFromStream(file, fileName.getPathName());
		file->close();
	}

	return result;
}

bool Document::loadFromStream(IStream* stream, const std::wstring& name)
{
	RefArray< Element > stack;
	XmlPullParser xpp(stream, name);

	for (;;)
	{
		XmlPullParser::EventType et = xpp.next();
		if (et == XmlPullParser::EventType::EndDocument)
			break;
		else if (et == XmlPullParser::EventType::Invalid)
			return false;

		const XmlPullParser::Event& e = xpp.getEvent();
		switch (e.type)
		{
		case XmlPullParser::EventType::StartElement:
			{
				Ref< Element > element = new Element(e.value);
				for (XmlPullParser::Attributes::const_iterator i = e.attr.begin(); i != e.attr.end(); ++i)
					element->setAttribute(i->first, i->second);

				if (!stack.empty())
					stack.back()->addChild(element);
				else
					m_docElement = element;

				stack.push_back(element);
			}
			break;

		case XmlPullParser::EventType::Text:
			{
				if (!stack.empty() && !trim(e.value).empty())
				{
					Ref< Text > text = new Text(e.value);
					stack.back()->addChild(text);
				}
			}
			break;

		case XmlPullParser::EventType::EndElement:
			{
				T_ASSERT(!stack.empty());
				T_ASSERT(stack.back()->getName() == e.value);
				stack.pop_back();
			}
			break;

		default:
			break;
		}
	}

	return true;
}

bool Document::loadFromText(const std::wstring& text, const std::wstring& name)
{
	return loadFromStream(new MemoryStream(
		(void*)text.c_str(),
		int(text.length() * sizeof(wchar_t)),
		true,
		false
	), name);
}

bool Document::saveAsFile(const Path& fileName)
{
	Ref< IStream > file = FileSystem::getInstance().open(fileName, File::FmWrite);
	if (!file)
		return false;

	const bool result = saveIntoStream(file);
	file->close();

	return result;
}

bool Document::saveIntoStream(IStream* stream)
{
	if (!stream)
		return false;

	FileOutputStream os(stream, new Utf8Encoding());
	os << L"<?xml version=\"1.0\" encoding=\"utf-8\"?>" << Endl;

	if (m_docElement)
		m_docElement->write(os);

	return true;
}

int32_t Document::get(const std::wstring& path, RefArray< Element >& outElements) const
{
	if (m_docElement)
	{
		if (!path.empty() && path[0] == L'/')
		{
			size_t p = path.find(L'/', 1);
			if (path.substr(1, p - 1) == m_docElement->getName())
				m_docElement->get(path.substr(p + 1), outElements);
		}
		else
			m_docElement->get(path, outElements);
	}
	return int32_t(outElements.size());
}

Element* Document::getSingle(const std::wstring& path) const
{
	RefArray< Element > elements;
	return (get(path, elements) > 0) ? elements.front() : nullptr;
}

void Document::setDocumentElement(Element* docElement)
{
	m_docElement = docElement;
}

Element* Document::getDocumentElement() const
{
	return m_docElement;
}

Ref< Document > Document::clone() const
{
	Ref< Document > cloneDocument = new Document();
	cloneDocument->m_docElement = m_docElement->clone();
	return cloneDocument;
}

}

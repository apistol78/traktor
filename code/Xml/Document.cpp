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

namespace traktor
{
	namespace xml
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.xml.Document", Document, Object)

bool Document::loadFromFile(const Path& fileName)
{
	Ref< IStream > file = FileSystem::getInstance().open(fileName, File::FmRead);
	bool result = false;
	
	if (file != 0)
	{
		result = loadFromStream(file);
		file->close();
	}
	
	return result;
}

bool Document::loadFromStream(IStream* stream)
{
	RefArray< Element > stack;
	XmlPullParser xpp(stream);

	for (;;)
	{
		XmlPullParser::EventType et = xpp.next();
		if (et == XmlPullParser::EtEndDocument)
			break;
		else if (et == XmlPullParser::EtInvalid)
			return false;

		const XmlPullParser::Event& e = xpp.getEvent();
		switch (e.type)
		{
		case XmlPullParser::EtStartElement:
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

		case XmlPullParser::EtText:
			{
				if (!stack.empty())
				{
					Ref< Text > text = new Text(e.value);
					stack.back()->addChild(text);
				}
			}
			break;

		case XmlPullParser::EtEndElement:
			{
				T_ASSERT (!stack.empty());
				T_ASSERT (stack.back()->getName() == e.value);
				stack.pop_back();
			}
			break;
			
		default:
			break;
		}
	}

	return true;
}

bool Document::loadFromText(const std::wstring& text)
{
	return loadFromStream(new MemoryStream(
		(void*)text.c_str(),
		int(text.length() * sizeof(wchar_t)),
		true,
		false
	));
}

bool Document::saveAsFile(const Path& fileName)
{
	Ref< IStream > file = FileSystem::getInstance().open(fileName, File::FmWrite);
	if (!file)
		return false;

	bool result = saveIntoStream(file);
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
	return (get(path, elements) > 0) ? elements.front() : 0;
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
}

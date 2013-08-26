#include <cstring>
#include <list>
#include "Core/RefArray.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/MemoryStream.h"
#include "Core/Io/Utf16Encoding.h"
#include "Core/Io/Utf32Encoding.h"
#include "Core/Log/Log.h"
#include "Core/Misc/Endian.h"
#include "Core/Misc/String.h"
#include "Core/Misc/Split.h"
#include "Html/Comment.h"
#include "Html/Document.h"
#include "Html/Element.h"
#include "Html/Text.h"

namespace traktor
{
	namespace html
	{
		namespace
		{

template < int >
struct CreateEncoding {};

template < >
struct CreateEncoding< 2 >
{
	static Ref< IEncoding > createInstance() { return new Utf16Encoding(); }
};

template < >
struct CreateEncoding< 4 >
{
	static Ref< IEncoding > createInstance() { return new Utf32Encoding(); }
};

class CharacterReader
{
public:
	CharacterReader(IStream* stream, const IEncoding* encoding)
	:	m_stream(stream)
	,	m_encoding(encoding)
	,	m_count(0)
	{
	}

	bool available() const
	{
		return m_count > 0 || m_stream->available() > 0;
	}

	bool read(wchar_t& outChar)
	{
		if (m_count < IEncoding::MaxEncodingSize)
		{
			int32_t nread = m_stream->read(&m_buffer[m_count], IEncoding::MaxEncodingSize - m_count);
			if (nread <= 0 && m_count <= 0)
				return false;

			m_count += nread;
		}

		int used = m_encoding->translate(m_buffer, m_count, outChar);
		std::memmove(m_buffer, &m_buffer[used], m_count - used);
		m_count -= used;

		return true;
	}

private:
	Ref< IStream > m_stream;
	Ref< const IEncoding > m_encoding;
	uint8_t m_buffer[IEncoding::MaxEncodingSize];
	int32_t m_count;
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.html.Document", Document, Object)

Document::Document(bool parseComments)
:	m_parseComments(parseComments)
{
}

bool Document::loadFromFile(const std::wstring& filename, const IEncoding* encoding)
{
	Ref< IStream > file = FileSystem::getInstance().open(filename, File::FmRead);
	if (!file)
		return false;

	bool result = loadFromStream(file, encoding);

	file->close();
	return result;
}

bool Document::loadFromStream(IStream* stream, const IEncoding* encoding)
{
	CharacterReader reader(stream, encoding);
	RefArray< Element > elm;
	std::wstring attr;
	std::wstring buf;
	int mode = 0;
	wchar_t pr[2] = { 0, 0 };
	wchar_t ch = 0;
	Ref< Element > e;

	m_docElement = new Element(L"$document");
	elm.push_front(m_docElement);
	
	while (reader.available())
	{
		pr[1] = pr[0];
		pr[0] = ch;

		if (!reader.read(ch))
			break;
		
		switch (mode)
		{
		case 0:	// Collect text.
			if (ch == L'<')
			{
				std::wstring text = trim(buf);
				if (text.length() && !elm.empty())
				{
					elm.front()->addChild(
						new Text(text)
					);
				}

				buf = L"";
				mode = 1;
			}
			else
			{
				buf += ch;
			}
			break;

		case 1:	// Read element name.
			if (ch == L'>' || ch == L' ' || ch == L'\t')
			{
				if (buf.length() > 0 && buf[0] != L'!')
				{
					if (buf[0] != L'/')
					{
						bool single = bool(buf[buf.length() - 1] == L'/');
						if (single)
							buf = buf.substr(0, buf.length() - 1);

						e = new Element(toLower(buf));

						if (single)
							elm.front()->addChild(e);
						else
							elm.push_front(e);
					}
					else
					{
						buf = toLower(buf.substr(1));

						RefArray< Element >::iterator i;
						for (i = elm.begin(); i != elm.end(); ++i)
						{
							if (!compareIgnoreCase((*i)->getName(), buf))
								break;
						}
						
						if (i != elm.end())
						{
							Ref< Element > element = *i;

							RefArray< Element > tmp;
							for (RefArray< Element >::iterator j = elm.begin(); j != i; ++j)
								tmp.push_front(*j);
							for (RefArray< Element >::iterator j = tmp.begin(); j != tmp.end(); ++j)
								element->addChild(*j);

							elm.erase(elm.begin(), i + 1);

							T_ASSERT (!elm.empty());

							elm.front()->addChild(element);
						}
						else
							log::warning << L"Invalid end-of-element \"" << buf << L"\"" << Endl;
					}
				}
				if (ch == L'>')
				{
					buf = L"";
					mode = 0;
				}
				else
				{
					buf = L"";
					mode = 2;
				}
			}
			else
			{
				buf += ch;
				if (buf == L"!--")
				{
					buf = L"";
					mode = 6;
				}
			}
			break;

		case 2:	// Read attribute name.
			if (ch == L'=')
			{
				attr = toLower(trim(buf));
				buf = L"";
				mode = 3;
			}
			else if (ch == L'>')
			{
				buf = L"";
				mode = 0;
			}
			else
			{
				buf += ch;
			}
			break;

		case 3:	// Skip until attribute value begin.
			if (ch == L' ' || ch == L'\t')
				break;
			if (ch == L'"' || ch == L'\'')
			{
				buf = L"";
				mode = 5;
				break;
			}
			else
			{
				buf = L"";
				mode = 4;
				// Fall-throu as this character is part of value.
			}

		case 4:	// Read non-apostrophe value.
			if (ch == L' ' || ch == L'\t' || ch == L'>')
			{
				if (e)
					e->setAttribute(attr, buf);
				if (ch != L'>')
				{
					buf = L"";
					mode = 2;
				}
				else
				{
					buf = L"";
					mode = 0;
				}
			}
			else
			{
				buf += ch;
			}
			break;

		case 5:	// Read apostrophe value.
			if (pr[0] != L'\\' && (ch == L'"' || ch == L'\''))
			{
				if (e)
					e->setAttribute(attr, buf);
				buf = L"";
				mode = 2;
			}
			else
			{
				buf += ch;
			}
			break;

		case 6:	// Skip until end-of-comment.
			if (pr[1] == L'-' && pr[0] == L'-' && ch == L'>')
			{
				if (m_parseComments)
				{
					std::wstring text = trim(buf.substr(0, buf.length() - 2));
					if (text.length() && !elm.empty())
					{
						elm.front()->addChild(
							new Comment(text)
						);
					}
				}
				buf = L"";
				mode = 0;
			}
			else
			{
				buf += ch;
			}
			break;
		}
	}

	return bool(elm.size() == 1);
}

bool Document::loadFromText(const std::wstring& text)
{
	if (text.empty())
		return false;

	std::vector< wchar_t > buffer(text.begin(), text.end());
#if defined(_PS3)
	for (std::vector< wchar_t >::iterator i = buffer.begin(); i != buffer.end(); ++i)
		swap8in16(*i);
#endif
	Ref< MemoryStream > stream = new MemoryStream(&buffer[0], buffer.size() * sizeof(wchar_t));
	Ref< IEncoding > encoding = CreateEncoding< sizeof(wchar_t) >::createInstance();

	return loadFromStream(stream, encoding);
}

bool Document::saveAsFile(const std::wstring& filename)
{
	Ref< IStream > file = FileSystem::getInstance().open(filename, File::FmWrite);
	if (!file)
		return false;

	bool result = saveIntoStream(file);
	file->close();

	return result;
}

bool Document::saveIntoStream(IStream* stream)
{
	if (!m_docElement)
		return false;

	m_docElement->writeHtml(stream);

	return true;
}

void Document::setDocumentElement(Element* docElement)
{
	m_docElement = docElement;
}

Element* Document::getDocumentElement() const
{
	return m_docElement;
}

	}
}

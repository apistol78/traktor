/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Io/BufferedStream.h"
#include "Core/Io/StringReader.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "I18N/Dictionary.h"
#include "I18N/Text.h"
#include "I18N/Editor/Translator.h"
#include "Net/UrlConnection.h"

namespace traktor
{
	namespace i18n
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.i18n.Translator", Translator, Object)

Translator::Translator(const std::wstring& from, const std::wstring& to)
:	m_from(from)
,	m_to(to)
{
}

bool Translator::translate(const std::wstring& text, std::wstring& outText) const
{
	uint8_t utf8buf[4096];

	int32_t utf8bufLen = Utf8Encoding().translate(text.c_str(), text.length(), utf8buf);
	if (utf8bufLen <= 0)
		return false;
	
	std::wstring encodedOriginalWord = net::Url::encode(utf8buf, utf8bufLen);
	
	Ref< net::UrlConnection > connection = net::UrlConnection::open(L"http://translate.google.com/translate_a/t?client=t&text=" + encodedOriginalWord + L"&hl=" + m_to + L"&sl=" + m_from + L"&tl=" + m_to + L"&ie=UTF-8&oe=UTF-8&multires=1&otf=1&pc=1&trs=1&ssel=3&tsel=6&sc=1");
	if (!connection)
		return false;

	Ref< IStream > stream = connection->getStream();
	T_ASSERT (stream);

	StringReader reader(stream, new Utf8Encoding());
	std::wstring tmp;
	bool result = false;

	if (reader.readLine(tmp) >= 0)
	{
		if (startsWith< std::wstring >(tmp, L"[[[\""))
		{
			tmp = tmp.substr(4);
			
			size_t np = tmp.find(L'\"');
			if (np != tmp.npos)
			{
				outText = tmp.substr(0, np);
				result = true;
			}
			else
				log::error << L"Unable to translate \"" << text << L"\"; malformed response" << Endl;
		}
		else
			log::error << L"Unable to translate \"" << text << L"\"; malformed response" << Endl;
	}
	else
		log::error << L"Unable to translate \"" << text << L"\"; no response" << Endl;

	stream->close();
	stream = 0;

	return result;
}

	}
}

/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Io/StringOutputStream.h"
#include "Core/Misc/String.h"
#include "Core/Serialization/AttributeMultiLine.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberStl.h"
#include "Script/IScriptContext.h"
#include "Script/Editor/Script.h"

namespace traktor
{
	namespace script
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.script.Script", 2, Script, ISerializable)

Script::Script()
{
}

Script::Script(const std::wstring& text)
:	m_text(text)
{
}

void Script::setTextDirect(const std::wstring& text)
{
	m_text = text;
}

std::wstring Script::escape(std::function< std::wstring (const Guid& g) > fn) const
{
	StringOutputStream ss;
	size_t s, e;
	Guid g;

	for (s = 0, e = m_text.find(L'\\'); e != m_text.npos; s = e, e = m_text.find(L'\\', e))
	{
		ss << m_text.substr(s, e - s);
		if (m_text[e + 1] == L'\\')
		{
			ss << L"\\";
			e += 2;
		}
		else if (m_text[e + 1] == L'{')
		{
			++e;
			if (e + 37 > m_text.size())
				break;
			if (m_text[e + 37] != L'}')
				continue;
			if (!g.create(m_text.substr(e, 38)))
				continue;
			if (g.isNotNull())
				ss << fn(g);
			e += 38;
		}
		else
			e++;
	}
	ss << m_text.substr(s);

	return ss.str();
}

void Script::serialize(ISerializer& s)
{
	if (s.getVersion< Script >() >= 2)
		s >> Member< std::wstring >(L"text", m_text, AttributeMultiLine());
	else
	{
		std::vector< Guid > dependencies;

		if (s.getVersion< Script >() == 1)
			s >> MemberStlVector< Guid >(L"dependencies", dependencies);
	
		s >> Member< std::wstring >(L"text", m_text, AttributeMultiLine());
		m_text = replaceAll< std::wstring >(m_text, L"\\", L"\\\\");

		if (!dependencies.empty())
		{
			StringOutputStream ss;
			for (std::vector< Guid >::const_iterator i = dependencies.begin(); i != dependencies.end(); ++i)
				ss << L"#using \\" << i->format() << Endl;
			ss << Endl;

			m_text = ss.str() + m_text;
		}
	}
}

	}
}

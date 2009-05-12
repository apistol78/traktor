#include "Script/Script.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace script
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.script.Script", Script, Serializable)

void Script::setText(const std::wstring& text)
{
	m_text = text;
}

const std::wstring& Script::getText() const
{
	return m_text;
}

bool Script::serialize(Serializer& s)
{
	return s >> Member< std::wstring >(L"text", m_text, true);
}

	}
}

/*
 * TRAKTOR
 * Copyright (c) 2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Guid.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Serialization/AttributeMultiLine.h"
#include "Core/Serialization/ISerializer.h"
#include "Render/Editor/Shader/ShaderModule.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.render.ShaderModule", 0, ShaderModule, ISerializable)

ShaderModule::ShaderModule(const std::wstring& text)
:	m_text(text)
{
}

void ShaderModule::setTextDirect(const std::wstring& text)
{
	m_text = text;
}

std::wstring ShaderModule::escape(std::function< std::wstring(const Guid& g) > fn) const
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

void ShaderModule::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"text", m_text, AttributeMultiLine());
}

}

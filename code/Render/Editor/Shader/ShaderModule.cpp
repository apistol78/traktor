/*
 * TRAKTOR
 * Copyright (c) 2023-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Editor/Shader/ShaderModule.h"

#include "Core/Guid.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Serialization/AttributeMultiLine.h"
#include "Core/Serialization/AttributePrivate.h"
#include "Core/Serialization/AttributeType.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberEnum.h"
#include "Core/Serialization/MemberSmallMap.h"
#include "Render/Editor/Shader/StructDeclaration.h"

namespace traktor::render
{
namespace
{

class MemberSamplerState : public MemberComplex
{
public:
	explicit MemberSamplerState(const wchar_t* const name, SamplerState& ref)
		: MemberComplex(name, true)
		, m_ref(ref)
	{
	}

	virtual void serialize(ISerializer& s) const
	{
		const MemberEnum< Filter >::Key kFilter[] = {
			{ L"Point", Filter::Point },
			{ L"Linear", Filter::Linear },
			{ 0 }
		};

		const MemberEnum< Address >::Key kAddress[] = {
			{ L"Wrap", Address::Wrap },
			{ L"Mirror", Address::Mirror },
			{ L"Clamp", Address::Clamp },
			{ L"Border", Address::Border },
			{ 0 }
		};

		const MemberEnum< CompareFunction >::Key kCompareFunctions[] = {
			{ L"Always", CompareFunction::Always },
			{ L"Never", CompareFunction::Never },
			{ L"Less", CompareFunction::Less },
			{ L"LessEqual", CompareFunction::LessEqual },
			{ L"Greater", CompareFunction::Greater },
			{ L"GreaterEqual", CompareFunction::GreaterEqual },
			{ L"Equal", CompareFunction::Equal },
			{ L"NotEqual", CompareFunction::NotEqual },
			{ L"None", CompareFunction::None },
			{ 0 }
		};

		s >> MemberEnum< Filter >(L"minFilter", m_ref.minFilter, kFilter);
		s >> MemberEnum< Filter >(L"mipFilter", m_ref.mipFilter, kFilter);
		s >> MemberEnum< Filter >(L"magFilter", m_ref.magFilter, kFilter);
		s >> MemberEnum< Address >(L"addressU", m_ref.addressU, kAddress);
		s >> MemberEnum< Address >(L"addressV", m_ref.addressV, kAddress);
		s >> MemberEnum< Address >(L"addressW", m_ref.addressW, kAddress);
		s >> MemberEnum< CompareFunction >(L"compare", m_ref.compare, kCompareFunctions);
		s >> Member< float >(L"mipBias", m_ref.mipBias);
		s >> Member< bool >(L"ignoreMips", m_ref.ignoreMips);
		s >> Member< bool >(L"useAnisotropic", m_ref.useAnisotropic);
	}

private:
	SamplerState& m_ref;
};

}

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.render.ShaderModule", 2, ShaderModule, ISerializable)

ShaderModule::ShaderModule(const std::wstring& text)
	: m_text(text)
{
}

ShaderModule::ShaderModule(const std::wstring& text, const SmallMap< std::wstring, SamplerState >& samplers)
	: m_text(text)
	, m_samplers(samplers)
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
	s >> Member< std::wstring >(L"text", m_text, AttributeMultiLine() | AttributePrivate());

	if (s.getVersion< ShaderModule >() >= 1)
		s >> MemberSmallMap< std::wstring, SamplerState, Member< std::wstring >, MemberSamplerState >(L"samplers", m_samplers);

	if (s.getVersion< ShaderModule >() >= 2)
		s >> MemberAlignedVector< Guid >(L"structDeclarations", m_structDeclarations, AttributeType(type_of< StructDeclaration >()));
}

}

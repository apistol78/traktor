/*
 * TRAKTOR
 * Copyright (c) 2025-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Editor/Shader/StructDeclaration.h"

#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberEnum.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.render.StructDeclaration", 0, StructDeclaration, ISerializable)

bool StructDeclaration::haveElement(const std::wstring& name) const
{
	const auto it = std::find_if(m_elements.begin(), m_elements.end(), [&](const StructDeclaration::NamedElement& elm) {
		return elm.name == name;
	});
	return it != m_elements.end();
}

DataType StructDeclaration::getElementType(const std::wstring& name) const
{
	const auto it = std::find_if(m_elements.begin(), m_elements.end(), [&](const StructDeclaration::NamedElement& elm) {
		return elm.name == name;
	});
	T_FATAL_ASSERT(it != m_elements.end());
	return it != m_elements.end() ? it->type : DtFloat1;
}

void StructDeclaration::serialize(ISerializer& s)
{
	s >> MemberAlignedVector< NamedElement, MemberComposite< NamedElement > >(L"elements", m_elements);
}

void StructDeclaration::NamedElement::serialize(ISerializer& s)
{
	const MemberEnum< DataType >::Key kDataType[] = {
		{ L"Float1", DtFloat1 },
		{ L"Float2", DtFloat2 },
		{ L"Float3", DtFloat3 },
		{ L"Float4", DtFloat4 },
		{ L"Byte4", DtByte4 },
		{ L"Byte4N", DtByte4N },
		{ L"Short2", DtShort2 },
		{ L"Short4", DtShort4 },
		{ L"Short2N", DtShort2N },
		{ L"Short4N", DtShort4N },
		{ L"Half2", DtHalf2 },
		{ L"Half4", DtHalf4 },
		{ L"Integer1", DtInteger1 },
		{ L"Integer2", DtInteger2 },
		{ L"Integer3", DtInteger3 },
		{ L"Integer4", DtInteger4 },
		{ 0 }
	};

	s >> Member< std::wstring >(L"name", name);
	s >> MemberEnum< DataType >(L"type", type, kDataType);
	s >> Member< int32_t >(L"length", length, AttributeRange(0));
}

}

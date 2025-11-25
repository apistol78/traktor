/*
 * TRAKTOR
 * Copyright (c) 2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Editor/Shader/Std430.h"

#include "Core/Misc/String.h"
#include "Render/Editor/Shader/StructDeclaration.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.Std430", Std430, Object)

Ref< Std430 > Std430::create(const StructDeclaration* decl)
{
	Ref< Std430 > std430 = new Std430();
	std430->m_decl = decl;

	const AlignedVector< StructDeclaration::NamedElement >& elms = decl->getElements();
	std430->m_offsets.reserve(elms.size());

	int32_t offset = 0;
	for (int32_t i = 0; i < (int32_t)elms.size(); ++i)
	{
		std430->m_offsets.push_back(offset);
		switch (elms[i].type)
		{
		case DtFloat1:
			{
				const int32_t pad = alignUp(offset, 4) - offset;
				offset += pad + 4;
			}
			break;

		case DtFloat2:
			{
				const int32_t pad = alignUp(offset, 8) - offset;
				offset += pad + 8;
			}
			break;

		case DtFloat3:
			{
				const int32_t pad = alignUp(offset, 16) - offset;
				offset += pad + 12;
			}
			break;

		case DtFloat4:
			{
				const int32_t pad = alignUp(offset, 16) - offset;
				offset += pad + 16;
			}
			break;

		case DtInteger1:
			{
				const int32_t pad = alignUp(offset, 4) - offset;
				offset += pad + 4;
			}
			break;

		case DtInteger2:
			{
				const int32_t pad = alignUp(offset, 8) - offset;
				offset += pad + 8;
			}
			break;

		case DtInteger3:
			{
				const int32_t pad = alignUp(offset, 16) - offset;
				offset += pad + 12;
			}
			break;

		case DtInteger4:
			{
				const int32_t pad = alignUp(offset, 16) - offset;
				offset += pad + 16;
			}
			break;

		default:
			return nullptr;
		}
	}

	const int32_t pad = alignUp(offset, 16) - offset;
	std430->m_size = (uint32_t)(offset + pad);

	return std430;
}

int32_t Std430::getElementIndex(const std::wstring& element) const
{
	const AlignedVector< StructDeclaration::NamedElement >& elms = m_decl->getElements();
	for (int32_t i = 0; i < (int32_t)elms.size(); ++i)
		if (elms[i].name == element)
			return i;
	return -1;
}

int32_t Std430::getElementOffset(const std::wstring& element) const
{
	const int32_t idx = getElementIndex(element);
	if (idx >= 0)
		return m_offsets[idx];
	else
		return -1;
}

std::wstring Std430::format(const std::wstring& element, const void* data) const
{
	const AlignedVector< StructDeclaration::NamedElement >& elms = m_decl->getElements();
	const int32_t idx = getElementIndex(element);
	if (idx >= 0)
	{
		const int32_t offset = m_offsets[idx];
		const uint8_t* ptr = (const uint8_t*)data + offset;
		switch (elms[idx].type)
		{
		case DtFloat1:
			return str(L"%f", *(float*)&ptr[0]);

		case DtFloat2:
			return str(L"%f, %f", *(float*)&ptr[0], *(float*)&ptr[4]);

		case DtFloat3:
			return str(L"%f, %f, %f", *(float*)&ptr[0], *(float*)&ptr[4], *(float*)&ptr[8]);

		case DtFloat4:
			return str(L"%f, %f, %f, %f", *(float*)&ptr[0], *(float*)&ptr[4], *(float*)&ptr[8], *(float*)&ptr[12]);

		case DtInteger1:
			return str(L"%d", *(int32_t*)&ptr[0]);

		case DtInteger2:
			return str(L"%d, %d", *(int32_t*)&ptr[0], *(int32_t*)&ptr[4]);

		case DtInteger3:
			return str(L"%d, %d, %d", *(int32_t*)&ptr[0], *(int32_t*)&ptr[4], *(int32_t*)&ptr[8]);

		case DtInteger4:
			return str(L"%d, %d, %d, %d", *(int32_t*)&ptr[0], *(int32_t*)&ptr[4], *(int32_t*)&ptr[8], *(int32_t*)&ptr[12]);

		default:
			return L"";
		}
	}
	else
		return L"";
}

}

/*
 * TRAKTOR
 * Copyright (c) 2025-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Serialization/ISerializable.h"
#include "Render/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class T_DLLCLASS StructDeclaration : public ISerializable
{
	T_RTTI_CLASS;

public:
	struct NamedElement
	{
		std::wstring name;
		DataType type = DtFloat1;
		int32_t length = 0;

		void serialize(ISerializer& s);
	};

	bool haveElement(const std::wstring& name) const;

	DataType getElementType(const std::wstring& name) const;

	const AlignedVector< NamedElement >& getElements() const { return m_elements; }

	virtual void serialize(ISerializer& s) override final;

private:
	friend class ListUniformsTool;

	AlignedVector< NamedElement > m_elements;
};

}

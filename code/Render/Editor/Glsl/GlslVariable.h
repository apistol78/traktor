/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Render/Editor/Glsl/GlslType.h"
#include "Render/Editor/Shader/StructDeclaration.h"

#include <string>

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class Node;

/*!
 * \ingroup Render
 */
class T_DLLCLASS GlslVariable : public Object
{
public:
	GlslVariable() = default;

	explicit GlslVariable(const Node* node, const std::wstring& name, GlslType type, bool array = false);

	explicit GlslVariable(const Node* node, const std::wstring& name, const std::wstring& structTypeName, const StructDeclaration& structDeclaration, GlslType type);

	const Node* getNode() const { return m_node; }

	const std::wstring& getName() const { return m_name; }

	void setStructTypeName(const std::wstring& structTypeName) { m_structTypeName = structTypeName; }

	const std::wstring& getStructTypeName() const { return m_structTypeName; }

	void setStructDeclaration(const StructDeclaration& structDeclaration) { m_structDeclaration = structDeclaration; }

	const StructDeclaration& getStructDeclaration() const { return m_structDeclaration; }

	void setType(GlslType type) { m_type = type; }

	GlslType getType() const { return m_type; }

	bool isArray() const { return m_array; }

	std::wstring cast(GlslType to) const;

	GlslVariable& operator=(const GlslVariable& other);

private:
	const Node* m_node = nullptr;
	std::wstring m_name;
	std::wstring m_structTypeName;
	StructDeclaration m_structDeclaration;
	GlslType m_type = GlslType::Void;
	bool m_array = false;
};

}

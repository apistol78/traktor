/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Object.h"
#include "Render/Dx11/Editor/Hlsl/HlslType.h"

namespace traktor
{
	namespace render
	{

class Node;

/*!
 * \ingroup DX11
 */
class HlslVariable : public Object
{
public:
	HlslVariable() = default;

	HlslVariable(const Node* node, const std::wstring& name, HlslType type);

	const Node* getNode() const { return m_node; }

	const std::wstring& getName() const { return m_name; }

	void setType(HlslType type) { m_type = type; }

	HlslType getType() const { return m_type; }

	std::wstring cast(HlslType to) const;

	HlslVariable& operator = (const HlslVariable& other);

private:
	const Node* m_node = nullptr;
	std::wstring m_name;
	HlslType m_type = HtVoid;
};

	}
}


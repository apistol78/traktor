/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Containers/SmallMap.h"
#include "Render/Editor/Shader/PinType.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class ShaderGraph;
class Node;
class OutputPin;

/*! Evaluate algorithmic order of a shader graph node.
 * \ingroup Render
 */
class T_DLLCLASS ShaderGraphOrderEvaluator : public Object
{
	T_RTTI_CLASS;

public:
	explicit ShaderGraphOrderEvaluator(const ShaderGraph* shaderGraph);

	PinOrder evaluate(const Node* node, const std::wstring& inputPinName) const;

	PinOrder evaluate(const OutputPin* outputPin) const;

private:
	Ref< const ShaderGraph > m_shaderGraph;
	mutable SmallMap< const OutputPin*, PinOrder > m_evaluated;
};

}

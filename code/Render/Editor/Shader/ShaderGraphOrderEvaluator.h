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

namespace traktor
{
	namespace render
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
	ShaderGraphOrderEvaluator(
		const ShaderGraph* shaderGraph,
		bool frequentUniformsAsLinear
	);

	PinOrderType evaluate(const Node* node, const std::wstring& inputPinName) const;

	PinOrderType evaluate(const OutputPin* outputPin) const;

private:
	Ref< const ShaderGraph > m_shaderGraph;
	bool m_frequentUniformsAsLinear;
	mutable SmallMap< const OutputPin*, PinOrderType > m_evaluated;
};

	}
}


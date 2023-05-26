/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <map>
#include "Core/Object.h"
#include "Core/Ref.h"
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

class InputPin;
class Node;
class OutputPin;
class ShaderGraph;

/*! Shader graph output pin type evaluator.
 * \ingroup Render
 */
class T_DLLCLASS ShaderGraphTypeEvaluator : public Object
{
	T_RTTI_CLASS;

public:
	explicit ShaderGraphTypeEvaluator(const ShaderGraph* shaderGraph);

	PinType evaluate(const InputPin* inputPin) const;

	PinType evaluate(const Node* node, const std::wstring& inputPinName) const;

	PinType evaluate(const OutputPin* outputPin) const;

private:
	Ref< const ShaderGraph > m_shaderGraph;
	mutable std::map< const OutputPin*, PinType > m_cache;
};

}

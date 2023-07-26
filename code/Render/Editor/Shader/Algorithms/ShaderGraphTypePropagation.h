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
#include "Core/Ref.h"
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

class InputPin;
class Node;
class OutputPin;
class ShaderGraph;

/*! Shader graph pin type propagation.
 * \ingroup Render
 *
 * Pin types are determined through propagating
 * types "backward" through the shader graph thus
 * being able to determine least required type
 * in order satisfy result.
 */
class T_DLLCLASS ShaderGraphTypePropagation : public Object
{
	T_RTTI_CLASS;

public:
	explicit ShaderGraphTypePropagation(const ShaderGraph* shaderGraph, const Guid& shaderGraphId);

	PinType evaluate(const InputPin* inputPin) const;

	PinType evaluate(const OutputPin* outputPin) const;

	void set(const InputPin* inputPin, PinType inputPinType);

	void set(const OutputPin* outputPin, PinType outputPinType);

	bool valid() const { return m_valid; }

private:
	Ref< const ShaderGraph > m_shaderGraph;
	SmallMap< const InputPin*, PinType > m_inputPinTypes;
	SmallMap< const OutputPin*, PinType > m_outputPinTypes;
	bool m_valid;
};

}

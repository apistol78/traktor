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
#include "Render/Editor/Shader/Constant.h"

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
class OutputPin;

/*! Evaluate constant value of shader graph pins.
 * \ingroup Render
 *
 * Currently only usable for debugging help in editor as it's
 * not especially reliable and will fail to evaluate in
 * many situations.
 *
 * It's more designed to be resilient to incorrect
 * shader graphs than for reliable results.
 */
class T_DLLCLASS ShaderGraphEvaluator : public Object
{
	T_RTTI_CLASS;

public:
	explicit ShaderGraphEvaluator(const ShaderGraph* shaderGraph);

	Constant evaluate(const OutputPin* outputPin) const;

private:
	Ref< const ShaderGraph > m_shaderGraph;
};

}

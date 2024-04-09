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
#include "Render/Shader.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class Buffer;
class IProgram;
class IRenderSystem;
class IRenderView;
class IVertexLayout;
class ProgramParameters;
class RenderContext;

/*! Screen renderer.
 * \ingroup Render
 *
 * Render full screen quads with given shader.
 * Useful for example when rendering post processing
 * effects and such.
 */
class T_DLLCLASS ScreenRenderer : public Object
{
	T_RTTI_CLASS;

public:
	bool create(IRenderSystem* renderSystem);

	void destroy();

	// \name Direct draws.
	// !{

	void draw(IRenderView* renderView, IProgram* program);

	void draw(IRenderView* renderView, const Shader* shader);

	void draw(IRenderView* renderView, const Shader* shader, const Shader::Permutation& permutation);

	// !}

	// \name Context queued draws.
	// !{

	void draw(RenderContext* renderContext, IProgram* program, ProgramParameters* programParams, const wchar_t* name = nullptr);

	void draw(RenderContext* renderContext, const Shader* shader, ProgramParameters* programParams, const wchar_t* name = nullptr);

	void draw(RenderContext* renderContext, const Shader* shader, const Shader::Permutation& permutation, ProgramParameters* programParams, const wchar_t* name = nullptr);

	// !}

private:
	Ref< const IVertexLayout > m_vertexLayout;
	Ref< Buffer > m_vertexBuffer;
	Primitives m_primitives;
};

}

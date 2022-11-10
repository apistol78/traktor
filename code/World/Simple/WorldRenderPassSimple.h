/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "World/IWorldRenderPass.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class Buffer;
class ISimpleTexture;

	}

	namespace world
	{

/*! World render pass.
 * \ingroup World
 */
class T_DLLCLASS WorldRenderPassSimple : public IWorldRenderPass
{
	T_RTTI_CLASS;

public:
	WorldRenderPassSimple(
		render::handle_t technique,
		render::ProgramParameters* globalProgramParams,
		const Matrix44& view
	);

	virtual render::handle_t getTechnique() const override final;

	virtual uint32_t getPassFlags() const override final;

	virtual render::Shader::Permutation getPermutation(const render::Shader* shader) const override final;

	virtual void setProgramParameters(render::ProgramParameters* programParams) const override final;

	virtual void setProgramParameters(render::ProgramParameters* programParams, const Transform& lastWorld, const Transform& world) const override final;

private:
	render::handle_t m_technique;
	render::ProgramParameters* m_globalProgramParams;
	Matrix44 m_view;
	Matrix44 m_viewInverse;
};

	}
}

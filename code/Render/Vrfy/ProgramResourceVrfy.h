/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Render/Resource/ProgramResource.h"
#include "Render/Types.h"

#include <string>

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_VRFY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

/*!
 * \ingroup Vrfy
 */
class T_DLLCLASS ProgramResourceVrfy : public ProgramResource
{
	T_RTTI_CLASS;

public:
	struct Uniform
	{
		std::wstring name;
		ParameterType type = ParameterType::Scalar;
		int32_t length = 0; //!< Length of uniform array, 0 = non array uniform.
		bool initialized = false;

		void serialize(ISerializer& s);
	};

	const AlignedVector< Uniform >& getUniforms() const { return m_uniforms; }

	virtual void serialize(ISerializer& s) override;

private:
	friend class ProgramCompilerVrfy;
	friend class RenderSystemVrfy;

	Ref< ProgramResource > m_embedded;
	std::wstring m_vertexShader;
	std::wstring m_pixelShader;
	std::wstring m_computeShader;

	AlignedVector< Uniform > m_uniforms;
};

}

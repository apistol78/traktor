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
#include "Core/Ref.h"
#include "Render/Resource/ProgramResource.h"

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
	virtual void serialize(ISerializer& s) override;

private:
	friend class ProgramCompilerVrfy;
	friend class RenderSystemVrfy;

	Ref< ProgramResource > m_embedded;
	std::wstring m_vertexShader;
	std::wstring m_pixelShader;
	std::wstring m_computeShader;
};

}

/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Config.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class ProgramParameters;
class Shader;

}

namespace traktor::mesh
{

/*! Set custom shader parameter callback.
 * \ingroup Mesh
 *
 * Called when meshes are rendered in order to allow
 * user defined shader parameters.
 */
class T_DLLCLASS IMeshParameterCallback
{
public:
	virtual void setParameters(render::ProgramParameters* programParameters) const = 0;
};

}

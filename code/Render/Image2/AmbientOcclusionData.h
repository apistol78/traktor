/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Render/Image2/ImagePassOpData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class Shader;

/*!
 * \ingroup Render
 */
class T_DLLCLASS AmbientOcclusionData : public ImagePassOpData
{
	T_RTTI_CLASS;

public:
	virtual Ref< const ImagePassOp > createInstance(resource::IResourceManager* resourceManager, IRenderSystem* renderSystem) const override final;
};

}

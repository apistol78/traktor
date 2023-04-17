/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Math/Vector2.h"
#include "Render/Image2/ImagePassStepData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

/*!
 * \ingroup Render
 */
class T_DLLCLASS DirectionalBlurData : public ImagePassStepData
{
    T_RTTI_CLASS;

public:
    DirectionalBlurData();

    virtual Ref< const ImagePassStep > createInstance(resource::IResourceManager* resourceManager, IRenderSystem* renderSystem) const override final;

    virtual void serialize(ISerializer& s) override final;

private:
    friend class ImageGraphPipeline;

	enum BlurType
	{
		// 1D separable
		BtGaussian,
		BtSine,
		BtBox,
		// 2D combined
		BtBox2D,
		BtCircle2D
	};

	BlurType m_blurType;
	Vector2 m_direction;
	int32_t m_taps;
};

}

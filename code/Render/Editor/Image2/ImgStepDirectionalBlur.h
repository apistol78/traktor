/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <list>
#include "Core/Math/Vector2.h"
#include "Render/Editor/Image2/IImgStep.h"
#include "Resource/Id.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class Shader;

class T_DLLCLASS ImgStepDirectionalBlur : public IImgStep
{
    T_RTTI_CLASS;

public:
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

	ImgStepDirectionalBlur();

    virtual std::wstring getTitle() const override final;

    virtual void getInputs(std::set< std::wstring >& outInputs) const override final;

    virtual void serialize(ISerializer& s) override final;

private:
    friend class ImageGraphPipeline;

	BlurType m_blurType;
	Vector2 m_direction;
	int32_t m_taps;
    resource::Id< Shader > m_shader;
    std::list< std::wstring > m_parameters;
};

}

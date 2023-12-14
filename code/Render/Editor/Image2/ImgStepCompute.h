/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <list>
#include "Render/Image2/ImageGraphTypes.h"
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

class T_DLLCLASS ImgStepCompute : public IImgStep
{
	T_RTTI_CLASS;

public:
	virtual void getInputs(std::set< std::wstring >& outInputs) const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	friend class ImageGraphPipeline;

	resource::Id< Shader > m_shader;
	WorkSize m_workSize = WorkSize::Manual;
	int32_t m_manualWorkSize[3] = { 1, 1, 1 };
	std::list< std::wstring > m_parameters;
};

}

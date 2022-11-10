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
#include "Core/RefArray.h"
#include "Render/Types.h"
#include "Render/Image2/IImageStep.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class ImagePassOp;

/*!
 * \ingroup Render
 */
class T_DLLCLASS ImagePass : public IImageStep
{
	T_RTTI_CLASS;

public:
	virtual void addPasses(
		const ImageGraph* graph,
		const ImageGraphContext& context,
		const ImageGraphView& view,
		const targetSetVector_t& targetSetIds,
		ScreenRenderer* screenRenderer,
		RenderGraph& renderGraph
	) const override final;

private:
	friend class ImagePassData;

	std::wstring m_name;
	int32_t m_outputTargetSet;
	Clear m_clear;
	RefArray< const ImagePassOp > m_ops;
};

	}
}
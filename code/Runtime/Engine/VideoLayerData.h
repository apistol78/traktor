/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Math/Aabb2.h"
#include "Resource/Id.h"
#include "Runtime/Engine/LayerData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RUNTIME_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class Shader;

}

namespace traktor::video
{

class Video;

}

namespace traktor::runtime
{

/*! Video playback layer.
 * \ingroup Runtime
 */
class T_DLLCLASS VideoLayerData : public LayerData
{
	T_RTTI_CLASS;

public:
	VideoLayerData();

	virtual Ref< Layer > createInstance(Stage* stage, IEnvironment* environment) const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	friend class StagePipeline;

	resource::Id< video::Video > m_video;
	resource::Id< render::Shader > m_shader;
	Aabb2 m_screenBounds = { Vector2(0.0f, 0.0f), Vector2(1.0f, 1.0f) };
	bool m_visible = true;
	bool m_autoPlay = true;
	bool m_repeat = true;
};

}

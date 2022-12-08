/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Runtime/Engine/Layer.h"
#include "Core/Math/Aabb2.h"
#include "Resource/Proxy.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RUNTIME_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class ScreenRenderer;
class Shader;

}

namespace traktor::video
{

class Video;

}

namespace traktor::runtime
{

class IEnvironment;

/*!
 * \ingroup Runtime
 */
class T_DLLCLASS VideoLayer : public Layer
{
	T_RTTI_CLASS;

public:
	explicit VideoLayer(
		Stage* stage,
		const std::wstring& name,
		bool permitTransition,
		IEnvironment* environment,
		const resource::Proxy< video::Video >& video,
		const resource::Proxy< render::Shader >& shader,
		const Aabb2& screenBounds,
		bool visible,
		bool autoPlay,
		bool repeat
	);

	virtual void destroy() override;

	void play();

	void stop();

	void rewind();

	void show();

	void hide();

	bool isPlaying() const;

	bool isVisible() const;

	void setScreenBounds(const Aabb2& screenBounds);

	Aabb2 getScreenBounds() const;

	void setRepeat(bool repeat);

	bool getRepeat() const;

	virtual void transition(Layer* fromLayer) override final;

	virtual void preUpdate(const UpdateInfo& info) override final;

	virtual void update(const UpdateInfo& info) override final;

	virtual void preSetup(const UpdateInfo& info) override final;

	virtual void setup(const UpdateInfo& info, render::RenderGraph& renderGraph) override final;

	virtual void preReconfigured() override final;

	virtual void postReconfigured() override final;

	virtual void suspend() override final;

	virtual void resume() override final;

private:
	IEnvironment* m_environment;
	resource::Proxy< video::Video > m_video;
	resource::Proxy< render::Shader > m_shader;
	Ref< render::ScreenRenderer > m_screenRenderer;
	Aabb2 m_screenBounds;
	bool m_repeat;
	bool m_playing;
	bool m_visible;
};

}

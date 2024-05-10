/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <map>
#include "Core/Timer/Timer.h"
#include "Resource/Id.h"
#include "Resource/Proxy.h"
#include "Spray/Point.h"
#include "Spray/Types.h"
#include "Ui/Widget.h"
#include "World/WorldRenderView.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::editor
{

class IEditor;

}

namespace traktor::resource
{

class IResourceManager;

}

namespace traktor::render
{

class IRenderSystem;
class IRenderView;
class RenderContext;
class RenderGraph;
class PrimitiveRenderer;

}

namespace traktor::scene
{
	
class Scene;

}

namespace traktor::sound
{

class SoundPlayer;
class AudioSystem;

}

namespace traktor::world
{
	
class IWorldRenderer;

}

namespace traktor::spray
{

class Effect;
class EffectData;
class EffectLayer;
class SourceRenderer;

class T_DLLCLASS EffectPreviewControl : public ui::Widget
{
	T_RTTI_CLASS;

public:
	explicit EffectPreviewControl(editor::IEditor* editor);

	bool create(
		ui::Widget* parent,
		int style,
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		sound::AudioSystem* audioSystem
	);

	virtual void destroy() override final;

	void setEffect(const EffectData* effectData, Effect* effect);

	uint32_t getEffectLayerPoints(const EffectLayer* effectLayer) const;

	void setTimeScale(float timeScale);

	void setTotalTime(float totalTime);

	void showGuide(bool guideVisible);

	void showVelocity(bool velocityVisible);

	void setMoveEmitter(bool moveEmitter);

	void randomizeSeed();

	void syncEffect();

	void updateSettings();

private:
	editor::IEditor* m_editor;
	Ref< ui::EventSubject::IEventHandler > m_idleEventHandler;
	Ref< resource::IResourceManager > m_resourceManager;
	Ref< render::IRenderSystem > m_renderSystem;
	Ref< render::IRenderView > m_renderView;
	Ref< render::RenderGraph > m_renderGraph;
	Ref< render::RenderContext > m_renderContext;
	Ref< render::PrimitiveRenderer > m_primitiveRenderer;
	Ref< sound::AudioSystem > m_audioSystem;
	Ref< sound::SoundPlayer > m_soundPlayer;
	resource::Proxy< scene::Scene > m_sceneInstance;
	Ref< world::IWorldRenderer > m_worldRenderer;
	world::WorldRenderView m_worldRenderView;
	Ref< const EffectData > m_effectData;
	Ref< Effect > m_effect;
	Ref< world::Entity > m_effectEntity;
	Color4ub m_colorClear;
	Color4ub m_colorGrid;
	uint32_t m_randomSeed;
	Timer m_timer;
	std::map< const TypeInfo*, Ref< SourceRenderer > > m_sourceRenderers;
	Vector4 m_effectPosition;
	float m_angleHead;
	float m_anglePitch;
	ui::Point m_lastMousePosition;
	float m_timeScale;
	double m_lastDeltaTime;
	bool m_guideVisible;
	bool m_velocityVisible;
	bool m_moveEmitter;
	ui::Size m_dirtySize = ui::Size(0, 0);

	void updateWorldRenderer();

	void eventButtonDown(ui::MouseButtonDownEvent* event);

	void eventButtonUp(ui::MouseButtonUpEvent* event);

	void eventMouseMove(ui::MouseMoveEvent* event);

	void eventPaint(ui::PaintEvent* event);

	void eventIdle(ui::IdleEvent* event);
};

}

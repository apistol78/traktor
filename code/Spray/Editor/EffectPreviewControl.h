#ifndef traktor_spray_EffectPreviewControl_H
#define traktor_spray_EffectPreviewControl_H

#include <map>
#include "Core/Timer/Timer.h"
#include "Resource/Id.h"
#include "Resource/Proxy.h"
#include "Spray/Point.h"
#include "Spray/Types.h"
#include "Ui/Widget.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace editor
	{

class IEditor;

	}

	namespace resource
	{

class IResourceManager;

	}

	namespace render
	{

class ImageProcess;
class ImageProcessSettings;
class IRenderSystem;
class IRenderView;
class ISimpleTexture;
class RenderContext;
class PrimitiveRenderer;

	}

	namespace sound
	{

class SoundPlayer;
class SoundSystem;

	}

	namespace spray
	{

class Effect;
class EffectData;
class EffectInstance;
class EffectLayer;
class MeshRenderer;
class PointRenderer;
class SourceRenderer;
class TrailRenderer;

class T_DLLCLASS EffectPreviewControl : public ui::Widget
{
	T_RTTI_CLASS;

public:
	EffectPreviewControl(editor::IEditor* editor);

	bool create(
		ui::Widget* parent,
		int style,
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		sound::SoundSystem* soundSystem
	);

	virtual void destroy() T_OVERRIDE T_FINAL;

	void setEffect(const EffectData* effectData, Effect* effect);

	uint32_t getEffectLayerPoints(const EffectLayer* effectLayer) const;

	void setTimeScale(float timeScale);

	void setTotalTime(float totalTime);

	void setBackground(const resource::Id< render::ISimpleTexture >& background);

	void setImageProcess(const resource::Id< render::ImageProcessSettings >& postProcess);

	void showGuide(bool guideVisible);

	void showVelocity(bool velocityVisible);

	void setMoveEmitter(bool moveEmitter);

	void setGroundClip(bool groundClip);

	void randomizeSeed();

	void syncEffect();

	void updateSettings();

private:
	editor::IEditor* m_editor;
	Ref< ui::EventSubject::IEventHandler > m_idleEventHandler;
	Ref< resource::IResourceManager > m_resourceManager;
	Ref< render::IRenderSystem > m_renderSystem;
	Ref< render::IRenderView > m_renderView;
	Ref< render::RenderTargetSet > m_depthTexture;
	Ref< render::RenderTargetSet > m_postTargetSet;
	Ref< render::RenderContext > m_globalContext;
	resource::Proxy< render::ISimpleTexture > m_background;
	resource::Proxy< render::ImageProcessSettings > m_postProcessSettings;
	Ref< render::ImageProcess > m_postProcess;
	Ref< render::PrimitiveRenderer > m_primitiveRenderer;
	Ref< render::RenderContext > m_renderContext;
	Ref< sound::SoundSystem > m_soundSystem;
	Ref< sound::SoundPlayer > m_soundPlayer;
	Ref< PointRenderer > m_pointRenderer;
	Ref< MeshRenderer > m_meshRenderer;
	Ref< TrailRenderer > m_trailRenderer;
	Ref< const EffectData > m_effectData;
	Ref< Effect > m_effect;
	Ref< EffectInstance > m_effectInstance;
	Color4ub m_colorClear;
	Color4ub m_colorGrid;
	uint32_t m_randomSeed;
	Context m_context;
	Timer m_timer;
	std::map< const TypeInfo*, Ref< SourceRenderer > > m_sourceRenderers;
	Vector4 m_effectPosition;
	float m_angleHead;
	float m_anglePitch;
	ui::Point m_lastMousePosition;
	float m_timeScale;
	float m_lastDeltaTime;
	bool m_guideVisible;
	bool m_velocityVisible;
	bool m_moveEmitter;
	bool m_groundClip;

	void updateRenderer();

	void eventButtonDown(ui::MouseButtonDownEvent* event);

	void eventButtonUp(ui::MouseButtonUpEvent* event);

	void eventMouseMove(ui::MouseMoveEvent* event);

	void eventSize(ui::SizeEvent* event);

	void eventPaint(ui::PaintEvent* event);

	void eventIdle(ui::IdleEvent* event);
};

	}
}

#endif	// traktor_spray_EffectPreviewControl_H

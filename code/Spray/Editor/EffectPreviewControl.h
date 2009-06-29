#ifndef traktor_spray_EffectPreviewControl_H
#define traktor_spray_EffectPreviewControl_H

#include <map>
#include "Core/Heap/Ref.h"
#include "Ui/Widget.h"
#include "Spray/EmitterUpdateContext.h"
#include "Spray/Point.h"
#include "Core/Timer/Timer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class PopupMenu;

	}

	namespace resource
	{

class IResourceManager;

	}

	namespace render
	{

class RenderSystem;
class RenderView;
class RenderContext;
class PrimitiveRenderer;

	}

	namespace spray
	{

class PointRenderer;
class Effect;
class EffectInstance;

class SourceRenderer;

class T_DLLCLASS EffectPreviewControl : public ui::Widget
{
	T_RTTI_CLASS(EffectPreviewControl)

public:
	EffectPreviewControl();

	bool create(ui::Widget* parent, int style, resource::IResourceManager* resourceManager, render::RenderSystem* renderSystem);

	void destroy();

	void setEffect(Effect* effect);

	void setTimeScale(float timeScale);

	void setTotalTime(float totalTime);

	void showGuide(bool guideVisible);

	void showVelocity(bool velocityVisible);

	void randomizeSeed();

	void syncEffect();

private:
	Ref< ui::PopupMenu > m_menuOptions;
	Ref< ui::EventHandler > m_idleHandler;
	Ref< resource::IResourceManager > m_resourceManager;
	Ref< render::RenderView > m_renderView;
	Ref< render::PrimitiveRenderer > m_primitiveRenderer;
	Ref< render::RenderContext > m_renderContext;
	Ref< PointRenderer > m_pointRenderer;
	Ref< Effect > m_effect;
	Ref< EffectInstance > m_effectInstance;
	uint32_t m_randomSeed;
	EmitterUpdateContext m_context;
	Timer m_timer;
	std::map< const Type*, Ref< SourceRenderer > > m_sourceRenderers;
	Vector4 m_effectPosition;
	float m_angleHead;
	Color m_colorBackground;
	ui::Point m_lastMousePosition;
	float m_timeScale;
	float m_lastDeltaTime;
	bool m_guideVisible;
	bool m_velocityVisible;

	void eventButtonDown(ui::Event* event);

	void eventButtonUp(ui::Event* event);

	void eventMouseMove(ui::Event* event);

	void eventSize(ui::Event* event);

	void eventPaint(ui::Event* event);

	void eventIdle(ui::Event* event);
};

	}
}

#endif	// traktor_spray_EffectPreviewControl_H

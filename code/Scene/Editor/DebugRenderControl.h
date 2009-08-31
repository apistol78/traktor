#ifndef traktor_scene_DebugRenderControl_H
#define traktor_scene_DebugRenderControl_H

#include "Scene/Editor/ISceneRenderControl.h"
#include "Core/Heap/Ref.h"
#include "Ui/Size.h"

namespace traktor
{
	namespace ui
	{

class Widget;
class Event;

	}

	namespace render
	{

class IRenderView;
class ITexture;
class PrimitiveRenderer;

	}

	namespace scene
	{

class SceneEditorContext;

class DebugRenderControl : public ISceneRenderControl
{
	T_RTTI_CLASS(DebugRenderControl)

public:
	DebugRenderControl();

	bool create(ui::Widget* parent, SceneEditorContext* context);

	void destroy();

	virtual void setWorldRenderSettings(world::WorldRenderSettings* worldRenderSettings);

	virtual bool handleCommand(const ui::Command& command);

	virtual void update();

private:
	Ref< SceneEditorContext > m_context;
	Ref< ui::Widget > m_renderWidget;
	Ref< render::IRenderView > m_renderView;
	Ref< render::PrimitiveRenderer > m_primitiveRenderer;
	ui::Size m_dirtySize;

	void eventSize(ui::Event* event);

	void eventPaint(ui::Event* event);
};

	}
}

#endif	// traktor_scene_DebugRenderControl_H

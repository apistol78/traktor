#ifndef traktor_scene_RenderControlModel_H
#define traktor_scene_RenderControlModel_H

#include "Core/Object.h"
#include "Ui/Point.h"

namespace traktor
{
	namespace ui
	{

class Event;
class Widget;

	}

	namespace scene
	{

class ISceneRenderControl;
class SceneEditorContext;
class TransformChain;

/*! \brief Common model for 3d render control editors. */
class RenderControlModel : public Object
{
	T_RTTI_CLASS;

public:
	RenderControlModel();

	void eventButtonDown(ISceneRenderControl* renderControl, ui::Widget* renderWidget, ui::Event* event, SceneEditorContext* context, const TransformChain& transformChain);

	void eventButtonUp(ISceneRenderControl* renderControl, ui::Widget* renderWidget, ui::Event* event, SceneEditorContext* context, const TransformChain& transformChain);

	void eventDoubleClick(ISceneRenderControl* renderControl, ui::Widget* renderWidget, ui::Event* event, SceneEditorContext* context, const TransformChain& transformChain);

	void eventMouseMove(ISceneRenderControl* renderControl, ui::Widget* renderWidget, ui::Event* event, SceneEditorContext* context, const TransformChain& transformChain);

private:
	ui::Point m_mousePosition;
	int32_t m_mouseButton;
	bool m_modifyCamera;
	bool m_modifyAlternative;
	bool m_modifyClone;
	bool m_modifyBegun;
};

	}
}

#endif	// traktor_scene_RenderControlModel_H

#ifndef traktor_scene_RenderControlModel_H
#define traktor_scene_RenderControlModel_H

#include "Core/Object.h"
#include "Ui/Point.h"
#include "Ui/Events/AllEvents.h"

namespace traktor
{
	namespace ui
	{

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

	void eventButtonDown(ISceneRenderControl* renderControl, ui::Widget* renderWidget, ui::MouseButtonDownEvent* event, SceneEditorContext* context, const TransformChain& transformChain);

	void eventButtonUp(ISceneRenderControl* renderControl, ui::Widget* renderWidget, ui::MouseButtonUpEvent* event, SceneEditorContext* context, const TransformChain& transformChain);

	void eventDoubleClick(ISceneRenderControl* renderControl, ui::Widget* renderWidget, ui::MouseDoubleClickEvent* event, SceneEditorContext* context, const TransformChain& transformChain);

	void eventMouseMove(ISceneRenderControl* renderControl, ui::Widget* renderWidget, ui::MouseMoveEvent* event, SceneEditorContext* context, const TransformChain& transformChain);

private:
	enum ModifyType
	{
		MtNothing,
		MtModifier,
		MtCamera,
		MtSelection
	};

	ui::Point m_mousePosition0;
	ui::Point m_mousePosition;
	int32_t m_mouseButton;
	bool m_modifyBegun;
	bool m_modifyAlternative;
	bool m_modifyClone;
	ModifyType m_modify;
};

	}
}

#endif	// traktor_scene_RenderControlModel_H

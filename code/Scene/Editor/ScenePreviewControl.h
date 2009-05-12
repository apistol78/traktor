#ifndef traktor_scene_ScenePreviewControl_H
#define traktor_scene_ScenePreviewControl_H

#include "Core/Heap/Ref.h"
#include "Ui/Container.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class Slider;
class Command;

		namespace custom
		{

class ToolBar;
class ToolBarButton;
class StatusBar;

		}
	}

	namespace world
	{

class WorldRenderSettings;

	}

	namespace scene
	{

class SceneEditorContext;
class SceneRenderControl;
class Camera;
class Modifier;

class T_DLLCLASS ScenePreviewControl : public ui::Container
{
	T_RTTI_CLASS(ScenePreviewControl)

public:
	ScenePreviewControl();

	bool create(ui::Widget* parent, SceneEditorContext* context);

	void destroy();

	void setWorldRenderSettings(world::WorldRenderSettings* worldRenderSettings);

	bool handleCommand(const ui::Command& command);

	virtual ui::Size getPreferedSize() const;

	inline SceneRenderControl* getRenderControl() { return m_sceneRenderControl; }

private:
	Ref< ui::custom::ToolBar > m_toolBarActions;
	Ref< ui::custom::ToolBarButton > m_toolTogglePick;
	Ref< ui::custom::ToolBarButton > m_toolToggleX;
	Ref< ui::custom::ToolBarButton > m_toolToggleY;
	Ref< ui::custom::ToolBarButton > m_toolToggleZ;
	Ref< ui::custom::ToolBarButton > m_toolToggleGuide;
	Ref< ui::custom::ToolBarButton > m_toolToggleSnap;
	Ref< ui::Slider > m_sliderTimeScale;
	Ref< SceneRenderControl > m_sceneRenderControl;
	Ref< ui::Container > m_infoContainer;
	Ref< ui::custom::StatusBar > m_statusText;
	Ref< Modifier > m_modifierTranslate;
	Ref< Modifier > m_modifierRotate;
	Ref< Modifier > m_modifierScale;
	double m_frameTime;
	double m_renderTime;
	Ref< SceneEditorContext > m_context;

	void updateEditState();

	void updateInformation();

	void eventToolBarActionClicked(ui::Event* event);

	void eventTimeScaleChanged(ui::Event* event);

	void eventRenderModified(ui::Event* event);

	void eventRenderFrame(ui::Event* event);

	void eventCameraDoubleClick(ui::Event* event);
};

	}
}

#endif	// traktor_scene_ScenePreviewControl_H

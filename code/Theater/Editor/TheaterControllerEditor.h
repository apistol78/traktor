#ifndef traktor_theater_TheaterControllerEditor_H
#define traktor_theater_TheaterControllerEditor_H

#include "Scene/Editor/ISceneControllerEditor.h"

namespace traktor
{
	namespace scene
	{

class PostFrameEvent;

	}

	namespace ui
	{

class ListBox;
class SelectionChangeEvent;

		namespace custom
		{

class CursorMoveEvent;
class KeyMoveEvent;
class ToolBar;
class ToolBarButtonClickEvent;
class SequencerControl;

		}
	}

	namespace theater
	{

/*! \brief
 */
class TheaterControllerEditor : public scene::ISceneControllerEditor
{
	T_RTTI_CLASS;

public:
	virtual bool create(scene::SceneEditorContext* context, ui::Container* parent);

	virtual void destroy();

	virtual void entityRemoved(scene::EntityAdapter* entityAdapter);

	virtual void propertiesChanged();

	virtual bool handleCommand(const ui::Command& command);

	virtual void update();

	virtual void draw(render::PrimitiveRenderer* primitiveRenderer);

private:
	Ref< ui::ListBox > m_listActs;
	Ref< ui::custom::ToolBar > m_toolBar;
	Ref< ui::custom::SequencerControl > m_trackSequencer;
	Ref< scene::SceneEditorContext > m_context;

	void updateView();

	void captureEntities();

	void deleteSelectedKey();

	void setLookAtEntity();

	void easeVelocity();

	void gotoPreviousKey();

	void gotoNextKey();

	void eventActSelected(ui::SelectionChangeEvent* event);

	void eventToolBarClick(ui::custom::ToolBarButtonClickEvent* event);

	void eventSequencerCursorMove(ui::custom::CursorMoveEvent* event);

	void eventSequencerKeyMove(ui::custom::KeyMoveEvent* event);

	void eventContextPostFrame(scene::PostFrameEvent* event);
};

	}
}

#endif	// traktor_theater_TheaterControllerEditor_H

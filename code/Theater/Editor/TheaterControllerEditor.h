#ifndef traktor_theater_TheaterControllerEditor_H
#define traktor_theater_TheaterControllerEditor_H

#include "Core/Heap/Ref.h"
#include "Scene/Editor/ISceneControllerEditor.h"

namespace traktor
{
	namespace ui
	{

class Event;

		namespace custom
		{

class ToolBar;
class SequencerControl;

		}
	}

	namespace theater
	{

/*! \brief
 */
class TheaterControllerEditor : public scene::ISceneControllerEditor
{
	T_RTTI_CLASS(TheaterControllerEditor)

public:
	virtual bool create(scene::SceneEditorContext* context, ui::Container* parent);

	virtual void destroy();

	virtual void propertiesChanged();

	virtual bool handleCommand(const ui::Command& command);

private:
	Ref< ui::custom::ToolBar > m_toolBar;
	Ref< ui::custom::SequencerControl > m_trackSequencer;
	Ref< scene::SceneEditorContext > m_context;

	void updateSequencer();

	void captureEntities();

	void deleteSelectedKey();

	void eventToolBarClick(ui::Event* event);

	void eventSequencerCursorMove(ui::Event* event);

	void eventContextPostFrame(ui::Event* event);
};

	}
}

#endif	// traktor_theater_TheaterControllerEditor_H

#ifndef traktor_world_PostProcessEditor_H
#define traktor_world_PostProcessEditor_H

#include <map>
#include "Core/RefArray.h"
#include "Editor/IObjectEditor.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EDITOR_EXPORT)
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

	namespace ui
	{

class Event;

	}

	namespace world
	{

class IPostProcessDefineFacade;
class IPostProcessStepFacade;
class PostProcessDefineView;
class PostProcessSettings;
class PostProcessStep;
class PostProcessStepItem;
class PostProcessStepProperties;
class PostProcessView;

class T_DLLCLASS PostProcessEditor : public editor::IObjectEditor
{
	T_RTTI_CLASS;

public:
	PostProcessEditor(editor::IEditor* editor);

	virtual bool create(ui::Widget* parent, db::Instance* instance, ISerializable* object);

	virtual void destroy();

	virtual void apply();

	virtual bool handleCommand(const ui::Command& command);

	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId);

	virtual ui::Size getPreferredSize() const;

private:
	editor::IEditor* m_editor;
	Ref< db::Instance > m_instance;
	Ref< PostProcessSettings > m_asset;
	Ref< PostProcessView > m_postProcessView;
	Ref< PostProcessDefineView > m_postProcessDefineView;
	Ref< PostProcessStepProperties > m_postProcessProperties;
	std::map< const TypeInfo*, Ref< IPostProcessDefineFacade > > m_postProcessDefineFacades;
	std::map< const TypeInfo*, Ref< IPostProcessStepFacade > > m_postProcessStepFacades;

	void updateStepView(PostProcessStepItem* parent, const RefArray< PostProcessStep >& steps);

	void updateViews();

	void eventStepSelect(ui::Event* event);

	void eventStepPropertiesChange(ui::Event* event);
};

	}
}

#endif	// traktor_world_PostProcessEditor_H

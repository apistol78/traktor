#ifndef traktor_render_ImageProcessEditor_H
#define traktor_render_ImageProcessEditor_H

#include <map>
#include "Core/RefArray.h"
#include "Editor/IObjectEditor.h"
#include "Ui/Events/AllEvents.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
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
		namespace custom
		{

class GridRow;
class GridView;
class ToolBar;
class ToolBarButtonClickEvent;

		}
	}

	namespace render
	{

class IImageProcessDefineFacade;
class IImageProcessStepFacade;
class ImageProcessSettings;
class ImageProcessStep;
class ImageProcessProperties;

class T_DLLCLASS ImageProcessEditor : public editor::IObjectEditor
{
	T_RTTI_CLASS;

public:
	ImageProcessEditor(editor::IEditor* editor);

	virtual bool create(ui::Widget* parent, db::Instance* instance, ISerializable* object);

	virtual void destroy();

	virtual void apply();

	virtual bool handleCommand(const ui::Command& command);

	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId);

	virtual ui::Size getPreferredSize() const;

private:
	editor::IEditor* m_editor;
	Ref< db::Instance > m_instance;
	Ref< ImageProcessSettings > m_asset;
	Ref< ui::custom::ToolBar > m_toolBarSteps;
	Ref< ui::custom::GridView > m_gridSteps;
	Ref< ui::custom::ToolBar > m_toolBarDefinitions;
	Ref< ui::custom::GridView > m_gridDefinitions;
	Ref< ImageProcessProperties > m_properties;
	std::map< const TypeInfo*, Ref< IImageProcessDefineFacade > > m_imageProcessDefineFacades;
	std::map< const TypeInfo*, Ref< IImageProcessStepFacade > > m_imageProcessStepFacades;

	void updateStepView(ui::custom::GridRow* parentStepRow, const RefArray< ImageProcessStep >& steps, int32_t& inoutOrder);

	void updateViews();

	void eventStepToolClick(ui::custom::ToolBarButtonClickEvent* event);

	void eventGridStepSelect(ui::SelectionChangeEvent* event);

	void eventDefinitionToolClick(ui::custom::ToolBarButtonClickEvent* event);

	void eventGridDefinitionSelect(ui::SelectionChangeEvent* event);

	void eventStepPropertiesChange(ui::ContentChangeEvent* event);
};

	}
}

#endif	// traktor_render_ImageProcessEditor_H

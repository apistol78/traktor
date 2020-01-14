#pragma once

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

class GridRow;
class GridView;
class ToolBar;
class ToolBarButtonClickEvent;

	}

	namespace render
	{

class IImageProcessDefineFacade;
class IImageProcessStepFacade;
class ImageProcessData;
class ImageProcessStep;
class ImageProcessProperties;

class T_DLLCLASS ImageProcessEditor : public editor::IObjectEditor
{
	T_RTTI_CLASS;

public:
	ImageProcessEditor(editor::IEditor* editor);

	virtual bool create(ui::Widget* parent, db::Instance* instance, ISerializable* object) override final;

	virtual void destroy() override final;

	virtual void apply() override final;

	virtual bool handleCommand(const ui::Command& command) override final;

	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId) override final;

	virtual ui::Size getPreferredSize() const override final;

private:
	editor::IEditor* m_editor;
	Ref< db::Instance > m_instance;
	Ref< ImageProcessData > m_asset;
	Ref< ui::ToolBar > m_toolBarSteps;
	Ref< ui::GridView > m_gridSteps;
	Ref< ui::ToolBar > m_toolBarDefinitions;
	Ref< ui::GridView > m_gridDefinitions;
	Ref< ImageProcessProperties > m_properties;
	std::map< const TypeInfo*, Ref< IImageProcessDefineFacade > > m_imageProcessDefineFacades;
	std::map< const TypeInfo*, Ref< IImageProcessStepFacade > > m_imageProcessStepFacades;

	void updateStepView(ui::GridRow* parentStepRow, const RefArray< ImageProcessStep >& steps, int32_t& inoutOrder);

	void updateViews();

	void eventStepToolClick(ui::ToolBarButtonClickEvent* event);

	void eventGridStepSelect(ui::SelectionChangeEvent* event);

	void eventDefinitionToolClick(ui::ToolBarButtonClickEvent* event);

	void eventGridDefinitionSelect(ui::SelectionChangeEvent* event);

	void eventStepPropertiesChange(ui::ContentChangeEvent* event);
};

	}
}


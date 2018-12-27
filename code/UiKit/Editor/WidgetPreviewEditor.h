#ifndef traktor_uikit_WidgetPreviewEditor_H
#define traktor_uikit_WidgetPreviewEditor_H

#include "Core/Ref.h"
#include "Editor/IObjectEditor.h"

namespace traktor
{
	namespace editor
	{

class IEditor;

	}

	namespace resource
	{

class IResourceManager;

	}

	namespace script
	{
	
class IScriptContext;

	}

	namespace uikit
	{

class WidgetPreviewControl;

class WidgetPreviewEditor : public editor::IObjectEditor
{
	T_RTTI_CLASS;

public:
	WidgetPreviewEditor(editor::IEditor* editor);

	virtual bool create(ui::Widget* parent, db::Instance* instance, ISerializable* object) override final;

	virtual void destroy() override final;

	virtual void apply() override final;

	virtual bool handleCommand(const ui::Command& command) override final;

	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId) override final;

	virtual ui::Size getPreferredSize() const override final;

private:
	editor::IEditor* m_editor;
	Ref< script::IScriptContext > m_scriptContext;
	Ref< resource::IResourceManager > m_resourceManager;
	Ref< WidgetPreviewControl > m_previewControl;
};

	}
}

#endif	// traktor_uikit_WidgetPreviewEditor_H

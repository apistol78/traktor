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
class IScriptManager;

	}

	namespace uikit
	{

class WidgetPreviewControl;

class WidgetPreviewEditor : public editor::IObjectEditor
{
	T_RTTI_CLASS;

public:
	WidgetPreviewEditor(editor::IEditor* editor);

	virtual bool create(ui::Widget* parent, db::Instance* instance, ISerializable* object) T_OVERRIDE T_FINAL;

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual void apply() T_OVERRIDE T_FINAL;

	virtual bool handleCommand(const ui::Command& command) T_OVERRIDE T_FINAL;

	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId) T_OVERRIDE T_FINAL;

	virtual ui::Size getPreferredSize() const T_OVERRIDE T_FINAL;

private:
	editor::IEditor* m_editor;
	Ref< script::IScriptManager > m_scriptManager;
	Ref< script::IScriptContext > m_scriptContext;
	Ref< resource::IResourceManager > m_resourceManager;
	Ref< WidgetPreviewControl > m_previewControl;
};

	}
}

#endif	// traktor_uikit_WidgetPreviewEditor_H

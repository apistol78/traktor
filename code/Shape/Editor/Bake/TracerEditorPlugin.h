#pragma once

#include "Editor/IEditorPlugin.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SHAPE_EDITOR_EXPORT)
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

    namespace shape
    {

class T_DLLCLASS TracerEditorPlugin : public editor::IEditorPlugin
{
    T_RTTI_CLASS;

public:
    TracerEditorPlugin(editor::IEditor* editor);

    virtual bool create(ui::Widget* parent, editor::IEditorPageSite* site) override final;

    virtual void destroy() override final;

    virtual bool handleCommand(const ui::Command& command, bool result) override final;

    virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId) override final;

    virtual void handleWorkspaceOpened() override final;

    virtual void handleWorkspaceClosed() override final;

private:
    editor::IEditor* m_editor;
};

    }
}
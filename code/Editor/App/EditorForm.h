#ifndef traktor_editor_EditorForm_H
#define traktor_editor_EditorForm_H

#include "Core/Heap/Ref.h"
#include "Core/Io/Path.h"
#include "Core/Guid.h"
#include "Editor/Editor.h"
#include "Ui/Form.h"
#include "Ui/Command.h"

namespace traktor
{

class CommandLine;
class Thread;

	namespace ui
	{

class ShortcutTable;
class Dock;
class DockPane;
class MenuBar;
class PopupMenu;
class MenuItem;
class Tab;
class Event;

		namespace custom
		{

class ToolBar;
class StatusBar;
class ProgressBar;

		}
	}

	namespace db
	{

class Database;

	}

	namespace editor
	{

class DatabaseView;
class PropertiesView;
class HeapView;
class LogView;
class Settings;
class EditorPageFactory;
class EditorPage;
class ObjectEditorFactory;
class ObjectEditor;
class EditorTool;
class PipelineHash;

/*! \brief Main editor form.
 *
 * This is the surrounding form containing editor pages and the
 * database view.
 */
class EditorForm
:	public ui::Form
,	public Editor
{
	T_RTTI_CLASS(EditorForm)

public:
	bool create(const CommandLine& cmdLine);

	void destroy();

	/*! \name Editor implementation */
	//@{

	virtual Settings* getSettings();

	virtual db::Database* getSourceDatabase();

	virtual db::Database* getOutputDatabase();

	virtual render::RenderSystem* getRenderSystem();

	virtual void setPropertyObject(Object* properties);

	virtual Object* getPropertyObject();

	virtual void createAdditionalPanel(ui::Widget* widget, int size, bool south);

	virtual void destroyAdditionalPanel(ui::Widget* widget);

	virtual void showAdditionalPanel(ui::Widget* widget);

	virtual void hideAdditionalPanel(ui::Widget* widget);

	virtual const Type* browseType(const Type* base);

	virtual db::Instance* browseInstance(const BrowseFilter* filter);

	virtual bool isEditable(const Type& type) const;

	virtual bool openEditor(db::Instance* instance);

	virtual EditorPage* getActiveEditorPage();

	virtual void setActiveEditorPage(EditorPage* editorPage);

	virtual void buildAssets(const std::vector< Guid >& assetGuids, bool rebuild);

	virtual void buildAsset(const Guid& assetGuid, bool rebuild);

	virtual void buildAssets(bool rebuild);

	//@}

private:
	RefArray< EditorPageFactory > m_editorPageFactories;
	RefArray< ObjectEditorFactory > m_objectEditorFactories;
	RefArray< EditorTool > m_editorTools;
	std::list< ui::Command > m_shortcutCommands;
	Ref< ui::ShortcutTable > m_shortcutTable;
	Ref< ui::Dock > m_dock;
	Ref< ui::DockPane > m_paneAdditionalEast;
	Ref< ui::DockPane > m_paneAdditionalSouth;
	RefArray< ui::Widget > m_otherPanels;
	Ref< ui::MenuBar > m_menuBar;
	Ref< ui::MenuItem > m_menuItemOtherPanels;
	Ref< ui::custom::ToolBar > m_toolBar;
	Ref< ui::custom::StatusBar > m_statusBar;
	Ref< ui::custom::ProgressBar > m_buildProgress;
	Ref< ui::Tab > m_tab;
	Ref< ui::PopupMenu > m_menuTab;
	Ref< ui::MenuItem > m_menuTools;
	Ref< DatabaseView > m_dataBaseView;
	Ref< PropertiesView > m_propertiesView;
	Ref< HeapView > m_heapView;
	Ref< LogView > m_logView;
	Ref< db::Database > m_sourceDatabase;
	Ref< db::Database > m_outputDatabase;
	std::wstring m_sourceDatabasePath;
	std::wstring m_outputDatabasePath;
	Ref< render::RenderSystem > m_renderSystem;
	Ref< Settings > m_settings;
	Ref< EditorPage > m_activeEditorPage;
	Thread* m_threadBuild;

	void buildAssetsThread(std::vector< Guid > assetGuids, bool rebuild);

	void updateTitle();

	void updateOtherPanels();

	void openDatabases();

	void saveCurrentDocument();

	void saveAllDocuments();

	void closeCurrentEditor();

	void closeAllEditors();

	void closeAllOtherEditors();

	void activatePreviousEditor();

	void activateNextEditor();

	Settings* loadSettings(const std::wstring& settingsFile);

	void saveSettings(const std::wstring& settingsFile);

	void loadDictionary();

	PipelineHash* loadPipelineHash();

	void savePipelineHash(PipelineHash* pipelineHash);

	void checkModified();

	bool handleCommand(const ui::Command& command);

	/*! \name Event handlers. */
	//@{

	void eventShortcut(ui::Event* event);

	void eventMenuClick(ui::Event* event);

	void eventToolClicked(ui::Event* event);

	void eventTabButtonDown(ui::Event* event);

	void eventTabSelChange(ui::Event* event);

	void eventTabClose(ui::Event* event);

	void eventClose(ui::Event* event);

	void eventTimer(ui::Event* event);

	//@}
};

	}
}

#endif	// traktor_editor_EditorForm_H

#ifndef traktor_editor_Editor_H
#define traktor_editor_Editor_H

#include "Core/Config.h"
#include "Ui/Dock.h"

namespace traktor
{

class Guid;

	namespace db
	{

class Database;
class Instance;

	}

	namespace render
	{

class RenderSystem;

	}

	namespace editor
	{

class Settings;
class EditorPage;
class BrowseFilter;

/*! \brief Editor base interface.
 *
 * This is the interface by which other modules
 * communicate with the Editor application.
 */
class T_NOVTABLE Editor
{
public:
	/*! \brief Get editor settings. */
	virtual Settings* getSettings() = 0;

	/*! \brief Get source asset database. */
	virtual db::Database* getSourceDatabase() = 0;

	/*! \brief Get output database. */
	virtual db::Database* getOutputDatabase() = 0;

	/*! \brief Get editor render system. */
	virtual render::RenderSystem* getRenderSystem() = 0;

	/*! \brief Attach object to property list. */
	virtual void setPropertyObject(Object* properties) = 0;

	/*! \brief Get attached object in property list. */
	virtual Object* getPropertyObject() = 0;

	/*! \brief Create additional docking panel. */
	virtual void createAdditionalPanel(ui::Widget* widget, int size, bool south) = 0;

	/*! \brief Destroy additional docking panel. */
	virtual void destroyAdditionalPanel(ui::Widget* widget) = 0;

	/*! \brief Show additional docking panel. */
	virtual void showAdditionalPanel(ui::Widget* widget) = 0;

	/*! \brief Hide additional docking panel. */
	virtual void hideAdditionalPanel(ui::Widget* widget) = 0;

	/*! \brief Browse for rtti type. */
	virtual const Type* browseType(const Type* base = 0) = 0;

	/*! \brief Browse database instance. */
	virtual db::Instance* browseInstance(const BrowseFilter* filter = 0) = 0;

	/*! \brief Check if type is editable. */
	virtual bool isEditable(const Type& type) const = 0;

	/*! \brief Open instance in appropriate editor. */
	virtual bool openEditor(db::Instance* instance) = 0;

	/*! \brief Get active editor. */
	virtual EditorPage* getActiveEditorPage() = 0;

	/*! \brief Set active editor. */
	virtual void setActiveEditorPage(EditorPage* editorPage) = 0;

	/*! \brief Build assets. */
	virtual void buildAssets(const std::vector< Guid >& assetGuids, bool rebuild) = 0;

	/*! \brief Build asset. */
	virtual void buildAsset(const Guid& assetGuid, bool rebuild) = 0;

	/*! \brief Build assets. */
	virtual void buildAssets(bool rebuild) = 0;
};

	}
}

#endif	// traktor_editor_Editor_H

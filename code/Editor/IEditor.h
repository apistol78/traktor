#ifndef traktor_editor_IEditor_H
#define traktor_editor_IEditor_H

#include <set>
#include <vector>
#include "Core/Config.h"
#include "Core/RefArray.h"
#include "Core/Rtti/ITypedObject.h"
#include "Net/Url.h"

namespace traktor
{

class Guid;
class ILogTarget;
class ISerializable;
class Object;
class PropertyGroup;
class TypeInfo;

	namespace db
	{

class Database;
class Instance;

	}

	namespace editor
	{

class IEditorPage;
class IBrowseFilter;
class IPipelineDependencySet;

/*! \brief Editor base interface.
 * \ingroup Editor
 *
 * This is the interface by which other modules
 * communicate with the Editor application.
 */
class T_NOVTABLE IEditor
{
public:
	/*! \name Settings */
	/*! \{ */

	/*! \brief Get read-only settings; original settings. */
	virtual Ref< const PropertyGroup > getOriginalSettings() const = 0;

	/*! \brief Get read-only settings; contain merged workspace and global settings. */
	virtual Ref< const PropertyGroup > getSettings() const = 0;

	/*! \brief Get read-only global settings. */
	virtual Ref< const PropertyGroup > getGlobalSettings() const = 0;

	/*! \brief Get read-only workspace settings. */
	virtual Ref< const PropertyGroup > getWorkspaceSettings() const = 0;

	/*! \brief Checkout writable global settings. */
	virtual Ref< PropertyGroup > checkoutGlobalSettings() = 0;

	/*! \brief Commit global settings. */
	virtual void commitGlobalSettings() = 0;

	/*! \brief Revert global settings. */
	virtual void revertGlobalSettings() = 0;

	/*! \brief Checkout writable workspace settings. */
	virtual Ref< PropertyGroup > checkoutWorkspaceSettings() = 0;

	/*! \brief Commit workspace settings. */
	virtual void commitWorkspaceSettings() = 0;

	/*! \brief Revert workspace settings. */
	virtual void revertWorkspaceSettings() = 0;

	/*! \} */

	/*! \brief Create log targets. */
	virtual Ref< ILogTarget > createLogTarget(const std::wstring& title) = 0;

	/*! \brief Get source asset database. */
	virtual Ref< db::Database > getSourceDatabase() const = 0;

	/*! \brief Get output database. */
	virtual Ref< db::Database > getOutputDatabase() const = 0;

	/*! \brief Update database view. */
	virtual void updateDatabaseView() = 0;

	/*! \brief High light instance in database view. */
	virtual bool highlightInstance(const db::Instance* instance) = 0;

	/*! \brief Browse for rtti type. */
	virtual const TypeInfo* browseType(const TypeInfo* base = 0) = 0;

	/*! \brief Browse database instance. */
	virtual Ref< db::Instance > browseInstance(const TypeInfo& filterType) = 0;

	/*! \brief Browse database instance. */
	virtual Ref< db::Instance > browseInstance(const IBrowseFilter* filter = 0) = 0;

	/*! \brief Open instance in appropriate editor. */
	virtual bool openEditor(db::Instance* instance) = 0;

	/*! \brief Open instance using default editor. */
	virtual bool openDefaultEditor(db::Instance* instance) = 0;

	/*! \brief Open tool. */
	virtual bool openTool(const std::wstring& toolType, const std::wstring& param) = 0;

	/*! \brief Open browser to given resource. */
	virtual bool openBrowser(const net::Url& url) = 0;

	/*! \brief Get active editor. */
	virtual Ref< IEditorPage > getActiveEditorPage() = 0;

	/*! \brief Set active editor. */
	virtual void setActiveEditorPage(IEditorPage* editorPage) = 0;

	/*! \brief Build assets. */
	virtual void buildAssets(const std::vector< Guid >& assetGuids, bool rebuild) = 0;

	/*! \brief Build asset. */
	virtual void buildAsset(const Guid& assetGuid, bool rebuild) = 0;

	/*! \brief Build assets. */
	virtual void buildAssets(bool rebuild) = 0;

	/*! \brief Build asset dependencies.
	 *
	 * \param asset Source asset.
	 * \param recursionDepth Max dependency recursion depth.
	 * \return Dependency set.
	 */
	virtual Ref< IPipelineDependencySet > buildAssetDependencies(const ISerializable* asset, uint32_t recursionDepth) = 0;

	/*! \brief Set object in object store.
	 *
	 * \param name Name of object.
	 * \param object Object.
	 */
	virtual void setStoreObject(const std::wstring& name, Object* object) = 0;

	/*! \brief Get object from object store.
	 *
	 * \param name Name of object.
	 * \return Object, null if no such named object exists in store.
	 */
	virtual Object* getStoreObject(const std::wstring& name) const = 0;

	/*! \brief Get object from object store.
	 *
	 * \param name Name of object.
	 * \return Object, null if no such named object exists in store.
	 */
	template < typename ObjectType >
	ObjectType* getStoreObject(const std::wstring& name) const
	{
		return dynamic_type_cast< ObjectType* >(getStoreObject(name));
	}
};

	}
}

#endif	// traktor_editor_IEditor_H

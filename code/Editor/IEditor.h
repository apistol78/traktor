#pragma once

#include <set>
#include <vector>
#include "Core/Config.h"
#include "Core/Ref.h"
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
class Group;
class Instance;

	}

	namespace editor
	{

class IEditorPage;
class IBrowseFilter;
class IPipelineDependencySet;

/*! Editor base interface.
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

	/*! Get read-only settings; contain merged workspace and global settings. */
	virtual Ref< const PropertyGroup > getSettings() const = 0;

	/*! Checkout writable global settings. */
	virtual Ref< PropertyGroup > checkoutGlobalSettings() = 0;

	/*! Commit global settings. */
	virtual void commitGlobalSettings() = 0;

	/*! Revert global settings. */
	virtual void revertGlobalSettings() = 0;

	/*! Checkout writable workspace settings. */
	virtual Ref< PropertyGroup > checkoutWorkspaceSettings() = 0;

	/*! Commit workspace settings. */
	virtual void commitWorkspaceSettings() = 0;

	/*! Revert workspace settings. */
	virtual void revertWorkspaceSettings() = 0;

	/*! \} */

	/*! Create log targets. */
	virtual Ref< ILogTarget > createLogTarget(const std::wstring& title) = 0;

	/*! Get source asset database. */
	virtual Ref< db::Database > getSourceDatabase() const = 0;

	/*! Get output database. */
	virtual Ref< db::Database > getOutputDatabase() const = 0;

	/*! Update database view. */
	virtual void updateDatabaseView() = 0;

	/*! High light instance in database view. */
	virtual bool highlightInstance(const db::Instance* instance) = 0;

	/*! Browse for rtti type. */
	virtual const TypeInfo* browseType() = 0;

	/*! Browse for rtti type. */
	virtual const TypeInfo* browseType(const TypeInfoSet& base, bool onlyEditable, bool onlyInstantiable) = 0;

	/*! Browse database group. */
	virtual Ref< db::Group > browseGroup() = 0;

	/*! Browse database instance. */
	virtual Ref< db::Instance > browseInstance(const TypeInfo& filterType) = 0;

	/*! Browse database instance. */
	virtual Ref< db::Instance > browseInstance(const IBrowseFilter* filter = 0) = 0;

	/*! Open instance in appropriate editor. */
	virtual bool openEditor(db::Instance* instance) = 0;

	/*! Open instance using default editor. */
	virtual bool openDefaultEditor(db::Instance* instance) = 0;

	/*! Open tool. */
	virtual bool openTool(const std::wstring& toolType, const PropertyGroup* param) = 0;

	/*! Open browser to given resource. */
	virtual bool openBrowser(const net::Url& url) = 0;

	/*! Get active editor. */
	virtual Ref< IEditorPage > getActiveEditorPage() = 0;

	/*! Set active editor. */
	virtual void setActiveEditorPage(IEditorPage* editorPage) = 0;

	/*! Build assets. */
	virtual void buildAssets(const std::vector< Guid >& assetGuids, bool rebuild) = 0;

	/*! Build asset. */
	virtual void buildAsset(const Guid& assetGuid, bool rebuild) = 0;

	/*! Build asset. */
	virtual bool buildAsset(const ISerializable* sourceAsset, const std::wstring& outputPath, const Guid& outputGuid, const Object* buildParams = nullptr) = 0;

	/*! Build assets. */
	virtual void buildAssets(bool rebuild) = 0;

	/*! Cancel current build. */
	virtual void buildCancel() = 0;

	/*! Wait until current build has finished. */
	virtual void buildWaitUntilFinished() = 0;

	/*! Build asset dependencies.
	 *
	 * \param asset Source asset.
	 * \param recursionDepth Max dependency recursion depth.
	 * \return Dependency set.
	 */
	virtual Ref< IPipelineDependencySet > buildAssetDependencies(const ISerializable* asset, uint32_t recursionDepth) = 0;

	/*! Set object in object store.
	 *
	 * \param name Name of object.
	 * \param object Object.
	 */
	virtual void setStoreObject(const std::wstring& name, Object* object) = 0;

	/*! Get object from object store.
	 *
	 * \param name Name of object.
	 * \return Object, null if no such named object exists in store.
	 */
	virtual Object* getStoreObject(const std::wstring& name) const = 0;

	/*! Get object from object store.
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

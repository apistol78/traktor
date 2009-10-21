#ifndef traktor_editor_IEditor_H
#define traktor_editor_IEditor_H

#include <vector>
#include <set>
#include "Core/Config.h"
#include "Core/Heap/Ref.h"

namespace traktor
{

class Type;
class Guid;
class Serializable;

	namespace db
	{

class Database;
class Instance;

	}

	namespace render
	{

class IRenderSystem;

	}

	namespace editor
	{

class Settings;
class IProject;
class IEditorPage;
class IBrowseFilter;
class PipelineDependency;

/*! \brief Editor base interface.
 * \ingroup Editor
 *
 * This is the interface by which other modules
 * communicate with the Editor application.
 */
class T_NOVTABLE IEditor
{
public:
	/*! \brief Get editor settings. */
	virtual Settings* getSettings() = 0;

	/*! \brief Get project. */
	virtual IProject* getProject() = 0;

	/*! \brief Get editor render system. */
	virtual render::IRenderSystem* getRenderSystem() = 0;

	/*! \brief Browse for rtti type. */
	virtual const Type* browseType(const Type* base = 0) = 0;

	/*! \brief Browse database instance. */
	virtual db::Instance* browseInstance(const IBrowseFilter* filter = 0) = 0;

	/*! \brief Open instance in appropriate editor. */
	virtual bool openEditor(db::Instance* instance) = 0;

	/*! \brief Get active editor. */
	virtual IEditorPage* getActiveEditorPage() = 0;

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
	 * \param outDependencies Set of dependency asset guid;s.
	 * \return True if successful.
	 */
	virtual bool buildAssetDependencies(const Serializable* asset, uint32_t recursionDepth, RefArray< PipelineDependency >& outDependencies) = 0;
};

	}
}

#endif	// traktor_editor_IEditor_H

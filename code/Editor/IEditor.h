/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <set>
#include "Core/Config.h"
#include "Core/Ref.h"
#include "Core/RefArray.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Rtti/ITypedObject.h"
#include "Net/Url.h"

namespace traktor
{

class Guid;
class ILogTarget;
class ISerializable;
class Object;
class ObjectStore;
class PropertyGroup;
class TypeInfo;

}

namespace traktor::db
{

class Database;
class Group;
class Instance;

}

namespace traktor::editor
{

class IEditorPage;
class IBrowseFilter;
class IPipelineDepends;
class PipelineDependencySet;

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

	/*! Clone asset. */
	virtual Ref< ISerializable > cloneAsset(const ISerializable* asset) const = 0;

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

	/*! Open instance in a new editor. */
	virtual bool openInNewEditor(db::Instance* instance) = 0;

	/*! Open tool. */
	virtual bool openTool(const std::wstring& toolType, const PropertyGroup* param) = 0;

	/*! Get active editor. */
	virtual IEditorPage* getActiveEditorPage() = 0;

	/*! Set active editor. */
	virtual void setActiveEditorPage(IEditorPage* editorPage) = 0;

	/*! Build assets. */
	virtual void buildAssets(const AlignedVector< Guid >& assetGuids, bool rebuild) = 0;

	/*! Build asset. */
	virtual void buildAsset(const Guid& assetGuid, bool rebuild) = 0;

	/*! Build assets. */
	virtual void buildAssets(bool rebuild) = 0;

	/*! Cancel current build. */
	virtual void buildCancel() = 0;

	/*! Wait until current build has finished. */
	virtual void buildWaitUntilFinished() = 0;

	/*! Check if building. */
	virtual bool isBuilding() const = 0;

	/*! Create an instance of pipeline dependency walker.
	 *
	 * \return Pipeline dependency walker.
	 */
	virtual Ref< IPipelineDepends> createPipelineDepends(PipelineDependencySet* dependencySet, uint32_t recursionDepth) = 0;

	/*! Get object store. */
	virtual ObjectStore* getObjectStore() = 0;
};

}

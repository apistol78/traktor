/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/SmallMap.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Thread/ReaderWriterLock.h"
#include "Editor/IPipelineDb.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::editor
{

class T_DLLCLASS PipelineDbFlat : public IPipelineDb
{
	T_RTTI_CLASS;

public:
	virtual bool open(const std::wstring& connectionString) override final;

	virtual void close() override final;

	virtual void beginTransaction() override final;

	virtual void endTransaction() override final;

	virtual void setDependency(const Guid& guid, const PipelineDependencyHash& hash) override final;

	virtual bool getDependency(const Guid& guid, PipelineDependencyHash& outHash) const override final;

	virtual void setFile(const Path& path, const PipelineFileHash& file) override final;

	virtual bool getFile(const Path& path, PipelineFileHash& outFile) const override final;

	virtual uint32_t getDependencyCount() const override final;

	virtual bool getDependencyByIndex(uint32_t index, Guid& outGuid, PipelineDependencyHash& outHash) const override final;

	virtual uint32_t getFileCount() const override final;

	virtual bool getFileByIndex(uint32_t index, Path& outPath, PipelineFileHash& outFile) const override final;

private:
	mutable ReaderWriterLock m_lock;
	std::wstring m_file;
	SmallMap< Guid, PipelineDependencyHash > m_dependencies;
	SmallMap< std::wstring, PipelineFileHash > m_files;
	uint32_t m_changes = 0;
};

}

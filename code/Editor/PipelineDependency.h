/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <list>
#include "Core/Guid.h"
#include "Core/RefArray.h"
#include "Core/Containers/SmallSet.h"
#include "Core/Date/DateTime.h"
#include "Core/Io/Path.h"
#include "Core/Serialization/ISerializable.h"
#include "Editor/PipelineTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class ISerializable;
class OutputStream;

}

namespace traktor::editor
{

class IPipeline;

/*! Pipeline asset dependency.
 * \ingroup Editor
 *
 * Describe the dependencies of an asset;
 * which other assets are used, which files are used etc.
 */
class T_DLLCLASS PipelineDependency : public ISerializable
{
	T_RTTI_CLASS;

public:
	struct ExternalFile
	{
		Path filePath;
		DateTime lastWriteTime;

		void serialize(ISerializer& s);
	};

	typedef std::list< ExternalFile > external_files_t;

	const TypeInfo* pipelineType = nullptr;
	Guid sourceInstanceGuid;
	Ref< const ISerializable > sourceAsset;		/*!< Source asset. */
	external_files_t files;						/*!< External file dependencies. */
	std::wstring outputPath;					/*!< Database output path. */
	Guid outputGuid;							/*!< Database output guid. */
	uint32_t pipelineHash = 0;					/*!< Hash of pipeline settings. */
	uint32_t sourceAssetHash = 0;				/*!< Hash of source asset. */
	uint32_t sourceDataHash = 0;				/*!< Hash of source instance data. */
	uint32_t filesHash = 0;						/*!< Hash of external files. */
	uint32_t flags = 0;							/*!< Dependency flags. \sa PipelineDependencyFlags */
	SmallSet< uint32_t > children;				/*!< Child dependencies. */

	std::wstring information() const;

	void dump(OutputStream& os) const;

	virtual void serialize(ISerializer& s) override final;
};

}

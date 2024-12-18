/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Guid.h"
#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/Misc/Key.h"
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

class IStream;
class OutputStream;
class PropertyGroup;

}

namespace traktor::editor
{

class T_DLLCLASS IPipelineCache : public Object
{
	T_RTTI_CLASS;

public:
	/*!
	 */
	virtual bool create(const PropertyGroup* settings) = 0;

	/*!
	 */
	virtual void destroy() = 0;

	/*!
	 */
	virtual Ref< IStream > get(const Guid& guid, const PipelineDependencyHash& hash) = 0;

	/*!
	 */
	virtual Ref< IStream > put(const Guid& guid, const PipelineDependencyHash& hash) = 0;

	/*!
	 */
	virtual bool commit(const Guid& guid, const PipelineDependencyHash& hash) = 0;

	/*!
	 */
	virtual Ref< IStream > get(const Key& key) = 0;

	/*!
	 */
	virtual Ref< IStream > put(const Key& key) = 0;

	/*!
	 */
	virtual void getInformation(OutputStream& os) = 0;
};

}

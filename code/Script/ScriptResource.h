/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <vector>
#include "Core/Guid.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCRIPT_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::script
{

class IScriptBlob;

/*!
 * \ingroup Script
 */
class T_DLLCLASS ScriptResource : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual void serialize(ISerializer& s) override final;

	const std::vector< Guid >& getDependencies() const { return m_dependencies; }

	const IScriptBlob* getBlob() const { return m_blob; }

private:
	friend class ScriptPipeline;

	std::vector< Guid > m_dependencies;
	Ref< const IScriptBlob > m_blob;
};

}

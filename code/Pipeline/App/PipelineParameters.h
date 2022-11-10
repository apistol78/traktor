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
#include "Core/Serialization/ISerializer.h"

namespace traktor
{

class Environment;

class PipelineParameters : public ISerializable
{
	T_RTTI_CLASS;

public:
	PipelineParameters() = default;

	explicit PipelineParameters(
		const Environment* environment,
		const std::wstring& workingDirectory,
		const std::wstring& settings,
		bool verbose,
		bool progress,
		bool rebuild,
		bool noCache,
		const std::vector< Guid >& roots
	);

	virtual void serialize(ISerializer& s) override final;

	const Environment* getEnvironment() const { return m_environment; }

	const std::wstring& getWorkingDirectory() const { return m_workingDirectory; }

	const std::wstring& getSettings() const { return m_settings; }

	bool getVerbose() const { return m_verbose; }

	bool getProgress() const { return m_progress; }

	bool getRebuild() const { return m_rebuild; }

	bool getNoCache() const { return m_noCache; }

	const std::vector< Guid >& getRoots() const { return m_roots; }

private:
	Ref< const Environment > m_environment;
	std::wstring m_workingDirectory;
	std::wstring m_settings;
	bool m_verbose = false;
	bool m_progress = false;
	bool m_rebuild = false;
	bool m_noCache = false;
	std::vector< Guid > m_roots;
};

}

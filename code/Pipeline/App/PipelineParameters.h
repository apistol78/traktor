/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_PipelineParameters_H
#define traktor_PipelineParameters_H

#include "Core/Serialization/ISerializer.h"

namespace traktor
{

class PipelineParameters : public ISerializable
{
	T_RTTI_CLASS;

public:
	PipelineParameters();

	PipelineParameters(
		const std::wstring& workingDirectory,
		const std::wstring& settings,
		bool verbose,
		bool progress,
		bool rebuild,
		bool noCache,
		const std::vector< Guid >& roots
	);

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

	const std::wstring& getWorkingDirectory() const { return m_workingDirectory; }

	const std::wstring& getSettings() const { return m_settings; }

	bool getVerbose() const { return m_verbose; }

	bool getProgress() const { return m_progress; }

	bool getRebuild() const { return m_rebuild; }

	bool getNoCache() const { return m_noCache; }

	const std::vector< Guid >& getRoots() const { return m_roots; }

private:
	std::wstring m_workingDirectory;
	std::wstring m_settings;
	bool m_verbose;
	bool m_progress;
	bool m_rebuild;
	bool m_noCache;
	std::vector< Guid > m_roots;
};

}

#endif	// traktor_PipelineParameters_H


#pragma once

#include "Core/Serialization/ISerializable.h"

namespace traktor
{
	namespace sb
	{

class SolutionBuilderMsvcProject;

/*! Visual Studio solution settings. */
class SolutionBuilderMsvcSettings : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual void serialize(ISerializer& s) override final;

	inline const std::wstring& getSLNVersion() const { return m_slnVersion; }

	inline const std::wstring& getVSVersion() const { return m_vsVersion; }

	inline SolutionBuilderMsvcProject* getProject() const { return m_project; }

private:
	std::wstring m_slnVersion;
	std::wstring m_vsVersion;
	Ref< SolutionBuilderMsvcProject > m_project;
};

	}
}


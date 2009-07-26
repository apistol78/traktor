#ifndef SolutionBuilderMsvcSettings_H
#define SolutionBuilderMsvcSettings_H

#include <Core/Heap/Ref.h>
#include <Core/Serialization/Serializable.h>

class SolutionBuilderMsvcProject;

/*! Visual Studio solution settings. */
class SolutionBuilderMsvcSettings : public traktor::Serializable
{
	T_RTTI_CLASS(SolutionBuilderMsvcSettings)

public:
	virtual bool serialize(traktor::Serializer& s);

	inline const std::wstring& getSLNVersion() const { return m_slnVersion; }

	inline const std::wstring& getVSVersion() const { return m_vsVersion; }

	inline SolutionBuilderMsvcProject* getProject() const { return m_project; }

private:
	std::wstring m_slnVersion;
	std::wstring m_vsVersion;
	traktor::Ref< SolutionBuilderMsvcProject > m_project;
};

#endif	// SolutionBuilderMsvcSettings_H

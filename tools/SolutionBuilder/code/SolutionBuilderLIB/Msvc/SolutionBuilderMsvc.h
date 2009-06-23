#ifndef SolutionBuilderMsvc_H
#define SolutionBuilderMsvc_H

#include <map>
#include <Core/Serialization/Serializable.h>
#include <Core/Io/OutputStream.h>
#include "SolutionBuilderLIB/SolutionBuilder.h"

class Solution;
class Configuration;
class Project;
class ProjectItem;

/*! \brief Generator context. */
class GeneratorContext
{
public:
	void set(const std::wstring& key, const std::wstring& value);

	std::wstring get(const std::wstring& key) const;

	std::wstring format(const std::wstring& option) const;

private:
	std::map< std::wstring, std::wstring > m_values;
};

/*! \brief Visual Studio solution tool. */
class SolutionBuilderMsvcTool : public traktor::Serializable
{
	T_RTTI_CLASS(SolutionBuilderMsvcTool)

public:
	virtual bool generate(GeneratorContext& context, Solution* solution, Project* project, Configuration* configuration, traktor::OutputStream& os) const = 0;
};

/*! \brief Visual Studio compiler tool. */
class SolutionBuilderMsvcCompilerTool : public SolutionBuilderMsvcTool
{
	T_RTTI_CLASS(SolutionBuilderMsvcCompilerTool)

public:
	SolutionBuilderMsvcCompilerTool();

	virtual bool generate(GeneratorContext& context, Solution* solution, Project* project, Configuration* configuration, traktor::OutputStream& os) const;

	virtual bool serialize(traktor::Serializer& s);

private:
	bool m_resolvePaths;
	std::map< std::wstring, std::wstring > m_staticOptions;
};

/*! \brief Visual Studio librarian tool. */
class SolutionBuilderMsvcLibrarianTool : public SolutionBuilderMsvcTool
{
	T_RTTI_CLASS(SolutionBuilderMsvcLibrarianTool)

public:
	virtual bool generate(GeneratorContext& context, Solution* solution, Project* project, Configuration* configuration, traktor::OutputStream& os) const;

	virtual bool serialize(traktor::Serializer& s);

private:
	std::map< std::wstring, std::wstring > m_staticOptions;
};

/*! \brief Visual Studio linker tool. */
class SolutionBuilderMsvcLinkerTool : public SolutionBuilderMsvcTool
{
	T_RTTI_CLASS(SolutionBuilderMsvcLinkerTool)

public:
	SolutionBuilderMsvcLinkerTool();

	virtual bool generate(GeneratorContext& context, Solution* solution, Project* project, Configuration* configuration, traktor::OutputStream& os) const;

	virtual bool serialize(traktor::Serializer& s);

private:
	bool m_resolvePaths;
	std::map< std::wstring, std::wstring > m_staticOptions;

	void findDefinitions(
		GeneratorContext& context,
		Solution* solution,
		Project* project,
		const traktor::RefList< ProjectItem >& items
	) const;

	void collectAdditionalLibraries(
		Project* project,
		Configuration* configuration,
		std::set< std::wstring >& outAdditionalLibraries,
		std::set< std::wstring >& outAdditionalLibraryPaths
	) const;
};

/*! \brief Visual Studio generic tool. */
class SolutionBuilderMsvcGenericTool : public SolutionBuilderMsvcTool
{
	T_RTTI_CLASS(SolutionBuilderMsvcGenericTool)

public:
	virtual bool generate(GeneratorContext& context, Solution* solution, Project* project, Configuration* configuration, traktor::OutputStream& os) const;

	virtual bool serialize(traktor::Serializer& s);

private:
	std::wstring m_toolName;
	std::map< std::wstring, std::wstring > m_staticOptions;
};

/*! Visual Studio solution configuration settings. */
class SolutionBuilderMsvcConfiguration : public traktor::Serializable
{
	T_RTTI_CLASS(SolutionBuilderMsvcConfiguration)

public:
	bool generate(
		GeneratorContext& context,
		Solution* solution,
		Project* project,
		Configuration* configuration,
		const std::wstring& platform,
		traktor::OutputStream& os
	) const;

	virtual bool serialize(traktor::Serializer& s);

private:
	std::map< std::wstring, std::wstring > m_staticOptions;
	traktor::RefArray< SolutionBuilderMsvcTool > m_tools[2];				//< One array per profile (debug, release).
};

/*! \brief Visual Studio solution project settings. */
class SolutionBuilderMsvcProject : public traktor::Serializable
{
	T_RTTI_CLASS(SolutionBuilderMsvcProject)

public:
	inline const std::wstring& getPlatform() const { return m_platform; }

	bool generate(GeneratorContext& context, Solution* solution, Project* project, const std::wstring& projectGuid, traktor::OutputStream& os) const;

	virtual int getVersion() const;

	virtual bool serialize(traktor::Serializer& s);

private:
	std::map< std::wstring, std::wstring > m_staticOptions;
	std::wstring m_platform;
	traktor::Ref< SolutionBuilderMsvcConfiguration > m_configurations[4];	//< One array per target type (executable, console, static library, shared library).
	std::map< std::wstring, traktor::Ref< SolutionBuilderMsvcTool > > m_customTools;

	bool addItem(GeneratorContext& context, Solution* solution, Project* project, ProjectItem* item, traktor::OutputStream& os) const;
};

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

/*! Generate solution files for Visual Studio 2003, 2005 or 2008. */
class SolutionBuilderMsvc : public SolutionBuilder
{
	T_RTTI_CLASS(SolutionBuilderMsvc)

public:
	virtual bool create(const traktor::CommandLine& cmdLine);

	virtual bool generate(Solution* solution);

	virtual void showOptions() const;

private:
	traktor::Ref< SolutionBuilderMsvcSettings > m_settings;
};

#endif	// SolutionBuilderMsvc_H

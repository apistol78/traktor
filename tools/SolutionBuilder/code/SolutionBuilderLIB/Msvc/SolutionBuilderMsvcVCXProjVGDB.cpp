#include <Core/Io/FileSystem.h>
#include <Core/Io/DynamicMemoryStream.h>
#include <Core/Io/FileOutputStream.h>
#include <Core/Io/AnsiEncoding.h>
#include <Core/Serialization/ISerializer.h>
#include <Core/Serialization/Member.h>
#include <Core/Serialization/MemberStaticArray.h>
#include <Core/Serialization/MemberStl.h>
#include <Core/Serialization/MemberRefArray.h>
#include <Core/Misc/String.h>
#include <Core/Misc/MD5.h>
#include <Core/Log/Log.h>
#include <Xml/Document.h>
#include <Xml/Element.h>
#include <Xml/XmlDeserializer.h>
#include "SolutionBuilderLIB/Configuration.h"
#include "SolutionBuilderLIB/ExternalDependency.h"
#include "SolutionBuilderLIB/File.h"
#include "SolutionBuilderLIB/Filter.h"
#include "SolutionBuilderLIB/Project.h"
#include "SolutionBuilderLIB/ProjectDependency.h"
#include "SolutionBuilderLIB/Solution.h"
#include "SolutionBuilderLIB/Msvc/GeneratorContext.h"
#include "SolutionBuilderLIB/Msvc/SolutionBuilderMsvcVCXBuildTool.h"
#include "SolutionBuilderLIB/Msvc/SolutionBuilderMsvcVCXDefinition.h"
#include "SolutionBuilderLIB/Msvc/SolutionBuilderMsvcVCXProjVGDB.h"
#include "SolutionBuilderLIB/Msvc/VGDBCredentials.h"

using namespace traktor;

namespace
{

	std::wstring systemPath(const Path& path)
	{
		return replaceAll< std::wstring >(path.getPathName(), L'/', L'\\');
	}

	std::wstring getElementValue(xml::Document* document, const std::wstring& elementPath, const std::wstring& defaultValue)
	{
		if (!document)
			return defaultValue;

		xml::Element* element = document->getSingle(elementPath);
		if (element)
			return element->getValue();
		else
			return defaultValue;
	}

}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"SolutionBuilderMsvcVCXProjVGDB", 3, SolutionBuilderMsvcVCXProjVGDB, SolutionBuilderMsvcVCXProj)

SolutionBuilderMsvcVCXProjVGDB::SolutionBuilderMsvcVCXProjVGDB()
{
}

bool SolutionBuilderMsvcVCXProjVGDB::generate(
	GeneratorContext& context,
	Solution* solution,
	Project* project
) const
{
	// Read user credentials.
	Ref< IStream > file = FileSystem::getInstance().open(L"VGDB.user", traktor::File::FmRead);
	if (!file)
	{
		log::error << L"No VGDB.user file found; unable to create solution" << Endl;
		return false;
	}

	Ref< VGDBCredentials > credentials = xml::XmlDeserializer(file).readObject< VGDBCredentials >();
	if (!credentials)
	{
		log::error << L"Failed to read VGDB.user file; unable to create solution" << Endl;
		return false;
	}

	file->close();
	file = 0;

	if (!generateProject(context, solution, project))
		return false;

	if (!generateFilters(context, solution, project))
		return false;

	if (!generateMakefiles(context, solution, project, credentials))
		return false;

	return true;
}

bool SolutionBuilderMsvcVCXProjVGDB::generateProject(
	GeneratorContext& context,
	Solution* solution,
	Project* project
) const
{
	std::wstring projectPath, projectFileName, projectGuid;
	if (!getInformation(
		context,
		solution,
		project,
		projectPath,
		projectFileName,
		projectGuid
	))
		return false;

	if (!FileSystem::getInstance().makeDirectory(projectPath))
		return false;

	traktor::log::info << L"Generating msbuild project \"" << projectFileName << L"\"" << Endl;

	context.set(L"PROJECT_PLATFORM", m_platform);
	context.set(L"PROJECT_NAME", project->getName());
	context.set(L"PROJECT_PATH", projectPath);
	context.set(L"PROJECT_FILENAME", projectFileName);
	context.set(L"PROJECT_GUID", projectGuid);

	context.set(L"MODULE_DEFINITION_FILE", L"");
	findDefinitions(context, solution, project, project->getItems());

	std::vector< uint8_t > buffer;
	buffer.reserve(40000);

	DynamicMemoryStream bufferStream(buffer, false, true);
	FileOutputStream os(&bufferStream, new AnsiEncoding());

	os << L"<Project DefaultTargets=\"Build\" ToolsVersion=\"4.0\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">" << Endl;
	os << IncreaseIndent;

	// Configurations
	os << L"<ItemGroup Label=\"ProjectConfigurations\">" << Endl;
	os << IncreaseIndent;

	const RefArray< Configuration >& configurations = project->getConfigurations();
	for (RefArray< Configuration >::const_iterator i = configurations.begin(); i != configurations.end(); ++i)
	{
		const Configuration* configuration = *i;
		T_ASSERT (configuration);

		os << L"<ProjectConfiguration Include=\"" << configuration->getName() << L"|" << m_platform << L"\">" << Endl;
		os << IncreaseIndent;

		os << L"<Configuration>" << configuration->getName() << L"</Configuration>" << Endl;
		os << L"<Platform>" << m_platform << L"</Platform>" << Endl;

		os << DecreaseIndent;
		os << L"</ProjectConfiguration>" << Endl;
	}

	os << DecreaseIndent;
	os << L"</ItemGroup>" << Endl;

	// Globals
	os << L"<PropertyGroup Label=\"Globals\">" << Endl;
	os << IncreaseIndent;

	if (!m_keyword.empty())
		os << L"<Keyword>" << m_keyword << L"</Keyword>" << Endl;

	os << L"<ProjectGUID>" << projectGuid << L"</ProjectGUID>" << Endl;
	os << L"<RootNamespace>" << project->getName() << L"</RootNamespace>" << Endl;

	os << DecreaseIndent;
	os << L"</PropertyGroup>" << Endl;

	os << L"<Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.Default.props\" />" << Endl;

	// Configurations
	for (RefArray< Configuration >::const_iterator i = configurations.begin(); i != configurations.end(); ++i)
	{
		const Configuration* configuration = *i;
		T_ASSERT (configuration);

		os << L"<PropertyGroup Label=\"Configuration\" Condition=\"'$(Configuration)|$(Platform)'=='" << configuration->getName() << L"|" << m_platform << L"'\">" << Endl;
		os << IncreaseIndent;

		if (configuration->getTargetProfile() == Configuration::TpDebug)
		{
			const std::map< std::wstring, std::wstring >& cd = m_configurationDefinitionsDebug[configuration->getTargetFormat()];
			for (std::map< std::wstring, std::wstring >::const_iterator i = cd.begin(); i != cd.end(); ++i)
				os << L"<" << i->first << L">" << i->second << L"</" << i->first << L">" << Endl;
		}
		else
		{
			const std::map< std::wstring, std::wstring >& cd = m_configurationDefinitionsRelease[configuration->getTargetFormat()];
			for (std::map< std::wstring, std::wstring >::const_iterator i = cd.begin(); i != cd.end(); ++i)
				os << L"<" << i->first << L">" << i->second << L"</" << i->first << L">" << Endl;
		}

		os << DecreaseIndent;
		os << L"</PropertyGroup>" << Endl;
	}

	os << L"<Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.props\" />" << Endl;
	os << L"<ImportGroup Label=\"PropertySheets\">" << Endl;
	os << IncreaseIndent;
	os << L"<Import Project=\"$(LocalAppData)\\Microsoft\\VisualStudio\\10.0\\Microsoft.Cpp.$(Platform).user.props\" Condition=\"exists('$(LocalAppData)\\Microsoft\\VisualStudio\\10.0\\Microsoft.Cpp.$(Platform).user.props')\" />" << Endl;
	os << DecreaseIndent;
	os << L"</ImportGroup>" << Endl;

	// Properties
	for (RefArray< Configuration >::const_iterator i = configurations.begin(); i != configurations.end(); ++i)
	{
		const Configuration* configuration = *i;
		T_ASSERT (configuration);

		os << L"<PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='" << configuration->getName() << L"|" << m_platform << L"'\">" << Endl;
		os << IncreaseIndent;

		os << L"<NMakeIncludeSearchPath />" << Endl;
		os << L"<NMakeForcedIncludes />" << Endl;

		if (configuration->getTargetProfile() == Configuration::TpDebug)
			os << L"<NMakePreprocessorDefinitions>__VisualGDB_CFG_Debug;$(NMakePreprocessorDefinitions)</NMakePreprocessorDefinitions>" << Endl;
		else
			os << L"<NMakePreprocessorDefinitions>__VisualGDB_CFG_Release;$(NMakePreprocessorDefinitions)</NMakePreprocessorDefinitions>" << Endl;

		os << L"<NMakeBuildCommandLine>\"$(VISUALGDB_DIR)\\VisualGDB.exe\" /build \"$(ProjectPath)\" \"/solution:$(SolutionPath)\" \"/config:$(Configuration)\" \"/platform:$(Platform)\"</NMakeBuildCommandLine>" << Endl;
		os << L"<NMakeCleanCommandLine>\"$(VISUALGDB_DIR)\\VisualGDB.exe\" /clean \"$(ProjectPath)\" \"/solution:$(SolutionPath)\" \"/config:$(Configuration)\" \"/platform:$(Platform)\"</NMakeCleanCommandLine>" << Endl;
		os << L"<NMakeReBuildCommandLine>\"$(VISUALGDB_DIR)\\VisualGDB.exe\" /rebuild \"$(ProjectPath)\" \"/solution:$(SolutionPath)\" \"/config:$(Configuration)\" \"/platform:$(Platform)\"</NMakeReBuildCommandLine>" << Endl;
		os << L"<NMakeOutput>$(ProjectDir)" << project->getName() << L"-" << configuration->getName() << L".vgdbsettings</NMakeOutput>" << Endl;

		os << L"<IncludePath />" << Endl;
		os << L"<ReferencePath />" << Endl;
		os << L"<LibraryPath />" << Endl;

		os << DecreaseIndent;
		os << L"</PropertyGroup>" << Endl;
	}

	// Build definitions.
	for (RefArray< Configuration >::const_iterator i = configurations.begin(); i != configurations.end(); ++i)
	{
		Ref< const Configuration > configuration = *i;
		std::wstring name = configuration->getName();

		os << L"<ItemDefinitionGroup Condition=\"'$(Configuration)|$(Platform)'=='" << name << L"|" << m_platform << L"'\">" << Endl;
		os << IncreaseIndent;

		int32_t format = int32_t(configuration->getTargetFormat());

		const RefArray< SolutionBuilderMsvcVCXDefinition >& buildDefinitions = (configuration->getTargetProfile() == Configuration::TpDebug) ? m_buildDefinitionsDebug[format] : m_buildDefinitionsRelease[format];
		for (RefArray< SolutionBuilderMsvcVCXDefinition >::const_iterator j = buildDefinitions.begin(); j != buildDefinitions.end(); ++j)
		{
			(*j)->generate(
				context,
				solution,
				project,
				configuration,
				os
			);
		}

		os << DecreaseIndent;
		os << L"</ItemDefinitionGroup>" << Endl;
	}

	// Collect all files.
	std::vector< std::pair< std::wstring, Path > > files;
	const RefArray< ProjectItem >& items = project->getItems();
	for (RefArray< ProjectItem >::const_iterator i = items.begin(); i != items.end(); ++i)
	{
		if (!SolutionBuilderMsvcVCXProj::collectFiles(project, *i, L"", files))
			return false;
	}

	// Create item groups.
	for (RefArray< SolutionBuilderMsvcVCXBuildTool >::const_iterator i = m_buildTools.begin(); i != m_buildTools.end(); ++i)
	{
		os << L"<ItemGroup>" << Endl;
		os << IncreaseIndent;

		for (std::vector< std::pair< std::wstring, Path > >::const_iterator j = files.begin(); j != files.end(); ++j)
		{
			Path itemPath;
			FileSystem::getInstance().getRelativePath(
				FileSystem::getInstance().getAbsolutePath(j->second),
				FileSystem::getInstance().getAbsolutePath(projectPath),
				itemPath
			);
			(*i)->generateProject(
				context,
				solution,
				project,
				j->first,
				itemPath,
				os
			);
		}

		os << DecreaseIndent;
		os << L"</ItemGroup>" << Endl;
	}

	os << L"<Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.targets\" />" << Endl;

	// Dependencies.
	os << L"<ItemGroup>" << Endl;
	os << IncreaseIndent;

	const RefArray< Dependency >& dependencies = project->getDependencies();
	for (RefArray< Dependency >::const_iterator i = dependencies.begin(); i != dependencies.end(); ++i)
	{
		Ref< const ProjectDependency > projectDependency = dynamic_type_cast< const ProjectDependency* >(*i);
		if (projectDependency)
		{
			Ref< Project > dependentProject = projectDependency->getProject();
			T_ASSERT (dependentProject);

			std::wstring dependentProjectPath;
			std::wstring dependentProjectFileName;
			std::wstring dependentProjectGuid;

			if (!getInformation(
				context,
				solution,
				dependentProject,
				dependentProjectPath,
				dependentProjectFileName,
				dependentProjectGuid
			))
				return false;

			Path relativePath;
			FileSystem::getInstance().getRelativePath(
				dependentProjectFileName,
				projectPath,
				relativePath
			);

			os << L"<ProjectReference Include=\"" << systemPath(relativePath.getPathName()) << L"\">" << Endl;
			os << IncreaseIndent;
			os << L"<Project>" << dependentProjectGuid << L"</Project>" << Endl;
			os << DecreaseIndent;
			os << L"</ProjectReference>" << Endl;
		}
	}

	os << DecreaseIndent;
	os << L"</ItemGroup>" << Endl;

	os << DecreaseIndent;
	os << L"</Project>" << Endl;

	os.close();

	if (!buffer.empty())
	{
		Ref< IStream > file = FileSystem::getInstance().open(
			projectFileName,
			traktor::File::FmWrite
		);
		if (!file)
			return false;
		file->write(&buffer[0], int(buffer.size()));
		file->close();
	}

	return true;
}

bool SolutionBuilderMsvcVCXProjVGDB::generateMakefiles(
	GeneratorContext& context,
	Solution* solution,
	Project* project,
	const VGDBCredentials* credentials
) const
{
	std::wstring projectPath, projectFileName, projectGuid;
	if (!getInformation(
		context,
		solution,
		project,
		projectPath,
		projectFileName,
		projectGuid
	))
		return false;

	if (!FileSystem::getInstance().makeDirectory(projectPath))
		return false;

	traktor::log::info << L"Generating VisualGDB makefiles..." << Endl;

	// Collect all files.
	std::vector< std::pair< std::wstring, Path > > files;
	const RefArray< ProjectItem >& items = project->getItems();
	for (RefArray< ProjectItem >::const_iterator i = items.begin(); i != items.end(); ++i)
	{
		if (!SolutionBuilderMsvcVCXProj::collectFiles(project, *i, L"", files))
			return false;
	}

	{
		std::vector< uint8_t > buffer;
		buffer.reserve(40000);

		DynamicMemoryStream bufferStream(buffer, false, true);
		FileOutputStream os(&bufferStream, new AnsiEncoding());

		os << L"#Generated by VisualGDB project wizard. " << Endl;
		os << L"#Note: VisualGDB will automatically update this file when you add new sources to the project." << Endl;
		os << L"#All other changes you make in this file will be preserved." << Endl;
		os << L"#Visit http://visualgdb.com/makefiles for more details" << Endl;
		os << Endl;

		os << L"TARGETNAME := " << project->getName() << Endl;
		os << Endl;

		os << L"to_lowercase = $(subst A,a,$(subst B,b,$(subst C,c,$(subst D,d,$(subst E,e,$(subst F,f,$(subst G,g,$(subst H,h,$(subst I,i,$(subst J,j,$(subst K,k,$(subst L,l,$(subst M,m,$(subst N,n,$(subst O,o,$(subst P,p,$(subst Q,q,$(subst R,r,$(subst S,s,$(subst T,t,$(subst U,u,$(subst V,v,$(subst W,w,$(subst X,x,$(subst Y,y,$(subst Z,z,$1))))))))))))))))))))))))))" << Endl;
		os << Endl;

		os << L"CONFIG ?= DEBUG" << Endl;
		os << Endl;

		os << L"CONFIGURATION_FLAGS_FILE := $(call to_lowercase,$(CONFIG)).mak" << Endl;
		os << Endl;

		os << L"include $(CONFIGURATION_FLAGS_FILE)" << Endl;
		os << L"include $(ADDITIONAL_MAKE_FILES)" << Endl;
		os << Endl;

		os << L"SOURCEFILES :=";
		for (std::vector< std::pair< std::wstring, Path > >::const_iterator i = files.begin(); i != files.end(); ++i)
		{
			std::wstring ext = toLower(i->second.getExtension());
			if (ext == L"c" || ext == L"cpp")
			{
				Path sourcePath;

				// Transform file's path to being relative to project path. Files are relative to source path by convention.
				FileSystem::getInstance().getRelativePath(
					FileSystem::getInstance().getAbsolutePath(i->second),
					projectPath,
					sourcePath
				);

				os << L" " << sourcePath.getPathName();
			}
		}
		os << Endl;
		os << Endl;

		os << L"CFLAGS += $(COMMONFLAGS)" << Endl;
		os << L"CXXFLAGS += $(COMMONFLAGS)" << Endl;
		os << L"ASFLAGS += $(COMMONFLAGS)" << Endl;
		os << L"LDFLAGS += $(COMMONFLAGS)" << Endl;
		os << Endl;
		os << L"CFLAGS += $(addprefix -I,$(INCLUDE_DIRS))" << Endl;
		os << L"CXXFLAGS += $(addprefix -I,$(INCLUDE_DIRS))" << Endl;
		os << Endl;
		os << L"CFLAGS += $(addprefix -D,$(PREPROCESSOR_MACROS))" << Endl;
		os << L"CXXFLAGS += $(addprefix -D,$(PREPROCESSOR_MACROS))" << Endl;
		os << L"ASFLAGS += $(addprefix -D,$(PREPROCESSOR_MACROS))" << Endl;
		os << Endl;
		os << L"CXXFLAGS += $(addprefix -framework ,$(MACOS_FRAMEWORKS))" << Endl;
		os << L"CFLAGS += $(addprefix -framework ,$(MACOS_FRAMEWORKS))" << Endl;
		os << L"LDFLAGS += $(addprefix -framework ,$(MACOS_FRAMEWORKS))" << Endl;
		os << Endl;
		os << L"LDFLAGS += $(addprefix -L,$(LIBRARY_DIRS))" << Endl;
		os << Endl;
		os << L"LIBRARY_LDFLAGS = $(addprefix -l,$(LIBRARY_NAMES))" << Endl;
		os << Endl;

		os << L"ifeq ($(IS_LINUX_PROJECT),1)" << Endl;
		os << L"	LIBRARY_LDFLAGS += $(EXTERNAL_LIBS)" << Endl;
		os << L"	LIBRARY_LDFLAGS += -Wl,--rpath='$$ORIGIN'" << Endl;
		os << L"	LIBRARY_LDFLAGS += -Wl,--rpath-link='$(OUTPUTDIR)'" << Endl;
		os << Endl;
		os << L"	ifeq ($(TARGETTYPE),SHARED)" << Endl;
		os << L"		LIBRARY_LDFLAGS += -Wl,-soname,$(TARGETNAME)" << Endl;
		os << L"	endif" << Endl;
		os << Endl;
		os << L"else" << Endl;
		os << L"	LIBRARY_LDFLAGS += $(EXTERNAL_LIBS)" << Endl;
		os << L"endif" << Endl;
		os << Endl;

		os << L"LIBRARY_LDFLAGS += $(ADDITIONAL_LINKER_INPUTS)" << Endl;
		os << Endl;

		os << L"ifeq ($(STARTUPFILES),)" << Endl;
		os << L"	all_source_files := $(SOURCEFILES)" << Endl;
		os << L"else" << Endl;
		os << L"	all_source_files := $(STARTUPFILES) $(filter-out $(STARTUPFILES),$(SOURCEFILES))" << Endl;
		os << L"endif" << Endl;
		os << Endl;

		os << L"source_obj1 := $(all_source_files:.cpp=.o)" << Endl;
		os << L"source_obj2 := $(source_obj1:.c=.o)" << Endl;
		os << L"source_obj3 := $(source_obj2:.s=.o)" << Endl;
		os << L"source_objs := $(source_obj3:.S=.o)" << Endl;
		os << Endl;

		os << L"all_objs := $(addprefix $(BINARYDIR)/, $(notdir $(source_objs)))" << Endl;
		os << Endl;

		os << L"all: $(OUTPUTDIR) $(BINARYDIR) $(OUTPUTDIR)/$(TARGETNAME)" << Endl;
		os << Endl;

		os << L"$(OUTPUTDIR):" << Endl;
		os << L"	$(MKDIR) $(OUTPUTDIR) > /dev/null 2>&1" << Endl;
		os << Endl;

		os << L"$(BINARYDIR):" << Endl;
		os << L"	$(MKDIR) $(BINARYDIR) > /dev/null 2>&1" << Endl;
		os << Endl;

		os << L"ifeq ($(TARGETTYPE),APP)" << Endl;
		os << L"$(OUTPUTDIR)/$(TARGETNAME): $(all_objs)" << Endl;
		os << L"	$(LD) -o $@ $(LDFLAGS) $(START_GROUP) $(all_objs) $(LIBRARY_LDFLAGS) $(END_GROUP)" << Endl;
		os << L"endif" << Endl;
		os << Endl;

		os << L"ifeq ($(TARGETTYPE),SHARED)" << Endl;
		os << L"$(OUTPUTDIR)/$(TARGETNAME): $(all_objs)" << Endl;
		os << L"	$(LD) -shared -o $@ $(LDFLAGS) $(START_GROUP) $(all_objs) $(LIBRARY_LDFLAGS) $(END_GROUP)" << Endl;
		os << L"endif" << Endl;
		os << Endl;

		os << L"ifeq ($(TARGETTYPE),STATIC)" << Endl;
		os << L"$(OUTPUTDIR)/$(TARGETNAME): $(all_objs)" << Endl;
		os << L"	$(AR) -r $@ $^" << Endl;
		os << L"endif" << Endl;
		os << Endl;

		os << L"-include $(all_objs:.o=.dep)" << Endl;
		os << Endl;

		os << L"clean:" << Endl;
		os << L"ifeq ($(USE_DEL_TO_CLEAN),1)" << Endl;
		os << L"	del /S /Q $(BINARYDIR)" << Endl;
		os << L"else" << Endl;
		os << L"	rm -rf $(BINARYDIR)" << Endl;
		os << L"endif" << Endl;
		os << Endl;

		os << L"$(BINARYDIR):" << Endl;
		os << L"	mkdir $(BINARYDIR)" << Endl;
		os << Endl;

		for (std::vector< std::pair< std::wstring, Path > >::const_iterator i = files.begin(); i != files.end(); ++i)
		{
			std::wstring ext = toLower(i->second.getExtension());
			if (ext == L"c" || ext == L"cpp")
			{
				Path sourcePath;

				// Transform file's path to being relative to project path. Files are relative to source path by convention.
				FileSystem::getInstance().getRelativePath(
					FileSystem::getInstance().getAbsolutePath(i->second),
					projectPath,
					sourcePath
				);

				if (ext == L"c")
				{
					os << L"$(BINARYDIR)/" << sourcePath.getFileNameNoExtension() << L".o : " << sourcePath.getPathName() << Endl;
					os << L"	$(CC) $(CFLAGS) -c $< -o $@ -MD -MF $(@:.o=.dep)" << Endl;
				}
				else
				{
					os << L"$(BINARYDIR)/" << sourcePath.getFileNameNoExtension() << L".o : " << sourcePath.getPathName() << Endl;
					os << L"	$(CXX) $(CXXFLAGS) -c $< -o $@ -MD -MF $(@:.o=.dep)" << Endl;
				}

				os << Endl;
			}
		}

		os.close();

		if (!buffer.empty())
		{
			Ref< IStream > file = FileSystem::getInstance().open(
				projectPath + L"/Makefile",
				traktor::File::FmWrite
			);
			if (!file)
				return false;
			file->write(&buffer[0], int(buffer.size()));
			file->close();
		}
	}

	const RefArray< Configuration >& configurations = project->getConfigurations();
	for (RefArray< Configuration >::const_iterator i = configurations.begin(); i != configurations.end(); ++i)
	{
		const Configuration* configuration = *i;
		T_ASSERT (configuration);

		std::wstring includeDirectories = L"";
		std::wstring definitions = L"";
		std::wstring libraries = L"";
		std::wstring libraryPaths = L"";
		std::wstring additionalCompileOptions = L"";
		std::wstring additionalLinkOptions = L"";

		// Include directories.
		for (std::vector< std::wstring >::const_iterator i = configuration->getIncludePaths().begin(); i != configuration->getIncludePaths().end(); ++i)
		{
			if (startsWith< std::wstring >(*i, L"`"))
				additionalCompileOptions += *i + L" ";
			else
			{
				Path relativePath;
				FileSystem& fileSystem = FileSystem::getInstance();
				if (fileSystem.getRelativePath(
					fileSystem.getAbsolutePath(*i),
					fileSystem.getAbsolutePath(projectPath),
					relativePath
				))
					includeDirectories += relativePath.getPathName() + L" ";
			}
		}

		// Preprocessor definitions.
		for (std::vector< std::wstring >::const_iterator i = solution->getDefinitions().begin(); i != solution->getDefinitions().end(); ++i)
			definitions += *i + L" ";
		for (std::vector< std::wstring >::const_iterator i = configuration->getDefinitions().begin(); i != configuration->getDefinitions().end(); ++i)
			definitions += *i + L" ";

		// Libraries.
		for (std::vector< std::wstring >::const_iterator i = configuration->getLibraries().begin(); i != configuration->getLibraries().end(); ++i)
		{
			if (startsWith< std::wstring >(*i, L"`"))
				additionalLinkOptions += *i + L" ";
			else
				libraries += *i + L" ";
		}

		for (std::vector< std::wstring >::const_iterator i = configuration->getLibraryPaths().begin(); i != configuration->getLibraryPaths().end(); ++i)
		{
			if (startsWith< std::wstring >(*i, L"`"))
				additionalLinkOptions += *i + L" ";
			else
			{
				Path relativePath;
				FileSystem& fileSystem = FileSystem::getInstance();
				if (fileSystem.getRelativePath(
					fileSystem.getAbsolutePath(*i),
					fileSystem.getAbsolutePath(projectPath),
					relativePath
				))
					libraryPaths += relativePath.getPathName() + L" ";
			}
		}

		std::vector< uint8_t > buffer;
		buffer.reserve(40000);

		DynamicMemoryStream bufferStream(buffer, false, true);
		FileOutputStream os(&bufferStream, new AnsiEncoding());

		if (configuration->getTargetProfile() == Configuration::TpDebug)
		{
			os << L"#Generated by VisualGDB (http://visualgdb.com)" << Endl;
			os << L"#DO NOT EDIT THIS FILE MANUALLY UNLESS YOU ABSOLUTELY NEED TO" << Endl;
			os << L"#USE VISUALGDB PROJECT PROPERTIES DIALOG INSTEAD" << Endl;
			os << Endl;

			os << L"BINARYDIR := " << configuration->getName() << Endl;
			os << Endl;

			os << L"OUTPUTDIR := ../" << configuration->getName() << Endl;
			switch (configuration->getTargetFormat())
			{
			case Configuration::TfStaticLibrary:
				os << L"TARGETNAME := lib" << project->getName() << L"_d.a" << Endl;
				os << L"TARGETTYPE := STATIC" << Endl;
				break;

			case Configuration::TfSharedLibrary:
				os << L"TARGETNAME := lib" << project->getName() << L"_d.so" << Endl;
				os << L"TARGETTYPE := SHARED" << Endl;
				break;

			case Configuration::TfExecutable:
			case Configuration::TfExecutableConsole:
				os << L"TARGETNAME := " << project->getName() << Endl;
				os << L"TARGETTYPE := APP" << Endl;
				break;
			}
			os << Endl;

			std::vector< std::wstring > externalLibraries;
			collectLinkDependencies(solution, project, configuration, externalLibraries);

			os << L"EXTERNAL_LIBS :=";
			for (std::vector< std::wstring >::const_iterator i = externalLibraries.begin(); i != externalLibraries.end(); ++i)
				os << L" " << *i;
			os << Endl;
			os << Endl;

			os << L"#Toolchain" << Endl;
			os << L"CC := gcc" << Endl;
			os << L"CXX := g++" << Endl;
			os << L"LD := $(CXX)" << Endl;
			os << L"AR := ar" << Endl;
			os << L"OBJCOPY := objcopy" << Endl;
			os << L"MKDIR := mkdir" << Endl;
			os << Endl;

			os << L"#Additional flags" << Endl;
			os << L"PREPROCESSOR_MACROS := __LINUX__ " << definitions << Endl;
			os << L"INCLUDE_DIRS := . " << includeDirectories << Endl;
			os << L"LIBRARY_NAMES := " << libraries << Endl;
			os << L"LIBRARY_DIRS := " << libraryPaths << Endl;
			os << Endl;

			os << L"CFLAGS := -ggdb -ffunction-sections -march=core2 -fPIC -O0 " << additionalCompileOptions << Endl;
			os << L"CXXFLAGS := -ggdb -ffunction-sections -march=core2 -fPIC -O0 " << additionalCompileOptions << Endl;
			os << L"ASFLAGS := " << Endl;
			os << L"LDFLAGS := -Wl,-gc-sections " << additionalLinkOptions << Endl;
			os << L"COMMONFLAGS := " << Endl;
			os << Endl;

			os << L"START_GROUP := -Wl,--start-group" << Endl;
			os << L"END_GROUP := -Wl,--end-group" << Endl;
			os << Endl;

			os << L"#Additional options detected from testing the toolchain" << Endl;
			os << L"IS_LINUX_PROJECT := 1" << Endl;
		}
		else
		{
			os << L"#Generated by VisualGDB (http://visualgdb.com)" << Endl;
			os << L"#DO NOT EDIT THIS FILE MANUALLY UNLESS YOU ABSOLUTELY NEED TO" << Endl;
			os << L"#USE VISUALGDB PROJECT PROPERTIES DIALOG INSTEAD" << Endl;
			os << Endl;

			os << L"BINARYDIR := " << configuration->getName() << Endl;
			os << Endl;

			os << L"OUTPUTDIR := ../" << configuration->getName() << Endl;
			switch (configuration->getTargetFormat())
			{
			case Configuration::TfStaticLibrary:
				os << L"TARGETNAME := lib" << project->getName() << L".a" << Endl;
				os << L"TARGETTYPE := STATIC" << Endl;
				break;

			case Configuration::TfSharedLibrary:
				os << L"TARGETNAME := lib" << project->getName() << L".so" << Endl;
				os << L"TARGETTYPE := SHARED" << Endl;
				break;

			case Configuration::TfExecutable:
			case Configuration::TfExecutableConsole:
				os << L"TARGETNAME := " << project->getName() << Endl;
				os << L"TARGETTYPE := APP" << Endl;
				break;
			}
			os << Endl;

			std::vector< std::wstring > externalLibraries;
			collectLinkDependencies(solution, project, configuration, externalLibraries);

			os << L"EXTERNAL_LIBS :=";
			for (std::vector< std::wstring >::const_iterator i = externalLibraries.begin(); i != externalLibraries.end(); ++i)
				os << L" " << *i;
			os << Endl;
			os << Endl;

			os << L"#Toolchain" << Endl;
			os << L"CC := gcc" << Endl;
			os << L"CXX := g++" << Endl;
			os << L"LD := $(CXX)" << Endl;
			os << L"AR := ar" << Endl;
			os << L"OBJCOPY := objcopy" << Endl;
			os << L"MKDIR := mkdir" << Endl;
			os << Endl;

			os << L"#Additional flags" << Endl;
			os << L"PREPROCESSOR_MACROS := __LINUX__ " << definitions << Endl;
			os << L"INCLUDE_DIRS := . " << includeDirectories << Endl;
			os << L"LIBRARY_NAMES := " << libraries << Endl;
			os << L"LIBRARY_DIRS := " << libraryPaths << Endl;
			os << Endl;

			os << L"CFLAGS := -ggdb -ffunction-sections -march=core2 -fPIC -O3 " << additionalCompileOptions << Endl;
			os << L"CXXFLAGS := -ggdb -ffunction-sections -march=core2 -fPIC -O3 " << additionalCompileOptions << Endl;
			os << L"ASFLAGS := " << Endl;
			os << L"LDFLAGS := -Wl,-gc-sections " << additionalLinkOptions << Endl;
			os << L"COMMONFLAGS := " << Endl;
			os << Endl;

			os << L"START_GROUP := -Wl,--start-group" << Endl;
			os << L"END_GROUP := -Wl,--end-group" << Endl;
			os << Endl;

			os << L"#Additional options detected from testing the toolchain" << Endl;
			os << L"IS_LINUX_PROJECT := 1" << Endl;
		}

		os.close();

		if (!buffer.empty())
		{
			Ref< IStream > file = FileSystem::getInstance().open(
				projectPath + L"/" + toLower(configuration->getName()) + L".mak",
				traktor::File::FmWrite
			);
			if (!file)
				return false;
			file->write(&buffer[0], int(buffer.size()));
			file->close();
		}
	}

	for (RefArray< Configuration >::const_iterator i = configurations.begin(); i != configurations.end(); ++i)
	{
		const Configuration* configuration = *i;
		T_ASSERT (configuration);

		Ref< xml::Document > current;

		// Read existing settings file; so we can keep some debug changes etc.
		Ref< IStream > file = FileSystem::getInstance().open(
			projectPath + L"/" + project->getName() + L"-" + configuration->getName() + L".vgdbsettings",
			traktor::File::FmRead
		);
		if (file)
		{
			current = new xml::Document();
			if (!current->loadFromStream(file))
				current = 0;

			file->close();
			file = 0;
		}

		std::vector< uint8_t > buffer;
		buffer.reserve(40000);

		DynamicMemoryStream bufferStream(buffer, false, true);
		FileOutputStream os(&bufferStream, new AnsiEncoding());

		os << L"<?xml version=\"1.0\"?>" << Endl;
		os << L"<VisualGDBProjectSettings2 xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">" << Endl;
		os << L"  <ConfigurationName>" << configuration->getName() << L"</ConfigurationName>" << Endl;
		os << L"  <Project xsi:type=\"com.visualgdb.project.linux\">" << Endl;
		os << L"    <CustomSourceDirectories>" << Endl;
		os << L"      <Directories />" << Endl;
		os << L"      <PathStyle>MinGWUnixSlash</PathStyle>" << Endl;
		os << L"    </CustomSourceDirectories>" << Endl;
		os << L"    <BuildHost>" << Endl;
		os << L"      <HostName>" << credentials->getHost() << L"</HostName>" << Endl;
		os << L"      <Transport>SSH</Transport>" << Endl;
		os << L"      <UserName>" << credentials->getUser() << L"</UserName>" << Endl;
		os << L"    </BuildHost>" << Endl;

		if (credentials->getLocalPath().empty())
		{
			os << L"    <MainSourceTransferCommand>" << Endl;
			os << L"      <SkipWhenRunningCommandList>false</SkipWhenRunningCommandList>" << Endl;
			os << L"      <RemoteHost>" << Endl;
			os << L"        <HostName>" << credentials->getHost() << L"</HostName>" << Endl;
			os << L"        <Transport>SSH</Transport>" << Endl;
			os << L"        <UserName>" << credentials->getUser() << L"</UserName>" << Endl;
			os << L"      </RemoteHost>" << Endl;
			os << L"      <LocalDirectory>$(ProjectDir)</LocalDirectory>" << Endl;
			os << L"      <RemoteDirectory>/tmp/VisualGDB/$(ProjectDirUnixStyle)</RemoteDirectory>" << Endl;
			os << L"      <FileMasks>" << Endl;
			os << L"        <string>*.mak</string>" << Endl;
			os << L"        <string>Makefile</string>" << Endl;
			os << L"      </FileMasks>" << Endl;
			os << L"      <TransferNewFilesOnly>true</TransferNewFilesOnly>" << Endl;
			os << L"      <IncludeSubdirectories>true</IncludeSubdirectories>" << Endl;
			os << L"    </MainSourceTransferCommand>" << Endl;
		}
		else
		{
			Path relativePath;
			FileSystem::getInstance().getRelativePath(
				projectPath,
				credentials->getLocalPath(),
				relativePath
			);

			os << L"    <MountInfo>" << Endl;
			os << L"      <Mode>UserProvidedMount</Mode>" << Endl;
			os << L"      <RemoteHost>" << Endl;
			os << L"        <HostName>" << credentials->getHost() << L"</HostName>" << Endl;
			os << L"        <Transport>SSH</Transport>" << Endl;
			os << L"        <UserName>" << credentials->getUser() << L"</UserName>" << Endl;
			os << L"      </RemoteHost>" << Endl;
			os << L"      <LocalDirectory>" << credentials->getLocalPath() << L"\\" << relativePath.getPathName() << L"</LocalDirectory>" << Endl;
			os << L"      <RemoteDirectory>" << credentials->getRemotePath() << L"/" << relativePath.getPathName() << L"</RemoteDirectory>" << Endl;
			os << L"    </MountInfo>" << Endl;
		}

		os << L"    <AllowChangingHostForMainCommands>false</AllowChangingHostForMainCommands>" << Endl;
		os << L"    <SkipBuildIfNoSourceFilesChanged>false</SkipBuildIfNoSourceFilesChanged>" << Endl;
		os << L"  </Project>" << Endl;
		os << L"  <Build xsi:type=\"com.visualgdb.build.make\">" << Endl;
		os << L"    <MakeFilePathRelativeToSourceDir>Makefile</MakeFilePathRelativeToSourceDir>" << Endl;
		os << L"    <MakeConfigurationName>" << configuration->getName() << L"</MakeConfigurationName>" << Endl;
		os << L"    <Toolchain>" << Endl;
		os << L"      <Name>Default GCC toolchain on " << credentials->getHost() << L"</Name>" << Endl;
		os << L"      <UnixSystem>true</UnixSystem>" << Endl;
		os << L"      <GCC>gcc</GCC>" << Endl;
		os << L"      <GXX>g++</GXX>" << Endl;
		os << L"      <GDB>gdb</GDB>" << Endl;
		os << L"      <AR>ar</AR>" << Endl;
		os << L"      <OBJCOPY>objcopy</OBJCOPY>" << Endl;
		os << L"      <Make>make</Make>" << Endl;
		os << L"      <RequireCmdExe>false</RequireCmdExe>" << Endl;
		os << L"      <RequireCtrlBreak>false</RequireCtrlBreak>" << Endl;
		os << L"      <RequiredLinuxPackages>" << Endl;
		os << L"        <TestableLinuxPackage>" << Endl;
		os << L"          <UserFriendlyName>GCC</UserFriendlyName>" << Endl;
		os << L"          <DefaultPackageName>g++</DefaultPackageName>" << Endl;
		os << L"          <AlternativeRPMPackageName>gcc-c++</AlternativeRPMPackageName>" << Endl;
		os << L"          <BinaryToCheck>g++</BinaryToCheck>" << Endl;
		os << L"        </TestableLinuxPackage>" << Endl;
		os << L"        <TestableLinuxPackage>" << Endl;
		os << L"          <UserFriendlyName>GDB</UserFriendlyName>" << Endl;
		os << L"          <DefaultPackageName>gdb</DefaultPackageName>" << Endl;
		os << L"          <BinaryToCheck>gdb</BinaryToCheck>" << Endl;
		os << L"        </TestableLinuxPackage>" << Endl;
		os << L"        <TestableLinuxPackage>" << Endl;
		os << L"          <UserFriendlyName>GNU Make</UserFriendlyName>" << Endl;
		os << L"          <DefaultPackageName>make</DefaultPackageName>" << Endl;
		os << L"          <BinaryToCheck>make</BinaryToCheck>" << Endl;
		os << L"        </TestableLinuxPackage>" << Endl;
		os << L"      </RequiredLinuxPackages>" << Endl;
		os << L"    </Toolchain>" << Endl;
		os << L"    <MakeCommandTemplate>" << Endl;
		os << L"      <SkipWhenRunningCommandList>false</SkipWhenRunningCommandList>" << Endl;
		os << L"      <Command>make</Command>" << Endl;
		os << L"      <WorkingDirectory>/tmp/VisualGDB/$(ProjectDirUnixStyle)</WorkingDirectory>" << Endl;
		os << L"      <Environment>" << Endl;
		os << L"        <Records>" << Endl;
		os << L"          <Record>" << Endl;
		os << L"            <VariableName>LANG</VariableName>" << Endl;
		os << L"            <Value>en_US.UTF-8</Value>" << Endl;
		os << L"          </Record>" << Endl;
		os << L"        </Records>" << Endl;
		os << L"      </Environment>" << Endl;
		os << L"    </MakeCommandTemplate>" << Endl;
		os << L"  </Build>" << Endl;
		os << L"  <Debug xsi:type=\"com.visualgdb.debug.remote\">" << Endl;
		os << L"    <AdditionalStartupCommands />" << Endl;
		os << L"    <AdditionalGDBSettings>" << Endl;
		os << L"      <FilterSpuriousStoppedNotifications>false</FilterSpuriousStoppedNotifications>" << Endl;
		os << L"      <ForceSingleThreadedMode>false</ForceSingleThreadedMode>" << Endl;
		os << L"      <PendingBreakpointsSupported>true</PendingBreakpointsSupported>" << Endl;
		os << L"      <DisableChildRanges>false</DisableChildRanges>" << Endl;
		os << L"      <UseAppleExtensions>false</UseAppleExtensions>" << Endl;
		os << L"      <CanAcceptCommandsWhileRunning>false</CanAcceptCommandsWhileRunning>" << Endl;
		os << L"      <MakeLogFile>false</MakeLogFile>" << Endl;
		os << L"      <IgnoreModuleEventsWhileStepping>true</IgnoreModuleEventsWhileStepping>" << Endl;
		os << L"      <UseRelativePathsOnly>false</UseRelativePathsOnly>" << Endl;
		os << L"      <ExitAction>None</ExitAction>" << Endl;
		os << L"      <Features>" << Endl;
		os << L"        <DisableAutoDetection>false</DisableAutoDetection>" << Endl;
		os << L"        <UseFrameParameter>false</UseFrameParameter>" << Endl;
		os << L"        <SimpleValuesFlagSupported>false</SimpleValuesFlagSupported>" << Endl;
		os << L"        <ListLocalsSupported>false</ListLocalsSupported>" << Endl;
		os << L"        <ByteLevelMemoryCommandsAvailable>false</ByteLevelMemoryCommandsAvailable>" << Endl;
		os << L"        <ThreadInfoSupported>false</ThreadInfoSupported>" << Endl;
		os << L"        <PendingBreakpointsSupported>false</PendingBreakpointsSupported>" << Endl;
		os << L"        <SupportTargetCommand>false</SupportTargetCommand>" << Endl;
		os << L"      </Features>" << Endl;
		os << L"      <DisableDisassembly>false</DisableDisassembly>" << Endl;
		os << L"      <ExamineMemoryWithXCommand>false</ExamineMemoryWithXCommand>" << Endl;
		os << L"      <StepIntoNewInstanceEntry>main</StepIntoNewInstanceEntry>" << Endl;
		os << L"      <ExamineRegistersInRawFormat>true</ExamineRegistersInRawFormat>" << Endl;
		os << L"    </AdditionalGDBSettings>" << Endl;

		os << L"    <LaunchGDBSettings xsi:type=\"GDBLaunchParametersNewInstance\">" << Endl;
		os << L"      <GDBEnvironment>" << Endl;
		os << L"        <Records>" << Endl;
		os << L"          <Record>" << Endl;
		os << L"            <VariableName>LANG</VariableName>" << Endl;
		os << L"            <Value>en_US.UTF-8</Value>" << Endl;
		os << L"          </Record>" << Endl;
		os << L"        </Records>" << Endl;
		os << L"      </GDBEnvironment>" << Endl;

		std::wstring binaryOutputPath;
		if (credentials->getRemotePath().empty())
			binaryOutputPath = L"/tmp/VisualGDB/$(ProjectDirUnixStyle)/../" + configuration->getName();
		else
		{
			Path relativePath;
			FileSystem::getInstance().getRelativePath(
				projectPath,
				credentials->getLocalPath(),
				relativePath
			);
			binaryOutputPath = credentials->getRemotePath() + L"/" + relativePath.getPathName() + L"/" + configuration->getName();
		}

		os << L"      <DebuggedProgram>" << getElementValue(current, L"/VisualGDBProjectSettings2/Debug/LaunchGDBSettings/DebuggedProgram", binaryOutputPath + L"/" + project->getName()) << L"</DebuggedProgram>" << Endl;
		os << L"      <GDBServerPort>2000</GDBServerPort>" << Endl;
		os << L"      <ProgramArguments>" << getElementValue(current, L"/VisualGDBProjectSettings2/Debug/LaunchGDBSettings/ProgramArguments", L"") << L"</ProgramArguments>" << Endl;
		os << L"      <WorkingDirectory>" << getElementValue(current, L"/VisualGDBProjectSettings2/Debug/LaunchGDBSettings/WorkingDirectory", binaryOutputPath) << L"</WorkingDirectory>" << Endl;
		os << L"    </LaunchGDBSettings>" << Endl;

		os << L"    <GenerateCtrlBreakInsteadOfCtrlC>false</GenerateCtrlBreakInsteadOfCtrlC>" << Endl;
		os << L"    <X11WindowMode>" << getElementValue(current, L"/VisualGDBProjectSettings2/Debug/X11WindowMode", L"Remote") << L"</X11WindowMode>" << Endl;
		os << L"    <KeepConsoleAfterExit>false</KeepConsoleAfterExit>" << Endl;
		os << L"    <RunGDBUnderSudo>false</RunGDBUnderSudo>" << Endl;
		os << L"    <SkipDeployment>false</SkipDeployment>" << Endl;
		os << L"    <LdLibraryPath>" << getElementValue(current, L"/VisualGDBProjectSettings2/Debug/LdLibraryPath", L"") << L"</LdLibraryPath>" << Endl;
		os << L"  </Debug>" << Endl;
		os << L"  <CustomBuild>" << Endl;
		os << L"    <PreBuildActions>" << Endl;

		if (credentials->getLocalPath().empty())
		{
			// Add default source transfer.
			Path sourcePath;
			FileSystem::getInstance().getRelativePath(
				FileSystem::getInstance().getAbsolutePath(project->getSourcePath()),
				projectPath,
				sourcePath
			);

			os << L"      <CustomActionBase xsi:type=\"SourceTransferAction\">" << Endl;
			os << L"        <SkipWhenRunningCommandList>false</SkipWhenRunningCommandList>" << Endl;
			os << L"        <RemoteHost>" << Endl;
			os << L"          <HostName>" << credentials->getHost() << L"</HostName>" << Endl;
			os << L"          <Transport>SSH</Transport>" << Endl;
			os << L"          <UserName>" << credentials->getUser() << L"</UserName>" << Endl;
			os << L"        </RemoteHost>" << Endl;
			os << L"        <LocalDirectory>$(ProjectDir)/" << sourcePath.getPathName() << L"</LocalDirectory>" << Endl;
			os << L"        <RemoteDirectory>/tmp/VisualGDB/$(ProjectDirUnixStyle)/" << sourcePath.getPathName() << L"</RemoteDirectory>" << Endl;
			os << L"        <FileMasks>" << Endl;
			os << L"          <string>*.cpp</string>" << Endl;
			os << L"          <string>*.c</string>" << Endl;
			os << L"          <string>*.hpp</string>" << Endl;
			os << L"          <string>*.h</string>" << Endl;
			os << L"          <string>*.in</string>" << Endl;
			os << L"          <string>*.inl</string>" << Endl;
			os << L"        </FileMasks>" << Endl;
			os << L"        <TransferNewFilesOnly>true</TransferNewFilesOnly>" << Endl;
			os << L"        <IncludeSubdirectories>true</IncludeSubdirectories>" << Endl;
			os << L"      </CustomActionBase>" << Endl;
		}

		// Setup actions to transfer source files.
		bool haveResources = false;
		const RefArray< ProjectItem >& items = project->getItems();
		for (RefArray< ProjectItem >::const_iterator i = items.begin(); i != items.end(); ++i)
		{
			std::vector< traktor::Path > files;
			collectFiles(project, *i, files);
			for (std::vector< traktor::Path >::const_iterator j = files.begin(); j != files.end(); ++j)
			{
				if (compareIgnoreCase< std::wstring >(j->getExtension(), L"png") == 0)
				{
					Path filePath = (Path(project->getSourcePath()) + *j).normalized();

					// First execute a custom build step locally to transform resource into a header.
					os << L"      <CustomActionBase xsi:type=\"CommandLineAction\">" << Endl;
					os << L"        <SkipWhenRunningCommandList>false</SkipWhenRunningCommandList>" << Endl;
					os << L"        <Command>$(TRAKTOR_HOME)/bin/win32/BinaryInclude</Command>" << Endl;
					os << L"        <Arguments>" << filePath.getPathName() << L" Resources/" << filePath.getFileNameNoExtension() << L".h c_Resource" << filePath.getFileNameNoExtension() << L"</Arguments>" << Endl;
					os << L"        <WorkingDirectory>$(ProjectDir)</WorkingDirectory>" << Endl;
					os << L"        <Environment>" << Endl;
					os << L"        <Records />" << Endl;
					os << L"        </Environment>" << Endl;
					os << L"      </CustomActionBase>" << Endl;

					haveResources = true;
				}
				else if (credentials->getLocalPath().empty())
				{
					Path sourcePath;

					// Transform file's path to being relative to project path. Files are relative to source path by convention.
					FileSystem::getInstance().getRelativePath(
						(Path(project->getSourcePath()) + *j).normalized(),
						projectPath,
						sourcePath
					);

					os << L"      <CustomActionBase xsi:type=\"SourceTransferAction\">" << Endl;
					os << L"        <SkipWhenRunningCommandList>false</SkipWhenRunningCommandList>" << Endl;
					os << L"        <RemoteHost>" << Endl;
					os << L"          <HostName>" << credentials->getHost() << L"</HostName>" << Endl;
					os << L"          <Transport>SSH</Transport>" << Endl;
					os << L"          <UserName>" << credentials->getUser() << L"</UserName>" << Endl;
					os << L"        </RemoteHost>" << Endl;
					os << L"        <LocalDirectory>$(ProjectDir)/" << sourcePath.getPathOnly() << L"</LocalDirectory>" << Endl;
					os << L"        <RemoteDirectory>/tmp/VisualGDB/$(ProjectDirUnixStyle)/" << sourcePath.getPathOnly() << L"</RemoteDirectory>" << Endl;
					os << L"        <FileMasks>" << Endl;
					os << L"          <string>" << sourcePath.getFileName() << L"</string>" << Endl;
					os << L"        </FileMasks>" << Endl;
					os << L"        <TransferNewFilesOnly>true</TransferNewFilesOnly>" << Endl;
					os << L"        <IncludeSubdirectories>false</IncludeSubdirectories>" << Endl;
					os << L"      </CustomActionBase>" << Endl;
				}
			}
		}

		if (credentials->getLocalPath().empty())
		{
			// Transfer converted resources.
			if (haveResources)
			{
				os << L"      <CustomActionBase xsi:type=\"SourceTransferAction\">" << Endl;
				os << L"        <SkipWhenRunningCommandList>false</SkipWhenRunningCommandList>" << Endl;
				os << L"        <RemoteHost>" << Endl;
				os << L"          <HostName>" << credentials->getHost() << L"</HostName>" << Endl;
				os << L"          <Transport>SSH</Transport>" << Endl;
				os << L"          <UserName>" << credentials->getUser() << L"</UserName>" << Endl;
				os << L"        </RemoteHost>" << Endl;
				os << L"        <LocalDirectory>$(ProjectDir)/Resources</LocalDirectory>" << Endl;
				os << L"        <RemoteDirectory>/tmp/VisualGDB/$(ProjectDirUnixStyle)/Resources</RemoteDirectory>" << Endl;
				os << L"        <FileMasks>" << Endl;
				os << L"          <string>*.h</string>" << Endl;
				os << L"        </FileMasks>" << Endl;
				os << L"        <TransferNewFilesOnly>true</TransferNewFilesOnly>" << Endl;
				os << L"        <IncludeSubdirectories>false</IncludeSubdirectories>" << Endl;
				os << L"      </CustomActionBase>" << Endl;
			}

			// Setup actions to transfer include files.
			const std::vector< std::wstring >& includePaths = configuration->getIncludePaths();
			for (std::vector< std::wstring >::const_iterator i = includePaths.begin(); i != includePaths.end(); ++i)
			{
				if (startsWith< std::wstring >(*i, L"`"))
					continue;

				// Transform include path to being relative to project path. Include paths are relative to source path by convention.
				Path sourcePath;
				FileSystem::getInstance().getRelativePath(
					(Path(project->getSourcePath()) + Path(*i)).normalized(),
					projectPath,
					sourcePath
				);

				os << L"      <CustomActionBase xsi:type=\"SourceTransferAction\">" << Endl;
				os << L"        <SkipWhenRunningCommandList>false</SkipWhenRunningCommandList>" << Endl;
				os << L"        <RemoteHost>" << Endl;
				os << L"          <HostName>" << credentials->getHost() << L"</HostName>" << Endl;
				os << L"          <Transport>SSH</Transport>" << Endl;
				os << L"          <UserName>" << credentials->getUser() << L"</UserName>" << Endl;
				os << L"        </RemoteHost>" << Endl;
				os << L"        <LocalDirectory>$(ProjectDir)/" << sourcePath.getPathName() << L"</LocalDirectory>" << Endl;
				os << L"        <RemoteDirectory>/tmp/VisualGDB/$(ProjectDirUnixStyle)/" << sourcePath.getPathName() << L"</RemoteDirectory>" << Endl;
				os << L"        <FileMasks>" << Endl;
				os << L"          <string>*.*</string>" << Endl;
				os << L"        </FileMasks>" << Endl;
				os << L"        <TransferNewFilesOnly>true</TransferNewFilesOnly>" << Endl;
				os << L"        <IncludeSubdirectories>true</IncludeSubdirectories>" << Endl;
				os << L"      </CustomActionBase>" << Endl;
			}

			// Setup action to transfer explicit libraries.
			const std::vector< std::wstring >& libraryPaths = configuration->getLibraryPaths();
			for (std::vector< std::wstring >::const_iterator i = libraryPaths.begin(); i != libraryPaths.end(); ++i)
			{
				if (startsWith< std::wstring >(*i, L"`"))
					continue;

				Path relativePath;
				FileSystem& fileSystem = FileSystem::getInstance();
				if (fileSystem.getRelativePath(
					fileSystem.getAbsolutePath(*i),
					fileSystem.getAbsolutePath(projectPath),
					relativePath
				))
				{
					os << L"      <CustomActionBase xsi:type=\"SourceTransferAction\">" << Endl;
					os << L"        <SkipWhenRunningCommandList>false</SkipWhenRunningCommandList>" << Endl;
					os << L"        <RemoteHost>" << Endl;
					os << L"          <HostName>" << credentials->getHost() << L"</HostName>" << Endl;
					os << L"          <Transport>SSH</Transport>" << Endl;
					os << L"          <UserName>" << credentials->getUser() << L"</UserName>" << Endl;
					os << L"        </RemoteHost>" << Endl;
					os << L"        <LocalDirectory>$(ProjectDir)/" << relativePath.getPathName() << L"</LocalDirectory>" << Endl;
					os << L"        <RemoteDirectory>/tmp/VisualGDB/$(ProjectDirUnixStyle)/" << relativePath.getPathName() << L"</RemoteDirectory>" << Endl;
					os << L"        <FileMasks>" << Endl;
					os << L"          <string>*.*</string>" << Endl;
					os << L"        </FileMasks>" << Endl;
					os << L"        <TransferNewFilesOnly>true</TransferNewFilesOnly>" << Endl;
					os << L"        <IncludeSubdirectories>true</IncludeSubdirectories>" << Endl;
					os << L"      </CustomActionBase>" << Endl;
				}
			}
		}

		os << L"    </PreBuildActions>" << Endl;
		os << L"    <PostBuildActions />" << Endl;
		os << L"    <PreCleanActions />" << Endl;
		os << L"    <PostCleanActions />" << Endl;
		os << L"  </CustomBuild>" << Endl;
		os << L"  <CustomDebug>" << Endl;
		os << L"    <PreDebugActions />" << Endl;
		os << L"    <PostDebugActions />" << Endl;
		os << L"    <BreakMode>Default</BreakMode>" << Endl;
		os << L"  </CustomDebug>" << Endl;
		os << L"  <CustomShortcuts>" << Endl;
		os << L"    <Shortcuts />" << Endl;
		os << L"    <ShowMessageAfterExecuting>true</ShowMessageAfterExecuting>" << Endl;
		os << L"  </CustomShortcuts>" << Endl;
		os << L"  <UserDefinedVariables />" << Endl;
		os << L"</VisualGDBProjectSettings2>" << Endl;

		os.close();

		if (!buffer.empty())
		{
			Ref< IStream > file = FileSystem::getInstance().open(
				projectPath + L"/" + project->getName() + L"-" + configuration->getName() + L".vgdbsettings",
				traktor::File::FmWrite
			);
			if (!file)
				return false;
			file->write(&buffer[0], int(buffer.size()));
			file->close();
		}
	}

	return true;
}

bool SolutionBuilderMsvcVCXProjVGDB::collectFiles(
	Project* project,
	ProjectItem* item,
	std::vector< traktor::Path >& outFiles
) const
{
	Ref< Filter > filter = dynamic_type_cast< Filter* >(item);
	if (filter)
	{
		const RefArray< ProjectItem >& items = item->getItems();
		for (RefArray< ProjectItem >::const_iterator i = items.begin(); i != items.end(); ++i)
			collectFiles(
				project,
				*i,
				outFiles
			);
	}

	Ref< ::File > file = dynamic_type_cast< ::File* >(item);
	if (file)
		outFiles.push_back(file->getFileName());

	return true;
}

void SolutionBuilderMsvcVCXProjVGDB::collectLinkDependencies(
	Solution* solution,
	Project* project,
	const Configuration* configuration,
	std::vector< std::wstring >& outLibraries
) const
{
	std::wstring projectPath = solution->getRootPath() + L"/" + project->getName();

	// Add products from project's dependencies.
	const RefArray< Dependency >& dependencies = project->getDependencies();
	for (RefArray< Dependency >::const_iterator i = dependencies.begin(); i != dependencies.end(); ++i)
	{
		if (!(*i)->shouldLinkWithProduct())
			continue;

		Ref< Solution > dependentSolution;
		Ref< Project > dependentProject;

		if (is_a< ProjectDependency >(*i))
		{
			dependentSolution = solution;
			dependentProject = static_cast< ProjectDependency* >(*i)->getProject();
		}
		else if (is_a< ExternalDependency >(*i))
		{
			dependentSolution = static_cast< ExternalDependency* >(*i)->getSolution();
			dependentProject = static_cast< ExternalDependency* >(*i)->getProject();
		}

		T_ASSERT (dependentSolution);
		T_ASSERT (dependentProject);

		Ref< Configuration > dependentConfiguration = dependentProject->getConfiguration(configuration->getName());
		if (dependentConfiguration)
		{
			Configuration::TargetFormat format = dependentConfiguration->getTargetFormat();
			if (format == Configuration::TfStaticLibrary || format == Configuration::TfSharedLibrary)
			{
				std::wstring dependentProductPath = dependentSolution->getRootPath() + L"/" + dependentConfiguration->getName();
				std::wstring dependentProduct = dependentProductPath + L"/lib" + dependentProject->getName();

				if (dependentConfiguration->getTargetProfile() == Configuration::TpDebug)
				{
					if (format == Configuration::TfStaticLibrary)
						dependentProduct += L"_d.a";
					else
						dependentProduct += L"_d.so";
				}
				else
				{
					if (format == Configuration::TfStaticLibrary)
						dependentProduct += L".a";
					else
						dependentProduct += L".so";
				}

				Path libraryPathRelative;
				if (!FileSystem::getInstance().getRelativePath(dependentProduct, projectPath, libraryPathRelative))
					continue;

				outLibraries.push_back(libraryPathRelative.getPathName());
			}
		}
	}
}

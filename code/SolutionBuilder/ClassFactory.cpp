/*
 * TRAKTOR
 * Copyright (c) 2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Core/Class/Boxes/BoxedAlignedVector.h"
#include "Core/Class/Boxes/BoxedRefArray.h"
#include "Core/Class/Boxes/BoxedStdVector.h"
#include "SolutionBuilder/AggregationItem.h"
#include "SolutionBuilder/ClassFactory.h"
#include "SolutionBuilder/Configuration.h"
#include "SolutionBuilder/Dependency.h"
#include "SolutionBuilder/ExternalDependency.h"
#include "SolutionBuilder/File.h"
#include "SolutionBuilder/Filter.h"
#include "SolutionBuilder/HeaderScanner.h"
#include "SolutionBuilder/Output.h"
#include "SolutionBuilder/Project.h"
#include "SolutionBuilder/ProjectDependency.h"
#include "SolutionBuilder/ProjectItem.h"
#include "SolutionBuilder/Solution.h"
#include "SolutionBuilder/SolutionLoader.h"

#define DEPRECATED(expr) (expr)

namespace traktor::sb
{
	namespace
	{

std::wstring Configuration_getTargetFormat(Configuration* configuration)
{
	switch (configuration->getTargetFormat())
	{
	case Configuration::TfStaticLibrary:
		return L"StaticLibrary";
	case Configuration::TfSharedLibrary:
		return L"SharedLibrary";
	case Configuration::TfExecutable:
		return L"Executable";
	case Configuration::TfExecutableConsole:
		return L"ExecutableConsole";
	default:
		return L"";
	}
}

std::wstring Configuration_getTargetProfile(Configuration* configuration)
{
	switch (configuration->getTargetProfile())
	{
	case Configuration::TpDebug:
		return L"Debug";
	case Configuration::TpRelease:
		return L"Release";
	default:
		return L"";
	}
}

std::wstring Dependency_getLink(Dependency* dependency)
{
	switch (dependency->getLink())
	{
	case Dependency::LnkNo:
		return L"No";
	case Dependency::LnkYes:
		return L"Yes";
	case Dependency::LnkForce:
		return L"Force";
	default:
		return L"";
	}
}

RefArray< Path > File_getSystemFiles(sb::File* file, const std::wstring& sourcePath)
{
	std::set< Path > systemFiles;
	file->getSystemFiles(sourcePath, systemFiles);

	RefArray< Path > systemFilesOut;
	for (auto systemFile : systemFiles)
		systemFilesOut.push_back(new Path(systemFile));

	return systemFilesOut;
}

Any HeaderScanner_get(HeaderScanner* self, const std::wstring& fileName, const std::wstring& projectPath)
{
	SmallSet< std::wstring > headerFiles;
	self->get(fileName, projectPath, headerFiles);
	return Any::fromObject(new BoxedAlignedVector(
		AlignedVector< std::wstring >(headerFiles.begin(), headerFiles.end())
	));
}

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sb.ClassFactory", 0, ClassFactory, IRuntimeClassFactory)

void ClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	auto classOutput = new AutoRuntimeClass< Output >();
	classOutput->addMethod("print", &Output::print);
	classOutput->addMethod("printLn", &Output::printLn);
	classOutput->addMethod("printSection", &Output::printSection);
	registrar->registerClass(classOutput);

	auto classSolution = new AutoRuntimeClass< Solution >();
	classSolution->addProperty("name", &Solution::setName, &Solution::getName);
	classSolution->addProperty("rootPath", &Solution::setRootPath, &Solution::getRootPath);
	classSolution->addProperty("aggregateOutputPath", &Solution::setAggregateOutputPath, &Solution::getAggregateOutputPath);
	classSolution->addProperty("projects", &Solution::setProjects, &Solution::getProjects);
	classSolution->addMethod("addProject", &Solution::addProject);
	classSolution->addMethod("removeProject", &Solution::removeProject);
	DEPRECATED(classSolution->addMethod("getName", &Solution::getName));
	DEPRECATED(classSolution->addMethod("getRootPath", &Solution::getRootPath));
	DEPRECATED(classSolution->addMethod("getAggregateOutputPath", &Solution::getAggregateOutputPath));
	DEPRECATED(classSolution->addMethod("getProjects", &Solution::getProjects));
	registrar->registerClass(classSolution);

	auto classProject = new AutoRuntimeClass< Project >();
	classProject->addProperty("enable", &Project::setEnable, &Project::getEnable);
	classProject->addProperty("name", &Project::setName, &Project::getName);
	classProject->addProperty("sourcePath", &Project::setSourcePath, &Project::getSourcePath);
	classProject->addProperty("configurations", &Project::getConfigurations);
	classProject->addProperty("items", &Project::getItems);
	classProject->addProperty("dependencies", &Project::getDependencies);
	classProject->addMethod("addConfiguration", &Project::addConfiguration);
	classProject->addMethod("removeConfiguration", &Project::removeConfiguration);
	classProject->addMethod("getConfiguration", &Project::getConfiguration);
	classProject->addMethod("addItem", &Project::addItem);
	classProject->addMethod("removeItem", &Project::removeItem);
	classProject->addMethod("addDependency", &Project::addDependency);
	classProject->addMethod("removeDependency", &Project::removeDependency);
	DEPRECATED(classProject->addMethod("getEnable", &Project::getEnable));
	DEPRECATED(classProject->addMethod("getName", &Project::getName));
	DEPRECATED(classProject->addMethod("getSourcePath", &Project::getSourcePath));
	DEPRECATED(classProject->addMethod("getConfigurations", &Project::getConfigurations));
	DEPRECATED(classProject->addMethod("getItems", &Project::getItems));
	DEPRECATED(classProject->addMethod("getDependencies", &Project::getDependencies));
	registrar->registerClass(classProject);

	auto classConfiguration = new AutoRuntimeClass< Configuration >();
	classConfiguration->addProperty("name", &Configuration::setName, &Configuration::getName);
	classConfiguration->addProperty("targetFormat", &Configuration_getTargetFormat);
	classConfiguration->addProperty("targetProfile", &Configuration_getTargetProfile);
	classConfiguration->addProperty("precompiledHeader", &Configuration::setPrecompiledHeader, &Configuration::getPrecompiledHeader);
	classConfiguration->addProperty("includePaths", &Configuration::setIncludePaths, &Configuration::getIncludePaths);
	classConfiguration->addProperty("definitions", &Configuration::setDefinitions, &Configuration::getDefinitions);
	classConfiguration->addProperty("libraryPaths", &Configuration::setLibraryPaths, &Configuration::getLibraryPaths);
	classConfiguration->addProperty("libraries", &Configuration::setLibraries, &Configuration::getLibraries);
	classConfiguration->addProperty("additionalCompilerOptions", &Configuration::setAdditionalCompilerOptions, &Configuration::getAdditionalCompilerOptions);
	classConfiguration->addProperty("additionalLinkerOptions", &Configuration::setAdditionalLinkerOptions, &Configuration::getAdditionalLinkerOptions);
	classConfiguration->addProperty("debugExecutable", &Configuration::setDebugExecutable, &Configuration::getDebugExecutable);
	classConfiguration->addProperty("debugArguments", &Configuration::setDebugArguments, &Configuration::getDebugArguments);
	classConfiguration->addProperty("debugEnvironment", &Configuration::setDebugEnvironment, &Configuration::getDebugEnvironment);
	classConfiguration->addProperty("debugWorkingDirectory", &Configuration::setDebugWorkingDirectory, &Configuration::getDebugWorkingDirectory);
	classConfiguration->addProperty("aggregationItems", &Configuration::setAggregationItems, &Configuration::getAggregationItems);
	classConfiguration->addProperty("consumerLibraryPath", &Configuration::setConsumerLibraryPath, &Configuration::getConsumerLibraryPath);
	DEPRECATED(classConfiguration->addMethod("getName", &Configuration::getName));
	DEPRECATED(classConfiguration->addMethod("getTargetFormat", &Configuration_getTargetFormat));
	DEPRECATED(classConfiguration->addMethod("getTargetProfile", &Configuration_getTargetProfile));
	DEPRECATED(classConfiguration->addMethod("getPrecompiledHeader", &Configuration::getPrecompiledHeader));
	DEPRECATED(classConfiguration->addMethod("getIncludePaths", &Configuration::getIncludePaths));
	DEPRECATED(classConfiguration->addMethod("getDefinitions", &Configuration::getDefinitions));
	DEPRECATED(classConfiguration->addMethod("getLibraryPaths", &Configuration::getLibraryPaths));
	DEPRECATED(classConfiguration->addMethod("getLibraries", &Configuration::getLibraries));
	DEPRECATED(classConfiguration->addMethod("getAdditionalCompilerOptions", &Configuration::getAdditionalCompilerOptions));
	DEPRECATED(classConfiguration->addMethod("getAdditionalLinkerOptions", &Configuration::getAdditionalLinkerOptions));
	DEPRECATED(classConfiguration->addMethod("getDebugExecutable", &Configuration::getDebugExecutable));
	DEPRECATED(classConfiguration->addMethod("getDebugArguments", &Configuration::getDebugArguments));
	DEPRECATED(classConfiguration->addMethod("getDebugEnvironment", &Configuration::getDebugEnvironment));
	DEPRECATED(classConfiguration->addMethod("getDebugWorkingDirectory", &Configuration::getDebugWorkingDirectory));
	DEPRECATED(classConfiguration->addMethod("getAggregationItems", &Configuration::getAggregationItems));
	DEPRECATED(classConfiguration->addMethod("getConsumerLibraryPath", &Configuration::getConsumerLibraryPath));
	registrar->registerClass(classConfiguration);

	auto classProjectItem = new AutoRuntimeClass< ProjectItem >();
	classProjectItem->addProperty("items", &ProjectItem::setItems, &ProjectItem::getItems);
	classProjectItem->addMethod("addItem", &ProjectItem::addItem);
	classProjectItem->addMethod("removeItem", &ProjectItem::removeItem);
	DEPRECATED(classProjectItem->addMethod("getItems", &ProjectItem::getItems));
	registrar->registerClass(classProjectItem);

	Ref< AutoRuntimeClass< sb::File > > classFile = new AutoRuntimeClass< sb::File >();
	classFile->addProperty("fileName", &sb::File::setFileName, &sb::File::getFileName);
	classFile->addProperty("excludeFilter", &sb::File::setExcludeFilter, &sb::File::getExcludeFilter);
	classFile->addMethod("getSystemFiles", &File_getSystemFiles);
	DEPRECATED(classFile->addMethod("getFileName", &sb::File::getFileName));
	registrar->registerClass(classFile);

	auto classFilter = new AutoRuntimeClass< Filter >();
	classFilter->addProperty("name", &Filter::setName, &Filter::getName);
	DEPRECATED(classFilter->addMethod("getName", &Filter::getName));
	registrar->registerClass(classFilter);

	auto classAggregationItem = new AutoRuntimeClass< AggregationItem >();
	classAggregationItem->addProperty("sourceFile", &AggregationItem::setSourceFile, &AggregationItem::getSourceFile);
	classAggregationItem->addProperty("targetPath", &AggregationItem::setTargetPath, &AggregationItem::getTargetPath);
	DEPRECATED(classAggregationItem->addMethod("getSourceFile", &AggregationItem::getSourceFile));
	DEPRECATED(classAggregationItem->addMethod("getTargetPath", &AggregationItem::getTargetPath));
	registrar->registerClass(classAggregationItem);

	auto classDependency = new AutoRuntimeClass< Dependency >();
	classDependency->addProperty("link", &Dependency_getLink);
	classDependency->addProperty("name", &Dependency::getName);
	classDependency->addProperty("location", &Dependency::getLocation);
	DEPRECATED(classDependency->addMethod("getLink", &Dependency_getLink));
	DEPRECATED(classDependency->addMethod("getName", &Dependency::getName));
	DEPRECATED(classDependency->addMethod("getLocation", &Dependency::getLocation));
	registrar->registerClass(classDependency);

	auto classExternalDependency = new AutoRuntimeClass< ExternalDependency >();
	classExternalDependency->addProperty("solutionFileName", &ExternalDependency::setSolutionFileName, &ExternalDependency::getSolutionFileName);
	classExternalDependency->addProperty("solution", &ExternalDependency::getSolution);
	classExternalDependency->addProperty("project", &ExternalDependency::getProject);
	DEPRECATED(classExternalDependency->addMethod("getSolutionFileName", &ExternalDependency::getSolutionFileName));
	DEPRECATED(classExternalDependency->addMethod("getSolution", &ExternalDependency::getSolution));
	DEPRECATED(classExternalDependency->addMethod("getProject", &ExternalDependency::getProject));
	registrar->registerClass(classExternalDependency);

	auto classProjectDependency = new AutoRuntimeClass< ProjectDependency >();
	classProjectDependency->addProperty("project", &ProjectDependency::setProject, &ProjectDependency::getProject);
	DEPRECATED(classProjectDependency->addMethod("getProject", &ProjectDependency::getProject));
	registrar->registerClass(classProjectDependency);

	auto classHeaderScanner = new AutoRuntimeClass< HeaderScanner >();
	classHeaderScanner->addConstructor();
	classHeaderScanner->addMethod("removeAllIncludePaths", &HeaderScanner::removeAllIncludePaths);
	classHeaderScanner->addMethod("addIncludePath", &HeaderScanner::addIncludePath);
	classHeaderScanner->addMethod("get", &HeaderScanner_get);
	registrar->registerClass(classHeaderScanner);

	auto classSolutionLoader = new AutoRuntimeClass< SolutionLoader >();
	classSolutionLoader->addConstructor();
	classSolutionLoader->addMethod("load", &SolutionLoader::load);
	registrar->registerClass(classSolutionLoader);
}

}

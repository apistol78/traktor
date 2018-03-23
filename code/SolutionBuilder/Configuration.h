/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef Configuration_H
#define Configuration_H

#include <string>
#include <vector>
#include "Core/RefArray.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOLUTIONBUILDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sb
	{

class AggregationItem;

/*! \brief Configuration settings.
 *
 * Describes one of a project's configurations.
 * A configuration contains information about the
 * format and profile of the final product.
 * It also contains project specific paths for headers
 * and/or libraries.
 */
class T_DLLCLASS Configuration : public ISerializable
{
	T_RTTI_CLASS;

public:
	enum TargetFormat
	{
		TfStaticLibrary,
		TfSharedLibrary,
		TfExecutable,
		TfExecutableConsole
	};

	enum TargetProfile
	{
		TpDebug,
		TpRelease
	};

	enum WarningLevel
	{
		WlNoWarnings,
		WlCriticalOnly,
		WlCompilerDefault,
		WlAllWarnings
	};

	Configuration();

	void setName(const std::wstring& name);

	const std::wstring& getName() const;

	void setTargetFormat(TargetFormat targetFormat);

	TargetFormat getTargetFormat() const;

	void setTargetProfile(TargetProfile targetProfile);

	TargetProfile getTargetProfile() const;

	void setPrecompiledHeader(const std::wstring& precompiledHeader);

	const std::wstring& getPrecompiledHeader() const;

	void addIncludePath(const std::wstring& includePath);

	void setIncludePaths(const std::vector< std::wstring >& includePaths);

	const std::vector< std::wstring >& getIncludePaths() const;

	void addDefinition(const std::wstring& definition);

	void setDefinitions(const std::vector< std::wstring >& definitions);

	const std::vector< std::wstring >& getDefinitions() const;

	void setLibraryPaths(const std::vector< std::wstring >& libraryPaths);

	const std::vector< std::wstring >& getLibraryPaths() const;

	void setLibraries(const std::vector< std::wstring >& libraries);

	const std::vector< std::wstring >& getLibraries() const;

	void setWarningLevel(WarningLevel warningLevel);

	WarningLevel getWarningLevel() const;

	void setAdditionalCompilerOptions(const std::wstring& additionalCompilerOptions);

	const std::wstring& getAdditionalCompilerOptions() const;

	void setAdditionalLinkerOptions(const std::wstring& additionalLinkerOptions);

	const std::wstring& getAdditionalLinkerOptions() const;

	/*! \name Debug options. */
	//@{

	void setDebugExecutable(const std::wstring& debugExecutable);

	const std::wstring& getDebugExecutable() const;

	void setDebugArguments(const std::wstring& debugArguments);

	const std::wstring& getDebugArguments() const;

	void setDebugEnvironment(const std::wstring& debugEnvironment);

	const std::wstring& getDebugEnvironment() const;

	void setDebugWorkingDirectory(const std::wstring& debugWorkingDirectory);

	const std::wstring& getDebugWorkingDirectory() const;

	//@}

	/*! \name Aggregation items. */
	//@{

	void addAggregationItem(AggregationItem* item);

	void removeAggregationItem(AggregationItem* item);

	void removeAllAggregationItems();

	void setAggregationItems(const RefArray< AggregationItem >& aggregationItems);

	const RefArray< AggregationItem >& getAggregationItems() const;

	//@}

	/*! \name Consumer */
	//@{

	void setConsumerLibraryPath(const std::wstring& consumerLibraryPath);

	const std::wstring& getConsumerLibraryPath() const;

	//@}

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	std::wstring m_name;
	TargetFormat m_targetFormat;
	TargetProfile m_targetProfile;
	std::wstring m_precompiledHeader;
	std::vector< std::wstring > m_includePaths;
	std::vector< std::wstring > m_definitions;
	std::vector< std::wstring > m_libraryPaths;
	std::vector< std::wstring > m_libraries;
	WarningLevel m_warningLevel;
	std::wstring m_additionalCompilerOptions;
	std::wstring m_additionalLinkerOptions;
	std::wstring m_debugExecutable;
	std::wstring m_debugArguments;
	std::wstring m_debugEnvironment;
	std::wstring m_debugWorkingDirectory;
	RefArray< AggregationItem > m_aggregationItems;
	std::wstring m_consumerLibraryPath;
};

	}
}

#endif	// Configuration_H

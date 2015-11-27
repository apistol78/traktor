#ifndef Configuration_H
#define Configuration_H

#include <string>
#include <vector>
#include <Core/RefArray.h>
#include <Core/Serialization/ISerializable.h>

class AggregationItem;

/*! \brief Configuration settings.
 *
 * Describes one of a project's configurations.
 * A configuration contains information about the
 * format and profile of the final product.
 * It also contains project specific paths for headers
 * and/or libraries.
 */
class Configuration : public traktor::ISerializable
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

	const traktor::RefArray< AggregationItem >& getAggregationItems() const;

	//@}

	virtual void serialize(traktor::ISerializer& s);

private:
	std::wstring m_name;
	TargetFormat m_targetFormat;
	TargetProfile m_targetProfile;
	std::wstring m_precompiledHeader;
	std::vector< std::wstring > m_includePaths;
	std::vector< std::wstring > m_definitions;
	std::vector< std::wstring > m_libraryPaths;
	std::vector< std::wstring > m_libraries;
	std::wstring m_additionalCompilerOptions;
	std::wstring m_additionalLinkerOptions;
	std::wstring m_debugExecutable;
	std::wstring m_debugArguments;
	std::wstring m_debugEnvironment;
	std::wstring m_debugWorkingDirectory;
	traktor::RefArray< AggregationItem > m_aggregationItems;
};

#endif	// Configuration_H

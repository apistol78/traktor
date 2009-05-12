#ifndef Configuration_H
#define Configuration_H

#include <string>
#include <vector>
#include <Core/Serialization/Serializable.h>

/*! \brief Configuration settings.
 *
 * Describes one of a project's configurations.
 * A configuration contains information about the
 * format and profile of the final product.
 * It also contains project specific paths for headers
 * and/or libraries.
 */
class Configuration : public traktor::Serializable
{
	T_RTTI_CLASS(Configuration)

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

	std::vector< std::wstring >& getIncludePaths();

	std::vector< std::wstring >& getDefinitions();

	std::vector< std::wstring >& getLibraryPaths();

	std::vector< std::wstring >& getLibraries();

	virtual bool serialize(traktor::Serializer& s);

private:
	std::wstring m_name;
	TargetFormat m_targetFormat;
	TargetProfile m_targetProfile;
	std::wstring m_precompiledHeader;
	std::vector< std::wstring > m_includePaths;
	std::vector< std::wstring > m_definitions;
	std::vector< std::wstring > m_libraryPaths;
	std::vector< std::wstring > m_libraries;
};

#endif	// Configuration_H

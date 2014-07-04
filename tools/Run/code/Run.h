#ifndef Run_H
#define Run_H

#include <list>
#include <Core/Object.h>

class Run : public traktor::Object
{
	T_RTTI_CLASS;

public:
	Run();

	void cd(const std::wstring& path);

	void pushd(const std::wstring& path);

	void popd();

	std::wstring cwd() const;

	int32_t run(const std::wstring& command, const std::wstring& saveOutputAs = L"");

	int32_t execute(const std::wstring& command, const std::wstring& saveOutputAs = L"");

	const std::wstring& stdOut() const;

	const std::wstring& stdErr() const;

	int32_t exitCode() const;

	bool copy(const std::wstring& source, const std::wstring& target);

	bool replace(const std::wstring& source, const std::wstring& target);

	bool mkdir(const std::wstring& path);

	bool rmdir(const std::wstring& path);

private:
	std::list< std::wstring > m_cwd;
	std::wstring m_stdOut;
	std::wstring m_stdErr;
	int32_t m_exitCode;
};

#endif	// Run_H

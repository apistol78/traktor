#ifndef traktor_run_Run_H
#define traktor_run_Run_H

#include <list>
#include "Core/Object.h"

namespace traktor
{

class Environment;

	namespace run
	{

class Run : public Object
{
	T_RTTI_CLASS;

public:
	Run();

	void cd(const std::wstring& path);

	void pushd(const std::wstring& path);

	void popd();

	std::wstring cwd() const;

	int32_t run(const std::wstring& command, const std::wstring& saveOutputAs = L"", const Environment* env = 0);

	int32_t execute(const std::wstring& command, const std::wstring& saveOutputAs = L"", const Environment* env = 0);

	const std::wstring& stdOut() const;

	const std::wstring& stdErr() const;

	int32_t exitCode() const;

	bool exist(const std::wstring& path);

	bool rm(const std::wstring& path);

	bool copy(const std::wstring& source, const std::wstring& target);

	bool replace(const std::wstring& source, const std::wstring& target);

	bool mkdir(const std::wstring& path);

	bool rmdir(const std::wstring& path);

	void sleep(int32_t ms);

private:
	std::list< std::wstring > m_cwd;
	std::wstring m_stdOut;
	std::wstring m_stdErr;
	int32_t m_exitCode;
};

	}
}

#endif	// traktor_run_Run_H

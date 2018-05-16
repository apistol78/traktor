/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_run_Run_H
#define traktor_run_Run_H

#include <list>
#include "Core/Object.h"
#include "Core/Class/Any.h"

namespace traktor
{

class Environment;

	namespace run
	{

/*! \brief Simplified interface for common scriptable actions.
 * \ingroup Run
 */
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

	Any getProperty(const std::wstring& fileName1, const std::wstring& fileName2, const std::wstring& propertyName, const Any& defaultValue) const;

	bool setProperty(const std::wstring& fileName, const std::wstring& propertyName, const Any& value) const;

private:
	std::list< std::wstring > m_cwd;
	std::wstring m_stdOut;
	std::wstring m_stdErr;
	int32_t m_exitCode;
};

	}
}

#endif	// traktor_run_Run_H

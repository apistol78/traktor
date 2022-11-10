/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/Io/Path.h"
#include "Core/Singleton/ISingleton.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Environment;
class IProcess;
class ISharedMemory;

/*! Operative System information.
 * \ingroup Core
 */
class T_DLLCLASS OS
:	public Object
,	public ISingleton
{
	T_RTTI_CLASS;

public:
	enum ExecuteFlags : uint32_t
	{
		EfNone = 0,
		EfRedirectStdIO = (1 << 0),	//< Redirect standard I/O.
		EfMute = (1 << 1),			//< Mute I/O of child process, still get redirected.
		EfDetach = (1 << 2)			//< Detach process, not child of calling process.
	};

	static OS& getInstance();

	/*! Get OS name. */
	std::wstring getName() const;

	/*! Get OS identifier. */
	std::wstring getIdentifier() const;

	/*! Get number of CPU cores.
	 *
	 * \return Number of CPU cores.
	 */
	uint32_t getCPUCoreCount() const;

	/*! Get path to current executable.
	 *
	 * \return Executable path.
	 */
	Path getExecutable() const;

	/*! Get command line.
	 *
	 * \return Process command line.
	 */
	std::wstring getCommandLine() const;

	/*! Get network name of computer.
	 *
	 * \return Computer network name.
	 */
	std::wstring getComputerName() const;

	/*! Get name of currently logged in user.
	 *
	 * \return User name.
	 */
	std::wstring getCurrentUser() const;

	/*! Get path to user home.
	 *
	 * \return User home path.
	 */
	std::wstring getUserHomePath() const;

	/*! Get path to user application data.
	 *
	 * \return User application data path.
	 */
	std::wstring getUserApplicationDataPath() const;

	/*! Get path to writable folder.
	 *
	 * \return Writable folder.
	 */
	std::wstring getWritableFolderPath() const;

	/*! Open associate application of given file.
	 *
	 * \param file File or URL to resource.
	 * \return True if associated application opened successfully.
	 */
	bool openFile(const std::wstring& file) const;

	/*! Launch associate editor of given file.
	 *
	 * \param file File or URL to resource.
	 * \return True if associate editor opened successfully.
	 */
	bool editFile(const std::wstring& file) const;

	/*! Open file explorer to given file.
	 *
	 * \param file File or URL to resource.
	 * \return True if explorer opened successfully.
	 */
	bool exploreFile(const std::wstring& file) const;

	/*! Set environment variable.
	 *
	 * \param name Name of variable.
	 * \param value New value of variable.
	 * \return True if variable set succeeded.
	 */
	bool setEnvironment(const std::wstring& name, const std::wstring& value) const;

	/*! Get process's environment variables.
	 *
	 * \return Environment variables.
	 */
	Ref< Environment > getEnvironment() const;

	/*! Get environment variable value.
	 *
	 * \param name Name of variable.
	 * \param outValue Value of variable if found.
	 * \return True if variable found.
	 */
	bool getEnvironment(const std::wstring& name, std::wstring& outValue) const;

	/*! Execute command.
	 *
	 * \param commandLine Execute command line.
	 * \param workingDirectory Process's initial working directory.
	 * \param env Optional environment.
	 * \param flags Execute flags. \sa ExecuteFlags
	 * \return Process instance, null if unable to execute.
	 */
	Ref< IProcess > execute(
		const std::wstring& commandLine,
		const Path& workingDirectory,
		const Environment* env,
		uint32_t flags
	) const;

	/*! Create shared memory object.
	 *
	 * \param name Name of shared memory object.
	 * \param size Size of shared memory.
	 * \return Shared memory object.
	 */
	Ref< ISharedMemory > createSharedMemory(const std::wstring& name, uint32_t size) const;

	/*! Set own process priority bias.
	 *
	 * \param priorityBias Priority bias, -1 = below normal, 0 = normal and 1 = above normal.
	 * \return True if priority bias applied successfully.
	 */
	bool setOwnProcessPriorityBias(int32_t priorityBias);

#if defined(_WIN32)
	/*! Get registry value.
	 *
	 * \param key Registry key.
	 * \param outValue Value of key if found.
	 * \return True if key found.
	 */
	bool getRegistry(const std::wstring& key, const std::wstring& subKey, const std::wstring& valueName, std::wstring& outValue) const;
#endif

	/*! Query OS about location of executable binary.
	 *
	 * \param executable Name of executable.
	 * \param outPath Path to executable.
	 * \return True if executable was found.
	 */
	bool whereIs(const std::wstring& executable, Path& outPath) const;

protected:
	OS();

	virtual ~OS();

	virtual void destroy() override final;

private:
	void* m_handle = nullptr;
};

}


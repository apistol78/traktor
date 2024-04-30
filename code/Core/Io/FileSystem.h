/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Core/RefArray.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Io/File.h"
#include "Core/Io/IVolume.h"
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

class IMappedFile;
class IStream;

/*! File system manager.
 * \ingroup Core
 *
 * Manages physical or virtual file volumes.
 * Initially all physical drives are mounted
 * in the file system.
 */
class T_DLLCLASS FileSystem
:	public Object
,	public ISingleton
{
	T_RTTI_CLASS;

public:
	FileSystem();

	static FileSystem& getInstance();

	/*! Mount custom volume.
	 *
	 * \param id Mount identifier.
	 * \param volume Mount volume.
	 */
	void mount(const std::wstring& id, IVolume* volume);

	/*! Unmount volume.
	 *
	 * \param id Mount identifier.
	 */
	void umount(const std::wstring& id);

	/*! Get number of volumes.
	 *
	 * \return Number of mounted volumes.
	 */
	int32_t getVolumeCount() const;

	/*! Get volume by index.
	 *
	 * \param index Index of mounted volume.
	 * \return Volume instance.
	 */
	IVolume* getVolume(int32_t index) const;

	/*! Get identifier of volume.
	 *
	 * \param index Index of mounted volume.
	 * \return Volume mount identifier.
	 */
	std::wstring getVolumeId(int32_t index) const;

	/*! Set volume as current volume.
	 *
	 * \param volume Current volume.
	 */
	void setCurrentVolume(IVolume* volume);

	/*! Get current volume.
	 *
	 * \return Current volume.
	 */
	IVolume* getCurrentVolume() const;

	/*! Set current volume and directory.
	 *
	 * \param directory New current directory.
	 * \return True if successfully changed.
	 */
	[[nodiscard]] bool setCurrentVolumeAndDirectory(const Path& directory);

	/*! Get current volume and directory.
	 */
	Path getCurrentVolumeAndDirectory() const;

	/*! Get file description.
	 *
	 * \param fileName Path to file to query.
	 * \return File description.
	 */
	Ref< File > get(const Path& fileName);

	/*! Find files.
	 *
	 * \param fileMask Path (with or without wild cards) of files to query.
	 * \return Array of file descriptions.
	 */
	RefArray< File > find(const Path& fileMask);

	/*! Modify file's flags.
	 *
	 * \param fileName Path to file to modify.
	 * \param flags New file flags.
	 * \return True if file flags was successfully modified.
	 */
	bool modify(const Path& fileName, uint32_t flags);

	/*! Modify file's time stamps.
	 *
	 * \param fileName Path to file to modify.
	 * \param creationTime New creation time stamp, optional.
	 * \param lastAccessTime New last access time stamp, optional.
	 * \param lastWriteTime New last write time stamp, optional.
	 * \return True if file time stamps was successfully modified.
	 */
	bool modify(const Path& fileName, const DateTime* creationTime, const DateTime* lastAccessTime, const DateTime* lastWriteTime);

	/*! Open file for reading or writing.
	 *
	 * \param fileName Path to file to open.
	 * \param mode Desired file mode.
	 * \return Stream to file, null if unable to access file.
	 */
	Ref< IStream > open(const Path& fileName, uint32_t mode);

	/*! Map file into memory for reading.
	 * 
	 * \param fileName Path to file to open.
	 * \return Mapped file view.
	 */
	Ref< IMappedFile > map(const Path& fileName);

	/*! Return true if file exists.
	 *
	 * \param fileName Path to file.
	 * \return True if file exists.
	 */
	bool exist(const Path& fileName);

	/*! Remove file.
	 *
	 * \param fileName Path to file.
	 * \return True if file was successfully removed.
	 */
	bool remove(const Path& fileName);

	/*! Move file to a different location.
	 *
	 * \param destination Path to destination file.
	 * \param source Path to source file.
	 * \param overwrite Overwrite destination file if it exists.
	 * \return True if file was successfully moved.
	 */
	bool move(const Path& destination, const Path& source, bool overwrite = false);

	/*! Copy file to another location.
	 *
	 * \param destination Path to destination file.
	 * \param source Path to source file.
	 * \param overwrite Overwrite destination file if it exists.
	 * \return True if file was successfully copied.
	 */
	bool copy(const Path& destination, const Path& source, bool overwrite = false);

	/*! Make a new directory.
	 *
	 * \param directory Path to new directory.
	 * \return True if directory was successfully created.
	 */
	bool makeDirectory(const Path& directory);

	/*! Make a new directory hierarchy.
	 *
	 * \param directory Path to new directory.
	 * \return True if directory was successfully created.
	 */
	bool makeAllDirectories(const Path& directory);

	/*! Remove directory.
	 *
	 * \param directory Path to directory.
	 * \return True if directory was successfully removed.
	 */
	bool removeDirectory(const Path& directory);

	/*! Rename directory.
	 *
	 * \param directory Path to directory.
	 * \param newName New name of directory.
	 * \return True if directory was successfully renamed.
	 */
	bool renameDirectory(const Path& directory, const std::wstring& newName);

	/*! Translate a relative path to an absolute path.
	 *
	 * \param relativePath Relative path.
	 * \return Absolute path.
	 */
	Path getAbsolutePath(const Path& relativePath) const;

	/*! Translate a relative path to an absolute path.
	 *
	 * \param basePath Base path.
	 * \param relativePath Relative path.
	 * \return Absolute path.
	 */
	Path getAbsolutePath(const Path& basePath, const Path& relativePath) const;

	/*! Get relative path between two absolute paths.
	 *
	 * \par
	 * absolutePath = c:/foo/bar/dummy.txt\n
	 * relativeToPath = c:/foo\n
	 * =>\n
	 * relativePath = bar/dummy.txt
	 *
	 * \param absolutePath Absolute path.
	 * \param relativeToPath Relative to path.
	 * \param relativePath Result relative path.
	 * \return True if relative path was successfully derived.
	 */
	bool getRelativePath(const Path& absolutePath, const Path& relativeToPath, Path& relativePath) const;

protected:
	virtual void destroy() override final;

private:
	SmallMap< std::wstring, Ref< IVolume > > m_volumes;
	Ref< IVolume > m_currentVolume;

	IVolume* getVolume(const Path& path) const;
};

}


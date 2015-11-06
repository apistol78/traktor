#ifndef traktor_FileSystem_H
#define traktor_FileSystem_H

#include <map>
#include "Core/Ref.h"
#include "Core/RefArray.h"
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

class IStream;

/*! \brief File system manager.
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

	/*! \brief Mount custom volume.
	 *
	 * \param id Mount identifier.
	 * \param volume Mount volume.
	 */
	void mount(const std::wstring& id, IVolume* volume);
	
	/*! \brief Unmount volume.
	 *
	 * \param id Mount identifier.
	 */
	void umount(const std::wstring& id);

	/*! \brief Get number of volumes.
	 *
	 * \return Number of mounted volumes.
	 */
	int getVolumeCount() const;

	/*! \brief Get volume by index.
	 *
	 * \param index Index of mounted volume.
	 * \return Volume instance.
	 */
	Ref< IVolume > getVolume(int index) const;

	/*! \brief Get identifier of volume.
	 *
	 * \param index Index of mounted volume.
	 * \return Volume mount identifier.
	 */
	std::wstring getVolumeId(int index) const;
	
	/*! \brief Set volume as current volume.
	 *
	 * \param volume Current volume.
	 */
	void setCurrentVolume(IVolume* volume);
	
	/*! \brief Get current volume.
	 *
	 * \return Current volume.
	 */
	Ref< IVolume > getCurrentVolume() const;

	/*! \brief Set current volume and directory.
	 *
	 * \param directory New current directory.
	 * \return True if successfully changed.
	 */
	bool setCurrentVolumeAndDirectory(const Path& directory);
	
	/*! \brief Get file description.
	 *
	 * \param fileName Path to file to query.
	 * \return File description.
	 */
	Ref< File > get(const Path& fileName);

	/*! \brief Find files.
	 *
	 * \param fileMask Path (with or without wild cards) of files to query.
	 * \param out Array of file descriptions.
	 * \return Number of files matching fileMask.
	 */
	int find(const Path& fileMask, RefArray< File >& out);

	/*! \brief Modify file's flags.
	 *
	 * \param fileName Path to file to modify.
	 * \param flags New file flags.
	 * \return True if file flags was successfully modified.
	 */
	bool modify(const Path& fileName, uint32_t flags);

	/*! \brief Open file for reading or writing.
	 *
	 * \param fileName Path to file to open.
	 * \param mode Desired file mode.
	 * \return Stream to file, null if unable to access file.
	 */
	Ref< IStream > open(const Path& fileName, uint32_t mode);
	
	/*! \brief Return true if file exists.
	 *
	 * \param fileName Path to file.
	 * \return True if file exists.
	 */
	bool exist(const Path& fileName);
	
	/*! \brief Remove file.
	 *
	 * \param fileName Path to file.
	 * \return True if file was successfully removed.
	 */
	bool remove(const Path& fileName);

	/*! \brief Move file to a different location.
	 *
	 * \param destination Path to destination file.
	 * \param source Path to source file.
	 * \param overwrite Overwrite destination file if it exists.
	 * \return True if file was successfully moved.
	 */
	bool move(const Path& destination, const Path& source, bool overwrite = false);

	/*! \brief Copy file to another location.
	 *
	 * \param destination Path to destination file.
	 * \param source Path to source file.
	 * \param overwrite Overwrite destination file if it exists.
	 * \return True if file was successfully copied.
	 */
	bool copy(const Path& destination, const Path& source, bool overwrite = false);

	/*! \brief Make a new directory.
	 *
	 * \param directory Path to new directory.
	 * \return True if directory was successfully created.
	 */
	bool makeDirectory(const Path& directory);

	/*! \brief Make a new directory hierarchy.
	 *
	 * \param directory Path to new directory.
	 * \return True if directory was successfully created.
	 */
	bool makeAllDirectories(const Path& directory);

	/*! \brief Remove directory.
	 *
	 * \param directory Path to directory.
	 * \return True if directory was successfully removed.
	 */
	bool removeDirectory(const Path& directory);

	/*! \brief Rename directory.
	 *
	 * \param directory Path to directory.
	 * \param newName New name of directory.
	 * \return True if directory was successfully renamed.
	 */
	bool renameDirectory(const Path& directory, const std::wstring& newName);
	
	/*! \brief Translate a relative path to an absolute path.
	 *
	 * \param relativePath Relative path.
	 * \return Absolute path.
	 */
	Path getAbsolutePath(const Path& relativePath) const;

	/*! \brief Translate a relative path to an absolute path.
	 *
	 * \param basePath Base path.
	 * \param relativePath Relative path.
	 * \return Absolute path.
	 */
	Path getAbsolutePath(const Path& basePath, const Path& relativePath) const;

	/*! \brief Get relative path between two absolute paths.
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
	virtual void destroy() T_OVERRIDE T_FINAL;

private:
	std::map< std::wstring, Ref< IVolume > > m_volumes;
	Ref< IVolume > m_currentVolume;

	Ref< IVolume > getVolume(const Path& path) const;
};

}

#endif	// traktor_FileSystem_H

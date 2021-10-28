#pragma once

#include <vector>
#include "Core/Ref.h"
#include "Core/RefArray.h"
#include "Core/Io/File.h"
#include "Core/Io/Path.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class FileArray;
class IStream;

/*! Virtual file system volume.
 * \ingroup Core
 *
 * Each object of this class represent an file system
 * volume. Instances of this class can be inserted into the
 * FS manager during runtime thus enabling virtually any
 * kind of storage, such as ZIP archives, network resources etc.
 * to be used through-out the entire framework.
 */
class T_DLLCLASS IVolume : public Object
{
	T_RTTI_CLASS;

public:
	/*! Get volume, human-readable, description.
	 *
	 * \return Description.
	 */
	virtual std::wstring getDescription() const = 0;

	/*! Get explicit file (or directory) from path.
	 *
	 * \param path Path to file.
	 * \return File object.
	 */
	virtual Ref< File > get(const Path& path) = 0;

	/*! Find files from wild-card mask.
	 *
	 * \param mask Wild-card mask.
	 * \param out Array of matching file.
	 * \return Array of files matched.
	 */
	virtual int find(const Path& mask, RefArray< File >& out) = 0;

	/*! Modify file flags.
	 *
	 * \param fileName Path to file.
	 * \param flags New file flags.
	 * \return True if successful.
	 */
	virtual bool modify(const Path& fileName, uint32_t flags) = 0;

	/*! Modify file's time stamps.
	 *
	 * \param fileName Path to file to modify.
	 * \param creationTime New creation time stamp, optional.
	 * \param lastAccessTime New last access time stamp, optional.
	 * \param lastWriteTime New last write time stamp, optional.
	 * \return True if file time stamps was successfully modified.
	 */
	virtual bool modify(const Path& fileName, const DateTime* creationTime, const DateTime* lastAccessTime, const DateTime* lastWriteTime) = 0;

	/*! Open stream into named resource.
	 *
	 * \param fileNname Name of file or directory.
	 * \param mode Read or write mode, binary or textual.
	 * \return Stream object at beginning of resource.
	 */
	virtual Ref< IStream > open(const Path& fileName, uint32_t mode) = 0;

	/*! Check if file or directory exists.
	 *
	 * \param fileName Name of file or directory.
	 * \return True if file or directory exists.
	 */
	virtual bool exist(const Path& fileName) = 0;

	/*! Remove file.
	 *
	 * \param fileName Name of file to remove.
	 * \return True if file removed successfully.
	 */
	virtual bool remove(const Path& fileName) = 0;

	/*! Move file.
	 *
	 * \param fileName Name of file to rename.
	 * \param newName New name of file.
	 * \param overwrite Overwrite if target exists.
	 * \return True if file renamed successfully.
	 */
	virtual bool move(const Path& fileName, const std::wstring& newName, bool overwrite) = 0;

	/*! Copy file.
	 *
	 * \param fileName Name of file to rename.
	 * \param newName New name of file.
	 * \param overwrite Overwrite if target exists.
	 * \return True if file renamed successfully.
	 */
	virtual bool copy(const Path& fileName, const std::wstring& newName, bool overwrite) = 0;

	/*! Make new directory.
	 *
	 * \param directory Path to new directory.
	 * \return True if directory created successfully.
	 */
	virtual bool makeDirectory(const Path& directory) = 0;

	/*! Remove directory.
	 *
	 * \param directory Path to directory.
	 * \return True if directory was removed.
	 */
	virtual bool removeDirectory(const Path& directory) = 0;

	/*! Rename directory.
	 *
	 * \param directory Path to directory.
	 * \param newName New name of directory.
	 * \return True if directory was successfully renamed.
	 */
	virtual bool renameDirectory(const Path& directory, const std::wstring& newName) = 0;

	/*! Set current directory.
	 *
	 * \param directory Path to directory.
	 * \return True if directory was successfully set to current directory.
	 */
	virtual bool setCurrentDirectory(const Path& directory) = 0;

	/*! Get current directory.
	 *
	 * \return Current directory.
	 */
	virtual Path getCurrentDirectory() const = 0;
};

}


#pragma once

#include <string>
#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! File path.
 * \ingroup Core
 */
class T_DLLCLASS Path : public Object
{
	T_RTTI_CLASS;

public:
	Path();

	Path(const Path& path);

	Path(const std::wstring& path);

	Path(const wchar_t* path);

	/*! Check if path is empty.
	 *
	 * \return True if path is empty.
	 */
	bool empty() const;

	/*! Get original, unresolved, path.
	 *
	 * \return Original path.
	 */
	std::wstring getOriginal() const;

	/*! Return true if path include volume.
	 *
	 * C:/foo/bar.txt => true
	 * /foo/bar.txt => false
	 * foo/bar.txt => false
	 *
	 * \return True if path include volume.
	 */
	bool hasVolume() const;

	/*! Return name of volume.
	 *
	 * C:/foo/bar.txt => C
	 * /foo/bar.txt => [empty]
	 * foo/bar.txt => [empty]
	 *
	 * \return Name of volume, empty string if no volume.
	 */
	std::wstring getVolume() const;

	/*! Return true if path is relative.
	 *
	 * C:/foo/bar.txt => false
	 * /foo/bar.txt => false
	 * foo/bar.txt => true
	 *
	 * \return True if path is relative.
	 */
	bool isRelative() const;

	/*! Return file name.
	 *
	 * C:/foo/bar.txt => bar.txt
	 * /foo/bar.txt => bar.txt
	 * foo/bar.txt => bar.txt
	 *
	 * \return File name.
	 */
	std::wstring getFileName() const;

	/*! Return file name without extension.
	 *
	 * C:/foo/bar.txt => bar
	 * /foo/bar.txt => bar
	 * foo/bar.txt => bar
	 *
	 * \return File name without extension.
	 */
	std::wstring getFileNameNoExtension() const;

	/*! Return path only, no file name.
	 *
	 * C:/foo/bar.txt => C:/foo
	 * /foo/bar.txt => /foo
	 * foo/bar.txt => foo
	 *
	 * \return Path only.
	 */
	std::wstring getPathOnly() const;

	/*! Return path only, no file name nor volume.
	 *
	 * C:/foo/bar.txt => /foo
	 * /foo/bar.txt => /foo
	 * foo/bar.txt => foo
	 *
	 * \return Path only.
	 */
	std::wstring getPathOnlyNoVolume() const;

	/*! Return OS matching path, ie with volume on Windows and no volume on all others. */
	std::wstring getPathOnlyOS() const;

	/*! Return path.
	 *
	 * C:/foo/bar.txt => C:/foo/bar.txt
	 * /foo/bar.txt => /foo/bar.txt
	 * foo/bar.txt => foo/bar.txt
	 *
	 * \return Encapsulated path.
	 */
	std::wstring getPathName() const;

	/*! Return path but without file extension.
	 *
	 * C:/foo/bar.txt => C:/foo/bar
	 * /foo/bar.txt => /foo/bar
	 * foo/bar.txt => foo/bar
	 *
	 * \return Encapsulated path without file extension.
	 */
	std::wstring getPathNameNoExtension() const;

	/*! Return path but without volume.
	 *
	 * C:/foo/bar.txt => /foo/bar.txt
	 * /foo/bar.txt => /foo/bar.txt
	 * foo/bar.txt => foo/bar.txt
	 *
	 * \return Encapsulated path without volume.
	 */
	std::wstring getPathNameNoVolume() const;

	/*! Return OS matching path, ie with volume on Windows and no volume on all others. */
	std::wstring getPathNameOS() const;

	/*! Return file extension.
	 *
	 * C:/foo/bar.txt => txt
	 * /foo/bar.txt => txt
	 * foo/bar.txt => txt
	 *
	 * \return File extension.
	 */
	std::wstring getExtension() const;

	/*! Return normalized path.
	 *
	 * C:/foo/../bar.txt => C:/bar.txt
	 *
	 * \return Normalized path.
	 */
	Path normalized() const;

	/*! Concate two paths.
	 *
	 * \return Concated path.
	 */
	Path operator + (const Path& rh) const;

	/*! Compare equal operator.
	 *
	 * \param rh Compare to path.
	 * \return True if paths is equal; depending on platform case sensitive.
	 */
	bool operator == (const Path& rh) const;

	/*! Compare less-than operator.
	 *
	 * Implemented to be able to have
	 * Path objects in an ordered stl map.
	 *
	 * \param rh Compare to path.
	 * \return True if path is lexically less than given path.
	 */
	bool operator < (const Path& rh) const;

	/*! Compare greater-than operator.
	 *
	 * Implemented to be able to have
	 * Path objects in an ordered stl map.
	 *
	 * \param rh Compare to path.
	 * \return True if path is lexically greater than given path.
	 */
	bool operator > (const Path& rh) const;

private:
	std::wstring m_original;
	std::wstring m_volume;
	bool m_relative;
	std::wstring m_path;
	std::wstring m_file;
	std::wstring m_ext;

	/*! Resolve path.
	 *
	 * Resolve environment variables in path.
	 */
	void resolve();
};

}


#ifndef traktor_Path_H
#define traktor_Path_H

#include <string>
#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief File path.
 * \ingroup Core
 */
class T_DLLCLASS Path : public Object
{
	T_RTTI_CLASS(Path)

public:
	Path();

	Path(const Path& path);

	Path(const std::wstring& path);

	Path(const wchar_t* path);

	/*! \brief Return true if path include volume.
	 *
	 * C:/foo/bar.txt => true
	 * /foo/bar.txt => false
	 * foo/bar.txt => false
	 *
	 * \return True if path include volume.
	 */
	bool hasVolume() const;
	
	/*! \brief Return name of volume.
	 *
	 * C:/foo/bar.txt => C
	 * /foo/bar.txt => [empty]
	 * foo/bar.txt => [empty]
	 *
	 * \return Name of volume, empty string if no volume.
	 */
	std::wstring getVolume() const;

	/*! \brief Return true if path is relative.
	 *
	 * C:/foo/bar.txt => false
	 * /foo/bar.txt => false
	 * foo/bar.txt => true
	 *
	 * \return True if path is relative.
	 */
	bool isRelative() const;
	
	/*! \brief Return file name.
	 *
	 * C:/foo/bar.txt => bar.txt
	 * /foo/bar.txt => bar.txt
	 * foo/bar.txt => bar.txt
	 *
	 * \return File name.
	 */
	std::wstring getFileName() const;

	/*! \brief Return file name without extension.
	 *
	 * C:/foo/bar.txt => bar
	 * /foo/bar.txt => bar
	 * foo/bar.txt => bar
	 *
	 * \return File name without extension.
	 */
	std::wstring getFileNameNoExtension() const;

	/*! \brief Return path only, no file name.
	 *
	 * C:/foo/bar.txt => C:/foo
	 * /foo/bar.txt => /foo
	 * foo/bar.txt => foo
	 *
	 * \return Path only.
	 */
	std::wstring getPathOnly() const;

	/*! \brief Return path only, no file name nor volume.
	 *
	 * C:/foo/bar.txt => /foo
	 * /foo/bar.txt => /foo
	 * foo/bar.txt => foo
	 *
	 * \return Path only.
	 */
	std::wstring getPathOnlyNoVolume() const;

	/*! \brief Return path.
	 *
	 * C:/foo/bar.txt => C:/foo/bar.txt
	 * /foo/bar.txt => /foo/bar.txt
	 * foo/bar.txt => foo/bar.txt
	 *
	 * \return Encapsulated path.
	 */
	std::wstring getPathName() const;

	/*! \brief Return path but without file extension.
	 *
	 * C:/foo/bar.txt => C:/foo/bar
	 * /foo/bar.txt => /foo/bar
	 * foo/bar.txt => foo/bar
	 *
	 * \return Encapsulated path without file extension.
	 */
	std::wstring getPathNameNoExtension() const;
	
	/*! \brief Return path but without volume.
	 *
	 * C:/foo/bar.txt => /foo/bar.txt
	 * /foo/bar.txt => /foo/bar.txt
	 * foo/bar.txt => foo/bar.txt
	 *
	 * \return Encapsulated path without volume.
	 */
	std::wstring getPathNameNoVolume() const;
	
	/*! \brief Return file extension.
	 *
	 * C:/foo/bar.txt => txt
	 * /foo/bar.txt => txt
	 * foo/bar.txt => txt
	 *
	 * \return File extension.
	 */
	std::wstring getExtension() const;

	/*! \brief Return normalized path.
	 *
	 * C:/foo/../bar.txt => C:/bar.txt
	 *
	 * \return Normalized path.
	 */
	Path normalized() const;

	/*! \brief String representation of path.
	 *
	 * \return Constant string.
	 */
	const wchar_t* c_str() const;

	/*! \brief String representation of path.
	 *
	 * \return String object.
	 */
	operator std::wstring () const;

	/*! \brief Concate two paths.
	 *
	 * \return Concated path.
	 */
	Path operator + (const Path& rh) const;

	/*! \brief Compare equal operator.
	 *
	 * \param rh Compare to path.
	 * \return True if paths is equal; depending on platform case sensitive.
	 */
	bool operator == (const Path& rh) const;

	/*! \brief Compare less-than operator.
	 *
	 * Implemented to be able to have
	 * Path objects in an ordered stl map.
	 *
	 * \param rh Compare to path.
	 * \return True if path is lexically less than given path.
	 */
	bool operator < (const Path& rh) const;

private:
	std::wstring m_volume;
	bool m_relative;
	std::wstring m_path;
	std::wstring m_file;
	std::wstring m_ext;

	/*! \brief Resolve path.
	 *
	 * Resolve environment variables in path.
	 * \param path System path including environment variables.
	 */
	void resolve(const std::wstring& path);
};
	
}

#endif	// traktor_Path_H

#ifndef traktor_Library_H
#define traktor_Library_H

#include <string>
#include <vector>
#include "Core/Object.h"
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

/*! \brief System dynamic library.
 * \ingroup Core
 */
class T_DLLCLASS Library : public Object
{
	T_RTTI_CLASS;

public:
	Library();
	
	virtual ~Library();
	
	/*! \brief Open library.
	 *
	 * \param libraryName Name of library.
	 * \return True if library opened successfully.
	 */
	bool open(const Path& libraryName);
	
	/*! \brief Open library.
	 *
	 * This method take user defined search paths
	 * which is used recursively, i.e. dependent
	 * libraries are also searched through the same
	 * paths.
	 *
	 * \param libraryName Name of library.
	 * \param searchPaths Custom search paths.
	 * \param includeDefaultPaths Include default search paths.
	 * \return True if library opened successfully.
	 */
	bool open(const Path& libraryName, const std::vector< Path >& searchPaths, bool includeDefaultPaths);

	/*! \brief Close library. */
	void close();

	/*! \brief Detach library.
	 *
	 * \note
	 * A detached library is not unloaded from the
	 * process until the process terminates.
	 */
	void detach();
	
	/*! \brief Find exported symbol from library.
	 *
	 * \param symbol Name of exported symbol.
	 * \return Pointer to symbol.
	 */
	void* find(const std::wstring& symbol);

	/*! \brief Get library path.
	 *
	 * \return Path to library.
	 */
	Path getPath() const;

private:
	void* m_handle;
};

}

#endif	// traktor_Library_H

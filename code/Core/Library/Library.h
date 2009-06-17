#ifndef traktor_Library_H
#define traktor_Library_H

#include <string>
#include "Core/Object.h"
#include "Core/Io/Path.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief System dynamic library.
 * \ingroup Core
 */
class T_DLLCLASS Library : public Object
{
	T_RTTI_CLASS(Library)

public:
	virtual ~Library();
	
	/*! \brief Open library.
	 *
	 * \param libraryName Name of library.
	 * \return True if library opened successfully.
	 */
	bool open(const Path& libraryName);
	
	/*! \brief Close library. */
	void close();
	
	/*! \brief Find exported symbol from library.
	 *
	 * \param symbol Name of exported symbol.
	 * \return Pointer to symbol.
	 */
	void* find(const std::wstring& symbol);

	/*! \brief Add additional search path for libraries.
	 *
	 * \param searchPath New search path.
	 * \return True if search path added successfully.
	 */
	static bool addSearchPath(const std::wstring& searchPath);

private:
	void* m_handle;
};

}

#endif	// traktor_Library_H

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
	
	bool open(const Path& libraryName);
	
	void close();
	
	void* find(const std::wstring& symbol);

private:
	void* m_handle;
};

}

#endif	// traktor_Library_H

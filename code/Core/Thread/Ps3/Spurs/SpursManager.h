#ifndef traktor_SpursManager_H
#define traktor_SpursManager_H

#include <cell/spurs.h>
#include "Core/Ref.h"
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

class SpursJobQueue;

/*! \brief Spurs job manager.
 * \ingroup Core
 */
class T_DLLCLASS SpursManager : public ISingleton
{
public:
	static SpursManager& getInstance();

	Ref< SpursJobQueue > createJobQueue(uint32_t descriptorSize, uint32_t submitCount);

protected:
	virtual void destroy();

private:
	CellSpurs* m_spurs;

	SpursManager();

	virtual ~SpursManager();
};

}

#endif	// traktor_SpursManager_H

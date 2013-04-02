#ifndef traktor_CallStack_H
#define traktor_CallStack_H

#include "Core/Config.h"

namespace traktor
{

/*! \brief Get call stack.
 *
 * \param ncs Number of call stack entries in cs.
 * \param outCs Array of call stack entries.
 * \return Number of call stack entries.
 */
uint32_t T_DLLCLASS getCallStack(uint32_t ncs, void** outCs, uint32_t skip);

}

#endif	// traktor_CallStack_H

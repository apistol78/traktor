#pragma once

#include <string>
#include "Core/Config.h"

namespace traktor
{

/*! Get call stack.
 *
 * \param ncs Number of call stack entries in cs.
 * \param outCs Array of call stack entries.
 * \return Number of call stack entries.
 */
uint32_t T_DLLCLASS getCallStack(uint32_t ncs, void** outCs, uint32_t skip);

/*! Get symbol from address. */
bool T_DLLCLASS getSymbolFromAddress(const void* address, std::wstring& outSymbol);

/*! Get source and line from address. */
bool T_DLLCLASS getSourceFromAddress(const void* address, std::wstring& outSource, int32_t& outLine);

}


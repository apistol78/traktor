#ifndef Utilities_H
#define Utilities_H

#include <vector>
#include <Core/Config.h>

bool writeFileIfMismatch(const std::wstring& fileName, const std::vector< uint8_t >& data);

#endif	// Utilities_H

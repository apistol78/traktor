/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef Utilities_H
#define Utilities_H

#include <vector>
#include <Core/Config.h>

bool writeFileIfMismatch(const std::wstring& fileName, const std::vector< uint8_t >& data);

#endif	// Utilities_H

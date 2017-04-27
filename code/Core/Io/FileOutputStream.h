/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_FileOutputStream_H
#define traktor_FileOutputStream_H

#include "Core/Io/OutputStream.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IEncoding;
class IStream;

/*! \brief File output stream.
 * \ingroup Core
 */
class T_DLLCLASS FileOutputStream : public OutputStream
{
	T_RTTI_CLASS;

public:
	FileOutputStream(IStream* stream, IEncoding* encoding, LineEnd lineEnd = LeAuto);

	void close();
}; 

}

#endif	// traktor_FileOutputStream_H

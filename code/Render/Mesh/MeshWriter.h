/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_MeshWriter_H
#define traktor_render_MeshWriter_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IStream;

	namespace render
	{

class Mesh;

/*! \brief Render mesh writer.
 * \ingroup Render
 */
class T_DLLCLASS MeshWriter : public Object
{
	T_RTTI_CLASS;

public:
	bool write(IStream* stream, const Mesh* mesh) const;
};

	}
}

#endif	// traktor_render_MeshWriter_H

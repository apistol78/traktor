/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_ProgramResourcePs4_H
#define traktor_render_ProgramResourcePs4_H

#include "Render/Resource/ProgramResource.h"

namespace traktor
{
	namespace render
	{

class BlobPs4;

/*!
 * \ingroup GNM
 */
class ProgramResourcePs4 : public ProgramResource
{
	T_RTTI_CLASS;

public:
	ProgramResourcePs4();

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	friend class ProgramPs4;
	friend class ProgramCompilerPs4;

	Ref< BlobPs4 > m_vertexShader;
	Ref< BlobPs4 > m_pixelShader;
};

	}
}

#endif	// traktor_render_ProgramResourcePs4_H

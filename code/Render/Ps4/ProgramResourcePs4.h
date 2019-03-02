#pragma once

#include "Render/Resource/ProgramResource.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_PS4_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class BlobPs4;

/*!
 * \ingroup GNM
 */
class T_DLLCLASS ProgramResourcePs4 : public ProgramResource
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


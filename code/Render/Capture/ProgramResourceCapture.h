#ifndef traktor_render_ProgramResourceCapture_H
#define traktor_render_ProgramResourceCapture_H

#include <string>
#include "Core/Ref.h"
#include "Render/Resource/ProgramResource.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_CAPTURE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

/*! \brief
 * \ingroup Render
 */
class T_DLLCLASS ProgramResourceCapture : public ProgramResource
{
	T_RTTI_CLASS;

public:
	virtual void serialize(ISerializer& s) override;

private:
	friend class ProgramCompilerCapture;
	friend class RenderSystemCapture;

	Ref< ProgramResource > m_embedded;
	std::wstring m_vertexShader;
	std::wstring m_pixelShader;
	std::wstring m_computeShader;
};

	}
}

#endif	// traktor_render_ProgramResourceCapture_H


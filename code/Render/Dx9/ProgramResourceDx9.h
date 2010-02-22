#ifndef traktor_render_ProgramResourceDx9_H
#define traktor_render_ProgramResourceDx9_H

#include <vector>
#include "Render/Dx9/Platform.h"
#include "Render/Dx9/StateBlockDx9.h"
#include "Render/Dx9/TypesDx9.h"
#include "Render/Resource/ProgramResource.h"
#include "Core/Misc/ComRef.h"

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup DX9 Xbox360
 */
class ProgramResourceDx9 : public ProgramResource
{
	T_RTTI_CLASS;

public:
	ProgramResourceDx9();

	virtual bool serialize(ISerializer& s);

private:
	friend class ProgramWin32;
	friend class ProgramXbox360;
	friend class ProgramCompilerWin32;
	friend class ProgramCompilerXbox360;
	friend class RenderSystemWin32;

	ComRef< ID3DXBuffer > m_vertexShader;
	ComRef< ID3DXBuffer > m_pixelShader;
	uint32_t m_vertexShaderHash;
	uint32_t m_pixelShaderHash;
	std::vector< ProgramScalar > m_vertexScalars;
	std::vector< ProgramScalar > m_pixelScalars;
	std::vector< ProgramSampler > m_vertexSamplers;
	std::vector< ProgramSampler > m_pixelSamplers;
	std::map< std::wstring, uint32_t > m_scalarParameterMap;
	std::map< std::wstring, uint32_t > m_textureParameterMap;
	uint32_t m_scalarParameterDataSize;
	uint32_t m_textureParameterDataSize;
	StateBlockDx9 m_state;
};

	}
}

#endif	// traktor_render_ProgramResourceDx9_H

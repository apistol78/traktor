#ifndef traktor_render_ProgramResourceDx9_H
#define traktor_render_ProgramResourceDx9_H

#include <vector>
#include "Render/Dx9/Platform.h"
#include "Render/Dx9/TypesDx9.h"
#include "Render/Dx9/StateBlockDx9.h"
#include "Render/ProgramResource.h"
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
	T_RTTI_CLASS(ProgramResourceDx9)

public:
	virtual bool serialize(Serializer& s);

private:
	friend class ProgramWin32;
	friend class ProgramXbox360;

	ComRef< ID3DXBuffer > m_vertexShader;
	ComRef< ID3DXBuffer > m_pixelShader;
	StateBlockDx9 m_state;
};

	}
}

#endif	// traktor_render_ProgramResourceDx9_H

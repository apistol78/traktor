#ifndef traktor_render_StateBlockDx9_H
#define traktor_render_StateBlockDx9_H

#include <vector>
#include <map>
#include "Core/Serialization/ISerializable.h"
#include "Render/Dx9/Platform.h"

namespace traktor
{
	namespace render
	{

class ParameterCache;

/*!
 * \ingroup DX9 Xbox360
 */
class StateBlockDx9 : public ISerializable
{
	T_RTTI_CLASS;

public:
	StateBlockDx9();

	StateBlockDx9(const StateBlockDx9& stateBlock);

	void setRenderState(D3DRENDERSTATETYPE state, DWORD value);

	void setVertexSamplerState(DWORD sampler, D3DSAMPLERSTATETYPE type, DWORD value);

	void setPixelSamplerState(DWORD sampler, D3DSAMPLERSTATETYPE type, DWORD value);

	void prepareAnisotropy(int32_t maxAnisotropy);

	void apply(ParameterCache* parameterCache);

	StateBlockDx9& operator = (const StateBlockDx9& stateBlock);

	virtual void serialize(ISerializer& s);

	inline bool isOpaque() const { return m_opaque; }

private:
	std::vector< std::pair< uint32_t, uint32_t > > m_renderStates;
	std::map< uint32_t, std::vector< std::pair< uint32_t, uint32_t > > > m_vertexSamplerStates;
	std::map< uint32_t, std::vector< std::pair< uint32_t, uint32_t > > > m_pixelSamplerStates;
	bool m_opaque;
};

	}
}

#endif	// traktor_render_StateBlockDx9_H

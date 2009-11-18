#ifndef traktor_render_StateBlockDx9_H
#define traktor_render_StateBlockDx9_H

#include <vector>
#include <map>
#include "Render/Dx9/Platform.h"
#include "Core/Serialization/ISerializable.h"

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

	void setSamplerState(DWORD sampler, D3DSAMPLERSTATETYPE type, DWORD value);

	void apply(ParameterCache* parameterCache);

	StateBlockDx9& operator = (const StateBlockDx9& stateBlock);

	virtual int getVersion() const;

	virtual bool serialize(ISerializer& s);

	inline bool isOpaque() const { return m_opaque; }

private:
	std::vector< std::pair< uint32_t, uint32_t > > m_renderStates;
	std::map< uint32_t, std::vector< std::pair< uint32_t, uint32_t > > > m_samplerStates;
	bool m_opaque;
};

	}
}

#endif	// traktor_render_StateBlockDx9_H

#ifndef traktor_render_ProgramXbox360_H
#define traktor_render_ProgramXbox360_H

#include <map>
#include <vector>
#include <string>
#include "Core/RefArray.h"
#include "Core/Misc/ComRef.h"
#include "Render/IProgram.h"
#include "Render/Types.h"
#include "Render/Dx9/StateBlockDx9.h"

namespace traktor
{
	namespace render
	{

class ContextDx9;
class HlslProgram;
class TextureBaseDx9;
class ProgramResourceDx9;
class ParameterCache;

/*!
 * \ingroup DX9
 */
class ProgramXbox360 : public IProgram
{
	T_RTTI_CLASS;

public:
	struct Uniform
	{
		uint16_t registerIndex;
		uint16_t registerCount;
		uint16_t offset;
		uint16_t length;
	};

	struct Sampler
	{
		uint16_t parameter;
		uint16_t stage;
		uint16_t texture;
	};

	ProgramXbox360(ParameterCache* parameterCache);

	virtual ~ProgramXbox360();

	bool create(
		IDirect3DDevice9* d3dDevice,
		const ProgramResourceDx9* resource
	);

	bool activate();

	virtual void destroy();

	virtual void setFloatParameter(handle_t handle, float param);

	virtual void setFloatArrayParameter(handle_t handle, const float* param, int length);
	
	virtual void setVectorParameter(handle_t handle, const Vector4& param);

	virtual void setVectorArrayParameter(handle_t handle, const Vector4* param, int length);

	virtual void setMatrixParameter(handle_t handle, const Matrix44& param);

	virtual void setMatrixArrayParameter(handle_t handle, const Matrix44* param, int length);

	virtual void setTextureParameter(handle_t handle, ITexture* texture);

	virtual void setStencilReference(uint32_t stencilReference);

	virtual bool isOpaque() const;

	/*! \brief Force shader to dirty state.
	 *
	 * Next time any shader is activated the shader's state are
	 * unconditionally uploaded to the device.
	 */
	static void forceDirty();

protected:
	virtual HRESULT lostDevice();

	virtual HRESULT resetDevice(IDirect3DDevice9* d3dDevice);

private:
	static ProgramXbox360* ms_activeProgram;

	ComRef< IDirect3DDevice9 > m_d3dDevice;
	ComRef< IDirect3DVertexShader9 > m_d3dVertexShader;
	ComRef< IDirect3DPixelShader9 > m_d3dPixelShader;
	ParameterCache* m_parameterCache;
	std::vector< Uniform > m_vertexUniforms;
	std::vector< Uniform > m_pixelUniforms;
	std::vector< Sampler > m_vertexSamplers;
	std::vector< Sampler > m_pixelSamplers;
	StateBlockDx9 m_state;
	std::vector< float > m_uniformFloatData;
	RefArray< ITexture > m_samplerTextures;
	bool m_dirty;
};

	}
}

#endif	// traktor_render_ProgramXbox360_H

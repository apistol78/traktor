#ifndef traktor_render_ProgramWin32_H
#define traktor_render_ProgramWin32_H

#include <map>
#include <vector>
#include <string>
#include "Core/Heap/Ref.h"
#include "Render/Program.h"
#include "Render/Types.h"
#include "Render/Dx9/Unmanaged.h"
#include "Render/Dx9/StateBlockDx9.h"
#include "Render/Dx9/ParameterMap.h"
#include "Core/Misc/ComRef.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_DX9_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

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
class T_DLLCLASS ProgramWin32
:	public Program
,	public Unmanaged
{
	T_RTTI_CLASS(ProgramWin32)

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

	ProgramWin32(UnmanagedListener* unmanagedListener, ContextDx9* context, ParameterCache* parameterCache);

	virtual ~ProgramWin32();

	static ProgramResourceDx9* compile(const HlslProgram& hlslProgram, int optimize, bool validate);

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

	virtual void setSamplerTexture(handle_t handle, Texture* texture);

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
	static ProgramWin32* ms_activeProgram;

	Ref< ContextDx9 > m_context;
	ComRef< IDirect3DDevice9 > m_d3dDevice;
	ComRef< IDirect3DVertexShader9 > m_d3dVertexShader;
	ComRef< IDirect3DPixelShader9 > m_d3dPixelShader;
	ParameterCache* m_parameterCache;
	std::vector< Uniform > m_vertexUniforms;
	std::vector< Uniform > m_pixelUniforms;
	std::vector< Sampler > m_vertexSamplers;
	std::vector< Sampler > m_pixelSamplers;
	StateBlockDx9 m_state;
	ParameterMap m_parameterMap;
	std::vector< float > m_uniformFloatData;
	RefArray< Texture > m_samplerTextures;
	bool m_dirty;
};

	}
}

#endif	// traktor_render_ProgramWin32_H

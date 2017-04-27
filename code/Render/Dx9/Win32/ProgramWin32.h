/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_ProgramWin32_H
#define traktor_render_ProgramWin32_H

#include <string>
#include <vector>
#include "Core/RefArray.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Misc/ComRef.h"
#include "Render/IProgram.h"
#include "Render/Types.h"
#include "Render/Dx9/IResourceDx9.h"
#include "Render/Dx9/StateBlockDx9.h"
#include "Render/Dx9/TypesDx9.h"

namespace traktor
{
	namespace render
	{

class HlslProgram;
class ParameterCache;
class ProgramResourceDx9;
class ResourceManagerDx9;
class ShaderCache;
class TextureBaseDx9;

/*!
 * \ingroup DX9
 */
class ProgramWin32
:	public IProgram
,	public IResourceDx9
{
	T_RTTI_CLASS;

public:
	ProgramWin32(ResourceManagerDx9* resourceManager, ParameterCache* parameterCache);

	virtual ~ProgramWin32();

	bool create(
		IDirect3DDevice9* d3dDevice,
		ShaderCache* shaderCache,
		const ProgramResourceDx9* resource,
		int32_t maxAnisotropy
	);

	bool activate();

	/*! \brief Force shader to dirty state.
	*
	* Next time any shader is activated the shader state are
	* unconditionally uploaded to the device.
	*/
	static void forceDirty();

	// \name IProgram
	// \{

	virtual void destroy();

	virtual void setFloatParameter(handle_t handle, float param);

	virtual void setFloatArrayParameter(handle_t handle, const float* param, int length);
	
	virtual void setVectorParameter(handle_t handle, const Vector4& param);

	virtual void setVectorArrayParameter(handle_t handle, const Vector4* param, int length);

	virtual void setMatrixParameter(handle_t handle, const Matrix44& param);

	virtual void setMatrixArrayParameter(handle_t handle, const Matrix44* param, int length);

	virtual void setTextureParameter(handle_t handle, ITexture* texture);

	virtual void setStencilReference(uint32_t stencilReference);

	// \}

	// \name IResourceDx9
	// \{

	virtual HRESULT lostDevice();

	virtual HRESULT resetDevice(IDirect3DDevice9* d3dDevice);

	// \}

private:
	static ProgramWin32* ms_activeProgram;
	Ref< ResourceManagerDx9 > m_resourceManager;
	Ref< ParameterCache > m_parameterCache;
	Ref< const ProgramResourceDx9 > m_resource;
	ComRef< IDirect3DDevice9 > m_d3dDevice;
	ComRef< IDirect3DVertexShader9 > m_d3dVertexShader;
	ComRef< IDirect3DPixelShader9 > m_d3dPixelShader;
	StateBlockDx9 m_state;
	SmallMap< handle_t, uint32_t > m_scalarParameterMap;
	SmallMap< handle_t, uint32_t > m_textureParameterMap;
	AlignedVector< float > m_scalarParameterData;
	RefArray< ITexture > m_textureParameterData;
	bool m_dirty;

#if defined(_DEBUG)
	SmallMap< handle_t, std::wstring > m_scalarParameterNames;
	std::vector< uint8_t > m_scalarParameterDataValid;

	void validateParameter(const ProgramScalar& scalar);
#endif
};

	}
}

#endif	// traktor_render_ProgramWin32_H

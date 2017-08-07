/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_ProgramResourceCapture_H
#define traktor_render_ProgramResourceCapture_H

#include "Core/RefArray.h"
#include "Render/Resource/ProgramResource.h"
#include "Render/Shader/Nodes.h"

namespace traktor
{
	namespace render
	{
	
/*! \brief
 * \ingroup Render
 */
class ProgramResourceCapture : public ProgramResource
{
	T_RTTI_CLASS;

public:
	virtual void serialize(ISerializer& s) T_OVERRIDE;

private:
	friend class ProgramCompilerCapture;
	friend class RenderSystemCapture;

	Ref< ProgramResource > m_embedded;
	RefArray< Uniform > m_uniforms;
	RefArray< IndexedUniform > m_indexedUniforms;
	std::wstring m_vertexShader;
	std::wstring m_pixelShader;
};
	
	}
}

#endif	// traktor_render_ProgramResourceCapture_H


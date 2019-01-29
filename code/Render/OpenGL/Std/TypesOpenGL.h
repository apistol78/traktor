/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_TypesOpenGL_H
#define traktor_render_TypesOpenGL_H

#include "Render/Types.h"
#include "Render/OpenGL/Std/Platform.h"

namespace traktor
{
	namespace render
	{

/*! \ingroup OGL */
//@{

struct NamedUniformType
{
	std::wstring name;
	GLenum type;
	GLuint length;

	NamedUniformType()
	:	type(GL_INVALID_ENUM)
	,	length(0)
	{
	}
};

struct SamplerBindingOpenGL
{
	std::wstring name;
	GLuint unit;
	GLenum target;
	int32_t texture;

	SamplerBindingOpenGL()
	:	unit(0)
	,	target(GL_INVALID_ENUM)
	,	texture(0)
	{
	}
};

struct SamplerStateOpenGL
{
	GLenum minFilter;
	GLenum magFilter;
	GLenum wrapS;
	GLenum wrapT;
	GLenum wrapR;
	GLenum compare;

	SamplerStateOpenGL()
	:	minFilter(GL_LINEAR)
	,	magFilter(GL_LINEAR)
	,	wrapS(GL_REPEAT)
	,	wrapT(GL_REPEAT)
	,	wrapR(GL_REPEAT)
	,	compare(GL_INVALID_ENUM)
	{
	}
};

struct RenderStateOpenGL
{
	enum
	{
		CmRed	= 1,
		CmGreen	= 2,
		CmBlue	= 4,
		CmAlpha	= 8,
		CmAll	= (CmRed | CmGreen | CmBlue | CmAlpha)
	};

	GLboolean cullFaceEnable;
	GLenum cullFace;
	GLboolean blendEnable;
	GLenum blendColorEquation;
	GLenum blendAlphaEquation;
	GLenum blendFuncColorSrc;
	GLenum blendFuncColorDest;
	GLenum blendFuncAlphaSrc;
	GLenum blendFuncAlphaDest;
	GLboolean depthTestEnable;
	uint32_t colorMask;
	GLboolean depthMask;
	GLenum depthFunc;
	GLboolean alphaTestEnable;
	GLenum alphaFunc;
	GLclampf alphaRef;
	GLboolean stencilTestEnable;
	GLenum stencilFunc;
	GLint stencilRef;
	GLenum stencilOpFail;
	GLenum stencilOpZFail;
	GLenum stencilOpZPass;
	SamplerStateOpenGL samplerStates[16];

	RenderStateOpenGL()
	:	cullFaceEnable(GL_TRUE)
	,	cullFace(GL_BACK)
	,	blendEnable(GL_FALSE)
	,	blendColorEquation(GL_FUNC_ADD)
	,	blendAlphaEquation(GL_FUNC_ADD)
	,	blendFuncColorSrc(GL_ONE)
	,	blendFuncColorDest(GL_ZERO)
	,	blendFuncAlphaSrc(GL_ONE)
	,	blendFuncAlphaDest(GL_ZERO)
	,	depthTestEnable(GL_TRUE)
	,	colorMask(CmAll)
	,	depthMask(GL_TRUE)
	,	depthFunc(GL_LEQUAL)
	,	alphaTestEnable(GL_FALSE)
	,	alphaFunc(GL_ALWAYS)
	,	alphaRef(0)
	,	stencilTestEnable(GL_FALSE)
	,	stencilFunc(GL_ALWAYS)
	,	stencilRef(0)
	,	stencilOpFail(GL_KEEP)
	,	stencilOpZFail(GL_KEEP)
	,	stencilOpZPass(GL_KEEP)
	{
	}
};

class T_DLLCLASS VertexAttribute
{;
public:
	static std::wstring getName(DataUsage usage, int32_t index);

	static int32_t getLocation(DataUsage usage, int32_t index);
};

//@}

	}
}

#endif	// traktor_render_TypesOpenGL_H

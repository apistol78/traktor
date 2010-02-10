#ifndef traktor_render_TypesOpenGL_H
#define traktor_render_TypesOpenGL_H

#include "Render/OpenGL/Platform.h"
#include "Render/Types.h"

namespace traktor
{
	namespace render
	{

/*! \ingroup OGL */
//@{

struct SamplerTexture
{
	std::wstring sampler;
	std::wstring texture;
};

struct SamplerState
{
	GLenum minFilter;
	GLenum magFilter;
	GLenum wrapS;
	GLenum wrapT;

	SamplerState()
	:	minFilter(GL_NEAREST)
	,	magFilter(GL_NEAREST)
#if defined(T_OPENGL_STD)
	,	wrapS(GL_CLAMP)
	,	wrapT(GL_CLAMP)
#elif defined(T_OPENGL_ES2)
	,	wrapS(GL_CLAMP_TO_EDGE)
	,	wrapT(GL_CLAMP_TO_EDGE)
#endif
	{
	}
};

struct RenderState
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
	GLenum blendEquation;
	GLenum blendFuncSrc;
	GLenum blendFuncDest;
	GLboolean depthTestEnable;
	unsigned colorMask;
	GLboolean depthMask;
	GLenum depthFunc;
	GLboolean alphaTestEnable;
	GLenum alphaFunc;
	GLclampf alphaRef;
	SamplerState samplerStates[16];

	RenderState()
	:	cullFaceEnable(GL_TRUE)
	,	cullFace(GL_BACK)
	,	blendEnable(GL_FALSE)
	,	blendEquation(GL_FUNC_ADD)
	,	blendFuncSrc(GL_ONE)
	,	blendFuncDest(GL_ZERO)
	,	depthTestEnable(GL_TRUE)
	,	colorMask(CmAll)
	,	depthMask(GL_TRUE)
	,	depthFunc(GL_LEQUAL)
	,	alphaTestEnable(GL_FALSE)
	,	alphaFunc(GL_ALWAYS)
	,	alphaRef(0)
	{
	}
};

//@}

	}
}

#endif	// traktor_render_TypesOpenGL_H

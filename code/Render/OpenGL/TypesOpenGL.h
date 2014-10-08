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
	GLuint stage;
	GLenum target;
	int32_t texture;

	SamplerBindingOpenGL()
	:	stage(0)
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

	SamplerStateOpenGL()
	:	minFilter(GL_LINEAR)
	,	magFilter(GL_LINEAR)
	,	wrapS(GL_REPEAT)
	,	wrapT(GL_REPEAT)
	,	wrapR(GL_REPEAT)
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
	GLenum blendEquation;
	GLenum blendFuncSrc;
	GLenum blendFuncDest;
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
	,	stencilTestEnable(GL_FALSE)
	,	stencilFunc(GL_ALWAYS)
	,	stencilRef(0)
	,	stencilOpFail(GL_KEEP)
	,	stencilOpZFail(GL_KEEP)
	,	stencilOpZPass(GL_KEEP)
	{
	}
};

//@}

	}
}

#endif	// traktor_render_TypesOpenGL_H

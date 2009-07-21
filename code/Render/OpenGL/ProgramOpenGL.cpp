#include "Render/OpenGL/Extensions.h"
#include "Render/OpenGL/ProgramOpenGL.h"
#include "Render/OpenGL/ContextOpenGL.h"
#include "Render/OpenGL/GlslType.h"
#include "Render/OpenGL/SimpleTextureOpenGL.h"
#include "Render/OpenGL/CubeTextureOpenGL.h"
#include "Render/OpenGL/VolumeTextureOpenGL.h"
#include "Render/OpenGL/RenderTargetOpenGL.h"
#include "Render/OpenGL/GlslProgram.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

struct DeleteObjectCallback : public ContextOpenGL::DeleteCallback
{
	GLhandleARB m_objectName;

	DeleteObjectCallback(GLhandleARB objectName)
	:	m_objectName(objectName)
	{
	}

	virtual void deleteResource()
	{
		T_OGL_SAFE(glDeleteObjectARB(m_objectName));
		delete this;
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ProgramOpenGL", ProgramOpenGL, IProgram)

ProgramOpenGL* ProgramOpenGL::ms_activeProgram = 0;

ProgramOpenGL::ProgramOpenGL(ContextOpenGL* context)
:	m_context(context)
,	m_program(0)
,	m_state(0)
,	m_dirty(true)
{
}

ProgramOpenGL::~ProgramOpenGL()
{
	destroy();
}

bool ProgramOpenGL::create(const GlslProgram& glslProgram)
{
	char errorBuf[32000];
	GLsizei errorBufLen;
	GLint status;

	GLhandleARB vertexObject = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);

	std::string vertexShader = wstombs(glslProgram.getVertexShader());
	const char* vertexShaderPtr = vertexShader.c_str();
	T_OGL_SAFE(glShaderSourceARB(vertexObject, 1, &vertexShaderPtr, NULL));
	T_OGL_SAFE(glCompileShaderARB(vertexObject));
	
	T_OGL_SAFE(glGetObjectParameterivARB(vertexObject, GL_OBJECT_COMPILE_STATUS_ARB, &status));
	if (status != 1)
	{
		T_OGL_SAFE(glGetInfoLogARB(vertexObject, sizeof(errorBuf), &errorBufLen, errorBuf));
		if (errorBufLen > 0)
		{
			log::error << L"GLSL vertex shader compile failed :" << Endl;
			log::error << mbstows(errorBuf) << Endl;
			log::error << Endl;
			log::error << glslProgram.getVertexShader() << Endl;
			return false;
		}
	}
	
	GLhandleARB fragmentObject = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);

	std::string fragmentShader = wstombs(glslProgram.getFragmentShader());
	const char* fragmentShaderPtr = fragmentShader.c_str();
	T_OGL_SAFE(glShaderSourceARB(fragmentObject, 1, &fragmentShaderPtr, NULL));
	T_OGL_SAFE(glCompileShaderARB(fragmentObject));

	T_OGL_SAFE(glGetObjectParameterivARB(fragmentObject, GL_OBJECT_COMPILE_STATUS_ARB, &status));
	if (status != 1)
	{
		T_OGL_SAFE(glGetInfoLogARB(fragmentObject, sizeof(errorBuf), &errorBufLen, errorBuf));
		if (errorBufLen > 0)
		{
			log::error << L"GLSL fragment shader compile failed :" << Endl;
			log::error << mbstows(errorBuf) << Endl;
			log::error << Endl;
			log::error << glslProgram.getFragmentShader() << Endl;
			return false;
		}
	}

	m_program = glCreateProgramObjectARB();

	T_OGL_SAFE(glAttachObjectARB(m_program, vertexObject));
	T_OGL_SAFE(glAttachObjectARB(m_program, fragmentObject));
	T_OGL_SAFE(glBindAttribLocationARB(m_program, 0, "in_Position0"));
	T_OGL_SAFE(glLinkProgramARB(m_program));

	T_OGL_SAFE(glGetObjectParameterivARB(m_program, GL_OBJECT_LINK_STATUS_ARB, &status));
	if (status != GL_TRUE)
	{
		T_OGL_SAFE(glGetInfoLogARB(m_program, sizeof(errorBuf), &errorBufLen, errorBuf));
		if (errorBufLen > 0)
		{
			log::error << L"GLSL program link failed :" << Endl;
			log::error << mbstows(errorBuf) << Endl;
			return false;
		}
	}

	GLint uniformCount;
	T_OGL_SAFE(glGetObjectParameterivARB(m_program, GL_OBJECT_ACTIVE_UNIFORMS_ARB, &uniformCount));

	// Merge samplers.
	std::set< std::wstring > samplers;
	samplers.insert(glslProgram.getVertexSamplers().begin(), glslProgram.getVertexSamplers().end());
	samplers.insert(glslProgram.getFragmentSamplers().begin(), glslProgram.getFragmentSamplers().end());

	for (GLint j = 0; j < uniformCount; ++j)
	{
		GLint uniformSize;
		GLenum uniformType;
		GLcharARB uniformName[256];

		T_OGL_SAFE(glGetActiveUniformARB(m_program, j, sizeof(uniformName), 0, &uniformSize, &uniformType, uniformName));

		if (uniformType == GL_SAMPLER_2D_ARB)
		{
			std::set< std::wstring >::iterator it = samplers.find(mbstows(uniformName));
			T_ASSERT (it != samplers.end());

			uint32_t unit = uint32_t(std::distance(samplers.begin(), it));
			handle_t handle = getParameterHandle(mbstows(uniformName));

			if (m_parameterMap.find(handle) == m_parameterMap.end())
			{
				uint32_t texture = uint32_t(m_samplerTextures.size());
				m_parameterMap[handle] = texture;

				SamplerTexture st = { 0, 0, Vector4(0.0f, 0.0f, 1.0f, 1.0f) };
				m_samplerTextures.push_back(st);
			}

			m_samplers.push_back(Sampler());
			m_samplers.back().location = glGetUniformLocationARB(m_program, uniformName);
			m_samplers.back().locationOriginScale = glGetUniformLocationARB(m_program, ("_sampler_" + std::string(uniformName) + "_OriginScale").c_str());
			m_samplers.back().texture = m_parameterMap[handle];
			m_samplers.back().unit = unit;
		}
		else
		{
			// Skip private uniform of format "_sampler_XXX_OriginScale".
			std::wstring uniforNameW = mbstows(uniformName);
			if (startsWith(uniforNameW, L"_sampler_") && endsWith(uniforNameW, L"_OriginScale"))
				continue;

			handle_t handle = getParameterHandle(uniforNameW);
			if (m_parameterMap.find(handle) == m_parameterMap.end())
			{
				uint32_t allocSize = 0;
				switch (uniformType)
				{
				case GL_FLOAT:
					allocSize = 1 * uniformSize;
					break;

				case GL_FLOAT_VEC4_ARB:
					allocSize = 4 * uniformSize;
					break;

				case GL_FLOAT_MAT4_ARB:
					allocSize = 16 * uniformSize;
					break;

				default:
					log::error << L"Invalid uniform type " << uint32_t(uniformType) << Endl;
					return false;
				}

				uint32_t offset = uint32_t(m_uniformData.size());
				m_parameterMap[handle] = offset;

				m_uniformData.resize(offset + allocSize, 0.0f);
				m_uniformDataDirty.resize(offset + allocSize, false);
			}

			m_uniforms.push_back(Uniform());
			m_uniforms.back().location = glGetUniformLocationARB(m_program, uniformName);
			m_uniforms.back().type = uniformType;
			m_uniforms.back().offset = m_parameterMap[handle];
			m_uniforms.back().length = uniformSize;
		}
	}

	for (int j = 0; j < sizeof_array(m_attributeLocs); ++j)
		m_attributeLocs[j] = -1;

	for (int j = 0; j < T_OGL_MAX_INDEX; ++j)
	{
		m_attributeLocs[T_OGL_USAGE_INDEX(DuPosition, j)] = glGetAttribLocationARB(m_program, wstombs(glsl_vertex_attr_name(DuPosition, j)).c_str());
		m_attributeLocs[T_OGL_USAGE_INDEX(DuNormal, j)] = glGetAttribLocationARB(m_program, wstombs(glsl_vertex_attr_name(DuNormal, j)).c_str());
		m_attributeLocs[T_OGL_USAGE_INDEX(DuTangent, j)] = glGetAttribLocationARB(m_program, wstombs(glsl_vertex_attr_name(DuTangent, j)).c_str());
		m_attributeLocs[T_OGL_USAGE_INDEX(DuBinormal, j)] = glGetAttribLocationARB(m_program, wstombs(glsl_vertex_attr_name(DuBinormal, j)).c_str());
		m_attributeLocs[T_OGL_USAGE_INDEX(DuColor, j)] = glGetAttribLocationARB(m_program, wstombs(glsl_vertex_attr_name(DuColor, j)).c_str());
		m_attributeLocs[T_OGL_USAGE_INDEX(DuCustom, j)] = glGetAttribLocationARB(m_program, wstombs(glsl_vertex_attr_name(DuCustom, j)).c_str());
	}

	// Create a display list from the render states.
	const RenderState& renderState = glslProgram.getRenderState();

	m_renderState = renderState;
	m_state = glGenLists(1);

	glNewList(m_state, GL_COMPILE);

	if (renderState.cullFaceEnable)
	{
		T_OGL_SAFE(glEnable(GL_CULL_FACE));
		T_OGL_SAFE(glCullFace(renderState.cullFace));
	}
	else
		T_OGL_SAFE(glDisable(GL_CULL_FACE));

	if (renderState.blendEnable)
	{
		T_OGL_SAFE(glEnable(GL_BLEND));
		T_OGL_SAFE(glBlendFunc(renderState.blendFuncSrc, renderState.blendFuncDest));
		T_OGL_SAFE(glBlendEquationEXT(renderState.blendEquation));
	}
	else
		T_OGL_SAFE(glDisable(GL_BLEND));

	if (renderState.depthTestEnable)
	{
		T_OGL_SAFE(glEnable(GL_DEPTH_TEST));
		T_OGL_SAFE(glDepthFunc(renderState.depthFunc));
	}
	else
		T_OGL_SAFE(glDisable(GL_DEPTH_TEST));

	T_OGL_SAFE(glColorMask(
		(renderState.colorMask & RenderState::CmRed) ? GL_TRUE : GL_FALSE,
		(renderState.colorMask & RenderState::CmGreen) ? GL_TRUE : GL_FALSE,
		(renderState.colorMask & RenderState::CmBlue) ? GL_TRUE : GL_FALSE,
		(renderState.colorMask & RenderState::CmAlpha) ? GL_TRUE : GL_FALSE
	));

	T_OGL_SAFE(glDepthMask(renderState.depthMask));

	if (renderState.alphaTestEnable)
	{
		T_OGL_SAFE(glEnable(GL_ALPHA_TEST));
		T_OGL_SAFE(glAlphaFunc(renderState.alphaFunc, renderState.alphaRef));
	}
	else
		T_OGL_SAFE(glDisable(GL_ALPHA_TEST));

	glEndList();
	return true;
}

void ProgramOpenGL::destroy()
{
	if (ms_activeProgram == this)
	{
		ms_activeProgram = 0;
		m_dirty = true;
	}

	if (m_program)
	{
		if (m_context)
			m_context->deleteResource(new DeleteObjectCallback(m_program));
		m_program = 0;
	}
}

void ProgramOpenGL::setFloatParameter(handle_t handle, float param)
{
	setFloatArrayParameter(handle, &param, 1);
}

void ProgramOpenGL::setFloatArrayParameter(handle_t handle, const float* param, int length)
{
	std::map< handle_t, uint32_t >::iterator i = m_parameterMap.find(handle);
	if (i == m_parameterMap.end())
		return;

	std::memcpy(&m_uniformData[i->second], param, length * sizeof(float));

	m_uniformDataDirty[i->second] = true;
	m_dirty = true;
}

void ProgramOpenGL::setVectorParameter(handle_t handle, const Vector4& param)
{
	setFloatArrayParameter(handle, reinterpret_cast< const float* >(&param), 4);
}

void ProgramOpenGL::setVectorArrayParameter(handle_t handle, const Vector4* param, int length)
{
	setFloatArrayParameter(handle, reinterpret_cast< const float* >(param), length * 4);
}

void ProgramOpenGL::setMatrixParameter(handle_t handle, const Matrix44& param)
{
	setFloatArrayParameter(handle, reinterpret_cast< const float* >(&param), 16);
}

void ProgramOpenGL::setMatrixArrayParameter(handle_t handle, const Matrix44* param, int length)
{
	setFloatArrayParameter(handle, reinterpret_cast< const float* >(param), length * 16);
}

void ProgramOpenGL::setSamplerTexture(handle_t handle, ITexture* texture)
{
	std::map< handle_t, uint32_t >::iterator i = m_parameterMap.find(handle);
	if (i == m_parameterMap.end())
		return;

	if (SimpleTextureOpenGL* st = dynamic_type_cast< SimpleTextureOpenGL* >(texture))
	{
		m_samplerTextures[i->second].target = GL_TEXTURE_2D;
		m_samplerTextures[i->second].name = st->getTextureName();
		m_samplerTextures[i->second].originScale = st->getTextureOriginAndScale();
		m_samplerTextures[i->second].mipCount = st->getMipCount();
	}
	else if (CubeTextureOpenGL* ct = dynamic_type_cast< CubeTextureOpenGL* >(texture))
	{
#if !defined(__APPLE__)
		m_samplerTextures[i->second].target = GL_TEXTURE_CUBE_MAP_EXT;
#else
		m_samplerTextures[i->second].target = GL_TEXTURE_CUBE_MAP_ARB;
#endif
		m_samplerTextures[i->second].name = ct->getTextureName();
		m_samplerTextures[i->second].originScale = Vector4(0.0f, 0.0f, 1.0f, 1.0f);
		m_samplerTextures[i->second].mipCount = 1;
	}
	else if (VolumeTextureOpenGL* vt = dynamic_type_cast< VolumeTextureOpenGL* >(texture))
	{
		m_samplerTextures[i->second].target = GL_TEXTURE_3D;
		m_samplerTextures[i->second].name = vt->getTextureName();
		m_samplerTextures[i->second].originScale = Vector4(0.0f, 0.0f, 1.0f, 1.0f);
		m_samplerTextures[i->second].mipCount = 1;
	}
	else if (RenderTargetOpenGL* rt = dynamic_type_cast< RenderTargetOpenGL* >(texture))
	{
		m_samplerTextures[i->second].target = rt->getTextureTarget();
		m_samplerTextures[i->second].name = rt->getTextureName();
		m_samplerTextures[i->second].originScale = rt->getTextureOriginAndScale();
		m_samplerTextures[i->second].mipCount = 1;
	}

	m_dirty = true;
}

void ProgramOpenGL::setStencilReference(uint32_t stencilReference)
{
}

bool ProgramOpenGL::isOpaque() const
{
	return true;
}

bool ProgramOpenGL::activate()
{
	if (ms_activeProgram == this && !m_dirty)
		return true;

	if (ms_activeProgram != this)
	{
		T_OGL_SAFE(glCallList(m_state));

		T_ASSERT (m_program);
		T_OGL_SAFE(glUseProgramObjectARB(m_program));

		for (std::vector< Uniform >::iterator i = m_uniforms.begin(); i != m_uniforms.end(); ++i)
		{
			const float* uniformData = &m_uniformData[i->offset];

			switch (i->type)
			{
			case GL_FLOAT:
				T_OGL_SAFE(glUniform1fvARB(i->location, i->length, uniformData));
				break;

			case GL_FLOAT_VEC4_ARB:
				T_OGL_SAFE(glUniform4fvARB(i->location, i->length, uniformData));
				break;

			case GL_FLOAT_MAT4_ARB:
				T_OGL_SAFE(glUniformMatrix4fvARB(i->location, i->length, GL_TRUE, uniformData));
				break;

			default:
				T_ASSERT (0);
			}

			m_uniformDataDirty[i->offset] = false;
		}
	}
	else
	{
		for (std::vector< Uniform >::iterator i = m_uniforms.begin(); i != m_uniforms.end(); ++i)
		{
			if (m_uniformDataDirty[i->offset])
			{
				const float* uniformData = &m_uniformData[i->offset];

				switch (i->type)
				{
				case GL_FLOAT:
					T_OGL_SAFE(glUniform1fvARB(i->location, i->length, uniformData));
					break;

				case GL_FLOAT_VEC4_ARB:
					T_OGL_SAFE(glUniform4fvARB(i->location, i->length, uniformData));
					break;

				case GL_FLOAT_MAT4_ARB:
					T_OGL_SAFE(glUniformMatrix4fvARB(i->location, i->length, GL_TRUE, uniformData));
					break;
					
				default:
					T_ASSERT (0);
				}

				m_uniformDataDirty[i->offset] = false;
			}
		}
	}

	if (!m_samplers.empty())
	{
		for (std::vector< Sampler >::iterator i = m_samplers.begin(); i != m_samplers.end(); ++i)
		{
			const SamplerTexture& st = m_samplerTextures[i->texture];
			if (!st.target)
				continue;

			T_OGL_SAFE(glEnable(st.target));

			T_OGL_SAFE(glActiveTexture(GL_TEXTURE0 + i->unit));
			T_OGL_SAFE(glBindTexture(st.target, st.name));

			if (st.mipCount > 1)
			{
				T_OGL_SAFE(glTexParameteri(st.target, GL_TEXTURE_MIN_FILTER, m_renderState.samplerStates[i->unit].minFilter));
			}
			else
			{
				T_OGL_SAFE(glTexParameteri(st.target, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
			}

			//T_OGL_SAFE(glTexParameteri(st.target, GL_TEXTURE_MAG_FILTER, m_renderState.samplerStates[i->unit].magFilter));
			//T_OGL_SAFE(glTexParameteri(st.target, GL_TEXTURE_WRAP_S, m_renderState.samplerStates[i->unit].wrapS));
			//T_OGL_SAFE(glTexParameteri(st.target, GL_TEXTURE_WRAP_T, m_renderState.samplerStates[i->unit].wrapT));

			T_OGL_SAFE(glUniform1iARB(i->location, i->unit));
			T_OGL_SAFE(glUniform4fARB(i->locationOriginScale, st.originScale.x(), st.originScale.y(), st.originScale.z(), st.originScale.w()));
		}
	}
	else
	{
		T_OGL_SAFE(glDisable(GL_TEXTURE_2D));
#if !defined(__APPLE__)
		T_OGL_SAFE(glDisable(GL_TEXTURE_CUBE_MAP_EXT));
#else
		T_OGL_SAFE(glDisable(GL_TEXTURE_CUBE_MAP_ARB));
#endif
	}

	ms_activeProgram = this;
	m_dirty = false;

	return true;
}

const GLint* ProgramOpenGL::getAttributeLocs() const
{
	return m_attributeLocs;
}

	}
}

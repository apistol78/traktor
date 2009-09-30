#include "Render/OpenGL/Platform.h"
#include "Render/OpenGL/IContext.h"
#include "Render/OpenGL/GlslType.h"
#include "Render/OpenGL/GlslProgram.h"
#include "Render/OpenGL/ProgramResourceOpenGL.h"
#include "Render/OpenGL/ES2/ProgramOpenGLES2.h"
#include "Render/OpenGL/ES2/ProgramResourceOpenGLES2.h"
#include "Render/OpenGL/ES2/SimpleTextureOpenGLES2.h"
#include "Render/OpenGL/ES2/RenderTargetOpenGLES2.h"
#include "Core/Heap/GcNew.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Core/Log/Log.h"

#define T_COMPILE_BINARY_PROGRAMS 0

#if T_COMPILE_BINARY_PROGRAMS
#	include <SBEntryPoints.h>
#endif

namespace traktor
{
	namespace render
	{
		namespace
		{

struct DeleteProgramCallback : public IContext::IDeleteCallback
{
	GLuint m_program;

	DeleteProgramCallback(GLuint program)
	:	m_program(program)
	{
	}

	virtual void deleteResource()
	{
		T_OGL_SAFE(glDeleteProgram(m_program));
		delete this;
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ProgramOpenGLES2", ProgramOpenGLES2, IProgram)

ProgramOpenGLES2* ProgramOpenGLES2::ms_activeProgram = 0;

ProgramOpenGLES2::ProgramOpenGLES2(IContext* context)
:	m_context(context)
,	m_program(0)
,	m_dirty(true)
{
}

ProgramOpenGLES2::~ProgramOpenGLES2()
{
	destroy();
}

ProgramResource* ProgramOpenGLES2::compile(const GlslProgram& glslProgram, int optimize, bool validate)
{
	Ref< ProgramResource > resource;

#if T_COMPILE_BINARY_PROGRAMS

	std::string vertexShader = wstombs(glslProgram.getVertexShader());
	std::string fragmentShader = wstombs(glslProgram.getFragmentShader());

	void* buffer = 0;
	int bufferSize = 0;

	// Use AMD OpenGL ES 2.0 Emulator library.
	if (!SBCreateBinary(
		vertexShader.c_str(),
		fragmentShader.c_str(),
		&buffer,
		&bufferSize,
		SB_BINARYTYPE_Z400,
		SB_SUPPRESS_SRC
	))
		return 0;

	resource = gc_new< ProgramResourceOpenGLES2 >(
		buffer,
		bufferSize,
		cref(glslProgram.getVertexSamplers()),
		cref(glslProgram.getFragmentSamplers()),
		cref(glslProgram.getRenderState())
	);

	SBFreeBuffer(buffer, SB_BINARYTYPE_Z400);

#else

	resource = gc_new< ProgramResourceOpenGL >(
		cref(glslProgram.getVertexShader()),
		cref(glslProgram.getFragmentShader()),
		cref(glslProgram.getVertexSamplers()),
		cref(glslProgram.getFragmentSamplers()),
		cref(glslProgram.getRenderState())
	);

#endif

	return resource;
}

bool ProgramOpenGLES2::create(const ProgramResource* resource)
{
	if (is_a< ProgramResourceOpenGL >(resource))
		return createFromSource(resource);
	else if (is_a< ProgramResourceOpenGLES2 >(resource))
		return createFromBinary(resource);
	else
		return 0;
}

void ProgramOpenGLES2::destroy()
{
	if (ms_activeProgram == this)
	{
		ms_activeProgram = 0;
		m_dirty = true;
	}

	if (m_program)
	{
		if (m_context)
			m_context->deleteResource(new DeleteProgramCallback(m_program));
		m_program = 0;
	}
}

void ProgramOpenGLES2::setFloatParameter(handle_t handle, float param)
{
	setFloatArrayParameter(handle, &param, 1);
}

void ProgramOpenGLES2::setFloatArrayParameter(handle_t handle, const float* param, int length)
{
	std::map< handle_t, uint32_t >::iterator i = m_parameterMap.find(handle);
	if (i == m_parameterMap.end())
		return;

	std::memcpy(&m_uniformData[i->second], param, length * sizeof(float));

	m_uniformDataDirty[i->second] = true;
	m_dirty = true;
}

void ProgramOpenGLES2::setVectorParameter(handle_t handle, const Vector4& param)
{
	setFloatArrayParameter(handle, reinterpret_cast< const float* >(&param), 4);
}

void ProgramOpenGLES2::setVectorArrayParameter(handle_t handle, const Vector4* param, int length)
{
	setFloatArrayParameter(handle, reinterpret_cast< const float* >(param), length * 4);
}

void ProgramOpenGLES2::setMatrixParameter(handle_t handle, const Matrix44& param)
{
	setFloatArrayParameter(handle, reinterpret_cast< const float* >(&param), 16);
}

void ProgramOpenGLES2::setMatrixArrayParameter(handle_t handle, const Matrix44* param, int length)
{
	setFloatArrayParameter(handle, reinterpret_cast< const float* >(param), length * 16);
}

void ProgramOpenGLES2::setSamplerTexture(handle_t handle, ITexture* texture)
{
	std::map< handle_t, uint32_t >::iterator i = m_parameterMap.find(handle);
	if (i == m_parameterMap.end())
		return;

	if (SimpleTextureOpenGLES2* st = dynamic_type_cast< SimpleTextureOpenGLES2* >(texture))
	{
		m_samplerTextures[i->second].target = GL_TEXTURE_2D;
		m_samplerTextures[i->second].name = st->getTextureName();
		m_samplerTextures[i->second].originScale = st->getTextureOriginAndScale();
		m_samplerTextures[i->second].mipCount = st->getMipCount();
	}
	else if (RenderTargetOpenGLES2* rt = dynamic_type_cast< RenderTargetOpenGLES2* >(texture))
	{
		m_samplerTextures[i->second].target = rt->getTextureTarget();
		m_samplerTextures[i->second].name = rt->getTextureName();
		m_samplerTextures[i->second].originScale = rt->getTextureOriginAndScale();
		m_samplerTextures[i->second].mipCount = 1;
	}

	m_dirty = true;
}

void ProgramOpenGLES2::setStencilReference(uint32_t stencilReference)
{
}

bool ProgramOpenGLES2::isOpaque() const
{
	return true;
}

bool ProgramOpenGLES2::activate()
{
	if (ms_activeProgram == this && !m_dirty)
		return true;

	if (ms_activeProgram != this)
	{
		if (m_renderState.cullFaceEnable)
		{
			T_OGL_SAFE(glEnable(GL_CULL_FACE));
			T_OGL_SAFE(glCullFace(m_renderState.cullFace));
		}
		else
			T_OGL_SAFE(glDisable(GL_CULL_FACE));

		if (m_renderState.blendEnable)
		{
			T_OGL_SAFE(glEnable(GL_BLEND));
			T_OGL_SAFE(glBlendFunc(m_renderState.blendFuncSrc, m_renderState.blendFuncDest));
			T_OGL_SAFE(glBlendEquation(m_renderState.blendEquation));
		}
		else
			T_OGL_SAFE(glDisable(GL_BLEND));

		if (m_renderState.depthTestEnable)
		{
			T_OGL_SAFE(glEnable(GL_DEPTH_TEST));
			T_OGL_SAFE(glDepthFunc(m_renderState.depthFunc));
		}
		else
			T_OGL_SAFE(glDisable(GL_DEPTH_TEST));

		T_OGL_SAFE(glColorMask(
			(m_renderState.colorMask & RenderState::CmRed) ? GL_TRUE : GL_FALSE,
			(m_renderState.colorMask & RenderState::CmGreen) ? GL_TRUE : GL_FALSE,
			(m_renderState.colorMask & RenderState::CmBlue) ? GL_TRUE : GL_FALSE,
			(m_renderState.colorMask & RenderState::CmAlpha) ? GL_TRUE : GL_FALSE
		));

		T_OGL_SAFE(glDepthMask(m_renderState.depthMask));

		//if (m_renderState.alphaTestEnable)
		//{
		//	T_OGL_SAFE(glEnable(GL_ALPHA_TEST));
		//	T_OGL_SAFE(glAlphaFunc(m_renderState.alphaFunc, m_renderState.alphaRef));
		//}
		//else
		//	T_OGL_SAFE(glDisable(GL_ALPHA_TEST));

		T_ASSERT (m_program);
		T_OGL_SAFE(glUseProgram(m_program));

		for (std::vector< Uniform >::iterator i = m_uniforms.begin(); i != m_uniforms.end(); ++i)
		{
			const float* uniformData = &m_uniformData[i->offset];

			switch (i->type)
			{
			case GL_FLOAT:
				T_OGL_SAFE(glUniform1fv(i->location, i->length, uniformData));
				break;

			case GL_FLOAT_VEC4:
				T_OGL_SAFE(glUniform4fv(i->location, i->length, uniformData));
				break;

			case GL_FLOAT_MAT4:
				T_OGL_SAFE(glUniformMatrix4fv(i->location, i->length, GL_FALSE, uniformData));
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
					T_OGL_SAFE(glUniform1fv(i->location, i->length, uniformData));
					break;

				case GL_FLOAT_VEC4:
					T_OGL_SAFE(glUniform4fv(i->location, i->length, uniformData));
					break;

				case GL_FLOAT_MAT4:
					T_OGL_SAFE(glUniformMatrix4fv(i->location, i->length, GL_FALSE, uniformData));
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

			//T_OGL_SAFE(glEnable(st.target));
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

			T_OGL_SAFE(glUniform1i(i->location, i->unit));
			T_OGL_SAFE(glUniform4f(i->locationOriginScale, st.originScale.x(), st.originScale.y(), st.originScale.z(), st.originScale.w()));
		}
	}
	else
	{
		//T_OGL_SAFE(glDisable(GL_TEXTURE_2D));
	}

	ms_activeProgram = this;
	m_dirty = false;

	return true;
}

const GLint* ProgramOpenGLES2::getAttributeLocs() const
{
	return m_attributeLocs;
}

bool ProgramOpenGLES2::createFromSource(const ProgramResource* resource)
{
	const ProgramResourceOpenGL* resourceOpenGL = checked_type_cast< const ProgramResourceOpenGL* >(resource);

	std::string vertexShader = wstombs(resourceOpenGL->getVertexShader());
	const char* vertexShaderPtr = vertexShader.c_str();

	std::string fragmentShader = wstombs(resourceOpenGL->getFragmentShader());
	const char* fragmentShaderPtr = fragmentShader.c_str();

	char errorBuf[32000];
	GLsizei errorBufLen;
	GLint status;

	m_program = glCreateProgram();

	GLuint vertexObject = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentObject = glCreateShader(GL_FRAGMENT_SHADER);

	T_OGL_SAFE(glShaderSource(vertexObject, 1, &vertexShaderPtr, NULL));
	T_OGL_SAFE(glCompileShader(vertexObject));
	T_OGL_SAFE(glGetShaderiv(vertexObject, GL_COMPILE_STATUS, &status));
	if (status != 1)
	{
		T_OGL_SAFE(glGetShaderInfoLog(vertexObject, sizeof(errorBuf), &errorBufLen, errorBuf));
		if (errorBufLen > 0)
		{
			log::error << L"GLSL vertex shader compile failed :" << Endl;
			log::error << mbstows(errorBuf) << Endl;
			log::error << Endl;
			FormatMultipleLines(log::error, resourceOpenGL->getVertexShader());
			return false;
		}
	}
	
	T_OGL_SAFE(glShaderSource(fragmentObject, 1, &fragmentShaderPtr, NULL));
	T_OGL_SAFE(glCompileShader(fragmentObject));
	T_OGL_SAFE(glGetShaderiv(fragmentObject, GL_COMPILE_STATUS, &status));
	if (status != 1)
	{
		T_OGL_SAFE(glGetShaderInfoLog(fragmentObject, sizeof(errorBuf), &errorBufLen, errorBuf));
		if (errorBufLen > 0)
		{
			log::error << L"GLSL fragment shader compile failed :" << Endl;
			log::error << mbstows(errorBuf) << Endl;
			log::error << Endl;
			FormatMultipleLines(log::error, resourceOpenGL->getFragmentShader());
			return false;
		}
	}

	T_OGL_SAFE(glAttachShader(m_program, vertexObject));
	T_OGL_SAFE(glAttachShader(m_program, fragmentObject));
	T_OGL_SAFE(glBindAttribLocation(m_program, 0, "in_Position0"));
	T_OGL_SAFE(glLinkProgram(m_program));

	T_OGL_SAFE(glGetProgramiv(m_program, GL_LINK_STATUS, &status));
	if (status != GL_TRUE)
	{
		T_OGL_SAFE(glGetProgramInfoLog(m_program, sizeof(errorBuf), &errorBufLen, errorBuf));
		if (errorBufLen > 0)
		{
			log::error << L"GLSL program link failed :" << Endl;
			log::error << mbstows(errorBuf) << Endl;
			return false;
		}
	}

	GLint uniformCount;
	T_OGL_SAFE(glGetProgramiv(m_program, GL_ACTIVE_UNIFORMS, &uniformCount));

	// Merge samplers.
	std::set< std::wstring > samplers;
	samplers.insert(resourceOpenGL->getVertexSamplers().begin(), resourceOpenGL->getVertexSamplers().end());
	samplers.insert(resourceOpenGL->getFragmentSamplers().begin(), resourceOpenGL->getFragmentSamplers().end());

	for (GLint j = 0; j < uniformCount; ++j)
	{
		GLint uniformSize;
		GLenum uniformType;
		char uniformName[256];

		T_OGL_SAFE(glGetActiveUniform(m_program, j, sizeof(uniformName), 0, &uniformSize, &uniformType, uniformName));

		if (uniformType == GL_SAMPLER_2D)
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
			m_samplers.back().location = glGetUniformLocation(m_program, uniformName);
			m_samplers.back().locationOriginScale = glGetUniformLocation(m_program, ("_sampler_" + std::string(uniformName) + "_OriginScale").c_str());
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

				case GL_FLOAT_VEC4:
					allocSize = 4 * uniformSize;
					break;

				case GL_FLOAT_MAT4:
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
			m_uniforms.back().location = glGetUniformLocation(m_program, uniformName);
			m_uniforms.back().type = uniformType;
			m_uniforms.back().offset = m_parameterMap[handle];
			m_uniforms.back().length = uniformSize;
		}
	}

	for (int j = 0; j < sizeof_array(m_attributeLocs); ++j)
		m_attributeLocs[j] = -1;

	for (int j = 0; j < T_OGL_MAX_INDEX; ++j)
	{
		m_attributeLocs[T_OGL_USAGE_INDEX(DuPosition, j)] = glGetAttribLocation(m_program, wstombs(glsl_vertex_attr_name(DuPosition, j)).c_str());
		m_attributeLocs[T_OGL_USAGE_INDEX(DuNormal, j)] = glGetAttribLocation(m_program, wstombs(glsl_vertex_attr_name(DuNormal, j)).c_str());
		m_attributeLocs[T_OGL_USAGE_INDEX(DuTangent, j)] = glGetAttribLocation(m_program, wstombs(glsl_vertex_attr_name(DuTangent, j)).c_str());
		m_attributeLocs[T_OGL_USAGE_INDEX(DuBinormal, j)] = glGetAttribLocation(m_program, wstombs(glsl_vertex_attr_name(DuBinormal, j)).c_str());
		m_attributeLocs[T_OGL_USAGE_INDEX(DuColor, j)] = glGetAttribLocation(m_program, wstombs(glsl_vertex_attr_name(DuColor, j)).c_str());
		m_attributeLocs[T_OGL_USAGE_INDEX(DuCustom, j)] = glGetAttribLocation(m_program, wstombs(glsl_vertex_attr_name(DuCustom, j)).c_str());
	}

	m_renderState = resourceOpenGL->getRenderState();
	return true;
}

bool ProgramOpenGLES2::createFromBinary(const ProgramResource* resource)
{
	const ProgramResourceOpenGLES2* resourceOpenGL = checked_type_cast< const ProgramResourceOpenGLES2* >(resource);

	char errorBuf[32000];
	GLsizei errorBufLen;
	GLint status;

	GLuint vertexObject = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentObject = glCreateShader(GL_FRAGMENT_SHADER);

	GLuint objects[] = { vertexObject, fragmentObject };
#if defined(GL_Z400_BINARY_AMD)
	T_OGL_SAFE(glShaderBinary(
		2,
		objects,
		GL_Z400_BINARY_AMD,
		resourceOpenGL->getBuffer(),
		GLint(resourceOpenGL->getBufferSize())
	));
#else
	T_FATAL_ERROR;
#endif

	m_program = glCreateProgram();

	T_OGL_SAFE(glAttachShader(m_program, vertexObject));
	T_OGL_SAFE(glAttachShader(m_program, fragmentObject));
	T_OGL_SAFE(glBindAttribLocation(m_program, 0, "in_Position0"));
	T_OGL_SAFE(glLinkProgram(m_program));

	T_OGL_SAFE(glGetProgramiv(m_program, GL_LINK_STATUS, &status));
	if (status != GL_TRUE)
	{
		T_OGL_SAFE(glGetProgramInfoLog(m_program, sizeof(errorBuf), &errorBufLen, errorBuf));
		if (errorBufLen > 0)
		{
			log::error << L"GLSL program link failed :" << Endl;
			log::error << mbstows(errorBuf) << Endl;
			return false;
		}
	}

	GLint uniformCount;
	T_OGL_SAFE(glGetProgramiv(m_program, GL_ACTIVE_UNIFORMS, &uniformCount));

	// Merge samplers.
	std::set< std::wstring > samplers;
	samplers.insert(resourceOpenGL->getVertexSamplers().begin(), resourceOpenGL->getVertexSamplers().end());
	samplers.insert(resourceOpenGL->getFragmentSamplers().begin(), resourceOpenGL->getFragmentSamplers().end());

	for (GLint j = 0; j < uniformCount; ++j)
	{
		GLint uniformSize;
		GLenum uniformType;
		char uniformName[256];

		T_OGL_SAFE(glGetActiveUniform(m_program, j, sizeof(uniformName), 0, &uniformSize, &uniformType, uniformName));

		if (uniformType == GL_SAMPLER_2D)
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
			m_samplers.back().location = glGetUniformLocation(m_program, uniformName);
			m_samplers.back().locationOriginScale = glGetUniformLocation(m_program, ("_sampler_" + std::string(uniformName) + "_OriginScale").c_str());
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

				case GL_FLOAT_VEC4:
					allocSize = 4 * uniformSize;
					break;

				case GL_FLOAT_MAT4:
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
			m_uniforms.back().location = glGetUniformLocation(m_program, uniformName);
			m_uniforms.back().type = uniformType;
			m_uniforms.back().offset = m_parameterMap[handle];
			m_uniforms.back().length = uniformSize;
		}
	}

	for (int j = 0; j < sizeof_array(m_attributeLocs); ++j)
		m_attributeLocs[j] = -1;

	for (int j = 0; j < T_OGL_MAX_INDEX; ++j)
	{
		m_attributeLocs[T_OGL_USAGE_INDEX(DuPosition, j)] = glGetAttribLocation(m_program, wstombs(glsl_vertex_attr_name(DuPosition, j)).c_str());
		m_attributeLocs[T_OGL_USAGE_INDEX(DuNormal, j)] = glGetAttribLocation(m_program, wstombs(glsl_vertex_attr_name(DuNormal, j)).c_str());
		m_attributeLocs[T_OGL_USAGE_INDEX(DuTangent, j)] = glGetAttribLocation(m_program, wstombs(glsl_vertex_attr_name(DuTangent, j)).c_str());
		m_attributeLocs[T_OGL_USAGE_INDEX(DuBinormal, j)] = glGetAttribLocation(m_program, wstombs(glsl_vertex_attr_name(DuBinormal, j)).c_str());
		m_attributeLocs[T_OGL_USAGE_INDEX(DuColor, j)] = glGetAttribLocation(m_program, wstombs(glsl_vertex_attr_name(DuColor, j)).c_str());
		m_attributeLocs[T_OGL_USAGE_INDEX(DuCustom, j)] = glGetAttribLocation(m_program, wstombs(glsl_vertex_attr_name(DuCustom, j)).c_str());
	}

	m_renderState = resourceOpenGL->getRenderState();
	return true;
}

	}
}

#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Render/OpenGL/Platform.h"
#include "Render/OpenGL/IContext.h"
#include "Render/OpenGL/GlslType.h"
#include "Render/OpenGL/GlslProgram.h"
#include "Render/OpenGL/ProgramResourceOpenGL.h"
#include "Render/OpenGL/ES2/ProgramOpenGLES2.h"
#include "Render/OpenGL/ES2/ProgramResourceOpenGLES2.h"
#include "Render/OpenGL/ES2/SimpleTextureOpenGLES2.h"
#include "Render/OpenGL/ES2/RenderTargetOpenGLES2.h"

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

struct FindSamplerTexture
{
	std::wstring m_sampler;

	FindSamplerTexture(const std::wstring& sampler)
		:	m_sampler(sampler)
	{
	}

	bool operator () (const SamplerTexture& st) const
	{
		return st.sampler == m_sampler;
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ProgramOpenGLES2", ProgramOpenGLES2, IProgram)

ProgramOpenGLES2* ProgramOpenGLES2::ms_activeProgram = 0;

ProgramOpenGLES2::ProgramOpenGLES2(IContext* context)
:	m_context(context)
,	m_program(0)
,	m_postOrientationCoeffs(0)
,	m_dirty(true)
{
}

ProgramOpenGLES2::~ProgramOpenGLES2()
{
	destroy();
}

Ref< ProgramResource > ProgramOpenGLES2::compile(const GlslProgram& glslProgram, int optimize, bool validate)
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

	resource = new ProgramResourceOpenGLES2(
		buffer,
		bufferSize,
		glslProgram.getSamplerTextures(),
		glslProgram.getRenderState()
	);

	SBFreeBuffer(buffer, SB_BINARYTYPE_Z400);

#else

	resource = new ProgramResourceOpenGL(
		glslProgram.getVertexShader(),
		glslProgram.getFragmentShader(),
		glslProgram.getSamplerTextures(),
		glslProgram.getRenderState()
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

void ProgramOpenGLES2::setTextureParameter(handle_t handle, ITexture* texture)
{
	std::map< handle_t, uint32_t >::iterator i = m_parameterMap.find(handle);
	if (i == m_parameterMap.end())
		return;

#if !defined(T_OFFLINE_ONLY)

	if (SimpleTextureOpenGLES2* st = dynamic_type_cast< SimpleTextureOpenGLES2* >(texture))
	{
		m_textureData[i->second].target = GL_TEXTURE_2D;
		m_textureData[i->second].name = st->getTextureName();
	}
	else if (RenderTargetOpenGLES2* rt = dynamic_type_cast< RenderTargetOpenGLES2* >(texture))
	{
		m_textureData[i->second].target = rt->getTextureTarget();
		m_textureData[i->second].name = rt->getTextureName();
	}

#endif

	m_dirty = true;
}

void ProgramOpenGLES2::setStencilReference(uint32_t stencilReference)
{
}

bool ProgramOpenGLES2::activate(bool landspace)
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

		// Update post orientation coefficients.
		if (!landspace)
			glUniform4f(m_postOrientationCoeffs, 1.0f, 0.0f, 0.0f, 1.0f);
		else
			glUniform4f(m_postOrientationCoeffs, 0.0f, -1.0f, 1.0f, 0.0f);

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

	for (uint32_t i = 0; i < m_samplers.size(); ++i)
	{
		const Sampler& sampler = m_samplers[i];
		const SamplerState& samplerState = m_renderState.samplerStates[sampler.stage];
		const TextureData& td = m_textureData[sampler.texture];

		T_OGL_SAFE(glActiveTexture(GL_TEXTURE0 + i));
		T_OGL_SAFE(glBindTexture(td.target, td.name));

		T_OGL_SAFE(glTexParameteri(td.target, GL_TEXTURE_MIN_FILTER, samplerState.minFilter));
		T_OGL_SAFE(glTexParameteri(td.target, GL_TEXTURE_MAG_FILTER, samplerState.magFilter));
		T_OGL_SAFE(glTexParameteri(td.target, GL_TEXTURE_WRAP_S, samplerState.wrapS));
		T_OGL_SAFE(glTexParameteri(td.target, GL_TEXTURE_WRAP_T, samplerState.wrapT));

		T_OGL_SAFE(glUniform1i(sampler.location, i));
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
	GLint status;

	m_program = glCreateProgram();

	GLuint vertexObject = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentObject = glCreateShader(GL_FRAGMENT_SHADER);

	T_OGL_SAFE(glShaderSource(vertexObject, 1, &vertexShaderPtr, NULL));
	T_OGL_SAFE(glCompileShader(vertexObject));
	T_OGL_SAFE(glGetShaderiv(vertexObject, GL_COMPILE_STATUS, &status));
	if (status == 0)
	{
		T_OGL_SAFE(glGetShaderInfoLog(vertexObject, sizeof(errorBuf), 0, errorBuf));
		log::error << L"GLSL vertex shader compile failed :" << Endl;
		log::error << mbstows(errorBuf) << Endl;
		log::error << Endl;
		FormatMultipleLines(log::error, resourceOpenGL->getVertexShader());
		return false;
	}

	T_OGL_SAFE(glShaderSource(fragmentObject, 1, &fragmentShaderPtr, NULL));
	T_OGL_SAFE(glCompileShader(fragmentObject));
	T_OGL_SAFE(glGetShaderiv(fragmentObject, GL_COMPILE_STATUS, &status));
	if (status == 0)
	{
		T_OGL_SAFE(glGetShaderInfoLog(fragmentObject, sizeof(errorBuf), 0, errorBuf));
		log::error << L"GLSL fragment shader compile failed :" << Endl;
		log::error << mbstows(errorBuf) << Endl;
		log::error << Endl;
		FormatMultipleLines(log::error, resourceOpenGL->getFragmentShader());
		return false;
	}

	T_OGL_SAFE(glAttachShader(m_program, vertexObject));
	T_OGL_SAFE(glAttachShader(m_program, fragmentObject));
	T_OGL_SAFE(glBindAttribLocation(m_program, 0, "in_Position0"));
	T_OGL_SAFE(glLinkProgram(m_program));

	T_OGL_SAFE(glGetProgramiv(m_program, GL_LINK_STATUS, &status));
	if (status == 0)
	{
		T_OGL_SAFE(glGetProgramInfoLog(m_program, sizeof(errorBuf), 0, errorBuf));
		log::error << L"GLSL program link failed :" << Endl;
		log::error << mbstows(errorBuf) << Endl;
		return false;
	}

	// Get post orientation uniform.
	m_postOrientationCoeffs = glGetUniformLocation(m_program, "t_internal_postOrientationCoeffs");

	// Map texture parameters.
	const std::vector< SamplerTexture >& samplerTextures = resourceOpenGL->getSamplerTextures();
	for (std::vector< SamplerTexture >::const_iterator i = samplerTextures.begin(); i != samplerTextures.end(); ++i)
	{
		handle_t handle = getParameterHandle(i->texture);
		if (m_parameterMap.find(handle) == m_parameterMap.end())
		{
			m_parameterMap[handle] = m_textureData.size();

			m_textureData.push_back(TextureData());
			m_textureData.back().target = 0;
			m_textureData.back().name = 0;
		}
	}

	// Map samplers and uniforms.
	GLint uniformCount;
	T_OGL_SAFE(glGetProgramiv(m_program, GL_ACTIVE_UNIFORMS, &uniformCount));

	for (GLint j = 0; j < uniformCount; ++j)
	{
		GLint uniformSize;
		GLenum uniformType;
		char uniformName[256];

		T_OGL_SAFE(glGetActiveUniform(m_program, j, sizeof(uniformName), 0, &uniformSize, &uniformType, uniformName));
		std::wstring uniformNameW = mbstows(uniformName);

		// Trim indexed uniforms; seems to vary dependending on OGL implementation.
		size_t p = uniformNameW.find('[');
		if (p != uniformNameW.npos)
			uniformNameW = uniformNameW.substr(0, p);
		if (uniformNameW.empty())
			continue;

		if (uniformType == GL_SAMPLER_2D)
		{
			const std::vector< SamplerTexture >& samplerTextures = resourceOpenGL->getSamplerTextures();

			std::vector< SamplerTexture >::const_iterator it = std::find_if(samplerTextures.begin(), samplerTextures.end(), FindSamplerTexture(uniformNameW));
			T_ASSERT (it != samplerTextures.end());

			Sampler sampler;
			sampler.location = glGetUniformLocation(m_program, uniformName);
			sampler.texture = m_parameterMap[getParameterHandle(it->texture)];
			sampler.stage = uint32_t(std::distance(samplerTextures.begin(), it));

			m_samplers.push_back(sampler);
		}
		else
		{
			handle_t handle = getParameterHandle(uniformNameW);
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
					log::error << L"Invalid uniform \"" << uniformNameW << L"\", type " << uint32_t(uniformType) << L" not supported" << Endl;
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

	// Map texture parameters.
	const std::vector< SamplerTexture >& samplerTextures = resourceOpenGL->getSamplerTextures();
	for (std::vector< SamplerTexture >::const_iterator i = samplerTextures.begin(); i != samplerTextures.end(); ++i)
	{
		handle_t handle = getParameterHandle(i->texture);
		if (m_parameterMap.find(handle) == m_parameterMap.end())
		{
			m_parameterMap[handle] = m_textureData.size();

			m_textureData.push_back(TextureData());
			m_textureData.back().target = 0;
			m_textureData.back().name = 0;
		}
	}

	// Map samplers and uniforms.
	GLint uniformCount;
	T_OGL_SAFE(glGetProgramiv(m_program, GL_ACTIVE_UNIFORMS, &uniformCount));

	for (GLint j = 0; j < uniformCount; ++j)
	{
		GLint uniformSize;
		GLenum uniformType;
		char uniformName[256];

		T_OGL_SAFE(glGetActiveUniform(m_program, j, sizeof(uniformName), 0, &uniformSize, &uniformType, uniformName));
		std::wstring uniformNameW = mbstows(uniformName);

		// Trim indexed uniforms; seems to vary dependending on OGL implementation.
		size_t p = uniformNameW.find('[');
		if (p != uniformNameW.npos)
			uniformNameW = uniformNameW.substr(0, p);
		if (uniformNameW.empty())
			continue;

		if (uniformType == GL_SAMPLER_2D)
		{
			const std::vector< SamplerTexture >& samplerTextures = resourceOpenGL->getSamplerTextures();

			std::vector< SamplerTexture >::const_iterator it = std::find_if(samplerTextures.begin(), samplerTextures.end(), FindSamplerTexture(uniformNameW));
			T_ASSERT (it != samplerTextures.end());

			Sampler sampler;
			sampler.location = glGetUniformLocation(m_program, uniformName);
			sampler.texture = m_parameterMap[getParameterHandle(it->texture)];
			sampler.stage = uint32_t(std::distance(samplerTextures.begin(), it));

			m_samplers.push_back(sampler);
		}
		else
		{
			handle_t handle = getParameterHandle(uniformNameW);
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

#include <cstring>
#include "Render/OpenGL/Platform.h"
#include "Render/OpenGL/GlslType.h"
#include "Render/OpenGL/GlslProgram.h"
#include "Render/OpenGL/ProgramResourceOpenGL.h"
#include "Render/OpenGL/Std/Extensions.h"
#include "Render/OpenGL/Std/ProgramOpenGL.h"
#include "Render/OpenGL/Std/SimpleTextureOpenGL.h"
#include "Render/OpenGL/Std/CubeTextureOpenGL.h"
#include "Render/OpenGL/Std/VolumeTextureOpenGL.h"
#include "Render/OpenGL/Std/RenderTargetOpenGL.h"
#include "Render/OpenGL/Std/ContextOpenGL.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

struct DeleteObjectCallback : public IContext::IDeleteCallback
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

struct DeleteListCallback : public IContext::IDeleteCallback
{
	GLuint m_listName;

	DeleteListCallback(GLuint listName)
	:	m_listName(listName)
	{
	}

	virtual void deleteResource()
	{
		T_OGL_SAFE(glDeleteLists(m_listName, 1));
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

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ProgramOpenGL", ProgramOpenGL, IProgram)

ProgramOpenGL* ProgramOpenGL::ms_activeProgram = 0;

ProgramOpenGL::ProgramOpenGL(ContextOpenGL* context)
:	m_context(context)
,	m_program(0)
,	m_state(0)
,	m_locationTargetSize(0)
,	m_dirty(true)
{
}

ProgramOpenGL::~ProgramOpenGL()
{
	destroy();
}

Ref< ProgramResource > ProgramOpenGL::compile(const GlslProgram& glslProgram, int optimize, bool validate)
{
	Ref< ProgramResource > resource;

	resource = new ProgramResourceOpenGL(
		glslProgram.getVertexShader(),
		glslProgram.getFragmentShader(),
		glslProgram.getSamplerTextures(),
		glslProgram.getRenderState()
	);

	return resource;
}

bool ProgramOpenGL::create(const ProgramResource* resource)
{
	const ProgramResourceOpenGL* resourceOpenGL = checked_type_cast< const ProgramResourceOpenGL* >(resource);

	std::string vertexShader = wstombs(resourceOpenGL->getVertexShader());
	const char* vertexShaderPtr = vertexShader.c_str();

	std::string fragmentShader = wstombs(resourceOpenGL->getFragmentShader());
	const char* fragmentShaderPtr = fragmentShader.c_str();

	char errorBuf[32000];
	GLsizei errorBufLen;
	GLint status;

	GLhandleARB vertexObject = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
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
			log::error << resourceOpenGL->getVertexShader() << Endl;
			return false;
		}
	}

	GLhandleARB fragmentObject = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
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
			log::error << resourceOpenGL->getFragmentShader() << Endl;
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
	
	// Get target size parameter.
	m_locationTargetSize = glGetUniformLocationARB(m_program, "_gl_targetSize");

	// Map texture parameters.
	const std::map< std::wstring, int32_t >& samplerTextures = resourceOpenGL->getSamplerTextures();
	for (std::map< std::wstring, int32_t >::const_iterator i = samplerTextures.begin(); i != samplerTextures.end(); ++i)
	{
		handle_t handle = getParameterHandle(i->first);
		
		if (m_parameterMap.find(handle) == m_parameterMap.end())
		{
			m_parameterMap[handle].offset = m_textureData.size();

			m_textureData.push_back(TextureData());
			m_textureData.back().target = 0;
			m_textureData.back().name = 0;
		}
		
		std::wstring samplerName = L"_gl_sampler_" + toString(i->second);
		std::wstring samplerOffset = L"_gl_sampler_" + toString(i->second) + L"_offset";
		
		Sampler sampler;
		sampler.locationTexture = glGetUniformLocationARB(m_program, wstombs(samplerName).c_str());
		sampler.locationOffset = glGetUniformLocationARB(m_program, wstombs(samplerOffset).c_str());
		sampler.texture = m_parameterMap[handle].offset;
		sampler.stage = i->second;

		m_samplers.push_back(sampler);
	}

	// Map samplers and uniforms.
	GLint uniformCount;
	T_OGL_SAFE(glGetObjectParameterivARB(m_program, GL_OBJECT_ACTIVE_UNIFORMS_ARB, &uniformCount));

	for (GLint j = 0; j < uniformCount; ++j)
	{
		GLint uniformSize;
		GLenum uniformType;
		GLcharARB uniformName[256];

		T_OGL_SAFE(glGetActiveUniformARB(m_program, j, sizeof(uniformName), 0, &uniformSize, &uniformType, uniformName));
		std::wstring uniformNameW = mbstows(uniformName);
		
		// Skip uniforms which starts with _gl_ as they are private.
		if (startsWith(uniformNameW, L"_gl_"))
			continue;

		// Trim indexed uniforms; seems to vary dependending on OGL implementation.
		size_t p = uniformNameW.find('[');
		if (p != uniformNameW.npos)
			uniformNameW = uniformNameW.substr(0, p);

		if (uniformType != GL_SAMPLER_2D_ARB)
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
				m_parameterMap[handle].offset = offset;
				m_parameterMap[handle].length = uniformSize;

				m_uniformData.resize(offset + allocSize, 0.0f);

				m_uniforms.push_back(Uniform());
				m_uniforms.back().location = glGetUniformLocationARB(m_program, uniformName);
				m_uniforms.back().type = uniformType;
				m_uniforms.back().offset = m_parameterMap[handle].offset;
				m_uniforms.back().length = m_parameterMap[handle].length;
			}
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
	const RenderState& renderState = resourceOpenGL->getRenderState();
	m_renderState = renderState;
	m_state = m_context->createStateList(renderState);

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
	std::map< handle_t, Parameter >::iterator i = m_parameterMap.find(handle);
	if (i == m_parameterMap.end())
		return;

	length = min< int >(i->second.length, length);

	std::memcpy(&m_uniformData[i->second.offset], param, length * sizeof(float));
	m_dirty = true;
}

void ProgramOpenGL::setVectorParameter(handle_t handle, const Vector4& param)
{
	std::map< handle_t, Parameter >::iterator i = m_parameterMap.find(handle);
	if (i == m_parameterMap.end())
		return;

	param.storeUnaligned(&m_uniformData[i->second.offset]);
	m_dirty = true;
}

void ProgramOpenGL::setVectorArrayParameter(handle_t handle, const Vector4* param, int length)
{
	std::map< handle_t, Parameter >::iterator i = m_parameterMap.find(handle);
	if (i == m_parameterMap.end())
		return;

	length = min< int >(i->second.length, length);

	for (int j = 0; j < length; ++j)
		param[j].storeUnaligned(&m_uniformData[i->second.offset + j * 4]);

	m_dirty = true;
}

void ProgramOpenGL::setMatrixParameter(handle_t handle, const Matrix44& param)
{
	std::map< handle_t, Parameter >::iterator i = m_parameterMap.find(handle);
	if (i == m_parameterMap.end())
		return;

	param.storeUnaligned(&m_uniformData[i->second.offset]);
	m_dirty = true;
}

void ProgramOpenGL::setMatrixArrayParameter(handle_t handle, const Matrix44* param, int length)
{
	std::map< handle_t, Parameter >::iterator i = m_parameterMap.find(handle);
	if (i == m_parameterMap.end())
		return;

	length = min< int >(i->second.length, length);

	for (int j = 0; j < length; ++j)
		param[j].storeUnaligned(&m_uniformData[i->second.offset + j * 16]);

	m_dirty = true;
}

void ProgramOpenGL::setTextureParameter(handle_t handle, ITexture* texture)
{
	std::map< handle_t, Parameter >::iterator i = m_parameterMap.find(handle);
	if (i == m_parameterMap.end())
		return;

	if (SimpleTextureOpenGL* st = dynamic_type_cast< SimpleTextureOpenGL* >(texture))
	{
		m_textureData[i->second.offset].target = GL_TEXTURE_2D;
		m_textureData[i->second.offset].name = st->getTextureName();
		m_textureData[i->second.offset].mipCount = st->getMipCount();
		st->getTextureOriginAndScale().storeUnaligned(m_textureData[i->second.offset].offset);
	}
	else if (CubeTextureOpenGL* ct = dynamic_type_cast< CubeTextureOpenGL* >(texture))
	{
#if !defined(__APPLE__)
		m_textureData[i->second.offset].target = GL_TEXTURE_CUBE_MAP_EXT;
#else
		m_textureData[i->second.offset].target = GL_TEXTURE_CUBE_MAP_ARB;
#endif
		m_textureData[i->second.offset].name = ct->getTextureName();
		m_textureData[i->second.offset].mipCount = 1;
	}
	else if (VolumeTextureOpenGL* vt = dynamic_type_cast< VolumeTextureOpenGL* >(texture))
	{
		m_textureData[i->second.offset].target = GL_TEXTURE_3D;
		m_textureData[i->second.offset].name = vt->getTextureName();
		m_textureData[i->second.offset].mipCount = 1;
	}
	else if (RenderTargetOpenGL* rt = dynamic_type_cast< RenderTargetOpenGL* >(texture))
	{
		m_textureData[i->second.offset].target = rt->getTextureTarget();
		m_textureData[i->second.offset].name = rt->getTextureName();
		m_textureData[i->second.offset].mipCount = 1;
		rt->getTextureOriginAndScale().storeUnaligned(m_textureData[i->second.offset].offset);
	}

	m_dirty = true;
}

void ProgramOpenGL::setStencilReference(uint32_t stencilReference)
{
}

bool ProgramOpenGL::activate(float targetSize[2])
{
	if (ms_activeProgram == this && !m_dirty)
		return true;
	
	if (ms_activeProgram != this)
	{
		m_context->callStateList(m_state);

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
				T_OGL_SAFE(glUniformMatrix4fvARB(i->location, i->length, GL_FALSE, uniformData));
				break;

			default:
				T_ASSERT (0);
			}
		}
	}
	else
	{
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
				T_OGL_SAFE(glUniformMatrix4fvARB(i->location, i->length, GL_FALSE, uniformData));
				break;

			default:
				T_ASSERT (0);
			}
		}
	}

	if (m_locationTargetSize != -1)
		T_OGL_SAFE(glUniform2fvARB(m_locationTargetSize, 1, targetSize));

	for (uint32_t i = 0; i < m_samplers.size(); ++i)
	{
		const Sampler& sampler = m_samplers[i];
		const SamplerState& samplerState = m_renderState.samplerStates[sampler.stage];
		const TextureData& td = m_textureData[sampler.texture];

		T_OGL_SAFE(glActiveTexture(GL_TEXTURE0 + i));
		T_OGL_SAFE(glBindTexture(td.target, td.name));
		
		// Override mip filter if texture doesn't have multiple mips.
		if (td.mipCount > 1)
		{
			T_OGL_SAFE(glTexParameteri(td.target, GL_TEXTURE_MIN_FILTER, samplerState.minFilter));
		}
		else
		{
			if (samplerState.minFilter != GL_NEAREST)
			{
				T_OGL_SAFE(glTexParameteri(td.target, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
			}
			else
			{
				T_OGL_SAFE(glTexParameteri(td.target, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
			}
		}

		T_OGL_SAFE(glTexParameteri(td.target, GL_TEXTURE_MAG_FILTER, samplerState.magFilter));

		T_OGL_SAFE(glTexParameteri(td.target, GL_TEXTURE_WRAP_S, samplerState.wrapS));
		T_OGL_SAFE(glTexParameteri(td.target, GL_TEXTURE_WRAP_T, samplerState.wrapT));

		T_OGL_SAFE(glUniform1iARB(sampler.locationTexture, i));
		
		if (sampler.locationOffset != -1)
			T_OGL_SAFE(glUniform4fvARB(sampler.locationOffset, 1, td.offset));
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

#include <cstring>
#include "Core/Log/Log.h"
#include "Core/Misc/Align.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Core/Serialization/DeepHash.h"
#include "Render/OpenGL/Platform.h"
#include "Render/OpenGL/GlslType.h"
#include "Render/OpenGL/GlslProgram.h"
#include "Render/OpenGL/ProgramResourceOpenGL.h"
#include "Render/OpenGL/ES2/ProgramOpenGLES2.h"
#include "Render/OpenGL/ES2/SimpleTextureOpenGLES2.h"
#include "Render/OpenGL/ES2/RenderTargetOpenGLES2.h"
#include "Render/OpenGL/ES2/StateCache.h"
#include "Render/OpenGL/ES2/ContextOpenGLES2.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

struct DeleteProgramCallback : public IContext::IDeleteCallback
{
	GLuint m_programName;

	DeleteProgramCallback(GLuint programName)
	:	m_programName(programName)
	{
	}

	virtual void deleteResource()
	{
		T_OGL_SAFE(glDeleteProgram(m_programName));
		delete this;
	}
};

bool storeIfNotEqual(const float* source, int length, float* dest)
{
	for (int i = 0; i < length; ++i)
	{
		if (dest[i] != source[i])
		{
			for (; i < length; ++i)
				dest[i] = source[i];
			return true;
		}
	}
	return false;
}

bool storeIfNotEqual(const Vector4* source, int length, float* dest)
{
	for (int i = 0; i < length; ++i)
	{
		if (Vector4::loadAligned(&dest[i * 4]) != source[i])
		{
			for (; i < length; ++i)
				source[i].storeAligned(&dest[i * 4]);
			return true;
		}
	}
	return false;	
}

bool storeIfNotEqual(const Matrix44* source, int length, float* dest)
{
	for (int i = 0; i < length; ++i)
	{
		if (Matrix44::loadAligned(&dest[i * 4 * 4]) != source[i])
		{
			for (; i < length; ++i)
				source[i].storeAligned(&dest[i * 4 * 4]);
			return true;
		}
	}
	return false;
}

void bindAttribute(GLuint programObject, DataUsage usage, int32_t index)
{
	std::string attributeName = wstombs(glsl_vertex_attr_name(usage, index));	

	int32_t attributeLocation = glsl_vertex_attr_location(usage, index);
	if (attributeLocation < 0)
		return;
	
	T_OGL_SAFE(glBindAttribLocation(
		programObject,
		attributeLocation,
		attributeName.c_str()
	));
}

std::map< uint32_t, ProgramOpenGLES2* > s_programCache;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ProgramOpenGLES2", ProgramOpenGLES2, IProgram)

ProgramOpenGLES2* ProgramOpenGLES2::ms_current = 0;

ProgramOpenGLES2::~ProgramOpenGLES2()
{
	destroy();
}

Ref< ProgramResource > ProgramOpenGLES2::compile(const GlslProgram& glslProgram, int optimize, bool validate)
{
	Ref< ProgramResourceOpenGL > resource;

	resource = new ProgramResourceOpenGL(
		wstombs(glslProgram.getVertexShader()),
		wstombs(glslProgram.getFragmentShader()),
		//glslProgram.getSamplerTextures(),
		glslProgram.getTextures(),
		glslProgram.getSamplers(),
		glslProgram.getRenderState()
	);

	uint32_t hash = DeepHash(resource).get();
	resource->setHash(hash);

	return resource;
}

Ref< ProgramOpenGLES2 > ProgramOpenGLES2::create(ContextOpenGLES2* resourceContext, const ProgramResource* resource)
{
#if !defined(T_OFFLINE_ONLY)

	const ProgramResourceOpenGL* resourceOpenGL = checked_type_cast< const ProgramResourceOpenGL* >(resource);
	char errorBuf[32000];
	GLsizei errorBufLen;
	GLint status;
	
	uint32_t hash = resourceOpenGL->getHash();

	std::map< uint32_t, ProgramOpenGLES2* >::iterator i = s_programCache.find(hash);
	if (i != s_programCache.end())
		return i->second;
	
	const std::string& vertexShader = resourceOpenGL->getVertexShader();
	const std::string& fragmentShader = resourceOpenGL->getFragmentShader();

	GLuint vertexObject = resourceContext->createShaderObject(vertexShader.c_str(), GL_VERTEX_SHADER);
	if (!vertexObject)
	{
		log::error << L"Unable to create vertex object" << Endl;
		return 0;
	}
		
	GLuint fragmentObject = resourceContext->createShaderObject(fragmentShader.c_str(), GL_FRAGMENT_SHADER);
	if (!fragmentObject)
	{
		log::error << L"Unable to create fragment object" << Endl;
		return 0;
	}

	GLuint programObject = glCreateProgram();
	T_ASSERT (programObject != 0);

	T_OGL_SAFE(glAttachShader(programObject, vertexObject));
	T_OGL_SAFE(glAttachShader(programObject, fragmentObject));
	
	for (int j = 0; j < T_OGL_MAX_INDEX; ++j)
	{
		bindAttribute(programObject, DuPosition, j);
		bindAttribute(programObject, DuNormal, j);
		bindAttribute(programObject, DuTangent, j);
		bindAttribute(programObject, DuBinormal, j);
		bindAttribute(programObject, DuColor, j);
		bindAttribute(programObject, DuCustom, j);
	}
	
	T_OGL_SAFE(glLinkProgram(programObject));

	T_OGL_SAFE(glGetProgramiv(programObject, GL_LINK_STATUS, &status));
	if (status != GL_TRUE)
	{
		T_OGL_SAFE(glGetProgramInfoLog(programObject, sizeof(errorBuf), &errorBufLen, errorBuf));
		if (errorBufLen > 0)
		{
			log::error << L"GLSL program link failed :" << Endl;
			log::error << mbstows(errorBuf) << Endl;
			return 0;
		}
	}

	Ref< ProgramOpenGLES2 > program = new ProgramOpenGLES2(resourceContext, programObject, resource);
	s_programCache.insert(std::make_pair(hash, program));
	
	return program;
	
#else
	return 0;
#endif
}

void ProgramOpenGLES2::destroy()
{
#if !defined(T_OFFLINE_ONLY)

	if (ms_current == this)
		ms_current = 0;

	for (std::map< uint32_t, ProgramOpenGLES2* >::iterator i = s_programCache.begin(); i != s_programCache.end(); ++i)
	{
		if (i->second == this)
		{
			s_programCache.erase(i);
			break;
		}
	}

	if (m_program)
	{
		if (m_resourceContext)
			m_resourceContext->deleteResource(new DeleteProgramCallback(m_program));
		m_program = 0;
	}
	
#endif
}

void ProgramOpenGLES2::setFloatParameter(handle_t handle, float param)
{
	setFloatArrayParameter(handle, &param, 1);
}

void ProgramOpenGLES2::setFloatArrayParameter(handle_t handle, const float* param, int length)
{
	SmallMap< handle_t, uint32_t >::iterator i = m_parameterMap.find(handle);
	if (i == m_parameterMap.end())
		return;
		
	Uniform& uniform = m_uniforms[i->second];
	length = std::min< int >(length, uniform.length);

	if (storeIfNotEqual(param, length, &m_uniformData[uniform.offset]))
		uniform.dirty = true;
}

void ProgramOpenGLES2::setVectorParameter(handle_t handle, const Vector4& param)
{
	setVectorArrayParameter(handle, &param, 1);
}

void ProgramOpenGLES2::setVectorArrayParameter(handle_t handle, const Vector4* param, int length)
{
	SmallMap< handle_t, uint32_t >::iterator i = m_parameterMap.find(handle);
	if (i == m_parameterMap.end())
		return;
		
	Uniform& uniform = m_uniforms[i->second];
	length = std::min< int >(length, uniform.length);

	if (storeIfNotEqual(param, length, &m_uniformData[uniform.offset]))
		uniform.dirty = true;
}

void ProgramOpenGLES2::setMatrixParameter(handle_t handle, const Matrix44& param)
{
	setMatrixArrayParameter(handle, &param, 1);
}

void ProgramOpenGLES2::setMatrixArrayParameter(handle_t handle, const Matrix44* param, int length)
{
	SmallMap< handle_t, uint32_t >::iterator i = m_parameterMap.find(handle);
	if (i == m_parameterMap.end())
		return;
		
	Uniform& uniform = m_uniforms[i->second];
	length = std::min< int >(length, uniform.length);

	if (storeIfNotEqual(param, length, &m_uniformData[uniform.offset]))
		uniform.dirty = true;
}

void ProgramOpenGLES2::setTextureParameter(handle_t handle, ITexture* texture)
{
#if !defined(T_OFFLINE_ONLY)
	SmallMap< handle_t, uint32_t >::iterator i = m_parameterMap.find(handle);
	if (i != m_parameterMap.end())
		m_textures[i->second] = texture;
#endif
}

void ProgramOpenGLES2::setStencilReference(uint32_t stencilReference)
{
	m_renderState.stencilRef = stencilReference;
}

bool ProgramOpenGLES2::activate(StateCache* stateCache, float targetSize[2], float postTransform[4], bool invertCull, uint32_t instanceID)
{
#if !defined(T_OFFLINE_ONLY)

	// Bind program and set state display list.
	stateCache->setRenderState(m_renderState, invertCull);
	stateCache->setProgram(m_program);
	
	// Update dirty uniforms.
	for (std::vector< Uniform >::iterator i = m_uniforms.begin(); i != m_uniforms.end(); ++i)
	{
		if (!i->dirty)
			continue;
			
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
		
		i->dirty = false;
	}

	// Update target size uniform if necessary.
	if (m_locationTargetSize != -1)
	{
		if (m_targetSize[0] != targetSize[0] || m_targetSize[1] != targetSize[1])
		{
			m_targetSize[0] = targetSize[0];
			m_targetSize[1] = targetSize[1];
			T_OGL_SAFE(glUniform4fv(m_locationTargetSize, 1, m_targetSize));
		}
	}
	
	// Update post transform.
	if (m_locationPostTransform != -1)
	{
		T_OGL_SAFE(glUniform4fv(m_locationPostTransform, 1, postTransform));
	}

	// Update instance id.
	if (m_locationInstanceID != -1)
	{
		T_OGL_SAFE(glUniform1f(m_locationInstanceID, GLfloat(instanceID)));
	}

	// Bind textures.
	T_ASSERT (m_samplers.size() <= 8);
	uint32_t nsamplers = m_samplers.size();
	for (uint32_t i = 0; i < nsamplers; ++i)
	{
		const Sampler& sampler = m_samplers[i];
		const SamplerStateOpenGL& samplerState = m_renderState.samplerStates[sampler.stage];

		if (!m_textures[sampler.texture])
			continue;

		Ref< ITexture > resolved = m_textures[sampler.texture]->resolve();
		if (!resolved)
			continue;

		ITextureBinding* binding = 0;

		if (SimpleTextureOpenGLES2* st = dynamic_type_cast< SimpleTextureOpenGLES2* >(resolved))
			binding = static_cast< ITextureBinding* >(st);
		else if (RenderTargetOpenGLES2* rt = dynamic_type_cast< RenderTargetOpenGLES2* >(resolved))
			binding = static_cast< ITextureBinding* >(rt);
			
		if (binding)
		{
			binding->bindSampler(
				i,
				samplerState,
				sampler.locationTexture
			);
		}
	}
	
#endif

	ms_current = this;
	return true;
}

ProgramOpenGLES2::ProgramOpenGLES2(ContextOpenGLES2* resourceContext, GLuint program, const ProgramResource* resource)
:	m_resourceContext(resourceContext)
,	m_program(program)
,	m_locationTargetSize(0)
,	m_locationPostTransform(0)
,	m_locationInstanceID(0)
{
	const ProgramResourceOpenGL* resourceOpenGL = checked_type_cast< const ProgramResourceOpenGL* >(resource);

	m_targetSize[0] =
	m_targetSize[1] =
	m_targetSize[2] =
	m_targetSize[3] = 0.0f;
	
	// Get target size parameter.
	m_locationTargetSize = glGetUniformLocation(m_program, "_gl_targetSize");
	m_locationPostTransform = glGetUniformLocation(m_program, "_gl_postTransform");
	m_locationInstanceID = glGetUniformLocation(m_program, "_gl_instanceID");

	const std::vector< std::wstring >& textures = resourceOpenGL->getTextures();
	const std::vector< SamplerBindingOpenGL >& samplers = resourceOpenGL->getSamplers();

	// Map texture parameters.
	for (std::vector< SamplerBindingOpenGL >::const_iterator i = samplers.begin(); i != samplers.end(); ++i)
	{
		const std::wstring& texture = textures[i->texture];

		handle_t handle = getParameterHandle(texture);
		if (m_parameterMap.find(handle) == m_parameterMap.end())
		{
			m_parameterMap[handle] = m_textures.size();
			m_textures.push_back(0);
		}
		
		std::wstring samplerName = L"_gl_sampler_" + texture + L"_" + toString(i->stage);
		
		Sampler sampler;
		sampler.locationTexture = glGetUniformLocation(m_program, wstombs(samplerName).c_str());
		sampler.texture = m_parameterMap[handle];
		sampler.stage = i->stage;

		m_samplers.push_back(sampler);
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
		
		// Skip uniforms which starts with _gl_ as they are private.
		if (startsWith< std::wstring >(uniformNameW, L"_gl_"))
			continue;

		// Trim indexed uniforms; seems to vary dependending on OGL implementation.
		size_t p = uniformNameW.find('[');
		if (p != uniformNameW.npos)
			uniformNameW = uniformNameW.substr(0, p);

		if (uniformType != GL_SAMPLER_2D)
		{
			handle_t handle = getParameterHandle(uniformNameW);
			if (m_parameterMap.find(handle) != m_parameterMap.end())
				continue;

			uint32_t allocSize = 0;
			switch (uniformType)
			{
			case GL_FLOAT:
				allocSize = alignUp(1 * uniformSize, 4);
				break;

			case GL_FLOAT_VEC4:
				allocSize = 4 * uniformSize;
				break;

			case GL_FLOAT_MAT4:
				allocSize = 16 * uniformSize;
				break;

			default:
				log::error << L"Invalid uniform type " << uint32_t(uniformType) << Endl;
				break;
			}

			uint32_t offsetUniform = uint32_t(m_uniforms.size());
			uint32_t offsetData = uint32_t(m_uniformData.size());
			
			m_parameterMap[handle] = offsetUniform;

			m_uniforms.push_back(Uniform());
			m_uniforms.back().location = glGetUniformLocation(m_program, uniformName);
			m_uniforms.back().type = uniformType;
			m_uniforms.back().offset = offsetData;
			m_uniforms.back().length = uniformSize;
			m_uniforms.back().dirty = true;

			m_uniformData.resize(offsetData + allocSize, 0.0f);
		}
	}

	/*
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
	*/

	// Create a display list from the render states.
	m_renderState = resourceOpenGL->getRenderState();
}

	}
}

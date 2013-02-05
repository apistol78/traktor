#include <cstring>
#include "Core/Log/Log.h"
#include "Core/Misc/Align.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Core/Serialization/DeepHash.h"
#include "Render/OpenGL/Platform.h"
#include "Render/OpenGL/GlslType.h"
#include "Render/OpenGL/GlslProgram.h"
#include "Render/OpenGL/ITextureBinding.h"
#include "Render/OpenGL/ProgramResourceOpenGL.h"
#include "Render/OpenGL/Std/ProgramOpenGL.h"
#include "Render/OpenGL/Std/SimpleTextureOpenGL.h"
#include "Render/OpenGL/Std/CubeTextureOpenGL.h"
#include "Render/OpenGL/Std/VolumeTextureOpenGL.h"
#include "Render/OpenGL/Std/RenderTargetOpenGL.h"
#include "Render/OpenGL/Std/ContextOpenGL.h"

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
		source[i].storeAligned(&dest[i * 4 * 4]);
	return true;
}

ITextureBinding* getTextureBinding(ITexture* texture)
{
	if (SimpleTextureOpenGL* st = dynamic_type_cast< SimpleTextureOpenGL* >(texture))
		return static_cast< ITextureBinding* >(st);
	else if (CubeTextureOpenGL* ct = dynamic_type_cast< CubeTextureOpenGL* >(texture))
		return static_cast< ITextureBinding* >(ct);
	else if (VolumeTextureOpenGL* vt = dynamic_type_cast< VolumeTextureOpenGL* >(texture))
		return static_cast< ITextureBinding* >(vt);
	else if (RenderTargetOpenGL* rt = dynamic_type_cast< RenderTargetOpenGL* >(texture))
		return static_cast< ITextureBinding* >(rt);
	else
		return 0;
}

std::map< uint32_t, ProgramOpenGL* > s_programCache;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ProgramOpenGL", ProgramOpenGL, IProgram)

ProgramOpenGL* ProgramOpenGL::ms_activeProgram = 0;

ProgramOpenGL::~ProgramOpenGL()
{
	destroy();
}

Ref< ProgramResource > ProgramOpenGL::compile(const GlslProgram& glslProgram, int optimize, bool validate)
{
	Ref< ProgramResourceOpenGL > resource;

	resource = new ProgramResourceOpenGL(
		wstombs(glslProgram.getVertexShader()),
		wstombs(glslProgram.getFragmentShader()),
		glslProgram.getTextures(),
		glslProgram.getSamplers(),
		glslProgram.getRenderState()
	);

	uint32_t hash = DeepHash(resource).get();
	resource->setHash(hash);

	return resource;
}

Ref< ProgramOpenGL > ProgramOpenGL::create(ContextOpenGL* resourceContext, const ProgramResource* resource)
{
	const ProgramResourceOpenGL* resourceOpenGL = checked_type_cast< const ProgramResourceOpenGL* >(resource);
	char errorBuf[32000];
	GLsizei errorBufLen;
	GLint status;
	
	uint32_t hash = resourceOpenGL->getHash();

	std::map< uint32_t, ProgramOpenGL* >::iterator i = s_programCache.find(hash);
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

	T_OGL_SAFE(glBindFragDataLocation(programObject, 0, "_gl_FragData_0"));
	T_OGL_SAFE(glBindFragDataLocation(programObject, 1, "_gl_FragData_1"));
	T_OGL_SAFE(glBindFragDataLocation(programObject, 2, "_gl_FragData_2"));
	T_OGL_SAFE(glBindFragDataLocation(programObject, 3, "_gl_FragData_3"));

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
	
	Ref< ProgramOpenGL > program = new ProgramOpenGL(resourceContext, programObject, resource);
	s_programCache.insert(std::make_pair(hash, program));
	
	return program;
}

void ProgramOpenGL::destroy()
{
	if (ms_activeProgram == this)
		ms_activeProgram = 0;
		
	for (std::map< uint32_t, ProgramOpenGL* >::iterator i = s_programCache.begin(); i != s_programCache.end(); ++i)
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
}

void ProgramOpenGL::setFloatParameter(handle_t handle, float param)
{
	setFloatArrayParameter(handle, &param, 1);
}

void ProgramOpenGL::setFloatArrayParameter(handle_t handle, const float* param, int length)
{
	SmallMap< handle_t, uint32_t >::const_iterator i = m_parameterMap.find(handle);
	if (i == m_parameterMap.end())
		return;
		
	Uniform& uniform = m_uniforms[i->second];
	length = std::min< int >(length, uniform.length);

	if (storeIfNotEqual(param, length, &m_uniformData[uniform.offset]))
		uniform.dirty = true;
}

void ProgramOpenGL::setVectorParameter(handle_t handle, const Vector4& param)
{
	setVectorArrayParameter(handle, &param, 1);
}

void ProgramOpenGL::setVectorArrayParameter(handle_t handle, const Vector4* param, int length)
{
	SmallMap< handle_t, uint32_t >::const_iterator i = m_parameterMap.find(handle);
	if (i == m_parameterMap.end())
		return;
		
	Uniform& uniform = m_uniforms[i->second];
	length = std::min< int >(length, uniform.length);

	if (storeIfNotEqual(param, length, &m_uniformData[uniform.offset]))
		uniform.dirty = true;
}

void ProgramOpenGL::setMatrixParameter(handle_t handle, const Matrix44& param)
{
	setMatrixArrayParameter(handle, &param, 1);
}

void ProgramOpenGL::setMatrixArrayParameter(handle_t handle, const Matrix44* param, int length)
{
	SmallMap< handle_t, uint32_t >::const_iterator i = m_parameterMap.find(handle);
	if (i == m_parameterMap.end())
		return;
		
	Uniform& uniform = m_uniforms[i->second];
	length = std::min< int >(length, uniform.length);

	if (storeIfNotEqual(param, length, &m_uniformData[uniform.offset]))
		uniform.dirty = true;
}

void ProgramOpenGL::setTextureParameter(handle_t handle, ITexture* texture)
{
	SmallMap< handle_t, uint32_t >::const_iterator i = m_parameterMap.find(handle);
	if (i == m_parameterMap.end())
		return;

	if (!texture)
		return;

	Ref< ITexture > resolved = texture->resolve();
	if (!resolved)
		return;

	if (m_textures[i->second] == resolved)
		return;

	m_textures[i->second] = resolved;
	m_textureDirty = true;
}

void ProgramOpenGL::setStencilReference(uint32_t stencilReference)
{
	m_renderState.stencilRef = stencilReference;
}

bool ProgramOpenGL::activate(ContextOpenGL* renderContext, float targetSize[2])
{
	if (!m_program)
		return false;

	// Bind program.
	T_OGL_SAFE(glUseProgram(m_program));

	// Setup our render state.
	if (m_renderStateList == ~0UL)
		m_renderStateList = renderContext->createStateList(m_renderState);

	renderContext->callStateList(m_renderStateList);
	if (m_renderState.stencilTestEnable)
	{
		T_OGL_SAFE(glStencilFunc(
			m_renderState.stencilFunc,
			m_renderState.stencilRef,
			~0UL
		));
	}
	
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
			T_OGL_SAFE(glUniform2fv(m_locationTargetSize, 1, targetSize));
			m_targetSize[0] = targetSize[0];
			m_targetSize[1] = targetSize[1];
		}
	}

	// Bind textures.
	if (m_textureDirty || ms_activeProgram != this)
	{
		T_ASSERT (m_samplers.size() <= 16);

		uint32_t nsamplers = m_samplers.size();
		for (uint32_t i = 0; i < nsamplers; ++i)
		{
			const Sampler& sampler = m_samplers[i];

			ITextureBinding* tb = getTextureBinding(m_textures[sampler.texture]);
			T_ASSERT (tb);
			
			if (tb)
			{
				tb->bindSampler(
					renderContext,
					sampler.stage,
					sampler.object,
					sampler.location
				);
			}
		}

		uint32_t ntextureSize = m_textureSize.size();
		for (uint32_t i = 0; i < ntextureSize; ++i)
		{
			const TextureSize& textureSize = m_textureSize[i];

			ITextureBinding* tb = getTextureBinding(m_textures[textureSize.texture]);
			T_ASSERT (tb);

			if (tb)
				tb->bindSize(textureSize.location);
		}

		m_textureDirty = false;
	}

	// Check if program and state is valid.
#if defined(_DEBUG)
	GLint status;
	T_OGL_SAFE(glValidateProgram(m_program));
	T_OGL_SAFE(glGetProgramiv(m_program, GL_VALIDATE_STATUS, &status));
	if (status != GL_TRUE)
	{
		GLchar errorBuf[512];
		GLint errorBufLen;
		
		T_OGL_SAFE(glGetProgramInfoLog(m_program, sizeof(errorBuf), &errorBufLen, errorBuf));
		if (errorBufLen > 0)
		{
			log::error << L"GLSL program validate failed :" << Endl;
			log::error << mbstows(errorBuf) << Endl;
			return false;
		}
	}
#endif

	ms_activeProgram = this;
	return true;
}

const GLint* ProgramOpenGL::getAttributeLocs() const
{
	return m_attributeLocs;
}

ProgramOpenGL::ProgramOpenGL(ContextOpenGL* resourceContext, GLuint program, const ProgramResource* resource)
:	m_resourceContext(resourceContext)
,	m_program(program)
,	m_renderStateList(~0UL)
,	m_locationTargetSize(0)
,	m_textureDirty(true)
{
	const ProgramResourceOpenGL* resourceOpenGL = checked_type_cast< const ProgramResourceOpenGL* >(resource);

	m_targetSize[0] =
	m_targetSize[1] = 0.0f;

	m_renderState = resourceOpenGL->getRenderState();

	// Get target size parameter.
	m_locationTargetSize = glGetUniformLocation(m_program, "_gl_targetSize");

	const std::vector< std::wstring >& textures = resourceOpenGL->getTextures();
	const std::vector< SamplerBindingOpenGL >& samplers = resourceOpenGL->getSamplers();

	// Map texture parameters.
	for (std::vector< SamplerBindingOpenGL >::const_iterator i = samplers.begin(); i != samplers.end(); ++i)
	{
		const std::wstring& texture = textures[i->texture];

		// Get texture parameter handle.
		handle_t handle = getParameterHandle(texture);
		if (m_parameterMap.find(handle) == m_parameterMap.end())
		{
			m_parameterMap[handle] = m_textures.size();
			m_textures.push_back(0);
		}
		
		std::wstring samplerName = L"_gl_sampler_" + texture + L"_" + toString(i->stage);
		
		Sampler sampler;
		sampler.location = glGetUniformLocation(m_program, wstombs(samplerName).c_str());
		sampler.texture = m_parameterMap[handle];
		sampler.stage = i->stage;

		// Create sampler object.
		T_OGL_SAFE(glGenSamplers(2, sampler.object));

		const SamplerStateOpenGL& samplerState = m_renderState.samplerStates[i->stage];

		if (samplerState.minFilter != GL_NEAREST)
			T_OGL_SAFE(glSamplerParameteri(sampler.object[0], GL_TEXTURE_MIN_FILTER, GL_LINEAR))
		else
			T_OGL_SAFE(glSamplerParameteri(sampler.object[0], GL_TEXTURE_MIN_FILTER, GL_NEAREST));
		T_OGL_SAFE(glSamplerParameteri(sampler.object[0], GL_TEXTURE_MAG_FILTER, samplerState.magFilter));
		T_OGL_SAFE(glSamplerParameteri(sampler.object[0], GL_TEXTURE_WRAP_S, samplerState.wrapS));
		T_OGL_SAFE(glSamplerParameteri(sampler.object[0], GL_TEXTURE_WRAP_T, samplerState.wrapT));
		T_OGL_SAFE(glSamplerParameteri(sampler.object[0], GL_TEXTURE_WRAP_R, samplerState.wrapR));

		T_OGL_SAFE(glSamplerParameteri(sampler.object[1], GL_TEXTURE_MIN_FILTER, samplerState.minFilter));
		T_OGL_SAFE(glSamplerParameteri(sampler.object[1], GL_TEXTURE_MAG_FILTER, samplerState.magFilter));
		T_OGL_SAFE(glSamplerParameteri(sampler.object[1], GL_TEXTURE_WRAP_S, samplerState.wrapS));
		T_OGL_SAFE(glSamplerParameteri(sampler.object[1], GL_TEXTURE_WRAP_T, samplerState.wrapT));
		T_OGL_SAFE(glSamplerParameteri(sampler.object[1], GL_TEXTURE_WRAP_R, samplerState.wrapR));

		m_samplers.push_back(sampler);
	}

	// Map texture size parameters.
	for (std::vector< std::wstring >::const_iterator i = textures.begin(); i != textures.end(); ++i)
	{
		const std::wstring& texture = *i;
		std::wstring textureSizeName = L"_gl_textureSize_" + texture;

		GLint location = glGetUniformLocation(m_program, wstombs(textureSizeName).c_str());
		if (location <= 0)
			continue;

		handle_t handle = getParameterHandle(texture);

		if (m_parameterMap.find(handle) == m_parameterMap.end())
		{
			m_parameterMap[handle] = m_textures.size();
			m_textures.push_back(0);
		}

		TextureSize textureSize;
		textureSize.location = location;
		textureSize.texture = m_parameterMap[handle];

		m_textureSize.push_back(textureSize);
	}

	// Map samplers and uniforms.
	GLint uniformCount;
	T_OGL_SAFE(glGetProgramiv(m_program, GL_ACTIVE_UNIFORMS, &uniformCount));

	for (GLint j = 0; j < uniformCount; ++j)
	{
		GLint uniformSize;
		GLenum uniformType;
		GLchar uniformName[256];

		T_OGL_SAFE(glGetActiveUniform(m_program, j, sizeof(uniformName), 0, &uniformSize, &uniformType, uniformName));
		std::wstring uniformNameW = mbstows(uniformName);
		
		// Skip uniforms which starts with _gl_ as they are private.
		if (startsWith< std::wstring >(uniformNameW, L"_gl_"))
			continue;

		// Trim indexed uniforms; seems to vary dependending on OGL implementation.
		size_t p = uniformNameW.find('[');
		if (p != uniformNameW.npos)
			uniformNameW = uniformNameW.substr(0, p);

		if (uniformType == GL_FLOAT || uniformType == GL_FLOAT_VEC4 || uniformType == GL_FLOAT_MAT4)
		{
			handle_t handle = getParameterHandle(uniformNameW);
			
			SmallMap< handle_t, uint32_t >::iterator k = m_parameterMap.find(handle);
			if (k != m_parameterMap.end())
			{
				const Uniform& uniform = m_uniforms[k->second];
				if (uniform.type != uniformType)
				{
					log::error << L"Parameter \"" << uniformNameW << L"\" already defined with another type" << Endl;
					continue;
				}
			}
			
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
}

	}
}

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
#include "Render/OpenGL/Std/Extensions.h"
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

	GLhandleARB vertexObject = resourceContext->createShaderObject(vertexShader.c_str(), GL_VERTEX_SHADER_ARB);
	if (!vertexObject)
	{
		log::error << L"Unable to create vertex object" << Endl;
		return 0;
	}
		
	GLhandleARB fragmentObject = resourceContext->createShaderObject(fragmentShader.c_str(), GL_FRAGMENT_SHADER_ARB);
	if (!fragmentObject)
	{
		log::error << L"Unable to create fragment object" << Endl;
		return 0;
	}

	GLhandleARB programObject = glCreateProgramObjectARB();
	T_ASSERT (programObject != 0);

	T_OGL_SAFE(glAttachObjectARB(programObject, vertexObject));
	T_OGL_SAFE(glAttachObjectARB(programObject, fragmentObject));
	T_OGL_SAFE(glLinkProgramARB(programObject));

	T_OGL_SAFE(glGetObjectParameterivARB(programObject, GL_OBJECT_LINK_STATUS_ARB, &status));
	if (status != GL_TRUE)
	{
		T_OGL_SAFE(glGetInfoLogARB(programObject, sizeof(errorBuf), &errorBufLen, errorBuf));
		if (errorBufLen > 0)
		{
			log::error << L"GLSL program link failed :" << Endl;
			log::error << mbstows(errorBuf) << Endl;
			return 0;
		}
	}

	GLuint renderStateList = resourceContext->createStateList(resourceOpenGL->getRenderState());

	Ref< ProgramOpenGL > program = new ProgramOpenGL(resourceContext, programObject, renderStateList, resource);
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
			m_resourceContext->deleteResource(new DeleteObjectCallback(m_program));
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

	// Bind program and set state display list.
	if (ms_activeProgram != this)
	{
		//stateCache->setRenderState(m_renderState, true);
		renderContext->callStateList(m_renderStateList);
		T_OGL_SAFE(glUseProgramObjectARB(m_program));
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
		
		i->dirty = false;
	}

	// Update target size uniform if necessary.
	if (m_locationTargetSize != -1)
	{
		if (m_targetSize[0] != targetSize[0] || m_targetSize[1] != targetSize[1])
		{
			T_OGL_SAFE(glUniform2fvARB(m_locationTargetSize, 1, targetSize));
			m_targetSize[0] = targetSize[0];
			m_targetSize[1] = targetSize[1];
		}
	}

	// Bind textures.
	if (m_textureDirty || ms_activeProgram != this)
	{
		T_ASSERT (m_samplers.size() <= 8);

		uint32_t nsamplers = m_samplers.size();
		for (uint32_t i = 0; i < nsamplers; ++i)
		{
			const Sampler& sampler = m_samplers[i];
			const SamplerState& samplerState = m_renderState.samplerStates[sampler.stage];

			ITextureBinding* tb = getTextureBinding(m_textures[sampler.texture]);
			T_ASSERT (tb);
			
			if (tb)
			{
				tb->bindSampler(
					i,
					samplerState,
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
	T_OGL_SAFE(glValidateProgramARB(m_program));
	T_OGL_SAFE(glGetObjectParameterivARB(m_program, GL_OBJECT_VALIDATE_STATUS_ARB, &status));
	if (status != GL_TRUE)
	{
		GLchar errorBuf[512];
		GLint errorBufLen;
		
		T_OGL_SAFE(glGetInfoLogARB(m_program, sizeof(errorBuf), &errorBufLen, errorBuf));
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

ProgramOpenGL::ProgramOpenGL(ContextOpenGL* resourceContext, GLhandleARB program, GLuint renderStateList, const ProgramResource* resource)
:	m_resourceContext(resourceContext)
,	m_program(program)
,	m_renderStateList(renderStateList)
,	m_locationTargetSize(0)
,	m_textureDirty(true)
{
	const ProgramResourceOpenGL* resourceOpenGL = checked_type_cast< const ProgramResourceOpenGL* >(resource);

	m_targetSize[0] =
	m_targetSize[1] = 0.0f;
	
	// Get target size parameter.
	m_locationTargetSize = glGetUniformLocationARB(m_program, "_gl_targetSize");

	const std::vector< std::wstring >& textures = resourceOpenGL->getTextures();
	const std::vector< std::pair< int32_t, int32_t > >& samplers = resourceOpenGL->getSamplers();

	// Map texture parameters.
	for (std::vector< std::pair< int32_t, int32_t > >::const_iterator i = samplers.begin(); i != samplers.end(); ++i)
	{
		const std::wstring& texture = textures[i->first];
		int32_t stage = i->second;

		handle_t handle = getParameterHandle(texture);

		if (m_parameterMap.find(handle) == m_parameterMap.end())
		{
			m_parameterMap[handle] = m_textures.size();
			m_textures.push_back(0);
		}
		
		std::wstring samplerName = L"_gl_sampler_" + texture + L"_" + toString(stage);
		
		Sampler sampler;
		sampler.location = glGetUniformLocationARB(m_program, wstombs(samplerName).c_str());
		sampler.texture = m_parameterMap[handle];
		sampler.stage = stage;

		m_samplers.push_back(sampler);
	}

	// Map texture size parameters.
	for (std::vector< std::wstring >::const_iterator i = textures.begin(); i != textures.end(); ++i)
	{
		const std::wstring& texture = *i;
		std::wstring textureSizeName = L"_gl_textureSize_" + texture;

		GLint location = glGetUniformLocationARB(m_program, wstombs(textureSizeName).c_str());
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
	T_OGL_SAFE(glGetObjectParameterivARB(m_program, GL_OBJECT_ACTIVE_UNIFORMS_ARB, &uniformCount));

	for (GLint j = 0; j < uniformCount; ++j)
	{
		GLint uniformSize;
		GLenum uniformType;
		GLcharARB uniformName[256];

		T_OGL_SAFE(glGetActiveUniformARB(m_program, j, sizeof(uniformName), 0, &uniformSize, &uniformType, uniformName));
		std::wstring uniformNameW = mbstows(uniformName);
		
		// Skip uniforms which starts with _gl_ as they are private.
		if (startsWith< std::wstring >(uniformNameW, L"_gl_"))
			continue;

		// Trim indexed uniforms; seems to vary dependending on OGL implementation.
		size_t p = uniformNameW.find('[');
		if (p != uniformNameW.npos)
			uniformNameW = uniformNameW.substr(0, p);

		if (uniformType == GL_FLOAT || uniformType == GL_FLOAT_VEC4_ARB || uniformType == GL_FLOAT_MAT4_ARB)
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

			case GL_FLOAT_VEC4_ARB:
				allocSize = 4 * uniformSize;
				break;

			case GL_FLOAT_MAT4_ARB:
				allocSize = 16 * uniformSize;
				break;

			default:
				break;
			}

			uint32_t offsetUniform = uint32_t(m_uniforms.size());
			uint32_t offsetData = uint32_t(m_uniformData.size());
			
			m_parameterMap[handle] = offsetUniform;

			m_uniforms.push_back(Uniform());
			m_uniforms.back().location = glGetUniformLocationARB(m_program, uniformName);
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
		m_attributeLocs[T_OGL_USAGE_INDEX(DuPosition, j)] = glGetAttribLocationARB(m_program, wstombs(glsl_vertex_attr_name(DuPosition, j)).c_str());
		m_attributeLocs[T_OGL_USAGE_INDEX(DuNormal, j)] = glGetAttribLocationARB(m_program, wstombs(glsl_vertex_attr_name(DuNormal, j)).c_str());
		m_attributeLocs[T_OGL_USAGE_INDEX(DuTangent, j)] = glGetAttribLocationARB(m_program, wstombs(glsl_vertex_attr_name(DuTangent, j)).c_str());
		m_attributeLocs[T_OGL_USAGE_INDEX(DuBinormal, j)] = glGetAttribLocationARB(m_program, wstombs(glsl_vertex_attr_name(DuBinormal, j)).c_str());
		m_attributeLocs[T_OGL_USAGE_INDEX(DuColor, j)] = glGetAttribLocationARB(m_program, wstombs(glsl_vertex_attr_name(DuColor, j)).c_str());
		m_attributeLocs[T_OGL_USAGE_INDEX(DuCustom, j)] = glGetAttribLocationARB(m_program, wstombs(glsl_vertex_attr_name(DuCustom, j)).c_str());
	}

	// Create a display list from the render states.
	m_renderState = resourceOpenGL->getRenderState();
}

	}
}

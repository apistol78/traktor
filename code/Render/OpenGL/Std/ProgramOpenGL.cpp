#include <cstring>
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Log/Log.h"
#include "Core/Math/Matrix44.h"
#include "Core/Misc/Adler32.h"
#include "Core/Misc/Align.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/System/OS.h"
#include "Render/OpenGL/Std/Platform.h"
#include "Render/OpenGL/Std/ITextureBinding.h"
#include "Render/OpenGL/Std/ProgramResourceOpenGL.h"
#include "Render/OpenGL/Std/ProgramOpenGL.h"
#include "Render/OpenGL/Std/SimpleTextureOpenGL.h"
#include "Render/OpenGL/Std/CubeTextureOpenGL.h"
#include "Render/OpenGL/Std/VolumeTextureOpenGL.h"
#include "Render/OpenGL/Std/RenderContextOpenGL.h"
#include "Render/OpenGL/Std/RenderTargetDepthOpenGL.h"
#include "Render/OpenGL/Std/RenderTargetOpenGL.h"
#include "Render/OpenGL/Std/ResourceContextOpenGL.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

struct DeleteProgramCallback : public ResourceContextOpenGL::IDeleteCallback
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
	else if (RenderTargetDepthOpenGL* rtd = dynamic_type_cast< RenderTargetDepthOpenGL* >(texture))
		return static_cast< ITextureBinding* >(rtd);
	else
		return 0;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ProgramOpenGL", ProgramOpenGL, IProgram)

ProgramOpenGL::~ProgramOpenGL()
{
	destroy();
}

Ref< ProgramOpenGL > ProgramOpenGL::create(ResourceContextOpenGL* resourceContext, const ProgramResource* resource, bool cacheEnable)
{
	const ProgramResourceOpenGL* resourceOpenGL = checked_type_cast< const ProgramResourceOpenGL* >(resource);
	char errorBuf[32000];
	AutoArrayPtr< GLint > binaryFormats;
	AutoArrayPtr< uint8_t > binary;
	GLint formats = 0;
	GLint binaryLength = 0;
	GLsizei errorBufLen;
	GLint status;
	bool needToCompile = true;

	GLuint programObject = glCreateProgram();
	T_ASSERT (programObject != 0);

#if defined(GL_ARB_get_program_binary)
	if (cacheEnable)
	{
		T_OGL_SAFE(glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &formats));
		if (formats > 0)
		{
			binaryFormats.reset(new GLint[formats]);
			T_OGL_SAFE(glGetIntegerv(GL_PROGRAM_BINARY_FORMATS, binaryFormats.ptr()));

			StringOutputStream ss;
			ss << OS::getInstance().getWritableFolderPath() << L"/Doctor Entertainment AB/ProgramCache/Program_" << resourceOpenGL->getHash() << L".cache";

			// Read and upload cached program if available.
			Ref< IStream > file = FileSystem::getInstance().open(ss.str(), File::FmRead);
			if (file)
			{
				binaryLength = file->available();
				binary.reset(new uint8_t [binaryLength]);
				file->read(binary.ptr(), binaryLength);
				file->close();

				T_OGL_SAFE(glProgramBinary(programObject, binaryFormats[0], binary.c_ptr(), binaryLength));
				T_OGL_SAFE(glGetProgramiv(programObject, GL_LINK_STATUS, &status));

				if (status == GL_TRUE)
					needToCompile = false;
			}
		}
	}
#endif

	// Re-compile program if not cached or something has changed.
	if (needToCompile)
	{
		const std::string& vertexShader = resourceOpenGL->getVertexShader();
		const std::string& fragmentShader = resourceOpenGL->getFragmentShader();
		const std::string& computeShader = resourceOpenGL->getComputeShader();

		if (!vertexShader.empty() && !fragmentShader.empty())
		{
			GLuint vertexObject = resourceContext->createShaderObject(vertexShader.c_str(), GL_VERTEX_SHADER);
			if (!vertexObject)
			{
				log::error << L"Unable to create vertex object" << Endl;
				return nullptr;
			}

			GLuint fragmentObject = resourceContext->createShaderObject(fragmentShader.c_str(), GL_FRAGMENT_SHADER);
			if (!fragmentObject)
			{
				log::error << L"Unable to create fragment object" << Endl;
				return nullptr;
			}

			T_OGL_SAFE(glAttachShader(programObject, vertexObject));
			T_OGL_SAFE(glAttachShader(programObject, fragmentObject));

			T_OGL_SAFE(glBindFragDataLocation(programObject, 0, "_gl_FragData_0"));
			T_OGL_SAFE(glBindFragDataLocation(programObject, 1, "_gl_FragData_1"));
			T_OGL_SAFE(glBindFragDataLocation(programObject, 2, "_gl_FragData_2"));
			T_OGL_SAFE(glBindFragDataLocation(programObject, 3, "_gl_FragData_3"));
		}
#if !defined(__APPLE__)
		else if (!computeShader.empty())
		{
			GLuint computeObject = resourceContext->createShaderObject(computeShader.c_str(), GL_COMPUTE_SHADER);
			if (!computeObject)
			{
				log::error << L"Unable to create compute object" << Endl;
				return nullptr;
			}

			T_OGL_SAFE(glAttachShader(programObject, computeObject));
		}
#endif
		else
			return nullptr;

#if defined(GL_ARB_get_program_binary)
		if (cacheEnable && formats > 0)
			T_OGL_SAFE(glProgramParameteri(programObject, GL_PROGRAM_BINARY_RETRIEVABLE_HINT, GL_TRUE));
#endif

		T_OGL_SAFE(glLinkProgram(programObject));

		T_OGL_SAFE(glGetProgramiv(programObject, GL_LINK_STATUS, &status));
		if (status != GL_TRUE)
		{
			T_OGL_SAFE(glGetProgramInfoLog(programObject, sizeof(errorBuf), &errorBufLen, errorBuf));
			if (errorBufLen > 0)
			{
				log::error << L"GLSL program link failed :" << Endl;
				log::error << mbstows(errorBuf) << Endl;
				return nullptr;
			}
		}
	}

#if defined(GL_ARB_get_program_binary)
	if (cacheEnable && needToCompile && formats > 0)
	{
		StringOutputStream ss;
		ss << OS::getInstance().getWritableFolderPath() << L"/Doctor Entertainment AB/ProgramCache/Program_" << resourceOpenGL->getHash() << L".cache";

		// Get and cache program binary.
		T_OGL_SAFE(glGetProgramiv(programObject, GL_PROGRAM_BINARY_LENGTH, &binaryLength));

		binary.reset(new uint8_t [binaryLength]);
		std::memset(binary.ptr(), 0, binaryLength);
		T_OGL_SAFE(glGetProgramBinary(programObject, binaryLength, 0, (GLenum*)binaryFormats.ptr(), binary.ptr()));

		FileSystem::getInstance().makeAllDirectories(Path(ss.str()).getPathOnly());
		Ref< IStream > file = FileSystem::getInstance().open(ss.str(), File::FmWrite);
		if (file)
		{
			file->write(binary.c_ptr(), binaryLength);
			file->close();
		}
	}
#endif

	return new ProgramOpenGL(resourceContext, programObject, resource);
}

void ProgramOpenGL::destroy()
{
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
	if (i != m_parameterMap.end())
	{
		m_textures[i->second] = texture;
		m_textureDirty = true;
	}
}

void ProgramOpenGL::setStencilReference(uint32_t stencilReference)
{
	m_renderState.stencilRef = stencilReference;
}

bool ProgramOpenGL::activateRender(RenderContextOpenGL* renderContext, float targetSize[2])
{
	if (!m_program || (m_validated && !m_valid))
		return false;

	// "Bind" ourself to render context; return true if program is already active.
	bool alreadyActive = renderContext->programActivate(this);

	// Bind program.
	if (!alreadyActive)
	{
		T_OGL_SAFE(glUseProgram(m_program));
	}

	// Set render state.
	renderContext->bindRenderStateObject(m_renderStateList);
	if (m_renderState.stencilTestEnable)
	{
		T_OGL_SAFE(glStencilFunc(
			m_renderState.stencilFunc,
			m_renderState.stencilRef,
			~0U
		));
	}

	// Update dirty uniforms.
	for (auto& uniform : m_uniforms)
	{
		if (!uniform.dirty)
		 	continue;

		const float* uniformData = &m_uniformData[uniform.offset];
		switch (uniform.type)
		{
		case GL_FLOAT:
			T_OGL_SAFE(glUniform1fv(uniform.location, uniform.length, uniformData));
			break;

		case GL_FLOAT_VEC4:
			T_OGL_SAFE(glUniform4fv(uniform.location, uniform.length, uniformData));
			break;

		case GL_FLOAT_MAT4:
			T_OGL_SAFE(glUniformMatrix4fv(uniform.location, uniform.length, GL_FALSE, uniformData));
			break;

		default:
			T_ASSERT (0);
		}

		uniform.dirty = false;
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
	if (!alreadyActive || m_textureDirty)
	{
		T_ASSERT (m_samplers.size() <= 16);

		for (const auto& sampler : m_samplers)
		{
			ITexture* texture = m_textures[sampler.texture];
			if (!texture)
				continue;

			Ref< ITexture > resolved = texture->resolve();
			if (!resolved)
				continue;

			const ITextureBinding* tb = getTextureBinding(resolved);
			T_ASSERT (tb);

			tb->bindTexture(sampler.unit);
			renderContext->bindSamplerStateObject(sampler.object, sampler.unit, tb->haveMips());

			T_OGL_SAFE(glUniform1i(sampler.location, sampler.unit));
		}

		for (const auto& textureSize : m_textureSize)
		{
			ITexture* texture = m_textures[textureSize.texture];
			if (!texture)
				continue;

			Ref< ITexture > resolved = texture->resolve();
			if (!resolved)
				continue;

			const ITextureBinding* tb = getTextureBinding(resolved);
			T_ASSERT (tb);

			tb->bindSize(textureSize.location);
		}

		m_textureDirty = false;
	}

	// Check if program and state is valid.
	if (!m_validated)
	{
		GLint status;

		T_OGL_SAFE(glValidateProgram(m_program));
		T_OGL_SAFE(glGetProgramiv(m_program, GL_VALIDATE_STATUS, &status));

		m_validated = true;
		m_valid = bool(status == GL_TRUE);

#if defined(_DEBUG)
		if (!m_valid)
		{
			GLchar errorBuf[512];
			GLint errorBufLen;

			T_OGL_SAFE(glGetProgramInfoLog(m_program, sizeof(errorBuf), &errorBufLen, errorBuf));
			if (errorBufLen > 0)
			{
				log::error << L"GLSL program validate failed :" << Endl;
				log::error << mbstows(errorBuf) << Endl;
			}
		}
#endif

		if (!m_valid)
			return false;
	}

	return true;
}

bool ProgramOpenGL::activateCompute(RenderContextOpenGL* renderContext)
{
	if (!m_program || (m_validated && !m_valid))
		return false;

	// "Bind" ourself to render context; return true if program is already active.
	bool alreadyActive = renderContext->programActivate(this);

	// Bind program.
	if (!alreadyActive)
	{
		T_OGL_SAFE(glUseProgram(m_program));
	}

	// Update dirty uniforms.
	for (auto& uniform : m_uniforms)
	{
		if (!uniform.dirty)
		 	continue;

		const float* uniformData = &m_uniformData[uniform.offset];
		switch (uniform.type)
		{
		case GL_FLOAT:
			T_OGL_SAFE(glUniform1fv(uniform.location, uniform.length, uniformData));
			break;

		case GL_FLOAT_VEC4:
			T_OGL_SAFE(glUniform4fv(uniform.location, uniform.length, uniformData));
			break;

		case GL_FLOAT_MAT4:
			T_OGL_SAFE(glUniformMatrix4fv(uniform.location, uniform.length, GL_FALSE, uniformData));
			break;

		default:
			T_ASSERT (0);
		}

		uniform.dirty = false;
	}

	// Bind textures.
	if (!alreadyActive || m_textureDirty)
	{
		for (uint32_t unit = 0; unit < m_textures.size(); ++unit)
		{
			ITexture* texture = m_textures[unit];
			if (!texture)
				continue;

			Ref< ITexture > resolved = texture->resolve();
			if (!resolved)
				continue;

			const ITextureBinding* tb = getTextureBinding(resolved);
			T_ASSERT (tb);

			// T_OGL_SAFE(glActiveTexture(GL_TEXTURE0 + unit));
			tb->bindImage(unit);
		}

		for (const auto& textureSize : m_textureSize)
		{
			ITexture* texture = m_textures[textureSize.texture];
			if (!texture)
				continue;

			Ref< ITexture > resolved = texture->resolve();
			if (!resolved)
				continue;

			const ITextureBinding* tb = getTextureBinding(resolved);
			T_ASSERT (tb);

			tb->bindSize(textureSize.location);
		}

		m_textureDirty = false;
	}

	// Check if program and state is valid.
	if (!m_validated)
	{
		GLint status;

		T_OGL_SAFE(glValidateProgram(m_program));
		T_OGL_SAFE(glGetProgramiv(m_program, GL_VALIDATE_STATUS, &status));

		m_validated = true;
		m_valid = bool(status == GL_TRUE);

#if defined(_DEBUG)
		if (!m_valid)
		{
			GLchar errorBuf[512];
			GLint errorBufLen;

			T_OGL_SAFE(glGetProgramInfoLog(m_program, sizeof(errorBuf), &errorBufLen, errorBuf));
			if (errorBufLen > 0)
			{
				log::error << L"GLSL program validate failed :" << Endl;
				log::error << mbstows(errorBuf) << Endl;
			}
		}
#endif

		if (!m_valid)
			return false;
	}

	return true;
}

ProgramOpenGL::ProgramOpenGL(ResourceContextOpenGL* resourceContext, GLuint program, const ProgramResource* resource)
:	m_resourceContext(resourceContext)
,	m_program(program)
,	m_renderStateList(0)
,	m_locationTargetSize(0)
,	m_attributeHash(0)
,	m_textureDirty(true)
,	m_validated(false)
,	m_valid(false)
{
	const ProgramResourceOpenGL* resourceOpenGL = checked_type_cast< const ProgramResourceOpenGL* >(resource);

	m_targetSize[0] =
	m_targetSize[1] = 0.0f;

	// Setup our render state.
	m_renderState = resourceOpenGL->getRenderState();
	m_renderStateList = resourceContext->createRenderStateObject(m_renderState);

	// Get target size parameter.
	m_locationTargetSize = glGetUniformLocation(m_program, "_gl_targetSize");

	const std::vector< std::wstring >& textures = resourceOpenGL->getTextures();
	const std::vector< SamplerBindingOpenGL >& samplers = resourceOpenGL->getSamplers();
	const std::vector< NamedUniformType >& uniforms = resourceOpenGL->getUniforms();

	// Map texture parameters.
	for (const auto& texture : textures)
	{
		handle_t handle = getParameterHandle(texture);
		T_ASSERT(m_parameterMap.find(handle) == m_parameterMap.end());
		m_parameterMap[handle] = m_textures.size();
		m_textures.push_back(nullptr);
	}

	// Map sampler parameters.
	for (const auto& sampler : samplers)
	{
		const std::wstring& texture = textures[sampler.texture];
		handle_t handle = getParameterHandle(texture);

		Sampler& s = m_samplers.push_back();
		s.location = glGetUniformLocation(m_program, wstombs(sampler.name).c_str());
		s.texture = m_parameterMap[handle];
		s.unit = sampler.unit;
		s.object = resourceContext->createSamplerStateObject(m_renderState.samplerStates[sampler.unit]);

		if (s.location < 0)
			log::warning << L"No GL sampler defined for texture \"" << texture << L"\"." << Endl;
	}

	// Map texture size parameters.
	for (const auto& texture : textures)
	{
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

		TextureSize& textureSize = m_textureSize.push_back();
		textureSize.location = location;
		textureSize.texture = m_parameterMap[handle];
	}

	// Map uniforms.
	for (uint32_t i = 0; i < uint32_t(uniforms.size()); ++i)
	{
		handle_t handle = getParameterHandle(uniforms[i].name);
		T_FATAL_ASSERT_M (m_parameterMap.find(handle) == m_parameterMap.end(), L"Duplicated uniform in resource");

		std::string uniformName = wstombs(uniforms[i].name);

		GLint location = glGetUniformLocation(m_program, uniformName.c_str());
		if (location < 0)
		{
			// This probably happen when the GLSL optimizer of the driver is more intelligent than
			// our GLSL shader generator, thus the uniform has been discarded when the program was loaded.
			T_DEBUG(L"No location of GL uniform \"" << uniforms[i].name << L"\"; shader parameter ignored.");
			continue;
		}

		uint32_t offsetUniform = uint32_t(m_uniforms.size());
		uint32_t offsetData = uint32_t(m_uniformData.size());
		uint32_t allocSize = 0;

		switch (uniforms[i].type)
		{
		case GL_FLOAT:
			allocSize = alignUp(1 * uniforms[i].length, 4);
			break;

		case GL_FLOAT_VEC4:
			allocSize = 4 * uniforms[i].length;
			break;

		case GL_FLOAT_MAT4:
			allocSize = 16 * uniforms[i].length;
			break;

		default:
			T_FATAL_ERROR;
			break;
		}

		m_parameterMap[handle] = offsetUniform;

		Uniform& uniform = m_uniforms.push_back();
		uniform.location = location;
		uniform.type = uniforms[i].type;
		uniform.offset = offsetData;
		uniform.length = uniforms[i].length;
		uniform.dirty = true;

		m_uniformData.resize(offsetData + allocSize, 0.0f);
	}

	for (int32_t j = 0; j < sizeof_array(m_attributeLocs); ++j)
		m_attributeLocs[j] = -1;

	for (int32_t j = 0; j < T_OGL_MAX_INDEX; ++j)
	{
		m_attributeLocs[T_OGL_USAGE_INDEX(DuPosition, j)] = glGetAttribLocation(m_program, wstombs(VertexAttribute::getName(DuPosition, j)).c_str());
		m_attributeLocs[T_OGL_USAGE_INDEX(DuNormal, j)] = glGetAttribLocation(m_program, wstombs(VertexAttribute::getName(DuNormal, j)).c_str());
		m_attributeLocs[T_OGL_USAGE_INDEX(DuTangent, j)] = glGetAttribLocation(m_program, wstombs(VertexAttribute::getName(DuTangent, j)).c_str());
		m_attributeLocs[T_OGL_USAGE_INDEX(DuBinormal, j)] = glGetAttribLocation(m_program, wstombs(VertexAttribute::getName(DuBinormal, j)).c_str());
		m_attributeLocs[T_OGL_USAGE_INDEX(DuColor, j)] = glGetAttribLocation(m_program, wstombs(VertexAttribute::getName(DuColor, j)).c_str());
		m_attributeLocs[T_OGL_USAGE_INDEX(DuCustom, j)] = glGetAttribLocation(m_program, wstombs(VertexAttribute::getName(DuCustom, j)).c_str());
	}

	Adler32 chk;
	chk.begin();
	chk.feed(m_attributeLocs, sizeof(m_attributeLocs));
	chk.end();
	m_attributeHash = chk.get();
}

	}
}

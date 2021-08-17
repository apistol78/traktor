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
#include "Render/OpenGL/Std/StructBufferOpenGL.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

handle_t s_handleTargetSize = 0;

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
		return nullptr;
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
	T_ASSERT(programObject != 0);

#if defined(GL_ARB_get_program_binary)
	 if (cacheEnable)
	 {
	 	T_OGL_SAFE(glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &formats));
	 	if (formats > 0)
	 	{
	 		binaryFormats.reset(new GLint[formats]);
	 		T_OGL_SAFE(glGetIntegerv(GL_PROGRAM_BINARY_FORMATS, binaryFormats.ptr()));

	 		StringOutputStream ss;
	 		ss << OS::getInstance().getWritableFolderPath() << L"/Traktor/OpenGL/Cache/Program_" << resourceOpenGL->m_hash << L".cache";

	 		// Read and upload cached program if available.
	 		Ref< IStream > file = FileSystem::getInstance().open(ss.str(), File::FmRead);
	 		if (file)
	 		{
	 			binaryLength = (GLint)file->available();
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
		const std::string& vertexShader = resourceOpenGL->m_vertexShader;
		const std::string& fragmentShader = resourceOpenGL->m_fragmentShader;
		const std::string& computeShader = resourceOpenGL->m_computeShader;

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
	 	ss << OS::getInstance().getWritableFolderPath() << L"/Traktor/OpenGL/Cache/Program_" << resourceOpenGL->m_hash << L".cache";

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
	auto i = m_parameterMap.find(handle);
	if (i != m_parameterMap.end())
	{
		auto& ub = m_uniformBuffers[i->second.buffer];
		if (storeIfNotEqual(&param, 1, &ub.data[i->second.offset]))
			ub.dirty = true;
	}
}

void ProgramOpenGL::setFloatArrayParameter(handle_t handle, const float* param, int length)
{
	auto i = m_parameterMap.find(handle);
	if (i != m_parameterMap.end())
	{
		auto& ub = m_uniformBuffers[i->second.buffer];
		if (storeIfNotEqual(param, length, &ub.data[i->second.offset]))
			ub.dirty = true;
	}
}

void ProgramOpenGL::setVectorParameter(handle_t handle, const Vector4& param)
{
	setVectorArrayParameter(handle, &param, 1);
}

void ProgramOpenGL::setVectorArrayParameter(handle_t handle, const Vector4* param, int length)
{
	auto i = m_parameterMap.find(handle);
	if (i != m_parameterMap.end())
	{
		T_FATAL_ASSERT(length * 4 <= (int)i->second.size);
		auto& ub = m_uniformBuffers[i->second.buffer];
		if (storeIfNotEqual(param, length, &ub.data[i->second.offset]))
			ub.dirty = true;
	}
}

void ProgramOpenGL::setMatrixParameter(handle_t handle, const Matrix44& param)
{
	auto i = m_parameterMap.find(handle);
	if (i != m_parameterMap.end())
	{
		auto& ub = m_uniformBuffers[i->second.buffer];
		param.storeAligned(&ub.data[i->second.offset]);
		ub.dirty = true;
	}
}

void ProgramOpenGL::setMatrixArrayParameter(handle_t handle, const Matrix44* param, int length)
{
	auto i = m_parameterMap.find(handle);
	if (i != m_parameterMap.end())
	{
		T_FATAL_ASSERT(length * 16 <= (int)i->second.size);
		auto& ub = m_uniformBuffers[i->second.buffer];
		for (int j = 0; j < length; ++j)
			param[j].storeAligned(&ub.data[i->second.offset + j * 16]);
		ub.dirty = true;
	}
}

void ProgramOpenGL::setTextureParameter(handle_t handle, ITexture* texture)
{
	auto i = m_parameterMap.find(handle);
	if (i != m_parameterMap.end())
	{
		T_FATAL_ASSERT(i->second.buffer < m_textures.size());
		m_textures[i->second.buffer] = texture;
	}
}

void ProgramOpenGL::setBufferViewParameter(handle_t handle, const IBufferView* bufferView) 
{
	auto i = m_parameterMap.find(handle);
	if (i != m_parameterMap.end())
	{
		T_FATAL_ASSERT(i->second.buffer < m_bufferViews.size());
		m_bufferViews[i->second.buffer] = (const BufferViewOpenGL*)bufferView;
	}
}

void ProgramOpenGL::setStencilReference(uint32_t stencilReference)
{
	m_stencilReference = stencilReference;
}

bool ProgramOpenGL::activateRender(RenderContextOpenGL* renderContext, float targetSize[2])
{
	if (!m_program)
		return false;

	// Set implicit parameters.
	setVectorParameter(
		s_handleTargetSize,
		Vector4(targetSize[0], targetSize[1], 0.0f, 0.0f)
	);		

	// "Bind" ourself to render context; return true if program is already active.
	bool alreadyActive = renderContext->programActivate(this);

	// Bind program.
	if (!alreadyActive)
		{ T_OGL_SAFE(glUseProgram(m_program)); }

	// Set render state.
	renderContext->bindRenderStateObject(m_renderStateList, m_stencilReference);

	// Update and bind scalar uniform buffers.
	for (uint32_t i = 0; i < 3; ++i)
	{
		if (!m_uniformBuffers[i].size)
			continue;

		if (m_uniformBuffers[i].dirty)
		{
			T_OGL_SAFE(glBindBuffer(GL_UNIFORM_BUFFER, m_uniformBuffers[i].object));
			T_OGL_SAFE(glBufferSubData(GL_UNIFORM_BUFFER, 0, m_uniformBuffers[i].size, m_uniformBuffers[i].data.c_ptr()));
			T_OGL_SAFE(glBindBuffer(GL_UNIFORM_BUFFER, 0));
			m_uniformBuffers[i].dirty = false;
		}

		T_OGL_SAFE(glBindBufferBase(GL_UNIFORM_BUFFER, i, m_uniformBuffers[i].object));
	}

	// Bind samplers.
	for (const auto& sampler : m_samplers)
	{
		ITexture* texture = m_textures[sampler.textureIndex];
		if (!texture)
			continue;

		Ref< ITexture > resolved = texture->resolve();
		if (!resolved)
			continue;

		const ITextureBinding* tb = getTextureBinding(resolved);
		T_ASSERT(tb);

		tb->bindTexture(sampler.unit);
		renderContext->bindSamplerStateObject(sampler.state, sampler.unit, tb->haveMips());
	}

	// Bind sbuffers.
	for (uint32_t i = 0; i < m_bufferViews.size(); ++i)
	{
#if !defined(__APPLE__)
		if (m_bufferViews[i])
			{ T_OGL_SAFE(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, i + 3, m_bufferViews[i]->getBuffer())); }
		else
			{ T_OGL_SAFE(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, i + 3, 0)); }
#endif
	}

	return true;
}

bool ProgramOpenGL::activateCompute(RenderContextOpenGL* renderContext)
{
	return false;
}

ProgramOpenGL::ProgramOpenGL(ResourceContextOpenGL* resourceContext, GLuint program, const ProgramResource* resource)
:	m_resourceContext(resourceContext)
,	m_program(program)
,	m_renderStateList(0)
,	m_stencilReference(0)
,	m_attributeHash(0)
{
	const ProgramResourceOpenGL* resourceOpenGL = checked_type_cast< const ProgramResourceOpenGL* >(resource);

	// Get handle to implicit parameters.
	s_handleTargetSize = getParameterHandle(L"_gl_targetSize");

	// Setup our render state.
	m_renderStateList = resourceContext->createRenderStateObject(resourceOpenGL->m_renderState);

	// Get vertex attribute locations.
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

	// Calculate hash of vertex attribute locations, used as signature with bound vertex buffer.
	Adler32 chk;
	chk.begin();
	chk.feed(m_attributeLocs, sizeof(m_attributeLocs));
	chk.end();
	m_attributeHash = chk.get();

	// Create uniform buffer objects.
	for (uint32_t i = 0; i < 3; ++i)
	{
		m_uniformBuffers[i].size = resourceOpenGL->m_uniformBufferSizes[i] * 4;
		m_uniformBuffers[i].data.resize(resourceOpenGL->m_uniformBufferSizes[i], 0.0f);

		T_OGL_SAFE(glGenBuffers(1, &m_uniformBuffers[i].object));
		T_OGL_SAFE(glBindBuffer(GL_UNIFORM_BUFFER, m_uniformBuffers[i].object));
		T_OGL_SAFE(glBufferData(GL_UNIFORM_BUFFER, m_uniformBuffers[i].size, NULL, GL_STATIC_DRAW));
		T_OGL_SAFE(glBindBuffer(GL_UNIFORM_BUFFER, 0));	
	}

	// Allocate texture slots.
	m_textures.resize(resourceOpenGL->m_texturesCount);

	// Setup samplers.
	for (const auto& resourceSampler : resourceOpenGL->m_samplers)
	{
		auto& s = m_samplers.push_back();
		s.unit = resourceSampler.unit;
		s.state = resourceContext->createSamplerStateObject(resourceSampler.state);
		s.textureIndex = resourceSampler.textureIndex;
	}

	// Allocate sbuffer slots.
	m_bufferViews.resize(resourceOpenGL->m_sbufferCount);

	// Setup parameter mapping.
	for (auto p : resourceOpenGL->m_parameters)
	{
		auto& pm = m_parameterMap[getParameterHandle(p.name)];
		pm.buffer = p.buffer;
		pm.offset = p.offset;
		pm.size = p.size;
	}
}

	}
}

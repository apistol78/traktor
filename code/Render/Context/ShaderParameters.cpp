#include "Render/Context/ShaderParameters.h"
#include "Render/Context/RenderContext.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

/*! \brief Move pointer to next aligned address for current type. */
template < typename Type >
inline void align(uint8_t*& ptr)
{
	ptr = (uint8_t*)((size_t(ptr) + (alignOf< Type >() - 1)) & ~(alignOf< Type >() - 1));
}

template < typename Type >
inline void write(uint8_t*& writePtr, const Type& value)
{
	*reinterpret_cast< Type* >(writePtr) = value;
	writePtr += sizeof(Type);
}

template < >
inline void write< Vector4 >(uint8_t*& writePtr, const Vector4& value)
{
	value.storeAligned(reinterpret_cast< float* >(writePtr));
	writePtr += sizeof(Vector4);
}

template < >
inline void write< Matrix44 >(uint8_t*& writePtr, const Matrix44& value)
{
	value.storeAligned(reinterpret_cast< float* >(writePtr));
	writePtr += sizeof(Matrix44);
}

template < typename Type >
inline void write(uint8_t*& writePtr, const Type* valueArray, int length)
{
	for (int i = 0; i < length; ++i)
	{
		*reinterpret_cast< Type* >(writePtr) = valueArray[i];
		writePtr += sizeof(Type);
	}
}

template < >
inline void write< Vector4 >(uint8_t*& writePtr, const Vector4* valueArray, int length)
{
	for (int i = 0; i < length; ++i)
	{
		valueArray[i].storeAligned(reinterpret_cast< float* >(writePtr));
		writePtr += sizeof(Vector4);
	}
}

template < >
inline void write< Matrix44 >(uint8_t*& writePtr, const Matrix44* valueArray, int length)
{
	for (int i = 0; i < length; ++i)
	{
		valueArray[i].storeAligned(reinterpret_cast< float* >(writePtr));
		writePtr += sizeof(Matrix44);
	}
}

template < typename Type >
inline const Type& read(uint8_t*& readPtr)
{
	uint8_t* valuePtr = readPtr; readPtr += sizeof(Type);
	return *reinterpret_cast< Type* >(valuePtr);
}

template < typename Type >
inline const Type* read(uint8_t*& readPtr, int length)
{
	uint8_t* valuePtr = readPtr; readPtr += length * sizeof(Type);
	return reinterpret_cast< Type* >(valuePtr);
}

enum ParameterTypes
{
	PmtCombination,
	PmtFloat,
	PmtFloatArray,
	PmtVector,
	PmtVectorArray,
	PmtMatrix,
	PmtMatrixArray,
	PmtTexture,
	PmtStencilReference
};

		}

ShaderParameters::ShaderParameters()
:	m_technique(0)
,	m_parameterFirst(0)
,	m_parameterLast(0)
{
}

void ShaderParameters::setTechnique(handle_t handle)
{
	m_technique = handle;
}

void ShaderParameters::beginParameters(RenderContext* context)
{
	m_parameterFirst =
	m_parameterLast = static_cast< uint8_t* >(context->alloc(0));
}

void ShaderParameters::endParameters(RenderContext* context)
{
	T_ASSERT (m_parameterFirst);
	uint32_t parametersSize = uint32_t(m_parameterLast - m_parameterFirst);
	if (!context->alloc(parametersSize))
		T_FATAL_ERROR;
}

void ShaderParameters::setCombination(handle_t handle, bool param)
{
	T_ASSERT (m_parameterLast);
	align< handle_t >(m_parameterLast);
	write< handle_t >(m_parameterLast, handle);
	write< int >(m_parameterLast, PmtCombination);
	write< bool >(m_parameterLast, param);
}

void ShaderParameters::setFloatParameter(handle_t handle, float param)
{
	T_ASSERT (m_parameterLast);
	align< handle_t >(m_parameterLast);
	write< handle_t >(m_parameterLast, handle);
	write< int >(m_parameterLast, PmtFloat);
	write< float >(m_parameterLast, param);
}

void ShaderParameters::setFloatArrayParameter(handle_t handle, const float* param, int length)
{
	T_ASSERT (m_parameterLast);
	align< handle_t >(m_parameterLast);
	write< handle_t >(m_parameterLast, handle);
	write< int >(m_parameterLast, PmtFloatArray);
	write< int >(m_parameterLast, length);
	write< float >(m_parameterLast, param, length);
}

void ShaderParameters::setVectorParameter(handle_t handle, const Vector4& param)
{
	T_ASSERT (m_parameterLast);
	align< handle_t >(m_parameterLast);
	write< handle_t >(m_parameterLast, handle);
	write< int >(m_parameterLast, PmtVector);
	align< Vector4 >(m_parameterLast);
	write< Vector4 >(m_parameterLast, param);
}

void ShaderParameters::setVectorArrayParameter(handle_t handle, const Vector4* param, int length)
{
	T_ASSERT (m_parameterLast);
	align< handle_t >(m_parameterLast);
	write< handle_t >(m_parameterLast, handle);
	write< int >(m_parameterLast, PmtVectorArray);
	write< int >(m_parameterLast, length);
	align< Vector4 >(m_parameterLast);
	write< Vector4 >(m_parameterLast, param, length);
}

void ShaderParameters::setMatrixParameter(handle_t handle, const Matrix44& param)
{
	T_ASSERT (m_parameterLast);
	align< handle_t >(m_parameterLast);
	write< handle_t >(m_parameterLast, handle);
	write< int >(m_parameterLast, PmtMatrix);
	align< Matrix44 >(m_parameterLast);
	write< Matrix44 >(m_parameterLast, param);
}

void ShaderParameters::setMatrixArrayParameter(handle_t handle, const Matrix44* param, int length)
{
	T_ASSERT (m_parameterLast);
	align< handle_t >(m_parameterLast);
	write< handle_t >(m_parameterLast, handle);
	write< int >(m_parameterLast, PmtMatrixArray);
	write< int >(m_parameterLast, length);
	align< Matrix44 >(m_parameterLast);
	write< Matrix44 >(m_parameterLast, param, length);
}

void ShaderParameters::setTextureParameter(handle_t handle, ITexture* texture)
{
	T_ASSERT (m_parameterLast);
	align< handle_t >(m_parameterLast);
	write< handle_t >(m_parameterLast, handle);
	write< int >(m_parameterLast, PmtTexture);
	write< ITexture* >(m_parameterLast, texture);
}

void ShaderParameters::setStencilReference(uint32_t stencilReference)
{
	T_ASSERT (m_parameterLast);
	align< handle_t >(m_parameterLast);
	write< handle_t >(m_parameterLast, 0);
	write< int >(m_parameterLast, PmtStencilReference);
	write< uint32_t >(m_parameterLast, stencilReference);
}

void ShaderParameters::fixup(Shader* shader) const
{
	if (m_technique)
		shader->setTechnique(m_technique);

	for (uint8_t* parameter = m_parameterFirst; parameter < m_parameterLast; )
	{
		align< handle_t >(parameter);

		handle_t handle = read< handle_t >(parameter);
		int type = read< int >(parameter);

		switch (type)
		{
		case PmtCombination:
			shader->setCombination(handle, read< bool >(parameter));
			break;

		case PmtFloat:
			shader->setFloatParameter(handle, read< float >(parameter));
			break;

		case PmtFloatArray:
			{
				int length = read< int >(parameter);
				shader->setFloatArrayParameter(handle, read< float >(parameter, length), length);
			}
			break;

		case PmtVector:
			align< Vector4 >(parameter);
			shader->setVectorParameter(handle, read< Vector4 >(parameter));
			break;

		case PmtVectorArray:
			{
				int length = read< int >(parameter);
				align< Vector4 >(parameter);
				shader->setVectorArrayParameter(handle, read< Vector4 >(parameter, length), length);
			}
			break;

		case PmtMatrix:
			align< Matrix44 >(parameter);
			shader->setMatrixParameter(handle, read< Matrix44 >(parameter));
			break;

		case PmtMatrixArray:
			{
				int length = read< int >(parameter);
				align< Matrix44 >(parameter);
				shader->setMatrixArrayParameter(handle, read< Matrix44 >(parameter, length), length);
			}
			break;

		case PmtTexture:
			shader->setTextureParameter(handle, read< ITexture* >(parameter));
			break;

		case PmtStencilReference:
			shader->setStencilReference(read< uint32_t >(parameter));
			break;
		}
	}
}

	}
}

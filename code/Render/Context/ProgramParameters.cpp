#include "Render/IProgram.h"
#include "Render/Context/ProgramParameters.h"
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
	size_t alignment = alignOf< Type >();
	ptr = (uint8_t*)((size_t(ptr) + (alignment - 1)) & ~(alignment - 1));
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

ProgramParameters::ProgramParameters()
:	m_attachParameters(0)
,	m_parameterFirst(0)
,	m_parameterLast(0)
{
}

void ProgramParameters::attachParameters(ProgramParameters* attachParameters)
{
	m_attachParameters = attachParameters;
}

void ProgramParameters::beginParameters(RenderContext* context)
{
	m_parameterFirst =
	m_parameterLast = static_cast< uint8_t* >(context->alloc(0));
}

void ProgramParameters::endParameters(RenderContext* context)
{
	T_ASSERT (m_parameterFirst);
	uint32_t parametersSize = uint32_t(m_parameterLast - m_parameterFirst);
	if (!context->alloc(parametersSize))
		T_FATAL_ERROR;
}

void ProgramParameters::setFloatParameter(handle_t handle, float param)
{
	T_ASSERT (m_parameterLast);
	align< handle_t >(m_parameterLast);
	write< handle_t >(m_parameterLast, handle);
	write< int >(m_parameterLast, PmtFloat);
	write< float >(m_parameterLast, param);
}

void ProgramParameters::setFloatArrayParameter(handle_t handle, const float* param, int length)
{
	T_ASSERT (m_parameterLast);
	align< handle_t >(m_parameterLast);
	write< handle_t >(m_parameterLast, handle);
	write< int >(m_parameterLast, PmtFloatArray);
	write< int >(m_parameterLast, length);
	write< float >(m_parameterLast, param, length);
}

void ProgramParameters::setVectorParameter(handle_t handle, const Vector4& param)
{
	T_ASSERT (m_parameterLast);
	align< handle_t >(m_parameterLast);
	write< handle_t >(m_parameterLast, handle);
	write< int >(m_parameterLast, PmtVector);
	align< Vector4 >(m_parameterLast);
	write< Vector4 >(m_parameterLast, param);
}

void ProgramParameters::setVectorArrayParameter(handle_t handle, const Vector4* param, int length)
{
	T_ASSERT (m_parameterLast);
	align< handle_t >(m_parameterLast);
	write< handle_t >(m_parameterLast, handle);
	write< int >(m_parameterLast, PmtVectorArray);
	write< int >(m_parameterLast, length);
	align< Vector4 >(m_parameterLast);
	write< Vector4 >(m_parameterLast, param, length);
}

void ProgramParameters::setMatrixParameter(handle_t handle, const Matrix44& param)
{
	T_ASSERT (m_parameterLast);
	align< handle_t >(m_parameterLast);
	write< handle_t >(m_parameterLast, handle);
	write< int >(m_parameterLast, PmtMatrix);
	align< Matrix44 >(m_parameterLast);
	write< Matrix44 >(m_parameterLast, param);
}

void ProgramParameters::setMatrixArrayParameter(handle_t handle, const Matrix44* param, int length)
{
	T_ASSERT (m_parameterLast);
	align< handle_t >(m_parameterLast);
	write< handle_t >(m_parameterLast, handle);
	write< int >(m_parameterLast, PmtMatrixArray);
	write< int >(m_parameterLast, length);
	align< Matrix44 >(m_parameterLast);
	write< Matrix44 >(m_parameterLast, param, length);
}

void ProgramParameters::setTextureParameter(handle_t handle, ITexture* texture)
{
	T_ASSERT (m_parameterLast);
	align< handle_t >(m_parameterLast);
	write< handle_t >(m_parameterLast, handle);
	write< int >(m_parameterLast, PmtTexture);
	write< ITexture* >(m_parameterLast, texture);
}

void ProgramParameters::setStencilReference(uint32_t stencilReference)
{
	T_ASSERT (m_parameterLast);
	align< handle_t >(m_parameterLast);
	write< handle_t >(m_parameterLast, 0);
	write< int >(m_parameterLast, PmtStencilReference);
	write< uint32_t >(m_parameterLast, stencilReference);
}

void ProgramParameters::fixup(IProgram* program) const
{
	T_ASSERT (program);

	for (uint8_t* parameter = m_parameterFirst; parameter < m_parameterLast; )
	{
		align< handle_t >(parameter);

		handle_t handle = read< handle_t >(parameter);
		int type = read< int >(parameter);

		switch (type)
		{
		case PmtFloat:
			program->setFloatParameter(handle, read< float >(parameter));
			break;

		case PmtFloatArray:
			{
				int length = read< int >(parameter);
				program->setFloatArrayParameter(handle, read< float >(parameter, length), length);
			}
			break;

		case PmtVector:
			align< Vector4 >(parameter);
			program->setVectorParameter(handle, read< Vector4 >(parameter));
			break;

		case PmtVectorArray:
			{
				int length = read< int >(parameter);
				align< Vector4 >(parameter);
				program->setVectorArrayParameter(handle, read< Vector4 >(parameter, length), length);
			}
			break;

		case PmtMatrix:
			align< Matrix44 >(parameter);
			program->setMatrixParameter(handle, read< Matrix44 >(parameter));
			break;

		case PmtMatrixArray:
			{
				int length = read< int >(parameter);
				align< Matrix44 >(parameter);
				program->setMatrixArrayParameter(handle, read< Matrix44 >(parameter, length), length);
			}
			break;

		case PmtTexture:
			{
				ITexture* texture = read< ITexture* >(parameter);
				program->setTextureParameter(handle, texture);
			}
			break;

		case PmtStencilReference:
			program->setStencilReference(read< uint32_t >(parameter));
			break;
		}
	}

	if (m_attachParameters)
		m_attachParameters->fixup(program);
}

	}
}

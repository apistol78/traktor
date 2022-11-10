/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/Vector4.h"
#include "Core/Math/Matrix44.h"
#include "Render/IProgram.h"
#include "Render/Context/ProgramParameters.h"
#include "Render/Context/RenderContext.h"

namespace traktor::render
{
	namespace
	{

/*! Move pointer to next aligned address for current type. */
template < typename Type >
inline void align(uint8_t*& ptr)
{
	size_t alignment = alignOf< Type >();
	ptr = (uint8_t*)((size_t(ptr) + (alignment - 1)) & ~(alignment - 1));
}

template < typename Type >
inline void write(uint8_t*& writePtr, const Type& value)
{
	align< Type >(writePtr);
	*reinterpret_cast< Type* >(writePtr) = value;
	writePtr += sizeof(Type);
}

template < >
inline void write< Vector4 >(uint8_t*& writePtr, const Vector4& value)
{
	align< Vector4 >(writePtr);
	value.storeAligned(reinterpret_cast< float* >(writePtr));
	writePtr += sizeof(Vector4);
}

template < >
inline void write< Matrix44 >(uint8_t*& writePtr, const Matrix44& value)
{
	align< Matrix44 >(writePtr);
	value.storeAligned(reinterpret_cast< float* >(writePtr));
	writePtr += sizeof(Matrix44);
}

template < typename Type >
inline void write(uint8_t*& writePtr, const Type* valueArray, int32_t length)
{
	align< Type >(writePtr);
	for (int32_t i = 0; i < length; ++i)
	{
		*reinterpret_cast< Type* >(writePtr) = valueArray[i];
		writePtr += sizeof(Type);
	}
}

template < >
inline void write< Vector4 >(uint8_t*& writePtr, const Vector4* valueArray, int32_t length)
{
	align< Vector4 >(writePtr);
	for (int32_t i = 0; i < length; ++i)
	{
		valueArray[i].storeAligned(reinterpret_cast< float* >(writePtr));
		writePtr += sizeof(Vector4);
	}
}

template < >
inline void write< Matrix44 >(uint8_t*& writePtr, const Matrix44* valueArray, int32_t length)
{
	align< Matrix44 >(writePtr);
	for (int32_t i = 0; i < length; ++i)
	{
		valueArray[i].storeAligned(reinterpret_cast< float* >(writePtr));
		writePtr += sizeof(Matrix44);
	}
}

template < typename Type >
inline const Type& read(uint8_t*& readPtr)
{
	align< Type >(readPtr);
	uint8_t* valuePtr = readPtr; readPtr += sizeof(Type);
	return *reinterpret_cast< Type* >(valuePtr);
}

template < typename Type >
inline const Type* read(uint8_t*& readPtr, int32_t length)
{
	align< Type >(readPtr);
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
	PmtImageView,
	PmtBufferView,
	PmtStencilReference,
	PmtAttachedParameters
};

	}

void ProgramParameters::beginParameters(RenderContext* context)
{
	m_parameterFirst =
	m_parameterLast = static_cast< uint8_t* >(context->alloc(0));
}

void ProgramParameters::endParameters(RenderContext* context)
{
	T_ASSERT(m_parameterFirst);
	uint32_t parametersSize = uint32_t(m_parameterLast - m_parameterFirst);
	if (!context->alloc(parametersSize))
		T_FATAL_ERROR;
}

void ProgramParameters::setFloatParameter(handle_t handle, float param)
{
	T_ASSERT(m_parameterLast);
	write< handle_t >(m_parameterLast, handle);
	write< int8_t >(m_parameterLast, PmtFloat);
	write< float >(m_parameterLast, param);
}

void ProgramParameters::setFloatArrayParameter(handle_t handle, const float* param, int length)
{
	T_ASSERT(m_parameterLast);
	write< handle_t >(m_parameterLast, handle);
	write< int8_t >(m_parameterLast, PmtFloatArray);
	write< int32_t >(m_parameterLast, length);
	write< float >(m_parameterLast, param, length);
}

void ProgramParameters::setVectorParameter(handle_t handle, const Vector4& param)
{
	T_ASSERT(m_parameterLast);
	write< handle_t >(m_parameterLast, handle);
	write< int8_t >(m_parameterLast, PmtVector);
	write< Vector4 >(m_parameterLast, param);
}

void ProgramParameters::setVectorArrayParameter(handle_t handle, const Vector4* param, int length)
{
	T_ASSERT(m_parameterLast);
	write< handle_t >(m_parameterLast, handle);
	write< int8_t >(m_parameterLast, PmtVectorArray);
	write< int32_t >(m_parameterLast, length);
	write< Vector4 >(m_parameterLast, param, length);
}

void ProgramParameters::setMatrixParameter(handle_t handle, const Matrix44& param)
{
	T_ASSERT(m_parameterLast);
	write< handle_t >(m_parameterLast, handle);
	write< int8_t >(m_parameterLast, PmtMatrix);
	write< Matrix44 >(m_parameterLast, param);
}

void ProgramParameters::setMatrixArrayParameter(handle_t handle, const Matrix44* param, int length)
{
	T_ASSERT(m_parameterLast);
	write< handle_t >(m_parameterLast, handle);
	write< int8_t >(m_parameterLast, PmtMatrixArray);
	write< int32_t >(m_parameterLast, length);
	write< Matrix44 >(m_parameterLast, param, length);
}

void ProgramParameters::setTextureParameter(handle_t handle, ITexture* texture)
{
	T_ASSERT(m_parameterLast);
	write< handle_t >(m_parameterLast, handle);
	write< int8_t >(m_parameterLast, PmtTexture);
	write< ITexture* >(m_parameterLast, texture);
}

void ProgramParameters::setImageViewParameter(handle_t handle, ITexture* imageView)
{
	T_ASSERT(m_parameterLast);
	write< handle_t >(m_parameterLast, handle);
	write< int8_t >(m_parameterLast, PmtImageView);
	write< ITexture* >(m_parameterLast, imageView);
}

void ProgramParameters::setBufferViewParameter(handle_t handle, const IBufferView* bufferView)
{
	T_ASSERT(m_parameterLast);
	write< handle_t >(m_parameterLast, handle);
	write< int8_t >(m_parameterLast, PmtBufferView);
	write< const IBufferView* >(m_parameterLast, bufferView);
}

void ProgramParameters::setStencilReference(uint32_t stencilReference)
{
	T_ASSERT(m_parameterLast);
	write< handle_t >(m_parameterLast, 0);
	write< int8_t >(m_parameterLast, PmtStencilReference);
	write< uint32_t >(m_parameterLast, stencilReference);
}

void ProgramParameters::attachParameters(const ProgramParameters* programParameters)
{
	T_ASSERT(m_parameterLast);
	T_ASSERT(programParameters);
	write< handle_t >(m_parameterLast, 0);
	write< int8_t >(m_parameterLast, PmtAttachedParameters);
	write< const ProgramParameters* >(m_parameterLast, programParameters);
}

void ProgramParameters::fixup(IProgram* program) const
{
	T_ASSERT(program);
	for (uint8_t* parameter = m_parameterFirst; parameter < m_parameterLast; )
	{
		handle_t handle = read< handle_t >(parameter);
		int8_t type = read< int8_t >(parameter);
		switch (type)
		{
		case PmtFloat:
			program->setFloatParameter(handle, read< float >(parameter));
			break;

		case PmtFloatArray:
			{
				auto length = read< int32_t >(parameter);
				program->setFloatArrayParameter(handle, read< float >(parameter, length), length);
			}
			break;

		case PmtVector:
			program->setVectorParameter(handle, read< Vector4 >(parameter));
			break;

		case PmtVectorArray:
			{
				auto length = read< int32_t >(parameter);
				program->setVectorArrayParameter(handle, read< Vector4 >(parameter, length), length);
			}
			break;

		case PmtMatrix:
			program->setMatrixParameter(handle, read< Matrix44 >(parameter));
			break;

		case PmtMatrixArray:
			{
				auto length = read< int32_t >(parameter);
				program->setMatrixArrayParameter(handle, read< Matrix44 >(parameter, length), length);
			}
			break;

		case PmtTexture:
			{
				auto texture = read< ITexture* >(parameter);
				program->setTextureParameter(handle, texture);
			}
			break;

		case PmtImageView:
			{
				auto imageView = read< ITexture* >(parameter);
				program->setImageViewParameter(handle, imageView);
			}
			break;

		case PmtBufferView:
			{
				auto bufferView = read< const IBufferView* >(parameter);
				program->setBufferViewParameter(handle, bufferView);
			}
			break;

		case PmtStencilReference:
			program->setStencilReference(read< uint32_t >(parameter));
			break;

		case PmtAttachedParameters:
			{
				auto programParameters = read< const ProgramParameters* >(parameter);
				programParameters->fixup(program);
			}
			break;
		}
	}
}

}

/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_VertexBuffer_H
#define traktor_render_VertexBuffer_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

/*! \brief Vertex buffer.
 * \ingroup Render
 */
class T_DLLCLASS VertexBuffer : public Object
{
	T_RTTI_CLASS;

public:
	VertexBuffer(uint32_t bufferSize);

	/*! \brief Get buffer size in bytes.
	 *
	 * \return Buffer size.
	 */
	uint32_t getBufferSize() const;

	/*! \brief Destroy resources allocated by this buffer. */
	virtual void destroy() = 0;

	/*! \brief Lock access to entire buffer data.
	 *
	 * \return Pointer to buffer beginning.
	 */
	virtual void* lock() = 0;

	/*! \brief Lock access to segment of buffer data.
	 *
	 * \param vertexOffset Vertex offset.
	 * \param vertexCount Number of vertices to lock.
	 * \return Pointer to segment beginning.
	 */
	virtual void* lock(uint32_t vertexOffset, uint32_t vertexCount) = 0;
	
	/*! \brief Unlock access. */
	virtual void unlock() = 0;

	/*! \brief Set content valid flag. */
	inline void setContentValid(bool contentValid) { m_contentValid = contentValid; }

	/*! \brief Check content valid flag. */
	inline bool isContentValid() const { return m_contentValid; }

private:
	uint32_t m_bufferSize;
	bool m_contentValid;
};
	
	}
}

#endif	// traktor_render_VertexBuffer_H

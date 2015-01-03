#ifndef traktor_render_RenderTargetSet_H
#define traktor_render_RenderTargetSet_H

#include "Render/ISimpleTexture.h"

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
	
/*! \brief Render target set.
 * \ingroup Render
 *
 * The content valid flag is automatically reset when
 * the content of the render target has been reset 
 * on the device.
 */
class T_DLLCLASS RenderTargetSet : public Object
{
	T_RTTI_CLASS;

public:
	RenderTargetSet();

	/*! \brief Destroy render targets.h */
	virtual void destroy() = 0;

	/*! \brief Get width of render targets. */
	virtual int getWidth() const = 0;
	
	/*! \brief Get height of render targets. */
	virtual int getHeight() const = 0;

	/*! \brief Get color target texture. */
	virtual ISimpleTexture* getColorTexture(int index) const = 0;

	/*! \brief Get depth target texture. */
	virtual ISimpleTexture* getDepthTexture() const = 0;

	/*! \brief Swap color targets. */
	virtual void swap(int index1, int index2) = 0;

	/*! \brief Discard target content. */
	virtual void discard() = 0;

	/*! \brief Read back color target into system memory.
	 *
	 * \note
	 * This is a very slow operation and is only
	 * designed to be for screen shots etc.
	 *
	 * \param index Color target index.
	 * \param buffer System memory buffer; target is copied into
	 *               this buffer so it's up to the caller to ensure
	 *               it large enough.
	 * \return True if successfully copied.
	 */
	virtual bool read(int index, void* buffer) const = 0;

	/*! \brief Set content valid flag. */
	inline void setContentValid(bool contentValid) { m_contentValid = contentValid; }

	/*! \brief Check content valid flag. */
	inline bool isContentValid() const { return m_contentValid; }

private:
	bool m_contentValid;
};
	
	}
}

#endif	// traktor_render_RenderTargetSet_H

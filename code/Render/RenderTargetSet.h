#ifndef traktor_render_RenderTargetSet_H
#define traktor_render_RenderTargetSet_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class Texture;
	
/*! \brief Render target set.
 * \ingroup Render
 *
 * The content valid flag is automatically reset when
 * the content of the render target has been reset 
 * on the device.
 */
class T_DLLCLASS RenderTargetSet : public Object
{
	T_RTTI_CLASS(RenderTargetSet)

public:
	RenderTargetSet();

	/*! \brief Destroy render targets.h */
	virtual void destroy() = 0;

	/*! \brief Get width of render targets. */
	virtual int getWidth() const = 0;
	
	/*! \brief Get height of render targets. */
	virtual int getHeight() const = 0;

	/*! \brief Get color target texture. */
	virtual Texture* getColorTexture(int index) const = 0;

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

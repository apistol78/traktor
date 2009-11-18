#ifndef traktor_render_SimpleTextureSw_H
#define traktor_render_SimpleTextureSw_H

#include "Render/ISimpleTexture.h"
#include "Render/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_SW_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{
		
/*!
 * \ingroup SW
 */
class T_DLLCLASS SimpleTextureSw : public ISimpleTexture
{
	T_RTTI_CLASS;

public:
	SimpleTextureSw();

	virtual ~SimpleTextureSw();
	
	bool create(const SimpleTextureCreateDesc& desc);

	virtual void destroy();

	virtual int getWidth() const;
	
	virtual int getHeight() const;
	
	virtual int getDepth() const;

	virtual bool lock(int level, Lock& lock);

	virtual void unlock(int level);

	inline const uint32_t* getData() const { return m_data; }

private:
	int m_width;
	int m_height;
	TextureFormat m_format;
	uint32_t* m_data;
	uint8_t* m_lock;
};
		
	}
}

#endif	// traktor_render_SimpleTextureSw_H

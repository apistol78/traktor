#ifndef traktor_flash_FlashSprite_H
#define traktor_flash_FlashSprite_H

#include <map>
#include "Core/Heap/RefArray.h"
#include "Flash/FlashCharacter.h"
#include "Flash/SwfTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

class FlashFrame;
class ActionVM;
class ActionContext;
class ActionScript;

/*! \brief Flash sprite.
 * \ingroup Flash
 */
class T_DLLCLASS FlashSprite : public FlashCharacter
{
	T_RTTI_CLASS(FlashSprite)

public:
	FlashSprite(uint16_t id, uint16_t frameRate);

	uint16_t getFrameRate() const;

	void addFrame(FlashFrame* frame);

	uint32_t getFrameCount() const;

	FlashFrame* getFrame(uint32_t frameId) const;

	int findFrame(const std::wstring& frameLabel) const;

	void addInitActionScript(ActionScript* initActionScript);

	const RefArray< ActionScript >& getInitActionScripts() const;

	virtual Ref< FlashCharacterInstance > createInstance(ActionContext* context, FlashCharacterInstance* parent) const;

private:
	uint16_t m_frameRate;
	RefArray< FlashFrame > m_frames;
	RefArray< ActionScript > m_initActionScripts;
};

	}
}

#endif	// traktor_flash_FlashSprite_H

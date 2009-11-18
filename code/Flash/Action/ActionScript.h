#ifndef traktor_flash_ActionScript_H
#define traktor_flash_ActionScript_H

#include "Core/Object.h"
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

/*! \brief ActionScript byte code container.
 * \ingroup Flash
 */
class T_DLLCLASS ActionScript : public Object
{
	T_RTTI_CLASS;

public:
	ActionScript(uint32_t codeSize);

	virtual ~ActionScript();
	
	uint8_t* getCode() const;

	uint32_t getCodeSize() const;

private:
	uint8_t* m_code;
	uint32_t m_codeSize;
};

	}
}

#endif	// traktor_flash_ActionScript_H

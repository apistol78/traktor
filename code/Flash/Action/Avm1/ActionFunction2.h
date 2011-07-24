#ifndef traktor_flash_ActionFunction2_H
#define traktor_flash_ActionFunction2_H

#include "Flash/Action/ActionFunction.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

class ActionDictionary;

/*! \brief ActionScript script function.
 * \ingroup Flash
 */
class T_DLLCLASS ActionFunction2 : public ActionFunction
{
	T_RTTI_CLASS;

public:
	enum Flags
	{
		AffPreloadThis			= 0x01,
		AffSuppressThis			= 0x02,
		AffPreloadArguments		= 0x04,
		AffSuppressArguments	= 0x08,
		AffPreloadSuper			= 0x10,
		AffSuppressSuper		= 0x20,
		AffPreloadRoot			= 0x40,
		AffPreloadParent		= 0x80,
		AffPreloadGlobal		= 0x100
	};

	ActionFunction2(
		const std::string& name,
		const IActionVMImage* image,
		uint8_t registerCount,
		uint16_t flags,
		const std::vector< std::pair< std::string, uint8_t > >& argumentsIntoRegisters,
		const std::map< std::string, ActionValue >& variables,
		ActionDictionary* dictionary
	);

	virtual ActionValue call(ActionContext* context, ActionObject* self, const ActionValueArray& args);

	virtual ActionValue call(ActionFrame* callerFrame, ActionObject* self);

protected:
	virtual void trace(const IVisitor& visitor) const;

	virtual void dereference();

private:
	Ref< const IActionVMImage > m_image;
	uint8_t m_registerCount;
	uint16_t m_flags;
	std::vector< std::pair< std::string, uint8_t > > m_argumentsIntoRegisters;
	std::map< std::string, ActionValue > m_variables;
	Ref< ActionDictionary > m_dictionary;
};

	}
}

#endif	// traktor_flash_ActionFunction2_H

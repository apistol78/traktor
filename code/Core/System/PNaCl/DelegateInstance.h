#ifndef traktor_DelegateInstance_H
#define traktor_DelegateInstance_H

#include <vector>
#include <ppapi/cpp/instance.h>
#include "Core/Config.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief PPAPI delegable instance.
 * \ingroup Core
 */
class T_DLLCLASS DelegateInstance : public pp::Instance
{
public:
	struct IDelegate
	{
		virtual void notifyDidChangeView(const pp::View& view) {}

		virtual void notifyDidChangeView(const pp::Rect& position, const pp::Rect& clip) {}

		virtual void notifyDidChangeFocus(bool has_focus) {}

		virtual bool notifyHandleInputEvent(const pp::InputEvent& event) { return false; }

		virtual bool notifyHandleDocumentLoad(const pp::URLLoader& url_loader) { return false; }

		virtual void notifyHandleMessage(const pp::Var& message) {}
	};

	explicit DelegateInstance(PP_Instance instance);

	void addDelegate(IDelegate* delegate);

	void removeDelegate(IDelegate* delegate);

	virtual void DidChangeView(const pp::View& view);

	virtual void DidChangeView(const pp::Rect& position, const pp::Rect& clip);

	virtual void DidChangeFocus(bool has_focus);

	virtual bool HandleInputEvent(const pp::InputEvent& event);

	virtual bool HandleDocumentLoad(const pp::URLLoader& url_loader);

	virtual void HandleMessage(const pp::Var& message);

private:
	std::vector< IDelegate* > m_delegates;
};

}

#endif	// traktor_DelegateInstance_H

#ifndef traktor_db_MessageListener_H
#define traktor_db_MessageListener_H

#include <map>
#include "Core/Heap/Ref.h"
#include "Core/Heap/GcNew.h"
#include "Database/Remote/Server/IMessageListener.h"
#include "Database/Remote/IMessage.h"

namespace traktor
{
	namespace db
	{

/*! \brief Message listener implementation.
 * \ingroup Database
 */
template < typename MessageListenerType >
class IMessageListenerImpl : public IMessageListener
{
public:
	virtual bool notify(const IMessage* message)
	{
		typename std::map< const Type*, Ref< IMethod > >::iterator i = m_listenerMethods.find(&type_of(message));
		return i != m_listenerMethods.end() ? i->second->invoke(reinterpret_cast< MessageListenerType* >(this), message) : false;
	}

protected:
	struct IMethod : public Object
	{
		virtual bool invoke(MessageListenerType* const this_, const IMessage* message) = 0;
	};
	
	template < typename MessageType >
	struct IMethodImpl : public IMethod
	{
		typedef bool (MessageListenerType::*method_t)(const MessageType* message);

		method_t m_method;

		IMethodImpl(method_t method)
		:	m_method(method)
		{
		}

		virtual bool invoke(MessageListenerType* const this_, const IMessage* message)
		{
			return (this_->*m_method)(checked_type_cast< const MessageType* >(message));
		}
	};

	template < typename MessageType >
	void registerMessage(typename IMethodImpl< MessageType >::method_t method)
	{
		m_listenerMethods[&type_of< MessageType >()] = gc_new< IMethodImpl< MessageType > >(method);
	}

private:
	std::map< const Type*, Ref< IMethod > > m_listenerMethods;
};

	}
}

#endif	// traktor_db_MessageListener_H

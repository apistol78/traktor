/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <map>
#include "Core/Ref.h"
#include "Database/Remote/IMessage.h"
#include "Database/Remote/Server/IMessageListener.h"

namespace traktor
{
	namespace db
	{

/*! Message listener implementation.
 * \ingroup Database
 */
template < typename MessageListenerType >
class IMessageListenerImpl : public IMessageListener
{
public:
	virtual bool notify(const IMessage* message)
	{
		typename std::map< const TypeInfo*, Ref< IMethod > >::iterator i = m_listenerMethods.find(&type_of(message));
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
		m_listenerMethods[&type_of< MessageType >()] = new IMethodImpl< MessageType >(method);
	}

private:
	std::map< const TypeInfo*, Ref< IMethod > > m_listenerMethods;
};

	}
}


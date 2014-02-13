/*
 * File description: contract.h
 * Author information: Mike Ranzinger mranzinger@alchemyapi.com
 * Copyright information: Copyright Orchestr8 LLC
 */

#ifndef CONTRACT_H_
#define CONTRACT_H_

#include <tuple>

#include "message.h"
#include "function_invoker.h"
#include "util/uuid.h"

namespace axon { namespace communication {

namespace s = serialization;

template<typename S>
class CContract;

template<typename TupleType, int Ct>
struct arg_unpacker;

template<typename Fn>
struct fn_invoker;

template<typename Ret, typename ...Args>
class CContract<Ret (Args...)>
{


private:
	std::string m_action;

public:
	typedef std::tuple<Args...> tuple_type;

	CContract(std::string a_action)
		: m_action(std::move(a_action))
	{

	}

	const std::string &GetAction() const { return m_action; }

	CMessage::Ptr Serialize(const Args &...a_vals) const
	{
		auto l_msg = std::make_shared<CMessage>();

		p_Serialize(*l_msg, a_vals...);

		return std::move(l_msg);
	}

	CMessage::Ptr SerializeVoidRet(const CMessage &a_inMsg) const
	{
		return std::make_shared<CMessage>(a_inMsg, MessageType::Normal);
	}

	template<typename TRet>
	CMessage::Ptr SerializeRet(const CMessage &a_inMsg, const TRet &a_val) const
	{
		auto l_msg = std::make_shared<CMessage>(a_inMsg, MessageType::Normal);

		p_SerializeRet(*l_msg, a_val);

		return std::move(l_msg);
	}

	tuple_type DeserializeArgs(const CMessage &a_msg) const
	{
		tuple_type l_tuple;

		arg_unpacker<tuple_type, sizeof...(Args)>::unpack(a_msg, l_tuple);

		return std::move(l_tuple);
	}

	template<typename Handler>
	CMessage::Ptr Invoke(const CMessage &a_msg, Handler &&a_handler) const
	{
		return fn_invoker<Ret (Args...)>::invoke(this, std::forward<Handler>(a_handler), a_msg);
	}

	template<typename TRet>
	void DeserializeRet(const CMessage &a_msg, TRet &a_val) const
	{
		a_val = a_msg.GetField<TRet>("Ret");
	}


private:
	// Base case for variadic serialize function
	void p_Serialize(CMessage &, const s::CSerializationContext &, size_t) const { }

	template<typename T, typename ...Args2>
	void p_Serialize(CMessage &a_msg, const s::CSerializationContext &a_context, size_t a_idx,
			const T &a_val, const Args2 &...a_vals) const
	{
		a_msg.Add(util::ToString(a_idx), s::Serialize(a_val, a_context));

		p_Serialize(a_msg, a_context, a_idx + 1, a_vals...);
	}

	void p_Serialize(CMessage &a_msg, const Args &...a_vals) const
	{
		s::CSerializationContext l_cxt;

		a_msg.SetId(util::ToString(util::make_uuid()));
		a_msg.SetAction(m_action);

		p_Serialize(a_msg, l_cxt, 0, a_vals...);
	}

	template<typename TRet>
	void p_SerializeRet(CMessage &a_msg, TRet &&a_val) const
	{
		a_msg.Add("Ret", s::Serialize(std::forward<TRet>(a_val)));
	}


};

template<typename TupleType>
struct arg_unpacker<TupleType, 0>
{
	static void unpack(const CMessage &l_msg, TupleType &a_tuple)
	{

	}
};

template<typename TupleType, int Ct>
struct arg_unpacker
{
	static void unpack(const CMessage &l_msg, TupleType &a_tuple)
	{
		typedef typename std::remove_reference<decltype(std::get<Ct-1>(a_tuple))>::type value_type;

		std::get<Ct-1>(a_tuple) = l_msg.GetField<value_type>(util::ToString(Ct-1));

		arg_unpacker<TupleType, Ct-1>::unpack(l_msg, a_tuple);
	}
};

template<typename ...Args>
struct fn_invoker<void (Args...)>
{
	template<typename Fn>
	static CMessage::Ptr invoke(const CContract<void (Args...)> *a_contract, Fn &&a_fn, const CMessage &a_msg)
	{
		auto l_tuple = a_contract->DeserializeArgs(a_msg);

		InvokeFunction(std::forward<Fn>(a_fn), l_tuple);

		return a_contract->SerializeVoidRet(a_msg);
	}
};

template<typename Ret, typename ...Args>
struct fn_invoker<Ret (Args...)>
{
	template<typename Fn>
	static CMessage::Ptr invoke(const CContract<Ret (Args...)> *a_contract, Fn &&a_fn, const CMessage &a_msg)
	{
		static_assert(std::is_convertible<Ret, typename util::return_type<Fn>::type>::value,
				"\n\n\nContract and Function return types differ in a non-convertible manner.\n\n\n");

		auto l_tuple = a_contract->DeserializeArgs(a_msg);

		auto l_ret = InvokeFunction(std::forward<Fn>(a_fn), l_tuple);

		return a_contract->SerializeRet(a_msg, l_ret);
	}
};





} }

#endif /* CONTRACT_H_ */

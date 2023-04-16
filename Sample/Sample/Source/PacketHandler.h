#pragma once
#include <iostream>
#include <unordered_map>
#include <functional>

namespace EJ
{
	using PacketType = uint16_t;
	using PacketSize = uint16_t;
	struct GamePacketHeader
	{
		PacketSize m_size = 0;
		PacketType m_type = 0;

		GamePacketHeader() {};

		GamePacketHeader(PacketSize size, PacketType type)
		{
			m_size = size;
			m_type = type;
		}
	};

	template<class T>
	class PacketTypeHandler
	{
	public:
		static uint16_t m_value;
		static void SetValue(uint16_t value)
		{
			m_value = value;
		}

		static uint16_t GetValue()
		{
			return m_value;
		}
	};

	template <typename T>
	uint16_t PacketTypeHandler<T>::m_value = 0;

	class PacketHandler
	{
	private:
		std::unordered_map<uint16_t, std::function<void(void*, void*)>> m_mapPacketHandler; // Session, Packet

	public:
		using CallBackFunction = std::function<void(void*, void*)>;

		template<class T>
		void ResistPacket(uint16_t packetType, CallBackFunction callback)
		{
			T::SetValue(packetType);
			m_mapPacketHandler.insert(std::unordered_map<uint16_t, CallBackFunction>::value_type(packetType, callback));
		}

		template<class T>
		uint16_t GetPacketType()
		{
			return T::GetValue();
		}

		void CallFunction(uint16_t packetIndex, void* value1, void* value2)
		{
			PacketHandler::CallBackFunction& func = m_mapPacketHandler[packetIndex];
			if (func == nullptr)
				return;

			func(value1, value2);
		}
	};

//#define RESIST_PACKET(PacketType, Packet, Callback) \
//	ResistPacket<EJ::PacketTypeHandler<Packet>>((uint16_t)PacketType, std::bind(&Callback, this, std::placeholders::_1, std::placeholders::_2))

#define RESIST_PACKET(PacketType, Packet, ObjectPointer, Callback) \
	ResistPacket<EJ::PacketTypeHandler<Packet>>((uint16_t)PacketType, std::bind(&Callback, ObjectPointer, std::placeholders::_1, std::placeholders::_2))
}
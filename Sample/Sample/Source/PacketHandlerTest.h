#pragma once

#include <Windows.h>
#include <thread>
#include <vector>
#include <chrono>

#include "PacketHandler.h"

using namespace std::chrono;


namespace EJ
{
	class PacketHandlerTest
	{
	public:
		static void Test();

	private:
		enum class EPacketHandlerType
		{
			Sample01 = 1,
			Sample02 = 2,
			Sample03 = 3,
		};

		class PacketProcessSample01
		{
		public:
			void Test(void* p1, void* p2) { std::cout << "Sample 01" << std::endl; }
		};

		class PacketProcessSample02
		{
		public:
			void Test(void* p1, void* p2) { std::cout << "Sample 02" << std::endl; }
		};
		class PacketProcessSample03
		{
		public:
			void Test(void* p1, void* p2) { std::cout << "Sample 03" << std::endl; }
		};
	};

	void PacketHandlerTest::Test()
	{
		PacketHandler handler;

		PacketProcessSample01 case01;
		PacketProcessSample02 case02;
		PacketProcessSample03 case03;

		// 매크로 없을 때의 길이...
		//handler.ResistPacket<PacketTypeHandler<PacketProcessSample01>>((uint16_t)EPacketHandlerType::Sample01, std::bind(&PacketProcessSample01::Test, &case01, std::placeholders::_1, std::placeholders::_2));
		//handler.ResistPacket<PacketTypeHandler<PacketProcessSample02>>((uint16_t)EPacketHandlerType::Sample02, std::bind(&PacketProcessSample02::Test, &case02, std::placeholders::_1, std::placeholders::_2));
		//handler.ResistPacket<PacketTypeHandler<PacketProcessSample03>>((uint16_t)EPacketHandlerType::Sample03, std::bind(&PacketProcessSample03::Test, &case03, std::placeholders::_1, std::placeholders::_2));

		handler.RESIST_PACKET(EPacketHandlerType::Sample01, PacketProcessSample01, &case01, PacketProcessSample01::Test);
		handler.RESIST_PACKET(EPacketHandlerType::Sample02, PacketProcessSample02, &case02, PacketProcessSample02::Test);
		handler.RESIST_PACKET(EPacketHandlerType::Sample03, PacketProcessSample03, &case03, PacketProcessSample03::Test);

		handler.CallFunction((uint16_t)EPacketHandlerType::Sample01, nullptr, nullptr);
		handler.CallFunction((uint16_t)EPacketHandlerType::Sample02, nullptr, nullptr);
		handler.CallFunction((uint16_t)EPacketHandlerType::Sample03, nullptr, nullptr);
		handler.CallFunction(99, nullptr, nullptr);

		return;
	}
}

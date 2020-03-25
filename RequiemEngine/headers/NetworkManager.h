#pragma once
#include <stdint.h>
#include <headers/ThreadManager.h>
#include <enet/enet.h>
#include <deque>

class NetworkManager : public ThreadJob
{
public:
	NetworkManager();
	void CreateAgent(bool server = false, uint32_t client_num = 1);
	void DestroyAgent();
	virtual void Work(ThreadWorker const* this_worker) override;

private:
	ENetHost* p_Agent;
	ENetAddress m_Address;

	std::deque<int> clientIds;

	bool client = false;
	bool inited = false;
};
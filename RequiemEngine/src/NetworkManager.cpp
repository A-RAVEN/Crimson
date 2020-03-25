#include <headers/NetworkManager.h>
#include <headers/Debug.h>

NetworkManager::NetworkManager()
{
	if (enet_initialize() != 0)
	{
		LOG_ERR("ENet Initialize Issue!");
	}
}

void NetworkManager::CreateAgent(bool server, uint32_t client_num)
{
	client = !server;
	std::cout << "Create Agent" << std::endl;
	m_Address.host = ENET_HOST_ANY;
	m_Address.port = 1234;

	p_Agent = enet_host_create(server ? &m_Address : nullptr /* the address to bind the server host to */,
		client_num      /* allow up to 32 clients and/or outgoing connections */,
		2      /* allow up to 2 channels to be used, 0 and 1 */,
		0      /* assume any amount of incoming bandwidth */,
		0      /* assume any amount of outgoing bandwidth */);
	if (p_Agent == nullptr)
	{
		LOG_ERR("ENet Host Initialize Issue!");
	}
}

void NetworkManager::DestroyAgent()
{
	if (p_Agent)
	{
		enet_host_destroy(p_Agent);
		p_Agent = nullptr;
	}
}

void NetworkManager::Work(ThreadWorker const* this_worker)
{
	while (this_worker->Working())
	{
		ENetEvent event;
		if (client && !inited)
		{
			inited = true;
			enet_address_set_host(&m_Address, "localhost");
			//address.port = 1234;
			/* Initiate the connection, allocating the two channels 0 and 1. */
			auto peer = enet_host_connect(p_Agent, &m_Address, 2, 0);
			if (peer == nullptr)
			{
				LOG_ERR("Peer Creation Failed");
			}
			/* Wait up to 5 seconds for the connection attempt to succeed. */
			//if (enet_host_service(p_Agent, &event, 5000) > 0 &&
			//	event.type == ENET_EVENT_TYPE_CONNECT)
			//{
			//	LOG_ERR("Connect Succeed");
			//}
			//else
			//{
			//	/* Either the 5 seconds are up or a disconnect event was */
			//	/* received. Reset the peer in the event the 5 seconds   */
			//	/* had run out without any significant event.            */
			//	enet_peer_reset(peer);
			//	LOG_ERR("Connect Failed");
			//}
		}

		std::cout << "Poll" << std::endl;
		if (enet_host_service(p_Agent, &event, 1000) > 0)
		{
			switch (event.type)
			{
			case ENET_EVENT_TYPE_CONNECT:
				printf("A new client connected from %x:%u.\n",
					event.peer->address.host,
					event.peer->address.port);
				/* Store any relevant client information here. */
				clientIds.push_back(clientIds.size());
				event.peer->data = &clientIds.back();
				break;
			case ENET_EVENT_TYPE_RECEIVE:
				printf("A packet of length %u containing %s was received from %s on channel %u.\n",
					event.packet->dataLength,
					event.packet->data,
					event.peer->data,
					event.channelID);
				/* Clean up the packet now that we're done using it. */
				enet_packet_destroy(event.packet);

				break;

			case ENET_EVENT_TYPE_DISCONNECT:
				printf("%s disconnected.\n", event.peer->data);
				/* Reset the peer's client information. */
				event.peer->data = NULL;
			}
		}
	}
}

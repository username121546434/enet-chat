#include <cstdlib>
#include <iostream>
#include <enet/enet.h>

const std::string server {"127.0.0.1"};

int main(int argc, char **argv) {
    if (enet_initialize() != 0) {
        std::cerr << "An error occurred while initializing Enet!" << std::endl;
        return EXIT_FAILURE;
    }
    atexit(enet_deinitialize);

    ENetHost *client {nullptr};
    client = enet_host_create(NULL, 1, 1, 0, 0);

    if (client == NULL) {
        std::cerr << "An error occured while creating ENetHost" << std::endl;
        return EXIT_FAILURE;
    }

    ENetAddress address;
    ENetEvent event;
    ENetPeer *peer {nullptr};
    
    enet_address_set_host(&address, server.c_str());
    address.port = 8888;

    peer = enet_host_connect(client, &address, 1, 0);
    if (peer == NULL) {
        std::cerr << "Failed to connect to peer" << std::endl;
        return EXIT_FAILURE;
    }

    if (enet_host_service(client, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
        std::cout << "Connection to " << server << ":" << address.port << " success" << std::endl;
    } else {
        enet_peer_reset(peer);
        std::cout << "Connection to " << server << ":" << address.port << " failed" << std::endl;
    }

    // Game loop start...
    while (enet_host_service(client, &event, 1000) > 0) {
        switch (event.type) {
            case ENET_EVENT_TYPE_RECEIVE:
                std::cout << "A packet of length " << event.packet->dataLength
                          << " containing " << event.packet->data << " "
                          << "was received from " << event.peer->address.host << ':' << event.peer->address.port << " "
                          << "from channel " << event.channelID;
                break;
        }
    }
    // Game loop end...
    enet_peer_disconnect(peer, 0);

    while (enet_host_service(client, &event, 3000) > 0) {
        switch (event.type) {
            case ENET_EVENT_TYPE_RECEIVE:
                enet_packet_destroy(event.packet);
                break;
            case ENET_EVENT_TYPE_DISCONNECT:
                std::cout << "Disconnet event received" << std::endl;
                break;
        }
    }

    std::cout << "Disconnected" << std::endl;

    return EXIT_SUCCESS;
}


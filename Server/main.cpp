#include <cstdlib>
#include <iostream>
#include <enet/enet.h>

int main(int argc, char **argv) {
    if (enet_initialize() != 0) {
        std::cerr << "An error occurred while initializing Enet!" << std::endl;
        return EXIT_FAILURE;
    }
    atexit(enet_deinitialize);

    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = 8888;

    ENetEvent event;

    ENetHost *server {nullptr};
    server = enet_host_create(&address, 24, 1, 0, 0);

    if (server == NULL) {
        std::cerr << "An error occured while creating ENetHost" << std::endl;
        return EXIT_FAILURE;
    }

    // GAME LOOP START
    while (true) {
        while (enet_host_service(server, &event, 1000) > 0) {
            switch (event.type) {
                case ENET_EVENT_TYPE_CONNECT:
                    std::cout << "A new client connected from: " << event.peer->address.host << ':' << event.peer->address.port << std::endl;
                    break;
                case ENET_EVENT_TYPE_RECEIVE:
                    std::cout << "A packet of length " << event.packet->dataLength
                            << " containing \"" << event.packet->data << "\" "
                            << "was received from " << event.peer->address.host << ':' << event.peer->address.port << " "
                            << "from channel " << event.channelID << std::endl;
                    break;
                case ENET_EVENT_TYPE_DISCONNECT:
                    std::cout << event.peer->address.host << ':' << event.peer->address.port << " disconnected." << std::endl;
                    break;
            }
        }
    }
    // GAME LOOP END

    enet_host_destroy(server);
    return EXIT_SUCCESS;
}


#include <cstdlib>
#include <functional>
#include <iostream>
#include <enet/enet.h>
#include <string>
#include <thread>

const std::string server {"127.0.0.1"};

void send_packet(ENetPeer *peer, const std::string &data) {
    ENetPacket *packet = enet_packet_create(data.c_str(), data.size(), ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(peer, 0, packet);
}

void *msg_loop(ENetHost *server) {
    std::cout << "msg loop is running" << std::endl;
    while (true) {
        ENetEvent event;
        while (enet_host_service(server, &event, 1000) > 0) {
            switch (event.type) {
                case ENET_EVENT_TYPE_RECEIVE:
                    std::cout << "A packet of length " << event.packet->dataLength
                            << " containing " << event.packet->data << " "
                            << "was received from " << event.peer->address.host << ':' << event.peer->address.port << " "
                            << "from channel " << event.channelID;
                    break;
            }
        }
    }
}

void display_message(const std::string &username, const std::string &msg) {
    std::cout << username << ": " << msg << std::endl;
}

std::string get_message() {
    std::string msg;
    char curr_char {};
    while (curr_char != '\n') {
        curr_char = std::cin.get();
        if (curr_char != '\n')
            msg.push_back(curr_char);
    }
    return msg;
}

int main(int argc, char **argv) {
    std::string username;
    std::cout << "Please enter your username: ";
    std::getline(std::cin, username);

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
    
    enet_address_set_host(&address, server.c_str());
    address.port = 8888;

    
    ENetPeer *peer {enet_host_connect(client, &address, 1, 0)};
    if (peer == NULL || peer == nullptr) {
        std::cerr << "Failed to connect to peer" << std::endl;
        return EXIT_FAILURE;
    }

    if (enet_host_service(client, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
        std::cout << "Connection to " << server << ":" << address.port << " success" << std::endl;
    } else {
        enet_peer_reset(peer);
        std::cout << "Connection to " << server << ":" << address.port << " failed" << std::endl;
    }

    send_packet(peer, "this is a test data");

    // Game loop start...
    
    bool running = true;

    std::thread thread {msg_loop, client};
    thread.detach();

    while (running) {
        std::string msg = get_message();

        send_packet(peer, msg);
        if (msg == "/exit")
            running = false;

    }

    thread.join();

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


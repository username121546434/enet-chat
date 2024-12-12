#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <enet/enet.h>
#include <map>
#include <sec_api/stdio_s.h>
#include <string>

class ClientData {
private:
    int id;
    std::string username;

public:
    ClientData(int id) : id {id}, username {} {}
    void set_username(const std::string &username) {
        this->username = username;
    }

    int get_id() {return id;}
    std::string get_username() {return username;}
};

std::map<int, ClientData*> client_map;

void send_packet(ENetPeer *peer, const std::string &data) {
    ENetPacket *packet = enet_packet_create(data.c_str(), data.size(), ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(peer, 0, packet);
}

void broadcast_packet(ENetHost *server, const std::string &data) {
    ENetPacket *packet = enet_packet_create(data.c_str(), data.size(), ENET_PACKET_FLAG_RELIABLE);
    enet_host_broadcast(server, 0, packet);
}

void parse_data(ENetHost *server, int id, const char *data) {
    std::cout << "Parsing: " << data << std::endl;

    int data_type {};
    sscanf(data, "%d|", &data_type);
    std::cout << "scanf done " << data_type << std::endl;
    switch (data_type) {
        case 1: {
            char msg[80];
            sscanf(data, "%*d|%[^\n]", &msg);
            char send_data[1024] = {'\0'};
            sprintf(send_data, "1|%d|%s", id, msg);
            broadcast_packet(server, send_data);
            break;
        }
        case 2: {
            char username[80];
            sscanf(data, "2|%[^\n]", &username);
            
            char send_data[1024] = {'\0'};
            sprintf(send_data, "2|%d|%s", id, username);
            client_map[id]->set_username(username);

            broadcast_packet(server, send_data);
            std::cout << "SEND: " << send_data << std::endl;
            break;
        }
    }
}

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
    int new_player_id {0};
    while (true) {
        while (enet_host_service(server, &event, 1000) > 0) {
            switch (event.type) {
                case ENET_EVENT_TYPE_CONNECT: {
                    std::cout << "A new client connected from: " << event.peer->address.host << ':' << event.peer->address.port << std::endl;
                    for (auto const &x : client_map) {
                        char send_data[1024] = {'\0'};
                        sprintf(send_data, "2|%d|%s", x.first, x.second->get_username().c_str());
                        broadcast_packet(server, send_data);
                    }
                    new_player_id++;
                    client_map[new_player_id] = new ClientData(new_player_id);
                    event.peer->data = client_map[new_player_id];

                    char data_to_send[126] = {'\0'};
                    sprintf(data_to_send, "3|%d", new_player_id);
                    send_packet(event.peer, data_to_send);
                    break;
                }
                case ENET_EVENT_TYPE_RECEIVE: {
                    std::string data;
                    for (int i {0}; i < event.packet->dataLength; i++)
                        data.push_back(event.packet->data[i]);
                    std::cout << "A packet of length " << event.packet->dataLength
                            << " containing \"" << data << "\" "
                            << "was received from " << event.peer->address.host << ':' << event.peer->address.port << " "
                            << "from channel " << event.channelID << std::endl;
                    parse_data(server, static_cast<ClientData*>(event.peer->data)->get_id(), data.c_str());
                    enet_packet_destroy(event.packet);
                    break;
                }
                case ENET_EVENT_TYPE_DISCONNECT:
                    std::cout << event.peer->address.host << ':' << event.peer->address.port << " disconnected." << std::endl;
                    char disconnect_data[126] = {'\0'};
                    ClientData *cd {static_cast<ClientData*>(event.peer->data)};
                    sprintf(disconnect_data, "4|%d", cd->get_id());
                    broadcast_packet(server, disconnect_data);

                    delete cd;
                    break;
            }
        }
    }
    // GAME LOOP END

    enet_host_destroy(server);
    return EXIT_SUCCESS;
}


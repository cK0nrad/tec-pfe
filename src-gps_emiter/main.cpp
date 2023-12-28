/*
    Simple UDS server that simulate a GPS device
    allow for multiple client
*/

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <thread>
#include <vector>
#include <poll.h>
#include <memory>
#define SOCKET_PATH "/tmp/tec_gps.socket"

void handle_client(int client_fd)
{
    // Loop to continuously send values to this particular client
    while (true)
    {
        double values[2] = {(double)(rand() % 1000) / 10.0, (double)(rand() % 1000) / 10.0}; // Example values

        // Some times send fails (and indeed crash whole main process when connection is closed),
        // so we check if the client is still connected
        struct pollfd fds;
        fds.fd = client_fd;
        fds.events = POLLIN;
        int ret = poll(&fds, 1, 150);
        if (ret > 0)
        {
            std::cout << "Client disconnected or error occurred" << std::endl;
            break;
        }

        // Send the values
        ssize_t sent_bytes = send(client_fd, values, sizeof(values), 0);
        if (sent_bytes != sizeof(values))
        {
            std::cout << "Error sending data" << std::endl;
            break;
        }

        sleep(1);
    }
    close(client_fd);
}

int main()
{
    int server_fd, client_fd;
    struct sockaddr_un address;
    int addrlen = sizeof(address);

    // make socket
    if ((server_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Connectong
    unlink(SOCKET_PATH); // in case it already exists
    address.sun_family = AF_UNIX;
    strncpy(address.sun_path, SOCKET_PATH, sizeof(address.sun_path) - 1);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    std::cout << "Emulator listening on " << SOCKET_PATH << std::endl;
    std::vector<std::shared_ptr<std::thread>> threads;

    while (true)
    {

        if ((client_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
        {
            perror("accept");
            continue; // Continue accepting other clients even if one accept fails
        }

        // Spawn a new thread for each connected client
        threads.push_back(std::make_shared<std::thread>(handle_client, client_fd));
    }

    // this should never be reached since the server is always running
    for (std::shared_ptr<std::thread> t : threads)
    {
        t->join();
    }

    close(server_fd);
    unlink(SOCKET_PATH);
    return 0;
}

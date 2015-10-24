//
// Created by atticus on 10/20/15.
//
#include <cstring>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <thread>

#define HACKTTPPORT "8000"  // the port users will be connecting to
#define BACKLOG 10          // how many pending connections queue will hold

void newSocketFunction(int socket_fd){
    char reply[100], *msg;
    int msglen;

    std::string string = "Welcome to HACKttp: " + std::to_string(socket_fd) + "\nYour message: ";
    //msg = "Welcome to HACKttp " + (char) socket_fd;
    msg = (char *) string.c_str();
    send(socket_fd, msg, strlen(msg), 0);

    std::string response;
    while(1){
        memset(&reply, 0, sizeof reply);
        if(recv(socket_fd, reply, 100, 0) <= 0) break;
        response.append("HackTTP echo: ");
        response.append(reply);
		response.append("\nYour message: ");
        msg = (char *)response.c_str();
        send(socket_fd, msg, strlen(msg), 0);
        //reply[99] = '\0';
        printf("Sent back: %s", response.c_str());
        response.clear();
    }
}

int main(void) {
    struct sockaddr_storage incoming_connection_info;
    socklen_t addr_size;
    struct addrinfo listening_socket_description, *results;
    int listening_socket_fd, new_socket_fd, msglen;
    char *msg, reply[100];

    //TODO: Dodać obsługę błędów

    // first, load up address structs with getaddrinfo():
    memset(&listening_socket_description, 0, sizeof listening_socket_description);
    listening_socket_description.ai_family = AF_UNSPEC;  // use IPv4 or IPv6, whichever
    listening_socket_description.ai_socktype = SOCK_STREAM;
    listening_socket_description.ai_flags = AI_PASSIVE;     // fill in my IP for me
    getaddrinfo(NULL, HACKTTPPORT, &listening_socket_description, &results);

    // make a socket, bind it, and listen on it:
    listening_socket_fd = socket(results->ai_family, results->ai_socktype, results->ai_protocol);
    bind(listening_socket_fd, results->ai_addr, results->ai_addrlen);

    /*
     *  http://www.linuxhowtos.org/manpages/2/listen.htm
     * 	int listen(int sockfd, int backlog);
     * 		Oznacza socket jako pasywny: będzie przyjmował requesty połączeń używając accept()
     * 		sockfd: deskryptor pliku odnoszący się do socketa o typie SOCK_STREAM albo SOCK_SEQPACKET
     * 		backlog: maksymalna kolejka połączeń oczekujących na socket
     */
    listen(listening_socket_fd, BACKLOG);

    /*
    * 	http://www.linuxhowtos.org/manpages/2/accept.htm
    * 	int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
    * 		Bierze pierwszy connection request z kolejki połączeń, tworzy NOWY connected socket, zwraca
    * 		deskryptor pliku odnoszący się do nowoutworzonego socketa. Nie wpływa na socket przekazany jako sockfd.
    * 		sockfd: socket na którym wcześniej wywyołano socket(), bind() i listen()
     *
    */
    addr_size = sizeof incoming_connection_info;
    new_socket_fd = accept(listening_socket_fd, (struct sockaddr *) &incoming_connection_info, &addr_size);
    //incoming_connection_info potrzebne dla sendto i recvfrom

    /*
     * Żeby móco bsługiwać wiele połączeń jednocześnie trzeba będzie chyba zrobić coś takiego:
     * while(1){
     *  listen(listenieng_socket);
     *  new_socket_fd = accept();
     *  new socketThread(new_socket_fd);
     * }
     * Utworzone sockety będą obsługiwane przez nowe wątki (?). Zeby listening_socket mógł przyjąć następne połączenie
     * trzeba na nim wywołać kolejne listen();
     */


    std::thread t1(newSocketFunction, new_socket_fd); //nowy wątek
    t1.join();

    freeaddrinfo(results);
    close(listening_socket_fd);
    close(new_socket_fd);
}

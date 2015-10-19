/*
 * serwer.cpp
 *
 *  Created on: Oct 14, 2015
 *      Author: atticus
 */

#include<netinet/in.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<unistd.h>

int main() {
   int create_socket, new_socket;
   socklen_t addrlen;
   int bufsize = 1024;
   char *buffer = (char*) malloc(bufsize);
   struct sockaddr_in address;
   int counter = 1;
/*
   http://www.linuxhowtos.org/manpages/2/socket.htm
   int socket(int domain, int type, int protocol);
   domain: określa rodzinę protokołów, stała w socket.h, AF_INET -> IPv4
   type: określa rodzaj komunikacji
   protocol: zazwyczaj dla konkretnej kombinacji domain+type istnieje 1 protokół, można podać 0

   Funkcja zwraca deskryptor pliku albo -1 dla błędu. Utworzony socket istnieje, ale nie ma przypisanego adresu.
*/
   if ((create_socket = socket(AF_INET, SOCK_STREAM, 0)) > 0){
      printf("The socket was created\n");
   }


   address.sin_family = AF_INET;
   address.sin_addr.s_addr = INADDR_ANY; //Socket podpięty do tego adresu otrzymuje dane z wszystkich interfejsów.
   address.sin_port = htons(15000);
/*
	http://www.linuxhowtos.org/manpages/2/bind.htm
	int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
		sockfd: deskryptor pliku odpowiadający socketowi

		sockaddr: Struktura zawierająca adres internetowy, opisana w in.h
		http://beej.us/guide/bgnet/output/html/multipage/sockaddr_inman.html
		struct sockaddr_in
		{
  	  	  short   sin_family;  	must be AF_INET
  	  	  u_short sin_port;		musi być przekonwetrowany do Network Byte Order:
  	  	  	  	  	  	  	  	http://www.tutorialspoint.com/unix_sockets/network_byte_orders.htm
  	  	  struct  in_addr sin_addr;
  	  	  char    sin_zero[8];  Not used, must be zero
		};

		addrlen: rozmiar struktury, na którą pokazuje sockaddr

	Sukces: 0, porażka: -1
*/
   if (bind(create_socket, (struct sockaddr *) &address, sizeof(address)) == 0){
      printf("Binding Socket\n");
   }


   while (1) {
/*
 *  http://www.linuxhowtos.org/manpages/2/listen.htm
 * 	int listen(int sockfd, int backlog);
 * 		Oznacza socket jako pasywny: będzie przyjmował requesty połączeń używając accept()
 * 		sockfd: deskryptor pliku odnoszący się do socketa o typie SOCK_STREAM albo SOCK_SEQPACKET
 * 		backlog: maksymalna kolejka połączeń oczekujących na socket
 */
      if (listen(create_socket, 10) < 0) {
         perror("server: listen");
         exit(1);
      }

/*
 * 	http://www.linuxhowtos.org/manpages/2/accept.htm
 * 	int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
 * 		Bierze pierwszy connection request z kolejki połączeń, tworzy NOWY connected socket, zwraca
 * 		deskryptor pliku odnoszący się do nowoutworzonego socketa. Nie wpływa na socket przekazany jako sockfd.
 * 		sockfd: socket na którym wcześniej wywyołano socket(), bind() i listen()
 *
 */

      if ((new_socket = accept(create_socket, (struct sockaddr *) &address, &addrlen)) < 0) {
         perror("server: accept");
         exit(1);
      }

      if (new_socket > 0){
         printf("The Client is connected...\n");
         if(counter != 1) counter++;
      }

      recv(new_socket, buffer, bufsize, 0);
      printf("%s\n", buffer);
      write(new_socket, "hello world\n", 12);
      close(new_socket);
      counter--;
      printf("Socket is closed\n");
      if(counter == 0 ) break;
   }
   close(create_socket);
   printf("All sockets closed\n");
   return 0;
}

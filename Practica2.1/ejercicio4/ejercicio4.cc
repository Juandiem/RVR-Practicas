#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <cstdlib>
#include <iostream>

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		std::cerr << "Debes proveer la dirección. Por ejemplo: 0.0.0.0"
				  << std::endl;
		return EXIT_FAILURE;
	}

	if (argc < 3)
	{
		std::cerr << "Debes proveer el puerto. Por ejemplo: 2222"
				  << std::endl;
		return EXIT_FAILURE;
	}

	struct addrinfo hints;
	struct addrinfo *res;

	// INICIALIZACIÓN SOCKET & BIND //

	memset(&hints, 0, sizeof(struct addrinfo));

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	int rc = getaddrinfo(argv[1], argv[2], &hints, &res);

	if (rc != 0)
	{
		std::cerr << "getaddrinfo: " << gai_strerror(rc) << std::endl;
		return EXIT_FAILURE;
	}

	// res contiene la representación como sockaddr de dirección + puerto

	int sd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

	if (bind(sd, res->ai_addr, res->ai_addrlen) != 0)
	{
		std::cerr << "bind: " << std::endl;
		return EXIT_FAILURE;
	}

	// Liberar la información de la dirección despúes de usar los datos
	freeaddrinfo(res);

	// PUBLICAR EL SERVIDOR (LISTEN) //
	if (listen(sd, 16) == -1)
	{
		std::cerr << "La llamada LISTEN devolvió un error." << std::endl;
		return EXIT_FAILURE;
	}

	// GESTION DE LAS CONEXIONES AL SERVIDOR //
	struct sockaddr client_addr;
	socklen_t client_len = sizeof(struct sockaddr);

	char host[NI_MAXHOST];
	char service[NI_MAXSERV];

	int sd_client = accept(sd, &client_addr, &client_len);

	getnameinfo(&client_addr, client_len, host, NI_MAXHOST, service,
				NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV);

	std::cout << "Conexión desde: " << host << " " << service
			  << std::endl;

	while (true)
	{
		// RECEPCIÓN DEL MENSAJE DEL CLIENTE //
		char buffer[80];

		ssize_t bytes = recv(sd_client, buffer, sizeof(char) * 79, 0);
		if (bytes <= 0)
			break;

		// ECO DEL MENSAJE DEL CLIENTE //
		if (send(sd_client, buffer, bytes, 0) == -1)
			break;
	}

	std::cout << "Conexión terminada." << std::endl;

	// Cierra el socket:
	close(sd);

	return EXIT_SUCCESS;
}
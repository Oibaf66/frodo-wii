#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>

/* From glibc docs */
static int make_socket (uint16_t port)
{
	int sock;
	struct sockaddr_in name;

	/* Create the socket. */
	sock = socket (PF_INET, SOCK_STREAM, 0);
	if (sock < 0)
	{
		perror ("socket");
		exit (EXIT_FAILURE);
	}

	/* Give the socket a name. */
	name.sin_family = AF_INET;
	name.sin_port = htons (port);
	name.sin_addr.s_addr = htonl (INADDR_ANY);
	if (bind (sock, (struct sockaddr *) &name, sizeof (name)) < 0)
	{
		perror ("bind");
		exit (1);
	}

	return sock;
}


NetworkServer::NetworkServer()
{
	this->n_clients = 0;
	this->listen_sock = make_socket(27697);

	FD_ZERO(&this->listen_fds);
	FD_SET(this->listen_sock, &this->listen_fds);

	if (listen(this->listen_sock, 4) < 0)
	{
		perror("listen");
		exit(1);
	}
}

NetworkClient *NetworkServer::CheckNewConnection()
{
	struct timeval tv;
	struct sockaddr_in client_name;
	size_t size;
	int client_sock;

	/* If something connects, create a new client */
	memset(&tv, 0, sizeof(tv));
	if (select(1, &this->listen_fds, NULL, NULL, &tv) <= 0)
		return NULL;

	client_sock = accept(this->listen_sock, (struct sockaddr*)&client_name, &size);
	if (client_sock < 0)
	{
		fprintf(stderr, "Accepting client failed\n");
		return NULL;
	}

	return new NetworkClient(client_sock);
}


NetworkClient::NetworkClient(int sock)
{
	this->sock = sock;

	/* Assume black screen */
	memset(this->screen, 0, DISPLAY_X * DISPLAY_Y);
}

bool Network::ReceiveUpdate(NetworkUpdate *dst, int sock, struct timeval *tv)
{
	fd_set fds;
	int sz;
	int v;

	FD_ZERO(&fds);
	FD_SET(sock, &fds);

	v = select(1, &fds, NULL, NULL, tv);
	if (v < 0)
	{
		fprintf(stderr, "Select failed\n");
		return false;
	}
	sz = read(sock, (void*)dst, NETWORK_UPDATE_SIZE);

	if (sz < 0)
		return false;

	/* Byte swap stuff */
	this->DeMarshalData(dst);

	return true;
}

bool Network::SendUpdate(NetworkUpdate *src, int sock)
{
	int sz = src->size;

	this->MarshalData(src);
	sz = write(sock, (void*)src, sz);
	if (sz < src->size)
		return false;
	return true;
}

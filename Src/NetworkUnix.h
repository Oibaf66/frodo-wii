#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

static int set_sock_opts(int sock)
{
	struct timeval tv;
	int d = 1;

	memset(&tv, 0, sizeof(tv));
	tv.tv_sec = 2;
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,
			&tv, sizeof(struct timeval));
	setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO,
			&tv, sizeof(struct timeval));
	setsockopt(sock,SOL_SOCKET,SO_REUSEADDR, &d, sizeof(int));
}

/* From glibc docs */
static int make_socket (uint16_t port)
{
	struct sockaddr_in name;
	int sock;

	/* Create the socket. */
	sock = socket (PF_INET, SOCK_STREAM, 0);
	if (sock < 0)
	{
		perror ("socket");
		exit (EXIT_FAILURE);
	}

	set_sock_opts(sock);

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

bool init_sockaddr (struct sockaddr_in *name,
		const char *hostname, uint16_t port)
{
	struct hostent *hostinfo;

	name->sin_family = AF_INET;
	name->sin_port = htons (port);
	hostinfo = gethostbyname (hostname);
	if (hostinfo == NULL)
	{
		fprintf (stderr, "Unknown host %s.\n", hostname);
		return false;
	}
	name->sin_addr = *(struct in_addr *) hostinfo->h_addr;

	return true;
}


NetworkServer::NetworkServer(int port) : Network()
{
	this->n_clients = 0;
	this->listen_sock = make_socket(port);

	if (listen(this->listen_sock, MAX_NETWORK_CLIENTS) < 0)
	{
		perror("listen");
		exit(1);
	}
}

bool NetworkServer::CheckNewConnection()
{
	struct timeval tv;
	struct sockaddr_in client_name;
	size_t size;
	int client_sock;
	fd_set listen_fds;

	/* No more than that thanks... */
	if (this->n_clients >= MAX_NETWORK_CLIENTS)
		return false;

	FD_ZERO(&listen_fds);
	FD_SET(this->listen_sock, &listen_fds);

	/* If something connects, create a new client */
	memset(&tv, 0, sizeof(tv));
	int v = select(this->listen_sock + 1, &listen_fds, NULL, NULL, &tv);

	if ( v < 0)
	{
		perror("select");
		exit(1);
	}
	else if ( v == 0 )
		return false;

	size = sizeof(client_name);
	client_sock = accept(this->listen_sock, (struct sockaddr*)&client_name, &size);
	if (client_sock < 0)
	{
		fprintf(stderr, "Accepting client failed\n");
		return false;
	}

	/* And add the new one! */
	this->AddClient(client_sock);

	return true;
}

NetworkClient::NetworkClient(const char *hostname, int port)
{
	/* Again from glibc docs */
	struct sockaddr_in servername;

	this->Init();

	/* Create the socket. */
	this->sock = socket (PF_INET, SOCK_STREAM, 0);
	if (this->sock < 0)
	{
		perror ("socket (client)");
		return;
	}

	set_sock_opts(sock);

	/* Connect to the server. */
	init_sockaddr (&servername, hostname, port);
	if (connect(sock, (struct sockaddr *) &servername,
			sizeof (servername)) != 0)
	{
		perror ("connect (client)");
		return;
	}
}

bool Network::ReceiveData(void *dst, int sock, size_t sz)
{
	size_t received_sz = 0;

	while (received_sz < sz)
	{
		int v = read(sock, dst, sz);

		if (v < 0)
			return false;
		received_sz += v; 
	}
	this->traffic += received_sz;

	return sz > 0;
}

bool Network::SendData(void *src, int sock, size_t sz)
{
	size_t sent_sz = 0;
	
	while (sent_sz < sz)
	{
		int v = write(sock, (void*)src, sz);

		if (v < 0)
			return false;
		sent_sz += v;
	}

	return true;
}

bool Network::Select(int sock, struct timeval *tv)
{
	fd_set fds;
	int v;

	FD_ZERO(&fds);
	FD_SET(sock, &fds);

	v = select(sock + 1, &fds, NULL, NULL, tv);
	if (v < 0)
	{
			fprintf(stderr, "Select failed\n");
			return false;
	}

	/* v is 0 if the sock is not ready */
	return v > 0;
}

void Network::CloseSocket(int sock)
{
	close(sock);
}

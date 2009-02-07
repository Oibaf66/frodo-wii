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


bool Network::StartListener(int port)
{
	Network::listen_sock = make_socket(port);

	if (Network::listen_sock < 0)
		return false;
	if (listen(Network::listen_sock, MAX_NETWORK_PEERS) < 0)
	{
		perror("listen");
		return false;
	}

	return true;
}

bool Network::CheckNewConnection()
{
	struct timeval tv;
	struct sockaddr_in peer_name;
	size_t size;
	int peer_sock;
	fd_set listen_fds;
	Network *peer;

	/* Not initialized yet */
	if (Network::listen_sock <= 0)
		return false;

	/* No more than that thanks... */
	if (Network::n_peers >= MAX_NETWORK_PEERS)
		return false;

	FD_ZERO(&listen_fds);
	FD_SET(Network::listen_sock, &listen_fds);

	/* If something connects, create a new client */
	memset(&tv, 0, sizeof(tv));
	int v = select(Network::listen_sock + 1, &listen_fds, NULL, NULL, &tv);

	if ( v < 0)
	{
		perror("select");
		exit(1);
	}
	else if ( v == 0 )
		return false;

	size = sizeof(peer_name);
	peer_sock = accept(Network::listen_sock, (struct sockaddr*)&peer_name, &size);
	if (peer_sock < 0)
	{
		fprintf(stderr, "Accepting peer failed\n");
		return false;
	}

	/* And add the new one! */
	Network::AddPeer(new Network(peer_sock, true));

	return true;
}

bool Network::ConnectTo(const char *hostname, int port)
{
	/* Again from glibc docs */
	struct sockaddr_in servername;
	int sock;

	/* Create the socket. */
	sock = socket (PF_INET, SOCK_STREAM, 0);
	if (sock < 0)
	{
		perror ("socket (client)");
		return false;
	}

	set_sock_opts(sock);

	/* Connect to the server. */
	init_sockaddr (&servername, hostname, port);
	if (connect(sock, (struct sockaddr *) &servername,
			sizeof (servername)) != 0)
	{
		perror ("connect (client)");
		return false;
	}

	Network::AddPeer( new Network(sock, false) );

	return true;
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

ssize_t Network::ReceiveFrom(void *dst, int sock, size_t sz,
		struct sockaddr_in *from)
{
	socklen_t from_sz = from ? sizeof(struct sockaddr_in) : 0;

	return recvfrom(sock, dst, sz, 0, (struct sockaddr*)from, &from_sz);
}

ssize_t Network::SendTo(void *src, int sock, size_t sz, struct sockaddr_in *to)
{
	socklen_t to_sz = sizeof(struct sockaddr_in);

	assert(to);
	return sendto(sock, src, sz, 0, (struct sockaddr*)to, to_sz);
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

void Network::CloseSocket()
{
	close(this->sock);
}

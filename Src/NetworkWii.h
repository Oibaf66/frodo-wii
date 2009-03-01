#include <stdio.h>
#include <stdlib.h>
#include <network.h>

static int set_sock_opts(int sock)
{
	struct timeval tv;
	int d = 1;

	memset(&tv, 0, sizeof(tv));
	tv.tv_sec = 2;
	net_setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,
			&tv, sizeof(struct timeval));
	net_setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO,
			&tv, sizeof(struct timeval));
	return net_setsockopt(sock,SOL_SOCKET,SO_REUSEADDR, &d, sizeof(int));
}

/* From glibc docs */
static int make_socket (uint16_t port)
{
	struct sockaddr_in name;
	int sock;

	/* Create the socket. */
	sock = net_socket (PF_INET, SOCK_STREAM, 0);
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
	if (net_bind (sock, (struct sockaddr *) &name, sizeof (name)) < 0)
	{
		perror ("bind");
		exit (1);
	}

	return sock;
}

bool Network::InitSockaddr (struct sockaddr_in *name,
		const char *hostname, uint16_t port)
{
	struct hostent *hostinfo;

	name->sin_family = AF_INET;
	name->sin_port = htons (port);
	hostinfo = net_gethostbyname ((char*)hostname);
	if (hostinfo == NULL)
	{
		fprintf (stderr, "Unknown host %s.\n", hostname);
		return false;
	}
#warning this need to be fixed
	//name->sin_addr = *(struct in_addr *) hostinfo->h_addr;

	return true;
}

bool Network::InitSocket(const char *remote_host, int port)
{
	/* Create the socket. */
	this->sock = net_socket (PF_INET, SOCK_DGRAM, 0);
	if (this->sock < 0)
	{
		perror ("socket (client)");
		return false;
	}

	set_sock_opts(this->sock);

	/* Connect to the server. */
	this->InitSockaddr(&this->connection_addr, remote_host, port);

	return true;
}

bool Network::ReceiveData(void *dst, int sock, size_t sz)
{
	size_t received_sz = 0;

	while (received_sz < sz)
	{
		int v = net_read(sock, dst, sz);

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

	return net_recvfrom(sock, dst, sz, 0, (struct sockaddr*)from, &from_sz);
}

ssize_t Network::SendTo(void *src, int sock, size_t sz, struct sockaddr_in *to)
{
	socklen_t to_sz = sizeof(struct sockaddr_in);

	assert(to);
	return net_sendto(sock, src, sz, 0, (struct sockaddr*)to, to_sz);
}

bool Network::SendData(void *src, int sock, size_t sz)
{
	size_t sent_sz = 0;

	while (sent_sz < sz)
	{
		int v = net_write(sock, (void*)src, sz);

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

	v = net_select(sock + 1, &fds, NULL, NULL, tv);
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
	net_close(this->sock);
}

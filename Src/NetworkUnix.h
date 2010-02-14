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
	return setsockopt(sock,SOL_SOCKET,SO_REUSEADDR, &d, sizeof(int));
}

bool Network::InitSockaddr (struct sockaddr_in *name,
		const char *hostname, uint16_t port)
{
	struct hostent *hostinfo = gethostbyname (hostname);
	if (hostinfo == NULL)
	{
		fprintf (stderr, "Unknown host %s.\n", hostname);
		return false;
	}

	name->sin_family = AF_INET;
	name->sin_port = htons (port);
	name->sin_addr = *(struct in_addr *) hostinfo->h_addr;

	return true;
}

bool Network::InitSocket()
{
	/* Create the socket. */
	this->sock = socket (PF_INET, SOCK_DGRAM, 0);
	if (this->sock < 0)
	{
		perror ("socket (client)");
		return false;
	}

	set_sock_opts(this->sock);

	return true;
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

void Network::InitNetwork()
{
	/* Do nothing */
}

void Network::ShutdownNetwork()
{
}

#include <stdio.h>
#include <stdlib.h>
#include <network.h>

static int set_sock_opts(int sock)
{
	int d = 1;

	return net_setsockopt(sock,SOL_SOCKET,SO_REUSEADDR, &d, sizeof(int));
}

bool Network::InitSockaddr (struct sockaddr_in *name,
		const char *hostname, uint16_t port)
{
	struct hostent *hostinfo = net_gethostbyname ((char*)hostname);

	if (hostinfo == NULL)
	{
		fprintf (stderr, "Unknown host %s.\n", hostname);
		return false;
	}

	memset(name, 0, sizeof(name));
	name->sin_family = AF_INET;
	name->sin_port = htons (port);
	name->sin_len = 8; // sizeof(struct sockaddr_in);
	name->sin_addr.s_addr = *(uint32*)hostinfo->h_addr_list[0];

	return true;
}

bool Network::InitSocket()
{
	/* Create the socket. */
	this->sock = net_socket (PF_INET, SOCK_DGRAM, 0);
	if (this->sock < 0)
	{
		fprintf (stderr, "Could not init socket. Failed with %d\n", this->sock);
		sleep(5);
		return false;
	}
	int _true = false;
	if (net_ioctl (this->sock, FIONBIO, (char *)(&_true)) < 0)
		fprintf(stderr, "Could not set FIONBIO\n");

	set_sock_opts(this->sock);

	return true;
}

ssize_t Network::ReceiveFrom(void *dst, int sock, size_t sz,
		struct sockaddr_in *from)
{
	return net_recv(sock, dst, sz, 0);
}

ssize_t Network::SendTo(void *src, int sock, size_t sz, struct sockaddr_in *to)
{
	socklen_t to_sz = to->sin_len;

	assert(to);
	return net_sendto(sock, src, sz, 0, (struct sockaddr*)to, to_sz);
}

bool Network::Select(int sock, struct timeval *tv)
{
	struct pollsd sds;
	int v;

	sds.socket = sock;
	sds.events = POLLIN;
	sds.revents = 0;

	if (tv)
		v = net_poll(&sds, 1, tv->tv_sec * 1000 + tv->tv_usec / 1000);
	else
		v = net_poll(&sds, 1, 0);

	return v > 0; 
}

void Network::CloseSocket()
{
#define SHUT_RDWR 2
	net_shutdown(this->sock, SHUT_RDWR);
	net_close(this->sock);
}

void Network::InitNetwork()
{
        char myIP[16];

        memset(myIP, 0, sizeof(myIP));
	Gui::gui->status_bar->queueMessage("Getting IP address via DHCP...");

	if (if_config(myIP, NULL, NULL, true) < 0) {
	        	Gui::gui->status_bar->queueMessage("No DHCP reply");
	        	return;
        }
	Gui::gui->status_bar->queueMessage("Got an address");
}

void Network::ShutdownNetwork()
{
}

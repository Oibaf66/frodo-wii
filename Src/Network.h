#ifndef NETWORK_H
#define NETWORK_H

#if defined(GEKKO)
# include <network.h>
#else
 #include <arpa/inet.h>
#endif
#include <SDL.h>

#include "SID.h"

#define FRODO_NETWORK_PROTOCOL_VERSION 2

#define FRODO_NETWORK_MAGIC 0x1976

#define NETWORK_UPDATE_SIZE     (256 * 1024)
#define NETWORK_SOUND_BUF_SIZE   4096

typedef enum
{
	/* Connection-related messages */
	CONNECT_TO_BROKER  = 99, /* Hello, broker */
	LIST_PEERS         = 98, /* List of peers */
	CONNECT_TO_PEER    = 97, /* A peer wants to connect */
	DISCONNECT         = 96, /* Disconnect from a peer */
	SELECT_PEER        = 93, /* (client) Select who to connect to */
	PING               = 95, /* (broker) are you alive? */
	ACK                = 94, /* Answer to broker */
	/* Non-data messages */
	STOP               = 55, /* End of this update sequence */
	/* Data transfer of various kinds */
	DISPLAY_UPDATE_RAW = 1,
	DISPLAY_UPDATE_RLE = 2,
	DISPLAY_UPDATE_DIFF= 3,
	SOUND_UPDATE_RAW   = 4,
	SOUND_UPDATE_RLE   = 5,
	KEYBOARD_UPDATE    = 6,
	JOYSTICK_UPDATE    = 7,
	ENTER_MENU         = 8,
} network_message_type_t;

typedef enum
{
	CONN_CONNECTED,
	CONN_CONNECT_TO_BROKER,
	CONN_WAIT_FOR_PEER_ADDRESS,
	CONN_CONNECT_TO_PEER,
	CONN_SELECT_PEER,
	CONN_WAIT_FOR_PEER_REPLY,

	/* Client-only */
	CONN_WAIT_FOR_PEER_LIST,

	FAILED,
} network_connection_state_t;

typedef enum
{
	OK = 0,
	AGAIN_ERROR,
	VERSION_ERROR,
	SERVER_GARBAGE_ERROR,
	NO_PEERS_ERROR,
} network_connection_error_t;

struct NetworkUpdate
{
	uint16 magic;  /* Should be 0x1976 */
	uint16 type;   /* See above */
	uint32 size;

	/* The rest is just data of some type */
	uint8 data[];
};

struct NetworkUpdateDisplay
{
	uint8 square;
	uint8 data[];
};

struct NetworkUpdateJoystick
{
	uint8 val;
};

struct NetworkUpdateSelectPeer
{
	uint32 server_id;
};

struct NetworkUpdatePingAck
{
	uint32 seq;
};

/*
 * Sent by the third-party broker server when someone wants to connect
 * to this machine.
 *
 * See http://www.brynosaurus.com/pub/net/p2pnat/ for how the UDP hole
 * punching actually works.
 */
struct NetworkUpdatePeerInfo
{
	uint16 private_port;
	uint16 public_port;

	/* Encoded as hex numbers in a string - c0a80a02\0 -> 192.168.10.2.
	 * Some more space to fit IPv6 stuff */
	uint8 private_ip[16];
	uint8 public_ip[16];

	uint16 key;          /* Random value to separate same names */
	uint16 is_master;
	uint8 name[32];      /* "SIMON", "LINDA" etc */
	uint32 server_id;    /* Used by the server */
	uint32 version;      /* Version number */
};

struct NetworkUpdateListPeers
{
	uint32 n_peers;
	uint8  your_ip[16];
	uint16 your_port;
	uint8  d[2];         /* Pad to 4 bytes */

	/* Followed by the actual peers */
	NetworkUpdatePeerInfo peers[];
};

static inline NetworkUpdate *InitNetworkUpdate(NetworkUpdate *ud, uint16 type, uint32 size)
{
	ud->magic = FRODO_NETWORK_MAGIC;
	ud->size = size;
	ud->type = type;

	return ud;
}

class Network
{
public:
	Network(const char *remote_host, int port, bool is_master);

	~Network();

	void EncodeSound();

	void EncodeDisplay(Uint8 *master, Uint8 *remote);

	void EncodeJoystickUpdate(Uint8 v);


	bool DecodeUpdate(uint8 *screen, uint8 *js, MOS6581 *dst);

	void ResetNetworkUpdate(void);

	void DrawTransferredBlocks(SDL_Surface *screen);

	size_t GetKbps() {
		return this->kbps;
	}

	bool ThrottleTraffic() {
		return this->kbps > this->target_kbps;
	}

	void ResetBytesSent() {
		this->traffic = 0;
	}

	void Tick(int ms);

	void CloseSocket();

	bool SendUpdate();

	bool ReceiveUpdate();

	bool ReceiveUpdate(struct timeval *tv);

	static bool StartNetworkServer(int port);

	static bool CheckNewConnection();

	Uint8 *GetScreen()
	{
		return this->screen;
	}

	bool Connect();

	network_connection_error_t ConnectFSM();

	/**
	 * Disconnect from the other end. You should delete the object
	 * after having done this.
	 */
	void Disconnect();

	static void PushSound(uint8 vol);

	static bool is_master; /* Some peers are more equal than others */
protected:
	void InitNetwork();

	void ShutdownNetwork();

	size_t DecodeSoundRLE(struct NetworkUpdate *src, MOS6581 *dst);

	size_t DecodeSoundUpdate(struct NetworkUpdate *src, MOS6581 *dst);

	size_t EncodeSoundRLE(struct NetworkUpdate *dst,
			Uint8 *buffer, size_t len);
	size_t EncodeSoundRaw(struct NetworkUpdate *dst,
			Uint8 *buffer, size_t len);

	size_t GetSoundBufferSize();

	/** Encode part of a screen into @a dst in a single sweep
	 * 
	 * @param dst the destination update structure
	 * @param screen the screen to encode
	 * @param remote the current remote screen
	 * @param square the square index of the screen to encode
	 *
	 * @return the size of the encoded message
	 */
	size_t EncodeDisplaySquare(struct NetworkUpdate *dst,
			Uint8 *screen, Uint8 *remote, int square,
			bool use_diff = true);

	/**
	 * Encode the @a buf sound buffer into @a dst
	 *
	 * @param dst the destination update structure
	 * @param buf the buffer to encode
	 * @param len the length of the in-buffer
	 *
	 * @return the size of the encoded message
	 */
	size_t EncodeSoundBuffer(struct NetworkUpdate *dst,
			Uint8 *buf, size_t len);

	/**
	 * Decode a display update message onto @a screen
	 *
	 * @param screen the screen to draw to
	 * @param src the message to decode
	 */
	bool DecodeDisplayUpdate(Uint8 *screen, struct NetworkUpdate *src);

	void AddNetworkUpdate(struct NetworkUpdate *update);

	size_t GetNetworkUpdateSize(void)
	{
		return (Uint8*)this->cur_ud - (Uint8*)this->ud;
	}

	/**
	 * Compare two display squares.
	 *
	 * @param a the first square (first byte)
	 * @param b the second square (first byte)
	 *
	 * @return true if they are equal
	 */
	bool CompareSquare(Uint8 *a, Uint8 *b);

	bool DecodeDisplayDiff(Uint8 *screen, struct NetworkUpdate *src,
			int x, int y);
	bool DecodeDisplayRLE(Uint8 *screen, struct NetworkUpdate *src,
			int x, int y);
	bool DecodeDisplayRaw(Uint8 *screen, struct NetworkUpdate *src,
			int x, int y);

	void SendPingAck(int seq);

	bool ReceiveUpdate(NetworkUpdate *dst, size_t sz, struct timeval *tv);

	bool ReceiveData(void *dst, int sock, size_t sz);

	bool InitSocket(const char *remote_host, int port);

	/* Simple wrapper around our friend recvfrom */
	ssize_t ReceiveFrom(void *dst, int sock, size_t sz,
			struct sockaddr_in *from);

	ssize_t SendTo(void *src, int sock, size_t sz,
			struct sockaddr_in *to);

	bool SendData(void *src, int sock, size_t sz);

	virtual bool Select(int sock, struct timeval *tv);

	bool IpToStr(char *dst, uint8 *ip);

	bool InitSockaddr (struct sockaddr_in *name,
			const char *hostname, uint16_t port);

	bool MarshalData(NetworkUpdate *ud);

	bool MarshalAllData(NetworkUpdate *p);

	bool DeMarshalAllData(NetworkUpdate *ud, size_t max_size,
			bool *has_stop);

	bool DeMarshalData(NetworkUpdate *ud);

	bool ConnectToBroker();

	bool ConnectToPeer();

	bool WaitForPeerReply();

	network_connection_error_t WaitForPeerList();

	network_connection_error_t WaitForPeerAddress();

	bool SelectPeer(uint32 id);

	size_t FillNetworkBuffer(NetworkUpdate *p);

	NetworkUpdate *GetNext(NetworkUpdate *p)
	{
		return (NetworkUpdate*)((Uint8*)p + p->size);
	}
	
	NetworkUpdate *ud;
	NetworkUpdate *cur_ud;
	Uint8 *raw_buf;
	Uint8 *rle_buf;
	Uint8 *diff_buf;
	Uint8 *sound_buf;
	Uint32 *square_updated;

	size_t traffic, last_traffic;
	int time_since_last_reset;
	int target_kbps;
	int kbps;

	/* The current square to refresh */
	int refresh_square;

	Uint8 *screen;
	int joystick_port;
	bool connected;
	Uint8 cur_joystick_data;

	/* Connection to the peer */
	int sock;
	struct sockaddr_in connection_addr;

	const char *connection_error_message;

	network_connection_state_t network_connection_state;

	/* Sound */
	static uint8 sample_buf[NETWORK_SOUND_BUF_SIZE];
	static int sample_head;
	static int sample_tail;
};

#endif /* NETWORK_H */

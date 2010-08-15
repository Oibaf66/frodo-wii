#ifndef NETWORK_H
#define NETWORK_H

#if defined(GEKKO)
# include <network.h>
#else
 #include <arpa/inet.h>
#endif
#include <SDL.h>

#include "SID.h"
#include "Display.h"

#define FRODO_NETWORK_PROTOCOL_VERSION 4

#define FRODO_NETWORK_MAGIC 0x1976

#define NETWORK_UPDATE_SIZE     (128 * 1024)
#define NETWORK_SOUND_BUF_SIZE   8192

#define SCREENSHOT_FACTOR 4
#define SCREENSHOT_X (DISPLAY_X / SCREENSHOT_FACTOR)
#define SCREENSHOT_Y (DISPLAY_Y / SCREENSHOT_FACTOR)

typedef enum
{
	REGION_UNKNOWN = 0,
	REGION_EUROPE = 1,
	REGION_AFRICA = 2,
	REGION_NORTH_AMERICA = 3,
	REGION_SOUTH_AMERICA = 4,
	REGION_MIDDLE_EAST = 5,
	REGION_SOUTH_ASIA = 6,
	REGION_EAST_ASIA = 7,
	REGION_OCEANIA = 8,
	REGION_ANTARTICA = 9,
} network_region_t;

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
	BANDWIDTH_PING     = 92, /* Large packet to calculate bandwidth */
	BANDWIDTH_ACK	   = 91, /* Answer to BANDWIDTH_PING */
	REGISTER_DATA      = 90, /* Register data (screenshots typically) */
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
	TEXT_MESSAGE       = 9,
	SOUND_UPDATE	   = 10,
} network_message_type_t;


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

struct NetworkUpdateServerInfo
{
	uint32 game_info_seq_nr;
	uint32 dummy[3]; /* Future */
};


struct NetworkUpdateDisplay
{
	uint8 square;
	uint8 data[];
};

#define NETWORK_UPDATE_TEXT_MESSAGE_BROADCAST 1
struct NetworkUpdateTextMessage
{
	uint8 flags;  /* Broadcast or not */
	uint8 data[]; /* NULL-terminated string */
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
	uint8 data[]; /* Only used for bandwidth ping/acks */
};

struct NetworkUpdateRegisterData
{
	uint32_t key;
	uint32_t metadata; /* Type etc */
	uint8_t  data[];
};

struct NetworkUpdateGameInfoHash
{
	uint16_t hash;		/* Hash by filename */
	uint16_t seq_nr;
};

struct NetworkUpdateGameInfoHashList
{
	uint16_t n_entries;
	NetworkUpdateGameInfoHash hashes[];
};

struct NetworkUpdateGameInfoList
{
	uint16_t n_entries;
	uint32_t register_keys[];
};

struct NetworkUpdateSoundInfo
{
	uint16 delay_cycles;
	uint8 adr;
	uint8 val;
};

struct NetworkUpdateSound
{
	uint16 n_items;
	uint16 flags;
	NetworkUpdateSoundInfo info[];
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

	uint8  region;		 /* Europe, africa etc */
	uint8  d;		 /* Dummy */
	uint16 avatar;		 /* Hash of the avatar */
	uint32 screenshot_key;	 /* Key number of the screenshot */
};

#define NETWORK_UPDATE_LIST_PEERS_IS_CONNECT 1
struct NetworkUpdateListPeers
{
	uint32 n_peers;
	uint8  your_ip[16];
	uint16 your_port;
	uint8  flags;
	uint8  d;		/* Pad to 4 bytes */

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
	Network(const char *remote_host, int port);

	~Network();

	void EncodeDisplay(Uint8 *master, Uint8 *remote);

	void EncodeJoystickUpdate(Uint8 v);

	void EncodeTextMessage(const char *str, bool broadcast = false);

	void EnqueueSound(uint32 linecnt, uint8 addr, uint8 val);

	void RegisterSidWrite(uint32 linecnt, uint8 addr, uint8 val);

	void FlushSound(void);

	struct NetworkUpdateSoundInfo *DequeueSound();


	bool DecodeUpdate(C64Display *display, uint8 *js, MOS6581 *dst);

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

	bool SendUpdateDirect(struct sockaddr_in *addr, NetworkUpdate *what);

	bool SendUpdate(struct sockaddr_in *addr);

	bool SendPeerUpdate()
	{
		return this->SendUpdate(&this->peer_addr);
	}

	bool SendServerUpdate()
	{
		return this->SendUpdate(&this->server_addr);
	}

	bool ReceiveUpdate();

	bool ReceiveUpdate(struct timeval *tv);

	static bool StartNetworkServer(int port);

	static bool CheckNewConnection();

	Uint8 *GetScreen()
	{
		return this->screen;
	}

	bool SelectPeer(const char *hostname, uint16_t port, uint32_t server_id);

	bool CancelPeerSelection()
	{
		return this->SelectPeer(NULL,0,0);
	}

	network_connection_error_t ConnectFSM();

	/**
	 * Disconnect from the other end. You should delete the object
	 * after having done this.
	 */
	void Disconnect();

	bool is_master; /* Some peers are more equal than others */

	void InitNetwork();

	void ShutdownNetwork();


	bool ConnectToBroker();

	bool ConnectToPeer();

	bool WaitForPeerReply();

	bool SendBandWidthTest();

	network_connection_error_t WaitForBandWidthReply();

	network_connection_error_t WaitForPeerList();

	network_connection_error_t WaitForPeerAddress();

	network_connection_error_t WaitForPeerSelection();

	bool SelectPeer(uint32 id);

protected:
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
	 * Decode a display update message onto @a screen
	 *
	 * @param src the message to decode
	 */
	bool DecodeDisplayUpdate(struct NetworkUpdate *src);

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

	bool DecodeDisplayDiff(struct NetworkUpdate *src,
			int x, int y);
	bool DecodeDisplayRLE(struct NetworkUpdate *src,
			int x, int y);
	bool DecodeDisplayRaw(struct NetworkUpdate *src,
			int x, int y);

	void SendPingAck(struct sockaddr_in *addr, int seq, uint16 type, size_t data_size);

	bool ReceiveUpdate(NetworkUpdate *dst, size_t sz, struct timeval *tv);

	bool ReceiveData(void *dst, int sock, size_t sz);

	bool InitSocket();

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

	bool DeMarshalAllData(NetworkUpdate *ud, size_t max_size);

	bool DeMarshalData(NetworkUpdate *ud);

	bool ScanDataForStop(NetworkUpdate *ud, size_t max_size);

	bool AppendScreenshot(NetworkUpdatePeerInfo *pi);

	size_t FillNetworkBuffer(NetworkUpdate *p);

	NetworkUpdate *GetNext(NetworkUpdate *p)
	{
		return (NetworkUpdate*)((Uint8*)p + p->size);
	}
	
	NetworkUpdate *receive_ud;
	NetworkUpdate *ud;
	NetworkUpdate *cur_ud;
	Uint8 *raw_buf;
	Uint8 *rle_buf;
	Uint8 *diff_buf;
	Uint8 screenshot[SCREENSHOT_X * SCREENSHOT_Y / 2];
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
	int peer_selected;
	int sock;
	struct sockaddr_in peer_addr;
	struct sockaddr_in server_addr;

	const char *connection_error_message;

	uint32 bandwidth_ping_ms;

	NetworkUpdateSoundInfo sound_active[NETWORK_SOUND_BUF_SIZE];
	int sound_head;
	int sound_tail;
	uint32 sound_last_cycles;
	uint32 sound_last_send;

public:
	static bool networking_started;
};

#endif /* NETWORK_H */

#ifndef NETWORK_H
#define NETWORK_H

#if defined(GEKKO)
# include <network.h>
#else
 #include <arpa/inet.h>
#endif
#include <SDL.h>

#define MAX_NETWORK_PEERS 8

#define NETWORK_UPDATE_SIZE     (256 * 1024)
#define NETWORK_SOUND_BUF_SIZE   1024
enum
{
	STOP               = 99,
	DISCONNECT         = 77,
	DISPLAY_UPDATE_RAW = 1,
	DISPLAY_UPDATE_RLE = 2,
	DISPLAY_UPDATE_DIFF= 3,
	SOUND_UPDATE_RAW   = 4,
	SOUND_UPDATE_RLE   = 5,
	KEYBOARD_UPDATE    = 6,
	JOYSTICK_UPDATE    = 7,
	ENTER_MENU         = 8,
	GET_PEER_INFO      = 9,
};

struct NetworkUpdate
{
	Uint16 size;
	Uint8 type;

	union {
		struct {
			Uint8 square;
		} display;
		struct {
			Uint8 val;
		} joystick;
		struct {
			Uint8 val; /* Should be STOP */
		} stop;
	} u;

	/* The rest is just data of some type */
	Uint8 data[];
};


class Network
{
public:
	Network(int sock, bool is_master);

	~Network();

	void EncodeSound();

	void EncodeDisplay(Uint8 *master, Uint8 *remote);

	void EncodeJoystickUpdate(Uint8 v);


	bool DecodeUpdate(uint8 *screen, uint8 *js = NULL);

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

	static bool StartListener(int port);

	static bool CheckNewConnection();

	static bool ConnectTo(const char *hostname, int port);

	bool isConnected()
	{
		return this->sock >= 0;
	}

	Uint8 *GetScreen()
	{
		return this->screen;
	}

	/**
	 * Disconnect from the other end. You should delete the object
	 * after having done this.
	 */
	void Disconnect();

	static void AddPeer(Network *who);

	static void RemovePeer(Network *peer);

	static void PushSound(uint8 vol);

	/* Listener-related */
	static Network *peers[MAX_NETWORK_PEERS];
	static int n_peers;

protected:
	size_t DecodeSoundUpdate(struct NetworkUpdate *src, char *buf);

	size_t EncodeSoundRLE(struct NetworkUpdate *dst,
			Uint8 *buffer, size_t len);
	size_t EncodeSoundRaw(struct NetworkUpdate *dst,
			Uint8 *buffer, size_t len);

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
			Uint8 *screen, Uint8 *remote, int square);

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

	bool ReceiveUpdate(NetworkUpdate *dst, struct timeval *tv);

	bool ReceiveData(void *dst, int sock, size_t sz);

	bool SendData(void *src, int sock, size_t sz);

	virtual bool Select(int sock, struct timeval *tv);

	bool MarshalData(NetworkUpdate *ud);

	bool MarshalAllData(NetworkUpdate *p);

	bool DeMarshalData(NetworkUpdate *ud);

	NetworkUpdate *GetNext(NetworkUpdate *p)
	{
		return (NetworkUpdate*)((Uint8*)p + p->size);
	}
	
	NetworkUpdate *ud;
	NetworkUpdate *tmp_ud;
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

	Uint8 *screen;
	int joystick_port;
	bool is_master; /* Some peers are more equal than others */
	Uint8 cur_joystick_data;

	/* Connection to the peer */
	int sock;

	/* Listener-related */
	static int listen_sock;

	/* Sound */
	static uint8 sample_buf[NETWORK_SOUND_BUF_SIZE];
	static int sample_head;
	static int sample_tail;
};

#endif /* NETWORK_H */

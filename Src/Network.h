#ifndef NETWORK_H
#define NETWORK_H

#include <arpa/inet.h>
#include <SDL.h>

#define NETWORK_UPDATE_SIZE  (256 * 1024)
enum
{
	STOP               = 99,
	DISPLAY_UPDATE_RAW = 1,
	DISPLAY_UPDATE_RLE = 2,
	DISPLAY_UPDATE_DIFF= 3,
	SOUND_UPDATE_RAW   = 4,
	SOUND_UPDATE_RLE   = 5,
	KEYBOARD_UPDATE    = 6,
	JOYSTICK_UPDATE    = 7,
	DISCONNECT         = 8,
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
	Network();

	~Network();

	size_t EncodeDisplay(Uint8 *master, Uint8 *remote);

	bool DecodeUpdate(uint8 *screen);

	void ResetNetworkUpdate(void);

	void DrawTransferredBlocks(SDL_Surface *screen);

	size_t GetBytesSent() {
		return this->bytes_sent;
	}

	void ResetBytesSent() {
		this->bytes_sent = 0;
	}

	bool SendUpdate(int sock);

	bool ReceiveUpdate(int sock);

	bool ReceiveUpdateBlock(int sock);

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

	void EncodeJoystickUpdate(struct NetworkUpdate *dst,
			Uint8 which, Uint8 v);

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

	bool ReceiveUpdate(NetworkUpdate *dst, int sock, struct timeval *tv);

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
	size_t bytes_sent;
	Uint32 *square_updated;
};

class NetworkClient : public Network
{
public:
	NetworkClient(int sock);

	~NetworkClient();

	NetworkClient(const char *hostname, int port);

	bool isConnected()
	{
		return this->sock >= 0;
	}
	
	bool SendUpdate()
	{
		return ((Network*)this)->SendUpdate(this->sock);
	}

	bool ReceiveUpdate()
	{
		return ((Network*)this)->ReceiveUpdate(this->sock);
	}

	bool ReceiveUpdateBlock()
	{
		return ((Network*)this)->ReceiveUpdateBlock(this->sock);
	}

	bool ReceiveData(void *dst, int sock, size_t sz);

	/**
	 * Disconnect from the other end. You should delete the object
	 * after having done this.
	 */
	void Disconnect();

	Uint8 *screen;
	int joystick_port;
private:
	int sock;
};

#define MAX_NETWORK_CLIENTS 8

class NetworkServer : public Network
{
public:
	NetworkServer(int port);

	bool CheckNewConnection();

	NetworkClient *clients[MAX_NETWORK_CLIENTS];
	int n_clients;

	void RemoveClient(NetworkClient *client);

private:
	void AddClient(int sock);

	int listen_sock;
};

#endif /* NETWORK_H */

#ifndef __MOCKS_C64_H__
#define __MOCKS_C64_H__

/* Network connection type */
enum
{
 	NONE,
	CONNECT,
        MASTER,
        CLIENT
};

class C64
{
public:
	C64()
	{
		this->have_a_break = false;
	}

	void Pause()
	{
		this->have_a_break = true;
	}

	void Resume()
	{
		this->have_a_break = false;
	}

	bool IsPaused()
	{
		return this->have_a_break;
	}

	void startFakeKeySequence(const char *what)
	{
		printf("Faking %s\n", what);
	}

	void LoadSnapshot(const char *name)
	{
		printf("Loading savegame %s\n", name);
	}

	void SaveSnapshot(const char *name)
	{
		printf("Saving savegame %s\n", name);
	}

	int network_connection_type;

private:
	bool have_a_break;
};

extern C64 *TheC64;

#endif /*__MOCKS_C64_H__ */

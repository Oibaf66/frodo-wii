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
	int network_connection_type;
};

extern C64 *TheC64;

#endif /*__MOCKS_C64_H__ */

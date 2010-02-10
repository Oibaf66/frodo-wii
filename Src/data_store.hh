#ifndef __DATA_STORE_HH__
#define __DATA_STORE_HH__

#include <stdint.h>

#define DATA_KEY_RANGE 1000

struct ds_data
{
	uint32_t key;
	uint32_t metadata; /* Type etc */
	size_t   sz;
	uint8_t  data[];
};

class DataStore;

class DataStore
{
public:
	DataStore();

	~DataStore();

	/**
	 * Register a new datum from the network.
	 *
	 * @param key The key to register with
	 * @param metadata The metadata
	 * @param data The data to register
	 * @param data_sz The size of the data
	 *
	 * @return the old datum with that key, or NULL
	 */
	struct ds_data *registerNetworkData(uint32_t key, uint32_t metadata,
			void *data, size_t data_sz);

	/**
	 * Get the next key for registered data
	 *
	 * @return a valid key
	 */
	uint32_t getNextKey();

	/**
	 * Unregister a datum.
	 *
	 * @param key The key to unregister
	 *
	 * @return the unregistered datum, or NULL if it didn't exist
	 */
	struct ds_data *unregisterData(uint32_t key);

	struct ds_data *getData(uint32_t key);

	static DataStore *ds;

private:
	struct ds_data *registerData(uint32_t key, struct ds_data *data);

	struct ds_data **registeredData;
	int n_registeredData;

	uint32_t key_counter;
};

#endif /* __DATA_STORE_HH__ */

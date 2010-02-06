#ifndef __DATA_STORE_HH__
#define __DATA_STORE_HH__

#include <stdint.h>

struct ds_data
{
	uint32_t key;
	uint32_t metadata; /* Type etc */
	uint8_t  data[];
};

class DataStore;

class DataStore
{
public:
	DataStore();

	~DataStore();

	/**
	 * Register a new datum.
	 *
	 * @param key The key to register with
	 * @param data The data to register
	 *
	 * @return the old datum with that key, or NULL
	 */
	struct ds_data *registerData(uint32_t key, struct ds_data *data);

	/**
	 * Embed existing data into a data store. The new data is reallocated,
	 * but the old is not freed
	 *
	 * @param data The data to embed.
	 * @param sz the size of the data
	 *
	 * @return the new data store data.
	 */
	struct ds_data *embedData(void *data, size_t sz);

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
	struct ds_data **registeredData;
	int n_registeredData;

	uint32_t key_counter;
};

#endif /* __DATA_STORE_HH__ */

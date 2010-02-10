#include <string.h>

#include <data_store.hh>
#include <utils.hh>

DataStore::DataStore()
{
	this->registeredData = NULL;
	this->n_registeredData = 0;

	this->key_counter = 0;
}

DataStore::~DataStore()
{
	free(this->registeredData);
}

struct ds_data *DataStore::registerNetworkData(uint32_t key, uint32_t metadata,
		void *data, size_t data_sz)
{
	struct ds_data *out;

	out = (struct ds_data *)xmalloc(sizeof(struct ds_data) + data_sz);
	out->key = key;
	out->metadata = metadata;
	out->sz = data_sz;

	memcpy(out->data, data, data_sz);

	return this->registerData(key, out);
}

struct ds_data *DataStore::registerData(uint32_t key,
		struct ds_data *data)
{
	int i;

	/* Replacing an existing entry? */
	for (i = 0; i < this->n_registeredData; i++)
	{
		if (this->registeredData[i] &&
				this->registeredData[i]->key == key)
		{
			struct ds_data *old = this->registeredData[i];

			this->registeredData[i] = data;
			return old;
		}
	}

	for (i = 0; i < this->n_registeredData; i++)
	{
		/* Found free spot? */
		if (this->registeredData[i] == NULL)
			break;
	}

	if (i >= this->n_registeredData)
	{
		this->n_registeredData++;
		this->registeredData = (struct ds_data **)xrealloc(this->registeredData,
				this->n_registeredData * sizeof(void*));
	}
	this->registeredData[i] = data;
	data->key = key;

	return NULL;
}

uint32_t DataStore::getNextKey()
{
	uint32_t out = this->key_counter;

	this->key_counter++;

	if (this->key_counter >= DATA_KEY_RANGE)
		this->key_counter = 0;

	return out;
}

struct ds_data *DataStore::unregisterData(uint32_t key)
{
	for (int i = 0; i < this->n_registeredData; i++)
	{
		if (this->registeredData[i] &&
				this->registeredData[i]->key == key)
		{
			struct ds_data *old = this->registeredData[i];

			this->registeredData[i] = NULL;

			return old;
		}
	}

	return NULL;
}

struct ds_data *DataStore::getData(uint32_t key)
{
	for (int i = 0; i < this->n_registeredData; i++)
	{
		if (this->registeredData[i] &&
				this->registeredData[i]->key == key)
			return this->registeredData[i];
	}

	return NULL;
}

DataStore *DataStore::ds = NULL;

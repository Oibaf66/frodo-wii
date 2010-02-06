#include <string.h>

#include <data_store.hh>
#include <utils.hh>

DataStore::DataStore()
{
	this->registeredData = NULL;
	this->n_registeredData = 0;

	/* Convention: Odd numbers for the clients */
	this->key_counter = 1;
}

DataStore::~DataStore()
{
	free(this->registeredData);
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

	return NULL;
}

struct ds_data *DataStore::embedData(void *data, size_t sz)
{
	struct ds_data *out;

	out = (struct ds_data *)xmalloc(sizeof(struct ds_data) + sz);

	out->key = this->key_counter;
	out->metadata = 0; /* Setup by the embedder */
	memcpy(out->data, data, sz);

	panic_if(this->registerData(out->key, out) != NULL,
			"Registering new data with key %u was non-NULL\n",
			out->key);
	out->key += 2;

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

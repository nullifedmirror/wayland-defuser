#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "libwayland-base.h"

#define WL_MAP_SERVER_SIDE 0
#define WL_MAP_CLIENT_SIDE 1
#define WL_SERVER_ID_START 0xff000000
#define WL_MAP_MAX_OBJECTS 0x00f00000
#define WL_CLOSURE_MAX_ARGS 20

#define map_entry_is_free(entry) ((entry).next & 0x1)
#define map_entry_get_data(entry) ((void *)((entry).next & ~(uintptr_t)0x3))
#define map_entry_get_flags(entry) (((entry).next >> 1) & 0x1)

union map_entry
{
	uintptr_t next;
	void *data;
};

typedef enum wl_iterator_result (*wl_iterator_func_t)(void *element,
													  void *data,
													  uint32_t flags);

static enum wl_iterator_result
for_each_helper(struct wl_array *entries, wl_iterator_func_t func, void *data)
{
	enum wl_iterator_result ret = WL_ITERATOR_CONTINUE;
	union map_entry entry, *start;
	size_t count;

	start = (union map_entry *)entries->data;
	count = entries->size / sizeof(union map_entry);

	for (size_t idx = 0; idx < count; idx++)
	{
		entry = start[idx];
		if (entry.data && !map_entry_is_free(entry))
		{
			ret = func(map_entry_get_data(entry), data, map_entry_get_flags(entry));
			if (ret != WL_ITERATOR_CONTINUE)
				break;
		}
	}

	return ret;
}

void wl_map_init(struct wl_map *map, uint32_t side)
{
	memset(map, 0, sizeof *map);
	map->side = side;
}

void wl_map_release(struct wl_map *map)
{
	wl_array_release(&map->client_entries);
	wl_array_release(&map->server_entries);
}

uint32_t
wl_map_insert_new(struct wl_map *map, uint32_t flags, void *data)
{
	union map_entry *start, *entry;
	struct wl_array *entries;
	uint32_t base;
	uint32_t count;

	if (map->side == WL_MAP_CLIENT_SIDE)
	{
		entries = &map->client_entries;
		base = 0;
	}
	else
	{
		entries = &map->server_entries;
		base = WL_SERVER_ID_START;
	}

	if (map->free_list)
	{
		start = entries->data;
		entry = &start[map->free_list >> 1];
		map->free_list = entry->next;
	}
	else
	{
		entry = wl_array_add(entries, sizeof *entry);
		if (!entry)
			return 0;
		start = entries->data;
	}

	/* wl_array only grows, so if we have too many objects at
	 * this point there's no way to clean up. We could be more
	 * pro-active about trying to avoid this allocation, but
	 * it doesn't really matter because at this point there is
	 * nothing to be done but disconnect the client and delete
	 * the whole array either way.
	 */
	count = entry - start;
	if (count > WL_MAP_MAX_OBJECTS)
	{
		/* entry->data is freshly malloced garbage, so we'd
		 * better make it a NULL so wl_map_for_each doesn't
		 * dereference it later. */
		entry->data = NULL;
		errno = ENOSPC;
		return 0;
	}
	entry->data = data;
	entry->next |= (flags & 0x1) << 1;

	return count + base;
}

int wl_map_insert_at(struct wl_map *map, uint32_t flags, uint32_t i, void *data)
{
	union map_entry *start;
	uint32_t count;
	struct wl_array *entries;

	if (i < WL_SERVER_ID_START)
	{
		entries = &map->client_entries;
	}
	else
	{
		entries = &map->server_entries;
		i -= WL_SERVER_ID_START;
	}

	if (i > WL_MAP_MAX_OBJECTS)
	{
		errno = ENOSPC;
		return -1;
	}

	count = entries->size / sizeof *start;
	if (count < i)
	{
		errno = EINVAL;
		return -1;
	}

	if (count == i)
	{
		if (!wl_array_add(entries, sizeof *start))
			return -1;
	}

	start = entries->data;
	start[i].data = data;
	start[i].next |= (flags & 0x1) << 1;

	return 0;
}

int wl_map_reserve_new(struct wl_map *map, uint32_t i)
{
	union map_entry *start;
	uint32_t count;
	struct wl_array *entries;

	if (i < WL_SERVER_ID_START)
	{
		if (map->side == WL_MAP_CLIENT_SIDE)
		{
			errno = EINVAL;
			return -1;
		}

		entries = &map->client_entries;
	}
	else
	{
		if (map->side == WL_MAP_SERVER_SIDE)
		{
			errno = EINVAL;
			return -1;
		}

		entries = &map->server_entries;
		i -= WL_SERVER_ID_START;
	}

	if (i > WL_MAP_MAX_OBJECTS)
	{
		errno = ENOSPC;
		return -1;
	}

	count = entries->size / sizeof *start;
	if (count < i)
	{
		errno = EINVAL;
		return -1;
	}

	if (count == i)
	{
		if (!wl_array_add(entries, sizeof *start))
			return -1;

		start = entries->data;
		start[i].data = NULL;
	}
	else
	{
		start = entries->data;
		if (start[i].data != NULL)
		{
			errno = EINVAL;
			return -1;
		}
	}

	return 0;
}

void wl_map_remove(struct wl_map *map, uint32_t i)
{
	union map_entry *start;
	struct wl_array *entries;

	if (i < WL_SERVER_ID_START)
	{
		if (map->side == WL_MAP_SERVER_SIDE)
			return;

		entries = &map->client_entries;
	}
	else
	{
		if (map->side == WL_MAP_CLIENT_SIDE)
			return;

		entries = &map->server_entries;
		i -= WL_SERVER_ID_START;
	}

	start = entries->data;
	start[i].next = map->free_list;
	map->free_list = (i << 1) | 1;
}

void *
wl_map_lookup(struct wl_map *map, uint32_t i)
{
	union map_entry *start;
	uint32_t count;
	struct wl_array *entries;

	if (i < WL_SERVER_ID_START)
	{
		entries = &map->client_entries;
	}
	else
	{
		entries = &map->server_entries;
		i -= WL_SERVER_ID_START;
	}

	start = entries->data;
	count = entries->size / sizeof *start;

	if (i < count && !map_entry_is_free(start[i]))
		return map_entry_get_data(start[i]);

	return NULL;
}

uint32_t
wl_map_lookup_flags(struct wl_map *map, uint32_t i)
{
	union map_entry *start;
	uint32_t count;
	struct wl_array *entries;

	if (i < WL_SERVER_ID_START)
	{
		entries = &map->client_entries;
	}
	else
	{
		entries = &map->server_entries;
		i -= WL_SERVER_ID_START;
	}

	start = entries->data;
	count = entries->size / sizeof *start;

	if (i < count && !map_entry_is_free(start[i]))
		return map_entry_get_flags(start[i]);

	return 0;
}

void wl_map_for_each(struct wl_map *map, wl_iterator_func_t func, void *data)
{
	enum wl_iterator_result ret;

	ret = for_each_helper(&map->client_entries, func, data);
	if (ret == WL_ITERATOR_CONTINUE)
		for_each_helper(&map->server_entries, func, data);
}

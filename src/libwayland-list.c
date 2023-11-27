#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "libwayland-client.h"

/* Invalid memory address */
#define WL_ARRAY_POISON_PTR (void *)4

WL_EXPORT void
wl_list_init(struct wl_list *list)
{
	list->prev = list;
	list->next = list;
}

WL_EXPORT void
wl_list_insert(struct wl_list *list, struct wl_list *elm)
{
	elm->prev = list;
	elm->next = list->next;
	list->next = elm;
	elm->next->prev = elm;
}

WL_EXPORT void
wl_list_remove(struct wl_list *elm)
{
	elm->prev->next = elm->next;
	elm->next->prev = elm->prev;
	elm->next = NULL;
	elm->prev = NULL;
}

WL_EXPORT int
wl_list_length(const struct wl_list *list)
{
	struct wl_list *e;
	int count;

	count = 0;
	e = list->next;
	while (e != list)
	{
		e = e->next;
		count++;
	}

	return count;
}

WL_EXPORT int
wl_list_empty(const struct wl_list *list)
{
	return list->next == list;
}

WL_EXPORT void
wl_list_insert_list(struct wl_list *list, struct wl_list *other)
{
	if (wl_list_empty(other))
		return;

	other->next->prev = list;
	other->prev->next = list->next;
	list->next->prev = other->prev;
	list->next = other->next;
}

WL_EXPORT void
wl_array_init(struct wl_array *array)
{
	memset(array, 0, sizeof *array);
}

WL_EXPORT void
wl_array_release(struct wl_array *array)
{
	free(array->data);
	array->data = WL_ARRAY_POISON_PTR;
}

WL_EXPORT void *
wl_array_add(struct wl_array *array, size_t size)
{
	size_t alloc;
	void *data, *p;

	if (array->alloc > 0)
		alloc = array->alloc;
	else
		alloc = 16;

	while (alloc < array->size + size)
		alloc *= 2;

	if (array->alloc < alloc)
	{
		if (array->alloc > 0)
			data = realloc(array->data, alloc);
		else
			data = malloc(alloc);

		if (data == NULL)
			return NULL;
		array->data = data;
		array->alloc = alloc;
	}

	p = (char *)array->data + array->size;
	array->size += size;

	return p;
}

WL_EXPORT int
wl_array_copy(struct wl_array *array, struct wl_array *source)
{
	if (array->size < source->size)
	{
		if (!wl_array_add(array, source->size - array->size))
			return -1;
	}
	else
	{
		array->size = source->size;
	}

	if (source->size > 0)
		memcpy(array->data, source->data, source->size);

	return 0;
}
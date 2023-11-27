/*
 * Copyright © 2012 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>

#include "libwayland-base.h"

struct wl_cursor_theme;
struct wl_cursor_image
{
	uint32_t width;		/* actual width */
	uint32_t height;	/* actual height */
	uint32_t hotspot_x; /* hot spot x (must be inside image) */
	uint32_t hotspot_y; /* hot spot y (must be inside image) */
	uint32_t delay;		/* animation delay to next frame (ms) */
};

struct wl_cursor
{
	unsigned int image_count;
	struct wl_cursor_image **images;
	char *name;
};

#define ARRAY_LENGTH(a) (sizeof(a) / sizeof(a)[0])
struct shm_pool
{
	struct wl_shm_pool *pool;
	int fd;
	unsigned int size;
	unsigned int used;
	char *data;
};
static struct shm_pool *
shm_pool_create(struct wl_shm *shm, int size)
{
	return NULL;
}

static int
shm_pool_resize(struct shm_pool *pool, int size)
{
	return 0;
}

static int
shm_pool_allocate(struct shm_pool *pool, int size)
{
	return -1;
}

static void
shm_pool_destroy(struct shm_pool *pool)
{
	/* NO-OP */
}

struct wl_cursor_theme
{
	unsigned int cursor_count;
	struct wl_cursor **cursors;
	struct wl_shm *shm;
	struct shm_pool *pool;
	char *name;
	int size;
};

struct cursor_image
{
	struct wl_cursor_image image;
	struct wl_cursor_theme *theme;
	struct wl_buffer *buffer;
	int offset; /* data offset of this image in the shm pool */
};
struct cursor
{
	struct wl_cursor cursor;
	uint32_t total_delay; /* length of the animation in ms */
};
/** Get an shm buffer for a cursor image
 *
 * \param image The cursor image
 * \return An shm buffer for the cursor image. The user should not destroy
 * the returned buffer.
 */
WL_EXPORT struct wl_buffer *
wl_cursor_image_get_buffer(struct wl_cursor_image *_img)
{
	return NULL;
}
static void
wl_cursor_image_destroy(struct wl_cursor_image *_img)
{
	/* NO-OP */
}
static void
wl_cursor_destroy(struct wl_cursor *cursor)
{
	/* NO-OP */
}

static struct wl_cursor *
wl_cursor_create_from_data(struct cursor_metadata *metadata,
						   struct wl_cursor_theme *theme)
{
	return NULL;
}

static void
load_default_theme(struct wl_cursor_theme *theme)
{
	/* NO-OP */
}

static struct wl_cursor *
wl_cursor_create_from_xcursor_images(void *images,
									 struct wl_cursor_theme *theme)
{
	return NULL;
}

static void
load_callback(void *images, void *data)
{
	/* NO-OP */
}

/** Load a cursor theme to memory shared with the compositor
 *
 * \param name The name of the cursor theme to load. If %NULL, the default
 * theme will be loaded.
 * \param size Desired size of the cursor images.
 * \param shm The compositor's shm interface.
 *
 * \return An object representing the theme that should be destroyed with
 * wl_cursor_theme_destroy() or %NULL on error. If no theme with the given
 * name exists, a default theme will be loaded.
 */
WL_EXPORT struct wl_cursor_theme *
wl_cursor_theme_load(const char *name, int size, struct wl_shm *shm)
{
	return NULL;
}

/** Destroys a cursor theme object
 *
 * \param theme The cursor theme to be destroyed
 */
WL_EXPORT void
wl_cursor_theme_destroy(struct wl_cursor_theme *theme)
{
	/* NO-OP */
}

/** Get the cursor for a given name from a cursor theme
 *
 * \param theme The cursor theme
 * \param name Name of the desired cursor
 * \return The theme's cursor of the given name or %NULL if there is no
 * such cursor
 */
WL_EXPORT struct wl_cursor *
wl_cursor_theme_get_cursor(struct wl_cursor_theme *theme,
						   const char *name)
{
	return NULL;
}

/** Find the frame for a given elapsed time in a cursor animation
 *  as well as the time left until next cursor change.
 *
 * \param cursor The cursor
 * \param time Elapsed time in ms since the beginning of the animation
 * \param duration pointer to uint32_t to store time left for this image or
 *                 zero if the cursor won't change.
 *
 * \return The index of the image that should be displayed for the
 * given time in the cursor animation.
 */
WL_EXPORT int
wl_cursor_frame_and_duration(struct wl_cursor *_cursor, uint32_t time,
							 uint32_t *duration)
{
	return 0;
}
/** Find the frame for a given elapsed time in a cursor animation
 *
 * \param cursor The cursor
 * \param time Elapsed time in ms since the beginning of the animation
 *
 * \return The index of the image that should be displayed for the
 * given time in the cursor animation.
 */
WL_EXPORT int
wl_cursor_frame(struct wl_cursor *_cursor, uint32_t time)
{
	return wl_cursor_frame_and_duration(_cursor, time, NULL);
}
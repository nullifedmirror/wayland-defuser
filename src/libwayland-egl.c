#include <stdint.h>
#include <stdlib.h>
#include "libwayland-base.h"

struct wl_egl_window;
struct wl_surface;

WL_EXPORT void
wl_egl_window_resize(struct wl_egl_window *egl_window,
					 int width, int height,
					 int dx, int dy)
{
	/* NO-OP */
}

WL_EXPORT struct wl_egl_window *
wl_egl_window_create(struct wl_surface *surface,
					 int width, int height)
{
	return NULL;
}

WL_EXPORT void
wl_egl_window_destroy(struct wl_egl_window *egl_window)
{
	/* NO-OP */
}

WL_EXPORT void
wl_egl_window_get_attached_size(struct wl_egl_window *egl_window,
								int *width, int *height)
{
	/* NO-OP */
}

WL_EXPORT struct wl_egl_pixmap *
wl_egl_pixmap_create(int width, int height, uint32_t flags)
{
	return NULL;
}

WL_EXPORT void
wl_egl_pixmap_destroy(struct wl_egl_pixmap *egl_pixmap)
{
	/* NO-OP */
}

WL_EXPORT struct wl_buffer *
wl_egl_pixmap_create_buffer(struct wl_egl_pixmap *egl_pixmap)
{
	return NULL;
}
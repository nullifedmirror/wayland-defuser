/*
 * Copyright © 2008 Kristian Høgsberg
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

#define _GNU_SOURCE

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <dlfcn.h>
#include <assert.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/eventfd.h>
#include <sys/file.h>
#include <sys/stat.h>

#include "libwayland-server.h"

/* This is the size of the char array in struct sock_addr_un.
 * No Wayland socket can be created with a path longer than this,
 * including the null terminator.
 */
#ifndef UNIX_PATH_MAX
#define UNIX_PATH_MAX 108
#endif

#define LOCK_SUFFIX ".lock"
#define LOCK_SUFFIXLEN 5

struct wl_socket
{
	int fd;
	int fd_lock;
	struct sockaddr_un addr;
	char lock_addr[UNIX_PATH_MAX + LOCK_SUFFIXLEN];
	struct wl_list link;
	struct wl_event_source *source;
	char *display_name;
};

struct wl_client
{
	struct wl_connection *connection;
	struct wl_event_source *source;
	struct wl_display *display;
	struct wl_resource *display_resource;
	struct wl_list link;
	struct wl_map objects;
	struct wl_priv_signal destroy_signal;
	struct wl_priv_signal destroy_late_signal;
	pid_t pid;
	uid_t uid;
	gid_t gid;
	bool error;
	struct wl_priv_signal resource_created_signal;
};

struct wl_display
{
	struct wl_event_loop *loop;
	bool run;

	uint32_t next_global_name;
	uint32_t serial;

	struct wl_list registry_resource_list;
	struct wl_list global_list;
	struct wl_list socket_list;
	struct wl_list client_list;
	struct wl_list protocol_loggers;

	struct wl_priv_signal destroy_signal;
	struct wl_priv_signal create_client_signal;

	struct wl_array additional_shm_formats;

	wl_display_global_filter_func_t global_filter;
	void *global_filter_data;

	int terminate_efd;
	struct wl_event_source *term_source;
};

struct wl_global
{
	struct wl_display *display;
	const struct wl_interface *interface;
	uint32_t name;
	uint32_t version;
	void *data;
	wl_global_bind_func_t bind;
	struct wl_list link;
	bool removed;
};

struct wl_resource
{
	struct wl_object object;
	wl_resource_destroy_func_t destroy;
	struct wl_list link;
	/* Unfortunately some users of libwayland (e.g. mesa) still use the
	 * deprecated wl_resource struct, even if creating it with the new
	 * wl_resource_create(). So we cannot change the layout of the struct
	 * unless after the data field. */
	struct wl_signal deprecated_destroy_signal;
	struct wl_client *client;
	void *data;
	int version;
	wl_dispatcher_func_t dispatcher;
	struct wl_priv_signal destroy_signal;
};

struct wl_protocol_logger
{
	struct wl_list link;
	wl_protocol_logger_func_t func;
	void *user_data;
};

static int debug_server = 0;

static void
log_closure(struct wl_resource *resource,
			struct wl_closure *closure, int send)
{
	/* NO-OP */
}

static bool
verify_objects(struct wl_resource *resource, uint32_t opcode,
			   union wl_argument *args)
{
	return false;
}

static void
handle_array(struct wl_resource *resource, uint32_t opcode,
			 union wl_argument *args,
			 int (*send_func)(struct wl_closure *, struct wl_connection *))
{
	/* NO-OP */
}

WL_EXPORT void
wl_resource_post_event_array(struct wl_resource *resource, uint32_t opcode,
							 union wl_argument *args)
{
	/* NO-OP */
}

WL_EXPORT void
wl_resource_post_event(struct wl_resource *resource, uint32_t opcode, ...)
{
	/* NO-OP */
}

WL_EXPORT void
wl_resource_queue_event_array(struct wl_resource *resource, uint32_t opcode,
							  union wl_argument *args)
{
	/* NO-OP */
}

WL_EXPORT void
wl_resource_queue_event(struct wl_resource *resource, uint32_t opcode, ...)
{
	/* NO-OP */
}

static void
wl_resource_post_error_vargs(struct wl_resource *resource,
							 uint32_t code, const char *msg, va_list argp)
{
	/* NO-OP */
}

WL_EXPORT void
wl_resource_post_error(struct wl_resource *resource,
					   uint32_t code, const char *msg, ...)
{
	/* NO-OP */
}

static void
destroy_client_with_error(struct wl_client *client, const char *reason)
{
	wl_log("%s (pid %u)\n", reason, client->pid);
	wl_client_destroy(client);
}

static int
wl_client_connection_data(int fd, uint32_t mask, void *data)
{
	return 1;
}

/** Flush pending events to the client
 *
 * \param client The client object
 *
 * Events sent to clients are queued in a buffer and written to the
 * socket later - typically when the compositor has handled all
 * requests and goes back to block in the event loop.  This function
 * flushes all queued up events for a client immediately.
 *
 * \memberof wl_client
 */
WL_EXPORT void
wl_client_flush(struct wl_client *client)
{
	/* NO-OP */
}

/** Get the display object for the given client
 *
 * \param client The client object
 * \return The display object the client is associated with.
 *
 * \memberof wl_client
 */
WL_EXPORT struct wl_display *
wl_client_get_display(struct wl_client *client)
{
	return client->display;
}

static int
bind_display(struct wl_client *client, struct wl_display *display);

/** Create a client for the given file descriptor
 *
 * \param display The display object
 * \param fd The file descriptor for the socket to the client
 * \return The new client object or NULL on failure.
 *
 * Given a file descriptor corresponding to one end of a socket, this
 * function will create a wl_client struct and add the new client to
 * the compositors client list.  At that point, the client is
 * initialized and ready to run, as if the client had connected to the
 * servers listening socket.  When the client eventually sends
 * requests to the compositor, the wl_client argument to the request
 * handler will be the wl_client returned from this function.
 *
 * The other end of the socket can be passed to
 * wl_display_connect_to_fd() on the client side or used with the
 * WAYLAND_SOCKET environment variable on the client side.
 *
 * Listeners added with wl_display_add_client_created_listener() will
 * be notified by this function after the client is fully constructed.
 *
 * On failure this function sets errno accordingly and returns NULL.
 *
 * \memberof wl_display
 */
WL_EXPORT struct wl_client *
wl_client_create(struct wl_display *display, int fd)
{
	return NULL;
}

/** Return Unix credentials for the client
 *
 * \param client The display object
 * \param pid Returns the process ID
 * \param uid Returns the user ID
 * \param gid Returns the group ID
 *
 * This function returns the process ID, the user ID and the group ID
 * for the given client.  The credentials come from getsockopt() with
 * SO_PEERCRED, on the client socket fd.  All the pointers can be
 * NULL, if the caller is not interested in a particular ID.
 *
 * Note, process IDs are subject to race conditions and are not a reliable way
 * to identify a client.
 *
 * Be aware that for clients that a compositor forks and execs and
 * then connects using socketpair(), this function will return the
 * credentials for the compositor.  The credentials for the socketpair
 * are set at creation time in the compositor.
 *
 * \memberof wl_client
 */
WL_EXPORT void
wl_client_get_credentials(struct wl_client *client,
						  pid_t *pid, uid_t *uid, gid_t *gid)
{
	/* NO-OP */
}

/** Get the file descriptor for the client
 *
 * \param client The display object
 * \return The file descriptor to use for the connection
 *
 * This function returns the file descriptor for the given client.
 *
 * Be sure to use the file descriptor from the client for inspection only.
 * If the caller does anything to the file descriptor that changes its state,
 * it will likely cause problems.
 *
 * See also wl_client_get_credentials().
 * It is recommended that you evaluate whether wl_client_get_credentials()
 * can be applied to your use case instead of this function.
 *
 * If you would like to distinguish just between the client and the compositor
 * itself from the client's request, it can be done by getting the client
 * credentials and by checking the PID of the client and the compositor's PID.
 * Regarding the case in which the socketpair() is being used, you need to be
 * careful. Please note the documentation for wl_client_get_credentials().
 *
 * This function can be used for a compositor to validate a request from
 * a client if there are additional information provided from the client's
 * file descriptor. For instance, suppose you can get the security contexts
 * from the client's file descriptor. The compositor can validate the client's
 * request with the contexts and make a decision whether it permits or deny it.
 *
 * \memberof wl_client
 */
WL_EXPORT int
wl_client_get_fd(struct wl_client *client)
{
	return wl_connection_get_fd(client->connection);
}

/** Look up an object in the client name space
 *
 * \param client The client object
 * \param id The object id
 * \return The object or NULL if there is not object for the given ID
 *
 * This looks up an object in the client object name space by its
 * object ID.
 *
 * \memberof wl_client
 */
WL_EXPORT struct wl_resource *
wl_client_get_object(struct wl_client *client, uint32_t id)
{
	return wl_map_lookup(&client->objects, id);
}

WL_EXPORT void
wl_client_post_no_memory(struct wl_client *client)
{
	/* NO-OP */
}

/** Report an internal server error
 *
 * \param client The client object
 * \param msg A printf-style format string
 * \param ... Format string arguments
 *
 * Report an unspecified internal implementation error and disconnect
 * the client.
 *
 * \memberof wl_client
 */
WL_EXPORT void
wl_client_post_implementation_error(struct wl_client *client,
									char const *msg, ...)
{
	/* NO-OP */
}

WL_EXPORT void
wl_resource_post_no_memory(struct wl_resource *resource)
{
	/* NO-OP */
}

/** Detect if a wl_resource uses the deprecated public definition.
 *
 * Before Wayland 1.2.0, the definition of struct wl_resource was public.
 * It was made opaque just before 1.2.0, and later new fields were added.
 * The new fields cannot be accessed if a program is using the deprecated
 * definition, as there would not be memory allocated for them.
 *
 * The creation pattern for the deprecated definition was wl_resource_init()
 * followed by wl_client_add_resource(). wl_resource_init() was an inline
 * function and no longer exists, but binaries might still carry it.
 * wl_client_add_resource() still exists for ABI compatibility.
 */
static bool
resource_is_deprecated(struct wl_resource *resource)
{
	struct wl_map *map = &resource->client->objects;
	int id = resource->object.id;

	/* wl_client_add_resource() marks deprecated resources with the flag. */
	if (wl_map_lookup_flags(map, id) & WL_MAP_ENTRY_LEGACY)
		return true;

	return false;
}

static enum wl_iterator_result
destroy_resource(void *element, void *data, uint32_t flags)
{
	struct wl_resource *resource = element;

	wl_signal_emit(&resource->deprecated_destroy_signal, resource);
	/* Don't emit the new signal for deprecated resources, as that would
	 * access memory outside the bounds of the deprecated struct */
	if (!resource_is_deprecated(resource))
		wl_priv_signal_final_emit(&resource->destroy_signal, resource);

	if (resource->destroy)
		resource->destroy(resource);

	if (!(flags & WL_MAP_ENTRY_LEGACY))
		free(resource);

	return WL_ITERATOR_CONTINUE;
}

WL_EXPORT void
wl_resource_destroy(struct wl_resource *resource)
{
	struct wl_client *client = resource->client;
	uint32_t id;
	uint32_t flags;

	id = resource->object.id;
	flags = wl_map_lookup_flags(&client->objects, id);
	destroy_resource(resource, NULL, flags);

	if (id < WL_SERVER_ID_START)
	{
		if (client->display_resource)
		{
			wl_resource_queue_event(client->display_resource,
									WL_DISPLAY_DELETE_ID, id);
		}
		wl_map_insert_at(&client->objects, 0, id, NULL);
	}
	else
	{
		wl_map_remove(&client->objects, id);
	}
}

WL_EXPORT uint32_t
wl_resource_get_id(struct wl_resource *resource)
{
	return resource->object.id;
}

WL_EXPORT struct wl_list *
wl_resource_get_link(struct wl_resource *resource)
{
	return &resource->link;
}

WL_EXPORT struct wl_resource *
wl_resource_from_link(struct wl_list *link)
{
	struct wl_resource *resource;

	return wl_container_of(link, resource, link);
}

WL_EXPORT struct wl_resource *
wl_resource_find_for_client(struct wl_list *list, struct wl_client *client)
{
	return NULL;
}

WL_EXPORT struct wl_client *
wl_resource_get_client(struct wl_resource *resource)
{
	return resource->client;
}

WL_EXPORT void
wl_resource_set_user_data(struct wl_resource *resource, void *data)
{
	resource->data = data;
}

WL_EXPORT void *
wl_resource_get_user_data(struct wl_resource *resource)
{
	return resource->data;
}

WL_EXPORT int
wl_resource_get_version(struct wl_resource *resource)
{
	return resource->version;
}

WL_EXPORT void
wl_resource_set_destructor(struct wl_resource *resource,
						   wl_resource_destroy_func_t destroy)
{
	resource->destroy = destroy;
}

WL_EXPORT int
wl_resource_instance_of(struct wl_resource *resource,
						const struct wl_interface *interface,
						const void *implementation)
{
	return wl_interface_equal(resource->object.interface, interface) &&
		   resource->object.implementation == implementation;
}

WL_EXPORT void
wl_resource_add_destroy_listener(struct wl_resource *resource,
								 struct wl_listener *listener)
{
	/* NO-OP */
}

WL_EXPORT struct wl_listener *
wl_resource_get_destroy_listener(struct wl_resource *resource,
								 wl_notify_func_t notify)
{
	if (resource_is_deprecated(resource))
		return wl_signal_get(&resource->deprecated_destroy_signal, notify);
	return wl_priv_signal_get(&resource->destroy_signal, notify);
}

/** Retrieve the interface name (class) of a resource object.
 *
 * \param resource The resource object
 *
 * \memberof wl_resource
 */
WL_EXPORT const char *
wl_resource_get_class(struct wl_resource *resource)
{
	return resource->object.interface->name;
}

/**
 * Add a listener to be called at the beginning of wl_client destruction
 *
 * The listener provided will be called when wl_client destroy has begun,
 * before any of that client's resources have been destroyed.
 *
 * There is no requirement to remove the link of the wl_listener when the
 * signal is emitted.
 *
 * \memberof wl_client
 */
WL_EXPORT void
wl_client_add_destroy_listener(struct wl_client *client,
							   struct wl_listener *listener)
{
	/* NO-OP */
}

WL_EXPORT struct wl_listener *
wl_client_get_destroy_listener(struct wl_client *client,
							   wl_notify_func_t notify)
{
	return wl_priv_signal_get(&client->destroy_signal, notify);
}

/**
 * Add a listener to be called at the end of wl_client destruction
 *
 * The listener provided will be called when wl_client destroy is nearly
 * complete, after all of that client's resources have been destroyed.
 *
 * There is no requirement to remove the link of the wl_listener when the
 * signal is emitted.
 *
 * \memberof wl_client
 * \since 1.22.0
 */
WL_EXPORT void
wl_client_add_destroy_late_listener(struct wl_client *client,
									struct wl_listener *listener)
{
	/* NO-OP */
}

WL_EXPORT struct wl_listener *
wl_client_get_destroy_late_listener(struct wl_client *client,
									wl_notify_func_t notify)
{
	return wl_priv_signal_get(&client->destroy_late_signal, notify);
}

WL_EXPORT void
wl_client_destroy(struct wl_client *client)
{
	/* NO-OP */
}

/* Check if a global filter is registered and use it if any.
 *
 * If no wl_global filter has been registered, this function will
 * return true, allowing the wl_global to be visible to the wl_client
 */
static bool
wl_global_is_visible(const struct wl_client *client,
					 const struct wl_global *global)
{
	return false;
}

static void
registry_bind(struct wl_client *client,
			  struct wl_resource *resource, uint32_t name,
			  const char *interface, uint32_t version, uint32_t id)
{
	/* NO-OP */
}

static const struct wl_registry_interface registry_interface = {
	registry_bind};

static void
display_sync(struct wl_client *client,
			 struct wl_resource *resource, uint32_t id)
{
	/* NO-OP */
}

static void
unbind_resource(struct wl_resource *resource)
{
	wl_list_remove(&resource->link);
}

static void
display_get_registry(struct wl_client *client,
					 struct wl_resource *resource, uint32_t id)
{
	/* NO-OP */
}

static const struct wl_display_interface display_interface = {
	display_sync,
	display_get_registry};

static void
destroy_client_display_resource(struct wl_resource *resource)
{
	resource->client->display_resource = NULL;
}

static int
bind_display(struct wl_client *client, struct wl_display *display)
{
	return -1;
}

static int
handle_display_terminate(int fd, uint32_t mask, void *data)
{
	return -1;
}

/** Create Wayland display object.
 *
 * \return The Wayland display object. Null if failed to create
 *
 * This creates the wl_display object.
 *
 * \memberof wl_display
 */
WL_EXPORT struct wl_display *
wl_display_create(void)
{
	return NULL;
}

static void
wl_socket_destroy(struct wl_socket *s)
{
	/* NO-OP */
}

static struct wl_socket *
wl_socket_alloc(void)
{
	return NULL;
}

/** Destroy Wayland display object.
 *
 * \param display The Wayland display object which should be destroyed.
 *
 * This function emits the wl_display destroy signal, releases
 * all the sockets added to this display, free's all the globals associated
 * with this display, free's memory of additional shared memory formats and
 * destroy the display object.
 *
 * \sa wl_display_add_destroy_listener
 *
 * \memberof wl_display
 */
WL_EXPORT void
wl_display_destroy(struct wl_display *display)
{
	/* NO-OP */
}

/** Set a filter function for global objects
 *
 * \param display The Wayland display object.
 * \param filter  The global filter function.
 * \param data User data to be associated with the global filter.
 *
 * Set a filter for the wl_display to advertise or hide global objects
 * to clients.
 * The set filter will be used during wl_global advertisement to
 * determine whether a global object should be advertised to a
 * given client, and during wl_global binding to determine whether
 * a given client should be allowed to bind to a global.
 *
 * Clients that try to bind to a global that was filtered out will
 * have an error raised.
 *
 * Setting the filter NULL will result in all globals being
 * advertised to all clients. The default is no filter.
 *
 * The filter should be installed before any client connects and should always
 * take the same decision given a client and a global. Not doing so will result
 * in inconsistent filtering and broken wl_registry event sequences.
 *
 * \memberof wl_display
 */
WL_EXPORT void
wl_display_set_global_filter(struct wl_display *display,
							 wl_display_global_filter_func_t filter,
							 void *data)
{
	/* NO-OP */
}

WL_EXPORT struct wl_global *
wl_global_create(struct wl_display *display,
				 const struct wl_interface *interface, int version,
				 void *data, wl_global_bind_func_t bind)
{
	return NULL;
}

/** Remove the global
 *
 * \param global The Wayland global.
 *
 * Broadcast a global remove event to all clients without destroying the
 * global. This function can only be called once per global.
 *
 * wl_global_destroy() removes the global and immediately destroys it. On
 * the other end, this function only removes the global, allowing clients
 * that have not yet received the global remove event to continue to bind to
 * it.
 *
 * This can be used by compositors to mitigate clients being disconnected
 * because a global has been added and removed too quickly. Compositors can call
 * wl_global_remove(), then wait an implementation-defined amount of time, then
 * call wl_global_destroy(). Note that the destruction of a global is still
 * racy, since clients have no way to acknowledge that they received the remove
 * event.
 *
 * \since 1.17.90
 */
WL_EXPORT void
wl_global_remove(struct wl_global *global)
{
	/* NO-OP */
}

WL_EXPORT void
wl_global_destroy(struct wl_global *global)
{
	/* NO-OP */
}

WL_EXPORT const struct wl_interface *
wl_global_get_interface(const struct wl_global *global)
{
	return global->interface;
}

/** Get the name of the global.
 *
 * \param global The global object.
 * \param client Client for which to look up the global.
 * \return The name of the global, or 0 if the global is not visible to the
 *         client.
 *
 * \memberof wl_global
 * \since 1.22
 */
WL_EXPORT uint32_t
wl_global_get_name(const struct wl_global *global,
				   const struct wl_client *client)
{
	return wl_global_is_visible(client, global) ? global->name : 0;
}

/** Get the version of the given global.
 *
 * \param global The global object.
 * \return The version advertised by the global.
 *
 * \memberof wl_global
 * \since 1.21
 */
WL_EXPORT uint32_t
wl_global_get_version(const struct wl_global *global)
{
	return global->version;
}

/** Get the display object for the given global
 *
 * \param global The global object
 * \return The display object the global is associated with.
 *
 * \memberof wl_global
 * \since 1.20
 */
WL_EXPORT struct wl_display *
wl_global_get_display(const struct wl_global *global)
{
	return global->display;
}

WL_EXPORT void *
wl_global_get_user_data(const struct wl_global *global)
{
	return global->data;
}

/** Set the global's user data
 *
 * \param global The global object
 * \param data The user data pointer
 *
 * \since 1.17.90
 */
WL_EXPORT void
wl_global_set_user_data(struct wl_global *global, void *data)
{
	/* NO-OP */
}

/** Get the current serial number
 *
 * \param display The display object
 *
 * This function returns the most recent serial number, but does not
 * increment it.
 *
 * \memberof wl_display
 */
WL_EXPORT uint32_t
wl_display_get_serial(struct wl_display *display)
{
	return display->serial;
}

/** Get the next serial number
 *
 * \param display The display object
 *
 * This function increments the display serial number and returns the
 * new value.
 *
 * \memberof wl_display
 */
WL_EXPORT uint32_t
wl_display_next_serial(struct wl_display *display)
{
	display->serial++;

	return display->serial;
}

WL_EXPORT struct wl_event_loop *
wl_display_get_event_loop(struct wl_display *display)
{
	return display->loop;
}

WL_EXPORT void
wl_display_terminate(struct wl_display *display)
{
	/* NO-OP */
}

WL_EXPORT void
wl_display_run(struct wl_display *display)
{
	/* NO-OP */
}

WL_EXPORT void
wl_display_flush_clients(struct wl_display *display)
{
	/* NO-OP */
}

/** Destroy all clients connected to the display
 *
 * \param display The display object
 *
 * This function should be called right before wl_display_destroy() to ensure
 * all client resources are closed properly. Destroying a client from within
 * wl_display_destroy_clients() is safe, but creating one will leak resources
 * and raise a warning.
 *
 * \memberof wl_display
 */
WL_EXPORT void
wl_display_destroy_clients(struct wl_display *display)
{
	/* NO-OP */
}

static int
socket_data(int fd, uint32_t mask, void *data)
{
	return -1;
}

static int
wl_socket_lock(struct wl_socket *socket)
{
	return -1;
}

static int
wl_socket_init_for_display_name(struct wl_socket *s, const char *name)
{
	return -1;
}

static int
_wl_display_add_socket(struct wl_display *display, struct wl_socket *s)
{
	return -1;
}

WL_EXPORT const char *
wl_display_add_socket_auto(struct wl_display *display)
{
	return NULL;
}

/**  Add a socket with an existing fd to Wayland display for the clients to connect.
 *
 * \param display Wayland display to which the socket should be added.
 * \param sock_fd The existing socket file descriptor to be used
 * \return 0 if success. -1 if failed.
 *
 * The existing socket fd must already be created, opened, and locked.
 * The fd must be properly set to CLOEXEC and bound to a socket file
 * with both bind() and listen() already called.
 *
 * \memberof wl_display
 */
WL_EXPORT int
wl_display_add_socket_fd(struct wl_display *display, int sock_fd)
{
	return -1;
}

/** Add a socket to Wayland display for the clients to connect.
 *
 * \param display Wayland display to which the socket should be added.
 * \param name Name of the Unix socket.
 * \return 0 if success. -1 if failed.
 *
 * This adds a Unix socket to Wayland display which can be used by clients to
 * connect to Wayland display.
 *
 * If NULL is passed as name, then it would look for WAYLAND_DISPLAY env
 * variable for the socket name. If WAYLAND_DISPLAY is not set, then default
 * wayland-0 is used.
 *
 * If the socket name is a relative path, the Unix socket will be created in
 * the directory pointed to by environment variable XDG_RUNTIME_DIR. If
 * XDG_RUNTIME_DIR is invalid or not set, then this function fails and returns -1.
 *
 * If the socket name is an absolute path, then it is used as-is for the
 * the Unix socket.
 *
 * The length of the computed socket path must not exceed the maximum length
 * of a Unix socket path.
 * The function also fails if the user does not have write permission in the
 * directory or if the path is already in use.
 *
 * \memberof wl_display
 */
WL_EXPORT int
wl_display_add_socket(struct wl_display *display, const char *name)
{
	return -1;
}

WL_EXPORT void
wl_display_add_destroy_listener(struct wl_display *display,
								struct wl_listener *listener)
{
	/* NO-OP */
}

/** Registers a listener for the client connection signal.
 *  When a new client object is created, \a listener will be notified, carrying
 *  a pointer to the new wl_client object.
 *
 *  \ref wl_client_create
 *  \ref wl_display
 *  \ref wl_listener
 *
 * \param display The display object
 * \param listener Signal handler object
 */
WL_EXPORT void
wl_display_add_client_created_listener(struct wl_display *display,
									   struct wl_listener *listener)
{
	/* NO-OP */
}

WL_EXPORT struct wl_listener *
wl_display_get_destroy_listener(struct wl_display *display,
								wl_notify_func_t notify)
{
	return wl_priv_signal_get(&display->destroy_signal, notify);
}

WL_EXPORT void
wl_resource_set_implementation(struct wl_resource *resource,
							   const void *implementation,
							   void *data, wl_resource_destroy_func_t destroy)
{
	/* NO-OP */
}

WL_EXPORT void
wl_resource_set_dispatcher(struct wl_resource *resource,
						   wl_dispatcher_func_t dispatcher,
						   const void *implementation,
						   void *data, wl_resource_destroy_func_t destroy)
{
	/* NO-OP */
}

/** Create a new resource object
 *
 * \param client The client owner of the new resource.
 * \param interface The interface of the new resource.
 * \param version The version of the new resource.
 * \param id The id of the new resource. If 0, an available id will be used.
 *
 * Listeners added with \a wl_client_add_resource_created_listener will be
 * notified at the end of this function.
 *
 * \memberof wl_resource
 */
WL_EXPORT struct wl_resource *
wl_resource_create(struct wl_client *client,
				   const struct wl_interface *interface,
				   int version, uint32_t id)
{
	return NULL;
}

WL_EXPORT void
wl_log_set_handler_server(wl_log_func_t handler)
{
	/* NO-OP */
}

/** Adds a new protocol logger.
 *
 * When a new protocol message arrives or is sent from the server
 * all the protocol logger functions will be called, carrying the
 * \a user_data pointer, the type of the message (request or
 * event) and the actual message.
 * The lifetime of the messages passed to the logger function ends
 * when they return so the messages cannot be stored and accessed
 * later.
 *
 * \a errno is set on error.
 *
 * \param display The display object
 * \param func The function to call to log a new protocol message
 * \param user_data The user data pointer to pass to \a func
 *
 * \return The protol logger object on success, NULL on failure.
 *
 * \sa wl_protocol_logger_destroy
 *
 * \memberof wl_display
 */
WL_EXPORT struct wl_protocol_logger *
wl_display_add_protocol_logger(struct wl_display *display,
							   wl_protocol_logger_func_t func, void *user_data)
{
	return NULL;
}

/** Destroys a protocol logger.
 *
 * This function destroys a protocol logger and removes it from the display
 * it was added to with \a wl_display_add_protocol_logger.
 * The \a logger object becomes invalid after calling this function.
 *
 * \sa wl_display_add_protocol_logger
 *
 * \memberof wl_protocol_logger
 */
WL_EXPORT void
wl_protocol_logger_destroy(struct wl_protocol_logger *logger)
{
	/* NO-OP */
}

/** Add support for a wl_shm pixel format
 *
 * \param display The display object
 * \param format The wl_shm pixel format to advertise
 * \return A pointer to the wl_shm format that was added to the list
 * or NULL if adding it to the list failed.
 *
 * Add the specified wl_shm format to the list of formats the wl_shm
 * object advertises when a client binds to it.  Adding a format to
 * the list means that clients will know that the compositor supports
 * this format and may use it for creating wl_shm buffers.  The
 * compositor must be able to handle the pixel format when a client
 * requests it.
 *
 * The compositor by default supports WL_SHM_FORMAT_ARGB8888 and
 * WL_SHM_FORMAT_XRGB8888.
 *
 * \memberof wl_display
 */
WL_EXPORT uint32_t *
wl_display_add_shm_format(struct wl_display *display, uint32_t format)
{
	uint32_t *p = NULL;

	p = wl_array_add(&display->additional_shm_formats, sizeof *p);

	if (p != NULL)
		*p = format;
	return p;
}

/**
 * Get list of additional wl_shm pixel formats
 *
 * \param display The display object
 *
 * This function returns the list of addition wl_shm pixel formats
 * that the compositor supports.  WL_SHM_FORMAT_ARGB8888 and
 * WL_SHM_FORMAT_XRGB8888 are always supported and not included in the
 * array, but all formats added through wl_display_add_shm_format()
 * will be in the array.
 *
 * \sa wl_display_add_shm_format()
 *
 * \private
 *
 * \memberof wl_display
 */
struct wl_array *
wl_display_get_additional_shm_formats(struct wl_display *display)
{
	return &display->additional_shm_formats;
}

/** Get the list of currently connected clients
 *
 * \param display The display object
 *
 * This function returns a pointer to the list of clients currently
 * connected to the display. You can iterate on the list by using
 * the \a wl_client_for_each macro.
 * The returned value is valid for the lifetime of the \a display.
 * You must not modify the returned list, but only access it.
 *
 * \sa wl_client_for_each()
 * \sa wl_client_get_link()
 * \sa wl_client_from_link()
 *
 * \memberof wl_display
 */
WL_EXPORT struct wl_list *
wl_display_get_client_list(struct wl_display *display)
{
	return &display->client_list;
}

/** Get the link by which a client is inserted in the client list
 *
 * \param client The client object
 *
 * \sa wl_client_for_each()
 * \sa wl_display_get_client_list()
 * \sa wl_client_from_link()
 *
 * \memberof wl_client
 */
WL_EXPORT struct wl_list *
wl_client_get_link(struct wl_client *client)
{
	return &client->link;
}

/** Get a wl_client by its link
 *
 * \param link The link of a wl_client
 *
 * \sa wl_client_for_each()
 * \sa wl_display_get_client_list()
 * \sa wl_client_get_link()
 *
 * \memberof wl_client
 */
WL_EXPORT struct wl_client *
wl_client_from_link(struct wl_list *link)
{
	struct wl_client *client;

	return wl_container_of(link, client, link);
}

/** Add a listener for the client's resource creation signal
 *
 * \param client The client object
 * \param listener The listener to be added
 *
 * When a new resource is created for this client the listener
 * will be notified, carrying the new resource as the data argument.
 *
 * \memberof wl_client
 */
WL_EXPORT void
wl_client_add_resource_created_listener(struct wl_client *client,
										struct wl_listener *listener)
{
	/* NO-OP */
}

struct wl_resource_iterator_context
{
	void *user_data;
	wl_client_for_each_resource_iterator_func_t it;
};

static enum wl_iterator_result
resource_iterator_helper(void *res, void *user_data, uint32_t flags)
{
	return WL_ITERATOR_STOP;
}

/** Iterate over all the resources of a client
 *
 * \param client The client object
 * \param iterator The iterator function
 * \param user_data The user data pointer
 *
 * The function pointed by \a iterator will be called for each
 * resource owned by the client. The \a user_data will be passed
 * as the second argument of the iterator function.
 * If the \a iterator function returns \a WL_ITERATOR_CONTINUE the iteration
 * will continue, if it returns \a WL_ITERATOR_STOP it will stop.
 *
 * Creating and destroying resources while iterating is safe, but new
 * resources may or may not be picked up by the iterator.
 *
 * \sa wl_iterator_result
 *
 * \memberof wl_client
 */
WL_EXPORT void
wl_client_for_each_resource(struct wl_client *client,
							wl_client_for_each_resource_iterator_func_t iterator,
							void *user_data)
{
	/* NO-OP */
}

static void
handle_noop(struct wl_listener *listener, void *data)
{
	/* Do nothing */
}

/** Emits this signal, notifying all registered listeners.
 *
 * A safer version of wl_signal_emit() which can gracefully handle additions
 * and deletions of any signal listener from within listener notification
 * callbacks.
 *
 * Listeners deleted during a signal emission and which have not already been
 * notified at the time of deletion are not notified by that emission.
 *
 * Listeners added (or readded) during signal emission are ignored by that
 * emission.
 *
 * Note that repurposing a listener without explicitly removing it and readding
 * it is not supported and can lead to unexpected behavior.
 *
 * \param signal The signal object that will emit the signal
 * \param data The data that will be emitted with the signal
 *
 * \memberof wl_signal
 * \since 1.20.90
 */
WL_EXPORT void
wl_signal_emit_mutable(struct wl_signal *signal, void *data)
{
	/* NO-OP */
}

/** \cond INTERNAL */

/** Initialize a wl_priv_signal object
 *
 * wl_priv_signal is a safer implementation of a signal type, with the same API
 * as wl_signal, but kept as a private utility of libwayland-server.
 * It is safer because listeners can be removed from within wl_priv_signal_emit()
 * without corrupting the signal's list.
 *
 * Before passing a wl_priv_signal object to any other function it must be
 * initialized by using wl_priv_signal_init().
 *
 * \memberof wl_priv_signal
 */
void wl_priv_signal_init(struct wl_priv_signal *signal)
{
	/* NO-OP */
}

/** Get a listener added to a signal
 *
 * Returns the listener added to the given \a signal and with the given
 * \a notify function, or NULL if there isn't any.
 * Calling this function from within wl_priv_signal_emit() is safe and will
 * return the correct value.
 *
 * \memberof wl_priv_signal
 */
struct wl_listener *
wl_priv_signal_get(struct wl_priv_signal *signal, wl_notify_func_t notify)
{
	return NULL;
}

/** Emit the signal, calling all the installed listeners
 *
 * Iterate over all the listeners added to this \a signal and call
 * their \a notify function pointer, passing on the given \a data.
 * Removing or adding a listener from within wl_priv_signal_emit()
 * is safe.
 */
void wl_priv_signal_emit(struct wl_priv_signal *signal, void *data)
{
	/* NO-OP */
}

/** Emit the signal for the last time, calling all the installed listeners
 *
 * Iterate over all the listeners added to this \a signal and call
 * their \a notify function pointer, passing on the given \a data.
 * Removing or adding a listener from within wl_priv_signal_emit()
 * is safe, as is freeing the structure containing the listener.
 *
 * A large body of external code assumes it's ok to free a destruction
 * listener without removing that listener from the list.  Mixing code
 * that acts like this and code that doesn't will result in list
 * corruption.
 *
 * We resolve this by removing each item from the list and isolating it
 * in another list.  We discard it completely after firing the notifier.
 * This should allow interoperability between code that unlinks its
 * destruction listeners and code that just frees structures they're in.
 *
 */
void wl_priv_signal_final_emit(struct wl_priv_signal *signal, void *data)
{
	/* NO-OP */
}

/** \endcond INTERNAL */

/** \cond */ /* Deprecated functions below. */

uint32_t
wl_client_add_resource(struct wl_client *client,
					   struct wl_resource *resource) WL_DEPRECATED;

WL_EXPORT uint32_t
wl_client_add_resource(struct wl_client *client,
					   struct wl_resource *resource)
{
	return 0;
}

struct wl_resource *
wl_client_add_object(struct wl_client *client,
					 const struct wl_interface *interface,
					 const void *implementation,
					 uint32_t id, void *data) WL_DEPRECATED;

WL_EXPORT struct wl_resource *
wl_client_add_object(struct wl_client *client,
					 const struct wl_interface *interface,
					 const void *implementation, uint32_t id, void *data)
{
	struct wl_resource *resource;

	resource = wl_resource_create(client, interface, -1, id);
	if (resource == NULL)
		wl_client_post_no_memory(client);
	else
		wl_resource_set_implementation(resource,
									   implementation, data, NULL);

	return resource;
}

struct wl_resource *
wl_client_new_object(struct wl_client *client,
					 const struct wl_interface *interface,
					 const void *implementation, void *data) WL_DEPRECATED;

WL_EXPORT struct wl_resource *
wl_client_new_object(struct wl_client *client,
					 const struct wl_interface *interface,
					 const void *implementation, void *data)
{
	struct wl_resource *resource;

	resource = wl_resource_create(client, interface, -1, 0);
	if (resource == NULL)
		wl_client_post_no_memory(client);
	else
		wl_resource_set_implementation(resource,
									   implementation, data, NULL);

	return resource;
}

struct wl_global *
wl_display_add_global(struct wl_display *display,
					  const struct wl_interface *interface,
					  void *data, wl_global_bind_func_t bind) WL_DEPRECATED;

WL_EXPORT struct wl_global *
wl_display_add_global(struct wl_display *display,
					  const struct wl_interface *interface,
					  void *data, wl_global_bind_func_t bind)
{
	return wl_global_create(display, interface, interface->version, data, bind);
}

void wl_display_remove_global(struct wl_display *display,
							  struct wl_global *global) WL_DEPRECATED;

WL_EXPORT void
wl_display_remove_global(struct wl_display *display, struct wl_global *global)
{
	/* NO-OP */
}

/** \endcond */

/* Stub client implementation */

WL_EXPORT const struct wl_interface wl_display_interface = {
	"wl_display",
	1,
	2,
	NULL,
	2,
	NULL,
};

WL_EXPORT const struct wl_interface wl_registry_interface = {
	"wl_registry",
	1,
	1,
	NULL,
	2,
	NULL,
};

WL_EXPORT const struct wl_interface wl_callback_interface = {
	"wl_callback",
	1,
	0,
	NULL,
	1,
	NULL,
};

WL_EXPORT const struct wl_interface wl_compositor_interface = {
	"wl_compositor",
	4,
	2,
	NULL,
	0,
	NULL,
};

WL_EXPORT const struct wl_interface wl_shm_pool_interface = {
	"wl_shm_pool",
	1,
	3,
	NULL,
	0,
	NULL,
};

WL_EXPORT const struct wl_interface wl_shm_interface = {
	"wl_shm",
	1,
	1,
	NULL,
	1,
	NULL,
};

WL_EXPORT const struct wl_interface wl_buffer_interface = {
	"wl_buffer",
	1,
	1,
	NULL,
	1,
	NULL,
};

WL_EXPORT const struct wl_interface wl_data_offer_interface = {
	"wl_data_offer",
	3,
	5,
	NULL,
	3,
	NULL,
};

WL_EXPORT const struct wl_interface wl_data_source_interface = {
	"wl_data_source",
	3,
	3,
	NULL,
	6,
	NULL,
};

WL_EXPORT const struct wl_interface wl_data_device_interface = {
	"wl_data_device",
	3,
	3,
	NULL,
	6,
	NULL,
};

WL_EXPORT const struct wl_interface wl_data_device_manager_interface = {
	"wl_data_device_manager",
	3,
	2,
	NULL,
	0,
	NULL,
};

WL_EXPORT const struct wl_interface wl_shell_interface = {
	"wl_shell",
	1,
	1,
	NULL,
	0,
	NULL,
};

WL_EXPORT const struct wl_interface wl_shell_surface_interface = {
	"wl_shell_surface",
	1,
	10,
	NULL,
	3,
	NULL,
};

WL_EXPORT const struct wl_interface wl_surface_interface = {
	"wl_surface",
	4,
	10,
	NULL,
	2,
	NULL,
};

WL_EXPORT const struct wl_interface wl_seat_interface = {
	"wl_seat",
	6,
	4,
	NULL,
	2,
	NULL,
};

WL_EXPORT const struct wl_interface wl_pointer_interface = {
	"wl_pointer",
	6,
	2,
	NULL,
	9,
	NULL,
};

WL_EXPORT const struct wl_interface wl_keyboard_interface = {
	"wl_keyboard",
	6,
	1,
	NULL,
	6,
	NULL,
};

WL_EXPORT const struct wl_interface wl_touch_interface = {
	"wl_touch",
	6,
	1,
	NULL,
	7,
	NULL,
};

WL_EXPORT const struct wl_interface wl_output_interface = {
	"wl_output",
	3,
	1,
	NULL,
	4,
	NULL,
};

WL_EXPORT const struct wl_interface wl_region_interface = {
	"wl_region",
	1,
	3,
	NULL,
	0,
	NULL,
};

WL_EXPORT const struct wl_interface wl_subcompositor_interface = {
	"wl_subcompositor",
	1,
	2,
	NULL,
	0,
	NULL,
};

WL_EXPORT const struct wl_interface wl_subsurface_interface = {
	"wl_subsurface",
	1,
	6,
	NULL,
	0,
	NULL,
};
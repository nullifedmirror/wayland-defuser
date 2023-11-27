/*
 * Copyright © 2008-2012 Kristian Høgsberg
 * Copyright © 2010-2012 Intel Corporation
 *
 * Modified for wayland-defuser.
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

#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <ctype.h>
#include <assert.h>
#include <fcntl.h>
#include <poll.h>
#include <pthread.h>

#include "libwayland-client.h"

/** \cond */

enum wl_proxy_flag
{
	WL_PROXY_FLAG_ID_DELETED = (1 << 0),
	WL_PROXY_FLAG_DESTROYED = (1 << 1),
	WL_PROXY_FLAG_WRAPPER = (1 << 2),
};

struct wl_zombie
{
	int event_count;
	int *fd_count;
};

struct wl_proxy
{
	struct wl_object object;
	struct wl_display *display;
	struct wl_event_queue *queue;
	uint32_t flags;
	int refcount;
	void *user_data;
	wl_dispatcher_func_t dispatcher;
	uint32_t version;
	const char *const *tag;
	struct wl_list queue_link; /**< in struct wl_event_queue::proxy_list */
};

struct wl_event_queue
{
	struct wl_list event_list;
	struct wl_list proxy_list; /**< struct wl_proxy::queue_link */
	struct wl_display *display;
};

struct wl_display
{
	struct wl_proxy proxy;
	struct wl_connection *connection;

	/* errno of the last wl_display error */
	int last_error;

	/* When display gets an error event from some object, it stores
	 * information about it here, so that client can get this
	 * information afterwards */
	struct
	{
		/* Code of the error. It can be compared to
		 * the interface's errors enumeration. */
		uint32_t code;
		/* interface (protocol) in which the error occurred */
		const struct wl_interface *interface;
		/* id of the proxy that caused the error. There's no warranty
		 * that the proxy is still valid. It's up to client how it will
		 * use it */
		uint32_t id;
	} protocol_error;
	int fd;
	struct wl_map objects;
	struct wl_event_queue display_queue;
	struct wl_event_queue default_queue;
	pthread_mutex_t mutex;

	int reader_count;
	uint32_t read_serial;
	pthread_cond_t reader_cond;
};

/** \endcond */

static int debug_client = 0;

/**
 * This helper function wakes up all threads that are
 * waiting for display->reader_cond (i. e. when reading is done,
 * canceled, or an error occurred)
 *
 * NOTE: must be called with display->mutex locked
 */
static void display_wakeup_threads(struct wl_display *display)
{
	/* NO-OP */
}

/**
 * This function is called for local errors (no memory, server hung up)
 *
 * \param display
 * \param error    error value (EINVAL, EFAULT, ...)
 *
 * \note this function is called with display mutex locked
 */
static void
display_fatal_error(struct wl_display *display, int error)
{
	/* NO-OP */
}

/**
 * This function is called for error events
 * and indicates that in some object an error occurred.
 * The difference between this function and display_fatal_error()
 * is that this one handles errors that will come by wire,
 * whereas display_fatal_error() is called for local errors.
 *
 * \param display
 * \param code    error code
 * \param id      id of the object that generated the error
 * \param intf    protocol interface
 */
static void
display_protocol_error(struct wl_display *display, uint32_t code,
					   uint32_t id, const struct wl_interface *intf)
{
	/* NO-OP */
}

static void
wl_event_queue_init(struct wl_event_queue *queue, struct wl_display *display)
{
	/* NO-OP */
}

static void
wl_proxy_unref(struct wl_proxy *proxy)
{
	/* NO-OP */
}

static void
validate_closure_objects(struct wl_closure *closure)
{
	/* NO-OP */
}

/* Destroys a closure which was demarshaled for dispatch; unrefs all the
 * proxies in its arguments, as well as its own proxy, and destroys the
 * closure itself. */
static void
destroy_queued_closure(struct wl_closure *closure)
{
	/* NO-OP */
}

static void
wl_event_queue_release(struct wl_event_queue *queue)
{
	/* NO-OP */
}

/** Destroy an event queue
 *
 * \param queue The event queue to be destroyed
 *
 * Destroy the given event queue. Any pending event on that queue is
 * discarded.
 *
 * The \ref wl_display object used to create the queue should not be
 * destroyed until all event queues created with it are destroyed with
 * this function.
 *
 * \memberof wl_event_queue
 */
WL_EXPORT void
wl_event_queue_destroy(struct wl_event_queue *queue)
{
	/* NO-OP */
}

/** Create a new event queue for this display
 *
 * \param display The display context object
 * \return A new event queue associated with this display or NULL on
 * failure.
 *
 * \memberof wl_display
 */
WL_EXPORT struct wl_event_queue *
wl_display_create_queue(struct wl_display *display)
{
	return NULL;
}

static int
message_count_fds(const char *signature)
{
	return 0;
}

static struct wl_zombie *
prepare_zombie(struct wl_proxy *proxy)
{
	return NULL;
}

static enum wl_iterator_result
free_zombies(void *element, void *data, uint32_t flags)
{
	if (flags & WL_MAP_ENTRY_ZOMBIE)
		free(element);

	return WL_ITERATOR_CONTINUE;
}

static struct wl_proxy *
proxy_create(struct wl_proxy *factory, const struct wl_interface *interface,
			 uint32_t version)
{
	return NULL;
}

/** Create a proxy object with a given interface
 *
 * \param factory Factory proxy object
 * \param interface Interface the proxy object should use
 * \return A newly allocated proxy object or NULL on failure
 *
 * This function creates a new proxy object with the supplied interface. The
 * proxy object will have an id assigned from the client id space. The id
 * should be created on the compositor side by sending an appropriate request
 * with \ref wl_proxy_marshal().
 *
 * The proxy will inherit the display and event queue of the factory object.
 *
 * \note This should not normally be used by non-generated code.
 *
 * \sa wl_display, wl_event_queue, wl_proxy_marshal()
 *
 * \memberof wl_proxy
 */
WL_EXPORT struct wl_proxy *
wl_proxy_create(struct wl_proxy *factory, const struct wl_interface *interface)
{
	return NULL;
}

/* The caller should hold the display lock */
static struct wl_proxy *
wl_proxy_create_for_id(struct wl_proxy *factory,
					   uint32_t id, const struct wl_interface *interface)
{
	return NULL;
}

static void
proxy_destroy(struct wl_proxy *proxy)
{
	/* NO-OP */
}

static void
wl_proxy_destroy_caller_locks(struct wl_proxy *proxy)
{
	/* NO-OP */
}

/** Destroy a proxy object
 *
 * \param proxy The proxy to be destroyed
 *
 * \c proxy must not be a proxy wrapper.
 *
 * \note This function will abort in response to egregious
 * errors, and will do so with the display lock held. This means
 * SIGABRT handlers must not perform any actions that would
 * attempt to take that lock, or a deadlock would occur.
 *
 * \memberof wl_proxy
 */
WL_EXPORT void
wl_proxy_destroy(struct wl_proxy *proxy)
{
	/* NO-OP */
}

/** Set a proxy's listener
 *
 * \param proxy The proxy object
 * \param implementation The listener to be added to proxy
 * \param data User data to be associated with the proxy
 * \return 0 on success or -1 on failure
 *
 * Set proxy's listener to \c implementation and its user data to
 * \c data. If a listener has already been set, this function
 * fails and nothing is changed.
 *
 * \c implementation is a vector of function pointers. For an opcode
 * \c n, \c implementation[n] should point to the handler of \c n for
 * the given object.
 *
 * \c proxy must not be a proxy wrapper.
 *
 * \memberof wl_proxy
 */
WL_EXPORT int
wl_proxy_add_listener(struct wl_proxy *proxy,
					  void (**implementation)(void), void *data)
{
	return -1;
}

/** Get a proxy's listener
 *
 * \param proxy The proxy object
 * \return The address of the proxy's listener or NULL if no listener is set
 *
 * Gets the address to the proxy's listener; which is the listener set with
 * \ref wl_proxy_add_listener.
 *
 * This function is useful in clients with multiple listeners on the same
 * interface to allow the identification of which code to execute.
 *
 * \memberof wl_proxy
 */
WL_EXPORT const void *
wl_proxy_get_listener(struct wl_proxy *proxy)
{
	return NULL;
}

/** Set a proxy's listener (with dispatcher)
 *
 * \param proxy The proxy object
 * \param dispatcher The dispatcher to be used for this proxy
 * \param implementation The dispatcher-specific listener implementation
 * \param data User data to be associated with the proxy
 * \return 0 on success or -1 on failure
 *
 * Set proxy's listener to use \c dispatcher_func as its dispatcher and \c
 * dispatcher_data as its dispatcher-specific implementation and its user data
 * to \c data. If a listener has already been set, this function
 * fails and nothing is changed.
 *
 * The exact details of dispatcher_data depend on the dispatcher used.  This
 * function is intended to be used by language bindings, not user code.
 *
 * \c proxy must not be a proxy wrapper.
 *
 * \memberof wl_proxy
 */
WL_EXPORT int
wl_proxy_add_dispatcher(struct wl_proxy *proxy,
						wl_dispatcher_func_t dispatcher,
						const void *implementation, void *data)
{
	return -1;
}

static struct wl_proxy *
create_outgoing_proxy(struct wl_proxy *proxy, const struct wl_message *message,
					  union wl_argument *args,
					  const struct wl_interface *interface, uint32_t version)
{
	return NULL;
}

/** Prepare a request to be sent to the compositor
 *
 * \param proxy The proxy object
 * \param opcode Opcode of the request to be sent
 * \param args Extra arguments for the given request
 * \param interface The interface to use for the new proxy
 *
 * This function translates a request given an opcode, an interface and a
 * wl_argument array to the wire format and writes it to the connection
 * buffer.
 *
 * For new-id arguments, this function will allocate a new wl_proxy
 * and send the ID to the server.  The new wl_proxy will be returned
 * on success or NULL on error with errno set accordingly.  The newly
 * created proxy will inherit their version from their parent.
 *
 * \note This is intended to be used by language bindings and not in
 * non-generated code.
 *
 * \sa wl_proxy_marshal()
 *
 * \memberof wl_proxy
 */
WL_EXPORT struct wl_proxy *
wl_proxy_marshal_array_constructor(struct wl_proxy *proxy,
								   uint32_t opcode, union wl_argument *args,
								   const struct wl_interface *interface)
{
	return NULL;
}

/** Prepare a request to be sent to the compositor
 *
 * \param proxy The proxy object
 * \param opcode Opcode of the request to be sent
 * \param args Extra arguments for the given request
 * \param interface The interface to use for the new proxy
 * \param version The protocol object version for the new proxy
 *
 * Translates the request given by opcode and the extra arguments into the
 * wire format and write it to the connection buffer.  This version takes an
 * array of the union type wl_argument.
 *
 * For new-id arguments, this function will allocate a new wl_proxy
 * and send the ID to the server.  The new wl_proxy will be returned
 * on success or NULL on error with errno set accordingly.  The newly
 * created proxy will have the version specified.
 *
 * \note This is intended to be used by language bindings and not in
 * non-generated code.
 *
 * \sa wl_proxy_marshal()
 *
 * \memberof wl_proxy
 */
WL_EXPORT struct wl_proxy *
wl_proxy_marshal_array_constructor_versioned(struct wl_proxy *proxy,
											 uint32_t opcode,
											 union wl_argument *args,
											 const struct wl_interface *interface,
											 uint32_t version)
{
	return NULL;
}

/** Prepare a request to be sent to the compositor
 *
 * \param proxy The proxy object
 * \param opcode Opcode of the request to be sent
 * \param interface The interface to use for the new proxy
 * \param version The protocol object version of the new proxy
 * \param flags Flags that modify marshalling behaviour
 * \param ... Extra arguments for the given request
 * \return A new wl_proxy for the new_id argument or NULL on error
 *
 * Translates the request given by opcode and the extra arguments into the
 * wire format and write it to the connection buffer.
 *
 * For new-id arguments, this function will allocate a new wl_proxy
 * and send the ID to the server.  The new wl_proxy will be returned
 * on success or NULL on error with errno set accordingly.  The newly
 * created proxy will have the version specified.
 *
 * The flag WL_MARSHAL_FLAG_DESTROY may be passed to ensure the proxy
 * is destroyed atomically with the marshalling in order to prevent
 * races that can occur if the display lock is dropped between the
 * marshal and destroy operations.
 *
 * \note This should not normally be used by non-generated code.
 *
 * \memberof wl_proxy
 */
WL_EXPORT struct wl_proxy *
wl_proxy_marshal_flags(struct wl_proxy *proxy, uint32_t opcode,
					   const struct wl_interface *interface, uint32_t version,
					   uint32_t flags, ...)
{
	return NULL;
}

/** Prepare a request to be sent to the compositor
 *
 * \param proxy The proxy object
 * \param opcode Opcode of the request to be sent
 * \param interface The interface to use for the new proxy
 * \param version The protocol object version for the new proxy
 * \param flags Flags that modify marshalling behaviour
 * \param args Extra arguments for the given request
 *
 * Translates the request given by opcode and the extra arguments into the
 * wire format and write it to the connection buffer.  This version takes an
 * array of the union type wl_argument.
 *
 * For new-id arguments, this function will allocate a new wl_proxy
 * and send the ID to the server.  The new wl_proxy will be returned
 * on success or NULL on error with errno set accordingly.  The newly
 * created proxy will have the version specified.
 *
 * The flag WL_MARSHAL_FLAG_DESTROY may be passed to ensure the proxy
 * is destroyed atomically with the marshalling in order to prevent
 * races that can occur if the display lock is dropped between the
 * marshal and destroy operations.
 *
 * \note This is intended to be used by language bindings and not in
 * non-generated code.
 *
 * \sa wl_proxy_marshal_flags()
 *
 * \memberof wl_proxy
 */
WL_EXPORT struct wl_proxy *
wl_proxy_marshal_array_flags(struct wl_proxy *proxy, uint32_t opcode,
							 const struct wl_interface *interface, uint32_t version,
							 uint32_t flags, union wl_argument *args)
{
	return NULL;
}

/** Prepare a request to be sent to the compositor
 *
 * \param proxy The proxy object
 * \param opcode Opcode of the request to be sent
 * \param ... Extra arguments for the given request
 *
 * This function is similar to wl_proxy_marshal_constructor(), except
 * it doesn't create proxies for new-id arguments.
 *
 * \note This should not normally be used by non-generated code.
 *
 * \sa wl_proxy_create()
 *
 * \memberof wl_proxy
 */
WL_EXPORT void
wl_proxy_marshal(struct wl_proxy *proxy, uint32_t opcode, ...)
{
	/* NO-OP */
}

/** Prepare a request to be sent to the compositor
 *
 * \param proxy The proxy object
 * \param opcode Opcode of the request to be sent
 * \param interface The interface to use for the new proxy
 * \param ... Extra arguments for the given request
 * \return A new wl_proxy for the new_id argument or NULL on error
 *
 * This function translates a request given an opcode, an interface and extra
 * arguments to the wire format and writes it to the connection buffer. The
 * types of the extra arguments must correspond to the argument types of the
 * method associated with the opcode in the interface.
 *
 * For new-id arguments, this function will allocate a new wl_proxy
 * and send the ID to the server.  The new wl_proxy will be returned
 * on success or NULL on error with errno set accordingly.  The newly
 * created proxy will inherit their version from their parent.
 *
 * \note This should not normally be used by non-generated code.
 *
 * \memberof wl_proxy
 */
WL_EXPORT struct wl_proxy *
wl_proxy_marshal_constructor(struct wl_proxy *proxy, uint32_t opcode,
							 const struct wl_interface *interface, ...)
{
	return NULL;
}

/** Prepare a request to be sent to the compositor
 *
 * \param proxy The proxy object
 * \param opcode Opcode of the request to be sent
 * \param interface The interface to use for the new proxy
 * \param version The protocol object version of the new proxy
 * \param ... Extra arguments for the given request
 * \return A new wl_proxy for the new_id argument or NULL on error
 *
 * Translates the request given by opcode and the extra arguments into the
 * wire format and write it to the connection buffer.
 *
 * For new-id arguments, this function will allocate a new wl_proxy
 * and send the ID to the server.  The new wl_proxy will be returned
 * on success or NULL on error with errno set accordingly.  The newly
 * created proxy will have the version specified.
 *
 * \note This should not normally be used by non-generated code.
 *
 * \memberof wl_proxy
 */
WL_EXPORT struct wl_proxy *
wl_proxy_marshal_constructor_versioned(struct wl_proxy *proxy, uint32_t opcode,
									   const struct wl_interface *interface,
									   uint32_t version, ...)
{
	return NULL;
}

/** Prepare a request to be sent to the compositor
 *
 * \param proxy The proxy object
 * \param opcode Opcode of the request to be sent
 * \param args Extra arguments for the given request
 *
 * This function is similar to wl_proxy_marshal_array_constructor(), except
 * it doesn't create proxies for new-id arguments.
 *
 * \note This is intended to be used by language bindings and not in
 * non-generated code.
 *
 * \sa wl_proxy_marshal()
 *
 * \memberof wl_proxy
 */
WL_EXPORT void
wl_proxy_marshal_array(struct wl_proxy *proxy, uint32_t opcode,
					   union wl_argument *args)
{
	/* NO-OP */
}

static void
display_handle_error(void *data,
					 struct wl_display *display, void *object,
					 uint32_t code, const char *message)
{
	/* NO-OP */
}

static void
display_handle_delete_id(void *data, struct wl_display *display, uint32_t id)
{
	/* NO-OP */
}

static const struct wl_display_listener display_listener = {
	display_handle_error,
	display_handle_delete_id};

static int
connect_to_socket(const char *name)
{
	errno = ENOENT;
	return -1;
}

/** Connect to Wayland display on an already open fd
 *
 * \param fd The fd to use for the connection
 * \return A \ref wl_display object or \c NULL on failure
 *
 * The wl_display takes ownership of the fd and will close it when the
 * display is destroyed.  The fd will also be closed in case of
 * failure.
 *
 * \memberof wl_display
 */
WL_EXPORT struct wl_display *
wl_display_connect_to_fd(int fd)
{
	close(fd);
	return NULL;
}

/** Connect to a Wayland display
 *
 * \param name Name of the Wayland display to connect to
 * \return A \ref wl_display object or \c NULL on failure
 *
 * Connect to the Wayland display named \c name. If \c name is \c NULL,
 * its value will be replaced with the WAYLAND_DISPLAY environment
 * variable if it is set, otherwise display "wayland-0" will be used.
 *
 * If WAYLAND_SOCKET is set, it's interpreted as a file descriptor number
 * referring to an already opened socket. In this case, the socket is used
 * as-is and \c name is ignored.
 *
 * If \c name is a relative path, then the socket is opened relative to
 * the XDG_RUNTIME_DIR directory.
 *
 * If \c name is an absolute path, then that path is used as-is for
 * the location of the socket at which the Wayland server is listening;
 * no qualification inside XDG_RUNTIME_DIR is attempted.
 *
 * If \c name is \c NULL and the WAYLAND_DISPLAY environment variable
 * is set to an absolute pathname, then that pathname is used as-is
 * for the socket in the same manner as if \c name held an absolute
 * path. Support for absolute paths in \c name and WAYLAND_DISPLAY
 * is present since Wayland version 1.15.
 *
 * \memberof wl_display
 */
WL_EXPORT struct wl_display *
wl_display_connect(const char *name)
{
	return NULL;
}

/** Close a connection to a Wayland display
 *
 * \param display The display context object
 *
 * Close the connection to \c display. The \ref wl_proxy and
 * \ref wl_event_queue objects need to be manually destroyed by the caller
 * before disconnecting.
 *
 * \memberof wl_display
 */
WL_EXPORT void
wl_display_disconnect(struct wl_display *display)
{
	/* NO-OP */
}

/** Get a display context's file descriptor
 *
 * \param display The display context object
 * \return Display object file descriptor
 *
 * Return the file descriptor associated with a display so it can be
 * integrated into the client's main loop.
 *
 * \memberof wl_display
 */
WL_EXPORT int
wl_display_get_fd(struct wl_display *display)
{
	return -1;
}

static void
sync_callback(void *data, struct wl_callback *callback, uint32_t serial)
{
	/* NO-OP */
}

static const struct wl_callback_listener sync_listener = {
	sync_callback};

/** Block until all pending request are processed by the server
 *
 * \param display The display context object
 * \param queue The queue on which to run the roundtrip
 * \return The number of dispatched events on success or -1 on failure
 *
 * This function blocks until the server has processed all currently issued
 * requests by sending a request to the display server and waiting for a
 * reply before returning.
 *
 * This function uses wl_display_dispatch_queue() internally. It is not allowed
 * to call this function while the thread is being prepared for reading events,
 * and doing so will cause a dead lock.
 *
 * \note This function may dispatch other events being received on the given
 * queue.
 *
 * \sa wl_display_roundtrip()
 * \memberof wl_display
 */
WL_EXPORT int
wl_display_roundtrip_queue(struct wl_display *display, struct wl_event_queue *queue)
{
	return -1;
}

/** Block until all pending request are processed by the server
 *
 * \param display The display context object
 * \return The number of dispatched events on success or -1 on failure
 *
 * This function blocks until the server has processed all currently issued
 * requests by sending a request to the display server and waiting for a reply
 * before returning.
 *
 * This function uses wl_display_dispatch_queue() internally. It is not allowed
 * to call this function while the thread is being prepared for reading events,
 * and doing so will cause a dead lock.
 *
 * \note This function may dispatch other events being received on the default
 * queue.
 *
 * \memberof wl_display
 */
WL_EXPORT int
wl_display_roundtrip(struct wl_display *display)
{
	return wl_display_roundtrip_queue(display, &display->default_queue);
}

static int
create_proxies(struct wl_proxy *sender, struct wl_closure *closure)
{
	return -1;
}

static void
increase_closure_args_refcount(struct wl_closure *closure)
{
	/* NO-OP */
}

static int
queue_event(struct wl_display *display, int len)
{
	return -1;
}

static uint32_t
id_from_object(union wl_argument *arg)
{
	return 0;
}

static void
dispatch_event(struct wl_display *display, struct wl_event_queue *queue)
{
	/* NO-OP */
}

static int
read_events(struct wl_display *display)
{
	return -1;
}

static void
cancel_read(struct wl_display *display)
{
	/* NO-OP */
}

/** Read events from display file descriptor
 *
 * \param display The display context object
 * \return 0 on success or -1 on error.  In case of error errno will
 * be set accordingly
 *
 * Calling this function will result in data available on the display file
 * descriptor being read and read events will be queued on their corresponding
 * event queues.
 *
 * Before calling this function, depending on what thread it is to be called
 * from, wl_display_prepare_read_queue() or wl_display_prepare_read() needs to
 * be called. See wl_display_prepare_read_queue() for more details.
 *
 * When being called at a point where other threads have been prepared to read
 * (using wl_display_prepare_read_queue() or wl_display_prepare_read()) this
 * function will sleep until all other prepared threads have either been
 * cancelled (using wl_display_cancel_read()) or them self entered this
 * function. The last thread that calls this function will then read and queue
 * events on their corresponding event queues, and finally wake up all other
 * wl_display_read_events() calls causing them to return.
 *
 * If a thread cancels a read preparation when all other threads that have
 * prepared to read has either called wl_display_cancel_read() or
 * wl_display_read_events(), all reader threads will return without having read
 * any data.
 *
 * To dispatch events that may have been queued, call
 * wl_display_dispatch_pending() or wl_display_dispatch_queue_pending().
 *
 * \sa wl_display_prepare_read(), wl_display_cancel_read(),
 * wl_display_dispatch_pending(), wl_display_dispatch()
 *
 * \memberof wl_display
 */
WL_EXPORT int
wl_display_read_events(struct wl_display *display)
{
	return -1;
}

static int
dispatch_queue(struct wl_display *display, struct wl_event_queue *queue)
{
	return -1;
}

/** Prepare to read events from the display's file descriptor to a queue
 *
 * \param display The display context object
 * \param queue The event queue to use
 * \return 0 on success or -1 if event queue was not empty
 *
 * This function (or wl_display_prepare_read()) must be called before reading
 * from the file descriptor using wl_display_read_events(). Calling
 * wl_display_prepare_read_queue() announces the calling thread's intention to
 * read and ensures that until the thread is ready to read and calls
 * wl_display_read_events(), no other thread will read from the file descriptor.
 * This only succeeds if the event queue is empty, and if not -1 is returned and
 * errno set to EAGAIN.
 *
 * If a thread successfully calls wl_display_prepare_read_queue(), it must
 * either call wl_display_read_events() when it's ready or cancel the read
 * intention by calling wl_display_cancel_read().
 *
 * Use this function before polling on the display fd or integrate the fd into a
 * toolkit event loop in a race-free way. A correct usage would be (with most
 * error checking left out):
 *
 * \code
 * while (wl_display_prepare_read_queue(display, queue) != 0)
 *         wl_display_dispatch_queue_pending(display, queue);
 * wl_display_flush(display);
 *
 * ret = poll(fds, nfds, -1);
 * if (has_error(ret))
 *         wl_display_cancel_read(display);
 * else
 *         wl_display_read_events(display);
 *
 * wl_display_dispatch_queue_pending(display, queue);
 * \endcode
 *
 * Here we call wl_display_prepare_read_queue(), which ensures that between
 * returning from that call and eventually calling wl_display_read_events(), no
 * other thread will read from the fd and queue events in our queue. If the call
 * to wl_display_prepare_read_queue() fails, we dispatch the pending events and
 * try again until we're successful.
 *
 * The wl_display_prepare_read_queue() function doesn't acquire exclusive access
 * to the display's fd. It only registers that the thread calling this function
 * has intention to read from fd. When all registered readers call
 * wl_display_read_events(), only one (at random) eventually reads and queues
 * the events and the others are sleeping meanwhile. This way we avoid races and
 * still can read from more threads.
 *
 * \sa wl_display_cancel_read(), wl_display_read_events(),
 * wl_display_prepare_read()
 *
 * \memberof wl_display
 */
WL_EXPORT int
wl_display_prepare_read_queue(struct wl_display *display,
							  struct wl_event_queue *queue)
{
	return -1;
}

/** Prepare to read events from the display's file descriptor
 *
 * \param display The display context object
 * \return 0 on success or -1 if event queue was not empty
 *
 * This function does the same thing as wl_display_prepare_read_queue()
 * with the default queue passed as the queue.
 *
 * \sa wl_display_prepare_read_queue
 * \memberof wl_display
 */
WL_EXPORT int
wl_display_prepare_read(struct wl_display *display)
{
	return wl_display_prepare_read_queue(display, &display->default_queue);
}

/** Cancel read intention on display's fd
 *
 * \param display The display context object
 *
 * After a thread successfully called wl_display_prepare_read() it must
 * either call wl_display_read_events() or wl_display_cancel_read().
 * If the threads do not follow this rule it will lead to deadlock.
 *
 * \sa wl_display_prepare_read(), wl_display_read_events()
 *
 * \memberof wl_display
 */
WL_EXPORT void
wl_display_cancel_read(struct wl_display *display)
{
	/* NO-OP */
}

static int
wl_display_poll(struct wl_display *display, short int events)
{
	return -1;
}

/** Dispatch events in an event queue
 *
 * \param display The display context object
 * \param queue The event queue to dispatch
 * \return The number of dispatched events on success or -1 on failure
 *
 * Dispatch events on the given event queue.
 *
 * If the given event queue is empty, this function blocks until there are
 * events to be read from the display fd. Events are read and queued on
 * the appropriate event queues. Finally, events on given event queue are
 * dispatched. On failure -1 is returned and errno set appropriately.
 *
 * In a multi threaded environment, do not manually wait using poll() (or
 * equivalent) before calling this function, as doing so might cause a dead
 * lock. If external reliance on poll() (or equivalent) is required, see
 * wl_display_prepare_read_queue() of how to do so.
 *
 * This function is thread safe as long as it dispatches the right queue on the
 * right thread. It is also compatible with the multi thread event reading
 * preparation API (see wl_display_prepare_read_queue()), and uses the
 * equivalent functionality internally. It is not allowed to call this function
 * while the thread is being prepared for reading events, and doing so will
 * cause a dead lock.
 *
 * It can be used as a helper function to ease the procedure of reading and
 * dispatching events.
 *
 * \note Since Wayland 1.5 the display has an extra queue
 * for its own events (i. e. delete_id). This queue is dispatched always,
 * no matter what queue we passed as an argument to this function.
 * That means that this function can return non-0 value even when it
 * haven't dispatched any event for the given queue.
 *
 * \sa wl_display_dispatch(), wl_display_dispatch_pending(),
 * wl_display_dispatch_queue_pending(), wl_display_prepare_read_queue()
 *
 * \memberof wl_display
 */
WL_EXPORT int
wl_display_dispatch_queue(struct wl_display *display,
						  struct wl_event_queue *queue)
{
	return -1;
}

/** Dispatch pending events in an event queue
 *
 * \param display The display context object
 * \param queue The event queue to dispatch
 * \return The number of dispatched events on success or -1 on failure
 *
 * Dispatch all incoming events for objects assigned to the given
 * event queue. On failure -1 is returned and errno set appropriately.
 * If there are no events queued, this function returns immediately.
 *
 * \memberof wl_display
 * \since 1.0.2
 */
WL_EXPORT int
wl_display_dispatch_queue_pending(struct wl_display *display,
								  struct wl_event_queue *queue)
{
	return -1;
}

/** Process incoming events
 *
 * \param display The display context object
 * \return The number of dispatched events on success or -1 on failure
 *
 * Dispatch events on the default event queue.
 *
 * If the default event queue is empty, this function blocks until there are
 * events to be read from the display fd. Events are read and queued on
 * the appropriate event queues. Finally, events on the default event queue
 * are dispatched. On failure -1 is returned and errno set appropriately.
 *
 * In a multi threaded environment, do not manually wait using poll() (or
 * equivalent) before calling this function, as doing so might cause a dead
 * lock. If external reliance on poll() (or equivalent) is required, see
 * wl_display_prepare_read_queue() of how to do so.
 *
 * This function is thread safe as long as it dispatches the right queue on the
 * right thread. It is also compatible with the multi thread event reading
 * preparation API (see wl_display_prepare_read_queue()), and uses the
 * equivalent functionality internally. It is not allowed to call this function
 * while the thread is being prepared for reading events, and doing so will
 * cause a dead lock.
 *
 * \note It is not possible to check if there are events on the queue
 * or not. For dispatching default queue events without blocking, see \ref
 * wl_display_dispatch_pending().
 *
 * \sa wl_display_dispatch_pending(), wl_display_dispatch_queue(),
 * wl_display_read_events()
 *
 * \memberof wl_display
 */
WL_EXPORT int
wl_display_dispatch(struct wl_display *display)
{
	return wl_display_dispatch_queue(display, &display->default_queue);
}

/** Dispatch default queue events without reading from the display fd
 *
 * \param display The display context object
 * \return The number of dispatched events or -1 on failure
 *
 * This function dispatches events on the main event queue. It does not
 * attempt to read the display fd and simply returns zero if the main
 * queue is empty, i.e., it doesn't block.
 *
 * \sa wl_display_dispatch(), wl_display_dispatch_queue(),
 * wl_display_flush()
 *
 * \memberof wl_display
 */
WL_EXPORT int
wl_display_dispatch_pending(struct wl_display *display)
{
	return wl_display_dispatch_queue_pending(display,
											 &display->default_queue);
}

/** Retrieve the last error that occurred on a display
 *
 * \param display The display context object
 * \return The last error that occurred on \c display or 0 if no error occurred
 *
 * Return the last error that occurred on the display. This may be an error sent
 * by the server or caused by the local client.
 *
 * \note Errors are \b fatal. If this function returns non-zero the display
 * can no longer be used.
 *
 * \memberof wl_display
 */
WL_EXPORT int
wl_display_get_error(struct wl_display *display)
{
	return 0;
}

/** Retrieves the information about a protocol error:
 *
 * \param display    The Wayland display
 * \param interface  if not NULL, stores the interface where the error occurred,
 *                   or NULL, if unknown.
 * \param id         if not NULL, stores the object id that generated
 *                   the error, or 0, if the object id is unknown. There's no
 *                   guarantee the object is still valid; the client must know
 *                   if it deleted the object.
 * \return           The error code as defined in the interface specification.
 *
 * \code
 * int err = wl_display_get_error(display);
 *
 * if (err == EPROTO) {
 *        code = wl_display_get_protocol_error(display, &interface, &id);
 *        handle_error(code, interface, id);
 * }
 *
 * ...
 * \endcode
 * \memberof wl_display
 */
WL_EXPORT uint32_t
wl_display_get_protocol_error(struct wl_display *display,
							  const struct wl_interface **interface,
							  uint32_t *id)
{
	return 0;
}

/** Send all buffered requests on the display to the server
 *
 * \param display The display context object
 * \return The number of bytes sent on success or -1 on failure
 *
 * Send all buffered data on the client side to the server. Clients should
 * always call this function before blocking on input from the display fd.
 * On success, the number of bytes sent to the server is returned. On
 * failure, this function returns -1 and errno is set appropriately.
 *
 * wl_display_flush() never blocks.  It will write as much data as
 * possible, but if all data could not be written, errno will be set
 * to EAGAIN and -1 returned.  In that case, use poll on the display
 * file descriptor to wait for it to become writable again.
 *
 * \memberof wl_display
 */
WL_EXPORT int
wl_display_flush(struct wl_display *display)
{
	return -1;
}

/** Set the user data associated with a proxy
 *
 * \param proxy The proxy object
 * \param user_data The data to be associated with proxy
 *
 * Set the user data associated with \c proxy. When events for this
 * proxy are received, \c user_data will be supplied to its listener.
 *
 * \memberof wl_proxy
 */
WL_EXPORT void
wl_proxy_set_user_data(struct wl_proxy *proxy, void *user_data)
{
	proxy->user_data = user_data;
}

/** Get the user data associated with a proxy
 *
 * \param proxy The proxy object
 * \return The user data associated with proxy
 *
 * \memberof wl_proxy
 */
WL_EXPORT void *
wl_proxy_get_user_data(struct wl_proxy *proxy)
{
	return proxy->user_data;
}

/** Get the protocol object version of a proxy object
 *
 * \param proxy The proxy object
 * \return The protocol object version of the proxy or 0
 *
 * Gets the protocol object version of a proxy object, or 0
 * if the proxy was created with unversioned API.
 *
 * A returned value of 0 means that no version information is
 * available, so the caller must make safe assumptions about
 * the object's real version.
 *
 * wl_display's version will always return 0.
 *
 * \memberof wl_proxy
 */
WL_EXPORT uint32_t
wl_proxy_get_version(struct wl_proxy *proxy)
{
	return proxy->version;
}

/** Get the id of a proxy object
 *
 * \param proxy The proxy object
 * \return The id the object associated with the proxy
 *
 * \memberof wl_proxy
 */
WL_EXPORT uint32_t
wl_proxy_get_id(struct wl_proxy *proxy)
{
	return proxy->object.id;
}

/** Set the tag of a proxy object
 *
 * A toolkit or application can set a unique tag on a proxy in order to
 * identify whether an object is managed by itself or some external part.
 *
 * To create a tag, the recommended way is to define a statically allocated
 * constant char array containing some descriptive string. The tag will be the
 * pointer to the non-const pointer to the beginning of the array.
 *
 * For example, to define and set a tag on a surface managed by a certain
 * subsystem:
 *
 * 	static const char *my_tag = "my tag";
 *
 * 	wl_proxy_set_tag((struct wl_proxy *) surface, &my_tag);
 *
 * Then, in a callback with wl_surface as an argument, in order to check
 * whether it's a surface managed by the same subsystem.
 *
 * 	const char * const *tag;
 *
 * 	tag = wl_proxy_get_tag((struct wl_proxy *) surface);
 * 	if (tag != &my_tag)
 *		return;
 *
 *	...
 *
 * For debugging purposes, a tag should be suitable to be included in a debug
 * log entry, e.g.
 *
 * 	const char * const *tag;
 *
 * 	tag = wl_proxy_get_tag((struct wl_proxy *) surface);
 * 	printf("Got a surface with the tag %p (%s)\n",
 * 	       tag, (tag && *tag) ? *tag : "");
 *
 * \param proxy The proxy object
 * \param tag The tag
 *
 * \memberof wl_proxy
 * \since 1.17.90
 */
WL_EXPORT void
wl_proxy_set_tag(struct wl_proxy *proxy,
				 const char *const *tag)
{
	proxy->tag = tag;
}

/** Get the tag of a proxy object
 *
 * See wl_proxy_set_tag for details.
 *
 * \param proxy The proxy object
 *
 * \memberof wl_proxy
 * \since 1.17.90
 */
WL_EXPORT const char *const *
wl_proxy_get_tag(struct wl_proxy *proxy)
{
	return proxy->tag;
}

/** Get the interface name (class) of a proxy object
 *
 * \param proxy The proxy object
 * \return The interface name of the object associated with the proxy
 *
 * \memberof wl_proxy
 */
WL_EXPORT const char *
wl_proxy_get_class(struct wl_proxy *proxy)
{
	return NULL;
}

/** Get the display of a proxy object
 *
 * \param proxy The proxy object
 * \return The wl_display the proxy is associated with
 *
 * \memberof wl_proxy
 * \since 1.23
 */
WL_EXPORT struct wl_display *
wl_proxy_get_display(struct wl_proxy *proxy)
{
	return proxy->display;
}

/** Assign a proxy to an event queue
 *
 * \param proxy The proxy object
 * \param queue The event queue that will handle this proxy or NULL
 *
 * Assign proxy to event queue. Events coming from \c proxy will be
 * queued in \c queue from now. If queue is NULL, then the display's
 * default queue is set to the proxy.
 *
 * In order to guarantee proper handing of all events which were queued
 * before the queue change takes effect, it is required to dispatch the
 * proxy's old event queue after setting a new event queue.
 *
 * This is particularly important for multi-threaded setups, where it is
 * possible for events to be queued to the proxy's old queue from a
 * different thread during the invocation of this function.
 *
 * To ensure that all events for a newly created proxy are dispatched
 * on a particular queue, it is necessary to use a proxy wrapper if
 * events are read and dispatched on more than one thread. See
 * wl_proxy_create_wrapper() for more details.
 *
 * \note By default, the queue set in proxy is the one inherited from parent.
 *
 * \sa wl_display_dispatch_queue()
 *
 * \memberof wl_proxy
 */
WL_EXPORT void
wl_proxy_set_queue(struct wl_proxy *proxy, struct wl_event_queue *queue)
{
	/* NO-OP */
}

/** Create a proxy wrapper for making queue assignments thread-safe
 *
 * \param proxy The proxy object to be wrapped
 * \return A proxy wrapper for the given proxy or NULL on failure
 *
 * A proxy wrapper is type of 'struct wl_proxy' instance that can be used when
 * sending requests instead of using the original proxy. A proxy wrapper does
 * not have an implementation or dispatcher, and events received on the
 * object is still emitted on the original proxy. Trying to set an
 * implementation or dispatcher will have no effect but result in a warning
 * being logged.
 *
 * Setting the proxy queue of the proxy wrapper will make new objects created
 * using the proxy wrapper use the set proxy queue.
 * Even though there is no implementation nor dispatcher, the proxy queue can
 * be changed. This will affect the default queue of new objects created by
 * requests sent via the proxy wrapper.
 *
 * A proxy wrapper can only be destroyed using wl_proxy_wrapper_destroy().
 *
 * A proxy wrapper must be destroyed before the proxy it was created from.
 *
 * If a user reads and dispatches events on more than one thread, it is
 * necessary to use a proxy wrapper when sending requests on objects when the
 * intention is that a newly created proxy is to use a proxy queue different
 * from the proxy the request was sent on, as creating the new proxy and then
 * setting the queue is not thread safe.
 *
 * For example, a module that runs using its own proxy queue that needs to
 * do display roundtrip must wrap the wl_display proxy object before sending
 * the wl_display.sync request. For example:
 *
 * \code
 *
 *   struct wl_event_queue *queue = ...;
 *   struct wl_display *wrapped_display;
 *   struct wl_callback *callback;
 *
 *   wrapped_display = wl_proxy_create_wrapper(display);
 *   wl_proxy_set_queue((struct wl_proxy *) wrapped_display, queue);
 *   callback = wl_display_sync(wrapped_display);
 *   wl_proxy_wrapper_destroy(wrapped_display);
 *   wl_callback_add_listener(callback, ...);
 *
 * \endcode
 *
 * \memberof wl_proxy
 */
WL_EXPORT void *
wl_proxy_create_wrapper(void *proxy)
{
	return NULL;
}

/** Destroy a proxy wrapper
 * \param proxy_wrapper The proxy wrapper to be destroyed
 *
 * \memberof wl_proxy
 */
WL_EXPORT void
wl_proxy_wrapper_destroy(void *proxy_wrapper)
{
	/* NO-OP */
}

WL_EXPORT void
wl_log_set_handler_client(wl_log_func_t handler)
{
	/* NO-OP */
}

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
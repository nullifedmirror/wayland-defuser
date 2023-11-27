#include "libwayland-base.h"

/**
 * @ingroup iface_wl_display
 * @struct wl_display_listener
 */
struct wl_display_listener
{
	/**
	 * fatal error event
	 *
	 * The error event is sent out when a fatal (non-recoverable)
	 * error has occurred. The object_id argument is the object where
	 * the error occurred, most often in response to a request to that
	 * object. The code identifies the error and is defined by the
	 * object interface. As such, each interface defines its own set of
	 * error codes. The message is a brief description of the error,
	 * for (debugging) convenience.
	 * @param object_id object where the error occurred
	 * @param code error code
	 * @param message error description
	 */
	void (*error)(void *data,
				  struct wl_display *wl_display,
				  void *object_id,
				  uint32_t code,
				  const char *message);
	/**
	 * acknowledge object ID deletion
	 *
	 * This event is used internally by the object ID management
	 * logic. When a client deletes an object, the server will send
	 * this event to acknowledge that it has seen the delete request.
	 * When the client receives this event, it will know that it can
	 * safely reuse the object ID.
	 * @param id deleted object ID
	 */
	void (*delete_id)(void *data,
					  struct wl_display *wl_display,
					  uint32_t id);
};

/**
 * @ingroup iface_wl_callback
 * @struct wl_callback_listener
 */
struct wl_callback_listener
{
	/**
	 * done event
	 *
	 * Notify the client when the related request is done.
	 * @param callback_data request-specific data for the callback
	 */
	void (*done)(void *data,
				 struct wl_callback *wl_callback,
				 uint32_t callback_data);
};

/**
 * Dispatcher function type alias
 *
 * A dispatcher is a function that handles the emitting of callbacks in client
 * code. For programs directly using the C library, this is done by using
 * libffi to call function pointers. When binding to languages other than C,
 * dispatchers provide a way to abstract the function calling process to be
 * friendlier to other function calling systems.
 *
 * A dispatcher takes five arguments: The first is the dispatcher-specific
 * implementation associated with the target object. The second is the object
 * upon which the callback is being invoked (either wl_proxy or wl_resource).
 * The third and fourth arguments are the opcode and the wl_message
 * corresponding to the callback. The final argument is an array of arguments
 * received from the other process via the wire protocol.
 *
 * \param user_data Dispatcher-specific implementation data
 * \param target Callback invocation target (wl_proxy or `wl_resource`)
 * \param opcode Callback opcode
 * \param msg Callback message signature
 * \param args Array of received arguments
 *
 * \return 0 on success, or -1 on failure
 */
typedef int (*wl_dispatcher_func_t)(const void *user_data, void *target,
									uint32_t opcode, const struct wl_message *msg,
									union wl_argument *args);

/**
 * Log function type alias
 *
 * The C implementation of the Wayland protocol abstracts the details of
 * logging. Users may customize the logging behavior, with a function conforming
 * to the `wl_log_func_t` type, via `wl_log_set_handler_client` and
 * `wl_log_set_handler_server`.
 *
 * A `wl_log_func_t` must conform to the expectations of `vprintf`, and
 * expects two arguments: a string to write and a corresponding variable
 * argument list. While the string to write may contain format specifiers and
 * use values in the variable argument list, the behavior of any `wl_log_func_t`
 * depends on the implementation.
 *
 * \note Take care to not confuse this with `wl_protocol_logger_func_t`, which
 *       is a specific server-side logger for requests and events.
 *
 * \param fmt String to write to the log, containing optional format
 *            specifiers
 * \param args Variable argument list
 *
 * \sa wl_log_set_handler_client
 * \sa wl_log_set_handler_server
 */
typedef void (*wl_log_func_t)(const char *fmt, va_list args) WL_PRINTF(1, 0);

/**
 * @ingroup iface_wl_buffer
 * @struct wl_buffer_interface
 */
extern struct wl_buffer_interface
{
	/**
	 * destroy a buffer
	 *
	 * Destroy a buffer. If and how you need to release the backing
	 * storage is defined by the buffer factory interface.
	 *
	 * For possible side-effects to a surface, see wl_surface.attach.
	 */
	void (*destroy)(struct wl_client *client,
					struct wl_resource *resource);
};

/**
 * @defgroup iface_wl_buffer The wl_buffer interface
 *
 * A buffer provides the content for a wl_surface. Buffers are
 * created through factory interfaces such as wl_drm, wl_shm or
 * similar. It has a width and a height and can be attached to a
 * wl_surface, but the mechanism by which a client provides and
 * updates the contents is defined by the buffer factory interface.
 */
extern const struct wl_interface wl_buffer_interface;

/**
 * @defgroup iface_wl_surface The wl_surface interface
 *
 * A surface is a rectangular area that is displayed on the screen.
 * It has a location, size and pixel contents.
 *
 * The size of a surface (and relative positions on it) is described
 * in surface-local coordinates, which may differ from the buffer
 * coordinates of the pixel content, in case a buffer_transform
 * or a buffer_scale is used.
 *
 * A surface without a "role" is fairly useless: a compositor does
 * not know where, when or how to present it. The role is the
 * purpose of a wl_surface. Examples of roles are a cursor for a
 * pointer (as set by wl_pointer.set_cursor), a drag icon
 * (wl_data_device.start_drag), a sub-surface
 * (wl_subcompositor.get_subsurface), and a window as defined by a
 * shell protocol (e.g. wl_shell.get_shell_surface).
 *
 * A surface can have only one role at a time. Initially a
 * wl_surface does not have a role. Once a wl_surface is given a
 * role, it is set permanently for the whole lifetime of the
 * wl_surface object. Giving the current role again is allowed,
 * unless explicitly forbidden by the relevant interface
 * specification.
 *
 * Surface roles are given by requests in other interfaces such as
 * wl_pointer.set_cursor. The request should explicitly mention
 * that this request gives a role to a wl_surface. Often, this
 * request also creates a new protocol object that represents the
 * role and adds additional functionality to wl_surface. When a
 * client wants to destroy a wl_surface, they must destroy this 'role
 * object' before the wl_surface.
 *
 * Destroying the role object does not remove the role from the
 * wl_surface, but it may stop the wl_surface from "playing the role".
 * For instance, if a wl_subsurface object is destroyed, the wl_surface
 * it was created for will be unmapped and forget its position and
 * z-order. It is allowed to create a wl_subsurface for the same
 * wl_surface again, but it is not allowed to use the wl_surface as
 * a cursor (cursor is a different role than sub-surface, and role
 * switching is not allowed).
 */
extern const struct wl_interface wl_surface_interface;
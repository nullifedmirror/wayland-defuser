#include "libwayland-base.h"

/** Deprecated attribute */
#if defined(__GNUC__) && __GNUC__ >= 4
#define WL_DEPRECATED __attribute__((deprecated))
#else
#define WL_DEPRECATED
#endif

#define WL_MAP_SERVER_SIDE 0
#define WL_MAP_CLIENT_SIDE 1
#define WL_SERVER_ID_START 0xff000000
#define WL_MAP_MAX_OBJECTS 0x00f00000
#define WL_CLOSURE_MAX_ARGS 20

#define WL_DISPLAY_ERROR 0
#define WL_DISPLAY_DELETE_ID 1

/** \class wl_signal
 *
 * \brief A source of a type of observable event
 *
 * Signals are recognized points where significant events can be observed.
 * Compositors as well as the server can provide signals. Observers are
 * wl_listener's that are added through #wl_signal_add. Signals are emitted
 * using #wl_signal_emit, which will invoke all listeners until that
 * listener is removed by wl_list_remove() (or whenever the signal is
 * destroyed).
 *
 * \sa wl_listener for more information on using wl_signal
 */
struct wl_signal
{
	struct wl_list listener_list;
};

/**
 * @ingroup iface_wl_registry
 * @struct wl_registry_interface
 */
struct wl_registry_interface
{
	/**
	 * bind an object to the display
	 *
	 * Binds a new, client-created object to the server using the
	 * specified name as the identifier.
	 * @param name unique numeric name of the object
	 * @param interface name of the objects interface
	 * @param version version of the objects interface
	 * @param id bounded object
	 */
	void (*bind)(struct wl_client *client,
				 struct wl_resource *resource,
				 uint32_t name,
				 const char *interface, uint32_t version, uint32_t id);
};

/**
 * @ingroup iface_wl_display
 * @struct wl_display_interface
 */
struct wl_display_interface
{
	/**
	 * asynchronous roundtrip
	 *
	 * The sync request asks the server to emit the 'done' event on
	 * the returned wl_callback object. Since requests are handled
	 * in-order and events are delivered in-order, this can be used as
	 * a barrier to ensure all previous requests and the resulting
	 * events have been handled.
	 *
	 * The object returned by this request will be destroyed by the
	 * compositor after the callback is fired and as such the client
	 * must not attempt to use it after that point.
	 *
	 * The callback_data passed in the callback is the event serial.
	 * @param callback callback object for the sync request
	 */
	void (*sync)(struct wl_client *client,
				 struct wl_resource *resource,
				 uint32_t callback);
	/**
	 * get global registry object
	 *
	 * This request creates a registry object that allows the client
	 * to list and bind the global objects available from the
	 * compositor.
	 * @param registry global registry object
	 */
	void (*get_registry)(struct wl_client *client,
						 struct wl_resource *resource,
						 uint32_t registry);
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

typedef void (*wl_resource_destroy_func_t)(struct wl_resource *resource);

typedef void (*wl_notify_func_t)(struct wl_listener *listener, void *data);

typedef void (*wl_protocol_logger_func_t)(void *user_data,
										  enum wl_protocol_logger_type direction,
										  const struct wl_protocol_logger_message *message);

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

/** A filter function for wl_global objects
 *
 * \param client The client object
 * \param global The global object to show or hide
 * \param data   The user data pointer
 *
 * A filter function enables the server to decide which globals to
 * advertise to each client.
 *
 * When a wl_global filter is set, the given callback function will be
 * called during wl_global advertisement and binding.
 *
 * This function should return true if the global object should be made
 * visible to the client or false otherwise.
 */
typedef bool (*wl_display_global_filter_func_t)(const struct wl_client *client,
												const struct wl_global *global,
												void *data);

typedef void (*wl_global_bind_func_t)(struct wl_client *client, void *data,
									  uint32_t version, uint32_t id);

typedef enum wl_iterator_result (*wl_client_for_each_resource_iterator_func_t)(
	struct wl_resource *resource,
	void *user_data);

struct wl_priv_signal
{
	struct wl_list listener_list;
	struct wl_list emit_list;
};

void wl_priv_signal_init(struct wl_priv_signal *signal);

void wl_priv_signal_add(struct wl_priv_signal *signal, struct wl_listener *listener);

struct wl_listener *
wl_priv_signal_get(struct wl_priv_signal *signal, wl_notify_func_t notify);

void wl_priv_signal_emit(struct wl_priv_signal *signal, void *data);

void wl_priv_signal_final_emit(struct wl_priv_signal *signal, void *data);

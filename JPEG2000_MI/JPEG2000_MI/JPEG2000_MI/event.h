
#ifndef __EVENT_H
#define __EVENT_H
#include"openjpeg.h"
/**
@file event.h
@brief Implementation of a event callback system

The functions in EVENT.C have for goal to send output messages (errors, warnings, debug) to the user.
*/
/**
Message handler object
used for 
<ul>
<li>Error messages
<li>Warning messages
<li>Debugging messages
</ul>
*/
typedef struct mi_event_mgr 
{
	/** Data to call the event manager upon */
	void *			m_error_data;
	/** Data to call the event manager upon */
	void *			m_warning_data;
	/** Data to call the event manager upon */
	void *			m_info_data;
	/** Error message callback if available, NULL otherwise */
	mi_msg_callback error_handler;
	/** Warning message callback if available, NULL otherwise */
	mi_msg_callback warning_handler;
	/** Debug message callback if available, NULL otherwise */
	mi_msg_callback info_handler;
} mi_event_mgr_t;


#define EVT_ERROR	1	/**< Error event type */
#define EVT_WARNING	2	/**< Warning event type */
#define EVT_INFO	4	/**< Debug event type */

/** @defgroup EVENT EVENT - Implementation of a event callback system */
/*@{*/

/** @name Exported functions (see also openjpeg.h) */
/*@{*/
/* ----------------------------------------------------------------------- */


/* ----------------------------------------------------------------------- */

/**
 * Write formatted data to a string and send the string to a user callback.
 *
 * @param event_mgr			Event handler
 * @param event_type 		Event type or callback to use to send the message
 * @param fmt 				Format-control string (plus optional arguments)
 *
 * @return Returns true if successful, returns false otherwise
 */
mi_BOOL mi_event_msg(mi_event_mgr_t* event_mgr, mi_INT32 event_type, const char *fmt, ...);
/* ----------------------------------------------------------------------- */

/**
 * Set the event manager with the default callback function for the 3 levels.
 */
void mi_set_default_event_handler(mi_event_mgr_t * p_manager);

/*
#ifdef __GNUC__
#pragma GCC poison printf fprintf
#endif
*/

/*@}*/

/*@}*/

#endif /* __EVENT_H */

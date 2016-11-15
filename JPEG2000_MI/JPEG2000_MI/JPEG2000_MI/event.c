
#include "mi_includes.h"

/* ==========================================================
     Utility functions
   ==========================================================*/

/* ----------------------------------------------------------------------- */
/**
 * Default callback function.
 * Do nothing.
 */
static void mi_default_callback (const char *msg, void *client_data)
{
    mi_ARG_NOT_USED(msg);
    mi_ARG_NOT_USED(client_data);
}

/* ----------------------------------------------------------------------- */


/* ----------------------------------------------------------------------- */
mi_BOOL mi_event_msg(mi_event_mgr_t* p_event_mgr, mi_INT32 event_type, const char *fmt, ...) {
#define mi_MSG_SIZE 512 /* 512 bytes should be more than enough for a short message */
	mi_msg_callback msg_handler = 00;
	void * l_data = 00;

	if(p_event_mgr != 00) {
		switch(event_type) {
			case EVT_ERROR:
				msg_handler = p_event_mgr->error_handler;
				l_data = p_event_mgr->m_error_data;
				break;
			case EVT_WARNING:
				msg_handler = p_event_mgr->warning_handler;
				l_data = p_event_mgr->m_warning_data;
				break;
			case EVT_INFO:
				msg_handler = p_event_mgr->info_handler;
				l_data = p_event_mgr->m_info_data;
				break;
			default:
				break;
		}
		if(msg_handler == 00) {
			return mi_FALSE;
		}
	} else {
		return mi_FALSE;
	}

	if ((fmt != 00) && (p_event_mgr != 00)) {
		va_list arg;
		size_t str_length/*, i, j*/; /* UniPG */
		char message[mi_MSG_SIZE];
		memset(message, 0, mi_MSG_SIZE);
		/* initialize the optional parameter list */
		va_start(arg, fmt);
		/* check the length of the format string */
		str_length = (strlen(fmt) > mi_MSG_SIZE) ? mi_MSG_SIZE : strlen(fmt);
        (void)str_length;
		/* parse the format string and put the result in 'message' */
		vsnprintf(message, mi_MSG_SIZE, fmt, arg); /* UniPG */
		/* deinitialize the optional parameter list */
		va_end(arg);

		/* output the message to the user program */
		msg_handler(message, l_data);
	}

	return mi_TRUE;
}

void mi_set_default_event_handler(mi_event_mgr_t * p_manager)
{
	p_manager->m_error_data = 00;
	p_manager->m_warning_data = 00;
	p_manager->m_info_data = 00;
	p_manager->error_handler = mi_default_callback;
	p_manager->info_handler = mi_default_callback;
	p_manager->warning_handler = mi_default_callback;
}


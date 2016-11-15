
#ifndef __FUNCTION_LIST_H
#define __FUNCTION_LIST_H
#include"openjpeg.h"
#include"event.h"
/** 
 * @file function_list.h
 * @brief Implementation of a list of procedures.

 * The functions in validation.c aims to have access to a list of procedures.
*/

/** @defgroup VAL VAL - validation procedure*/
/*@{*/

/**************************************************************************************************
 ***************************************** FORWARD DECLARATION ************************************
 **************************************************************************************************/

/**
 * declare a function pointer
 */
typedef void (*mi_procedure)(void);

/**
 * A list of procedures.
*/
typedef struct mi_procedure_list 
{
	/**
	 * The number of validation procedures.
	 */
	mi_UINT32 m_nb_procedures;
	/**
	 * The number of the array of validation procedures.
	 */
	mi_UINT32 m_nb_max_procedures;
	/**
	 * The array of procedures.
	 */
	mi_procedure * m_procedures;

} mi_procedure_list_t;

/* ----------------------------------------------------------------------- */

/**
 * Creates a validation list.
 *
 * @return	the newly created validation list.
 */
mi_procedure_list_t *  mi_procedure_list_create(void);

/**
 * Destroys a validation list.
 *
 * @param p_list the list to destroy.
 */
void  mi_procedure_list_destroy(mi_procedure_list_t * p_list);

/**
 * Adds a new validation procedure.
 *
 * @param	p_validation_list the list of procedure to modify.
 * @param	p_procedure		the procedure to add.
 *
 * @return	mi_TRUE if the procedure could be added.
 */
mi_BOOL mi_procedure_list_add_procedure (mi_procedure_list_t * p_validation_list, mi_procedure p_procedure, mi_event_mgr_t* p_manager);

/**
 * Gets the number of validation procedures.
 *
 * @param	p_validation_list the list of procedure to modify.
 *
 * @return the number of validation procedures.
 */
mi_UINT32 mi_procedure_list_get_nb_procedures (mi_procedure_list_t * p_validation_list);

/**
 * Gets the pointer on the first validation procedure. This function is similar to the C++
 * iterator class to iterate through all the procedures inside the validation list.
 * the caller does not take ownership of the pointer.
 *
 * @param	p_validation_list the list of procedure to get the first procedure from.
 *
 * @return	a pointer to the first procedure.
 */
mi_procedure* mi_procedure_list_get_first_procedure (mi_procedure_list_t * p_validation_list);


/**
 * Clears the list of validation procedures.
 *
 * @param	p_validation_list the list of procedure to clear.
 *
 */
void mi_procedure_list_clear (mi_procedure_list_t * p_validation_list);
/*@}*/

#endif /* __FUNCTION_LIST_H */


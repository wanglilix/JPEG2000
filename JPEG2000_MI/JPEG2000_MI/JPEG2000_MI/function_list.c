
#include "mi_includes.h"

/**
 * Default size of the validation list, if not sufficient, data will be reallocated with a double size.
 */
#define mi_VALIDATION_SIZE 10

mi_procedure_list_t *  mi_procedure_list_create()
{
        /* memory allocation */
        mi_procedure_list_t * l_validation = (mi_procedure_list_t *) mi_calloc(1,sizeof(mi_procedure_list_t));
        if (! l_validation)
        {
                return 00;
        }
        /* initialization */
        l_validation->m_nb_max_procedures = mi_VALIDATION_SIZE;
        l_validation->m_procedures = (mi_procedure*)mi_calloc(mi_VALIDATION_SIZE, sizeof(mi_procedure));
        if (! l_validation->m_procedures)
        {
                mi_free(l_validation);
                return 00;
        }
        return l_validation;
}

void  mi_procedure_list_destroy(mi_procedure_list_t * p_list)
{
        if (! p_list)
        {
                return;
        }
        /* initialization */
        if (p_list->m_procedures)
        {
                mi_free(p_list->m_procedures);
        }
        mi_free(p_list);
}

mi_BOOL mi_procedure_list_add_procedure (mi_procedure_list_t * p_validation_list, mi_procedure p_procedure, mi_event_mgr_t* p_manager )
{
	
        assert(p_manager != NULL);
	
        if (p_validation_list->m_nb_max_procedures == p_validation_list->m_nb_procedures)
        {
                mi_procedure * new_procedures;

                p_validation_list->m_nb_max_procedures += mi_VALIDATION_SIZE;
                new_procedures = (mi_procedure*)mi_realloc(
                        p_validation_list->m_procedures,
                        p_validation_list->m_nb_max_procedures * sizeof(mi_procedure));
                if (! new_procedures)
                {
                        mi_free(p_validation_list->m_procedures);
                        p_validation_list->m_nb_max_procedures = 0;
                        p_validation_list->m_nb_procedures = 0;
                        mi_event_msg(p_manager, EVT_ERROR, "Not enough memory to add a new validation procedure\n");
                        return mi_FALSE;
                }
                else
                {
                        p_validation_list->m_procedures = new_procedures;
                }
        }
        p_validation_list->m_procedures[p_validation_list->m_nb_procedures] = p_procedure;
        ++p_validation_list->m_nb_procedures;

        return mi_TRUE;
}

mi_UINT32 mi_procedure_list_get_nb_procedures (mi_procedure_list_t * p_validation_list)
{
        return p_validation_list->m_nb_procedures;
}

mi_procedure* mi_procedure_list_get_first_procedure (mi_procedure_list_t * p_validation_list)
{
        return p_validation_list->m_procedures;
}

void mi_procedure_list_clear (mi_procedure_list_t * p_validation_list)
{
        p_validation_list->m_nb_procedures = 0;
}

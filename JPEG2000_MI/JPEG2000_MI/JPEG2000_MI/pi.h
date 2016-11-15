

#ifndef __PI_H
#define __PI_H
/**
@file pi.h
@brief Implementation of a packet iterator (PI)

The functions in PI.C have for goal to realize a packet iterator that permits to get the next
packet following the progression order and change of it. The functions in PI.C are used
by some function in T2.C.
*/

/** @defgroup PI PI - Implementation of a packet iterator */
/*@{*/

/**
FIXME DOC
*/
typedef struct mi_pi_resolution {
  mi_UINT32 pdx, pdy;
  mi_UINT32 pw, ph;
} mi_pi_resolution_t;

/**
FIXME DOC
*/
typedef struct mi_pi_comp {
  mi_UINT32 dx, dy;
  /** number of resolution levels */
  mi_UINT32 numresolutions;
  mi_pi_resolution_t *resolutions;
} mi_pi_comp_t;

/**
Packet iterator
*/
typedef struct mi_pi_iterator {
  /** Enabling Tile part generation*/
  mi_BYTE tp_on;
  /** precise if the packet has been already used (useful for progression order change) */
  mi_INT16 *include;
  /** layer step used to localize the packet in the include vector */
  mi_UINT32 step_l;
  /** resolution step used to localize the packet in the include vector */
  mi_UINT32 step_r;
  /** component step used to localize the packet in the include vector */
  mi_UINT32 step_c;
  /** precinct step used to localize the packet in the include vector */
  mi_UINT32 step_p;
  /** component that identify the packet */
  mi_UINT32 compno;
  /** resolution that identify the packet */
  mi_UINT32 resno;
  /** precinct that identify the packet */
  mi_UINT32 precno;
  /** layer that identify the packet */
  mi_UINT32 layno;
  /** 0 if the first packet */
  mi_BOOL first;
  /** progression order change information */
  mi_poc_t poc;
  /** number of components in the image */
  mi_UINT32 numcomps;
  /** Components*/
  mi_pi_comp_t *comps;
  /** FIXME DOC*/
  mi_INT32 tx0, ty0, tx1, ty1;
  /** FIXME DOC*/
  mi_INT32 x, y;
  /** FIXME DOC*/
  mi_UINT32 dx, dy;
} mi_pi_iterator_t;

/** @name Exported functions */
/*@{*/
/* ----------------------------------------------------------------------- */
/**
 * Creates a packet iterator for encoding.
 *
 * @param	image		the image being encoded.
 * @param	cp		the coding parameters.
 * @param	tileno	index of the tile being encoded.
 * @param	t2_mode	the type of pass for generating the packet iterator
 *
 * @return	a list of packet iterator that points to the first packet of the tile (not true).
*/
mi_pi_iterator_t *mi_pi_initialise_encode(const mi_image_t *image,
                                            mi_cp_t *cp,
                                            mi_UINT32 tileno,
                                            J2K_T2_MODE t2_mode);

/**
 * Updates the encoding parameters of the codec.
 *
 * @param	p_image		the image being encoded.
 * @param	p_cp		the coding parameters.
 * @param	p_tile_no	index of the tile being encoded.
*/
void mi_pi_update_encoding_parameters(	const mi_image_t *p_image,
                                        mi_cp_t *p_cp,
                                        mi_UINT32 p_tile_no );

/**
Modify the packet iterator for enabling tile part generation
@param pi Handle to the packet iterator generated in pi_initialise_encode
@param cp Coding parameters
@param tileno Number that identifies the tile for which to list the packets
@param pino   FIXME DOC
@param tpnum Tile part number of the current tile
@param tppos The position of the tile part flag in the progression order
@param t2_mode FIXME DOC
*/
void mi_pi_create_encode(  mi_pi_iterator_t *pi, 
                            mi_cp_t *cp,
                            mi_UINT32 tileno, 
                            mi_UINT32 pino,
                            mi_UINT32 tpnum, 
                            mi_INT32 tppos, 
                            J2K_T2_MODE t2_mode);

/**
Create a packet iterator for Decoder
@param image Raw image for which the packets will be listed
@param cp Coding parameters
@param tileno Number that identifies the tile for which to list the packets
@return Returns a packet iterator that points to the first packet of the tile
@see mi_pi_destroy
*/
mi_pi_iterator_t *mi_pi_create_decode(mi_image_t * image, 
                                        mi_cp_t * cp,
                                        mi_UINT32 tileno);
/**
 * Destroys a packet iterator array.
 *
 * @param	p_pi			the packet iterator array to destroy.
 * @param	p_nb_elements	the number of elements in the array.
 */
void mi_pi_destroy(mi_pi_iterator_t *p_pi,
                    mi_UINT32 p_nb_elements);

/**
Modify the packet iterator to point to the next packet
@param pi Packet iterator to modify
@return Returns false if pi pointed to the last packet or else returns true
*/
mi_BOOL mi_pi_next(mi_pi_iterator_t * pi);
/* ----------------------------------------------------------------------- */
/*@}*/

/*@}*/

#endif /* __PI_H */

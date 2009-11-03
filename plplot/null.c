/* $Id: null.c,v 1.3 2007/05/08 09:09:37 rice Exp $

	PLplot Null device driver.
*/
#include "plDevs.h"

#ifdef PLD_null

#include "plplotP.h"
#include "drivers.h"

/* Device info */
const char* plD_DEVICE_INFO_null = "null:Null device:-1:null:42:null";

/* pmr: defined in drivers.h */
/*void plD_dispatch_init_null	( PLDispatchTable *pdt );*/

void plD_init_null		(PLStream *);
void plD_line_null		(PLStream *, short, short, short, short);
void plD_polyline_null		(PLStream *, short *, short *, PLINT);
void plD_eop_null		(PLStream *);
void plD_bop_null		(PLStream *);
void plD_tidy_null		(PLStream *);
void plD_state_null		(PLStream *, PLINT);
void plD_esc_null		(PLStream *, PLINT, void *);

void plD_dispatch_init_null( PLDispatchTable *pdt )
{
#ifndef ENABLE_DYNDRIVERS
    pdt->pl_MenuStr  = "Null device";
    pdt->pl_DevName  = "null";
#endif
    pdt->pl_type     = plDevType_Null;
    pdt->pl_seq      = 42;
    pdt->pl_init     = (plD_init_fp)     plD_init_null;
    pdt->pl_line     = (plD_line_fp)     plD_line_null;
    pdt->pl_polyline = (plD_polyline_fp) plD_polyline_null;
    pdt->pl_eop      = (plD_eop_fp)      plD_eop_null;
    pdt->pl_bop      = (plD_bop_fp)      plD_bop_null;
    pdt->pl_tidy     = (plD_tidy_fp)     plD_tidy_null;
    pdt->pl_state    = (plD_state_fp)    plD_state_null;
    pdt->pl_esc      = (plD_esc_fp)      plD_esc_null;
}

/*--------------------------------------------------------------------------*\
 * plD_init_null()
 *
 * Initialize device (terminal).
\*--------------------------------------------------------------------------*/

void
plD_init_null(PLStream *pls)
{
    int xmin = 0;
    int xmax = PIXELS_X - 1;
    int ymin = 0;
    int ymax = PIXELS_Y - 1;

    PLFLT pxlx = (double) PIXELS_X / (double) LPAGE_X;
    PLFLT pxly = (double) PIXELS_Y / (double) LPAGE_Y;

    (void) pls;				/* pmr: make it used */

/* Set up device parameters */

    plP_setpxl(pxlx, pxly);
    plP_setphy(xmin, xmax, ymin, ymax);
}

/*--------------------------------------------------------------------------*\
 * The remaining driver functions are all null.
\*--------------------------------------------------------------------------*/

void
plD_line_null(PLStream *pls, short x1a, short y1a, short x2a, short y2a)
{
    (void) pls;				/* pmr: make it used */
    (void) x1a;
    (void) y1a;
    (void) x2a;
    (void) y2a;
}

void
plD_polyline_null(PLStream *pls, short *xa, short *ya, PLINT npts)
{
    (void) pls;				/* pmr: make it used */
    (void) xa;
    (void) ya;
    (void) npts;
}

void
plD_eop_null(PLStream *pls)
{
    (void) pls;				/* pmr: make it used */
}

void
plD_bop_null(PLStream *pls)
{
    (void) pls;				/* pmr: make it used */
}

void
plD_tidy_null(PLStream *pls)
{
    (void) pls;				/* pmr: make it used */
}

void 
plD_state_null(PLStream *pls, PLINT op)
{
    (void) pls;				/* pmr: make it used */
    (void) op;
}

void
plD_esc_null(PLStream *pls, PLINT op, void *ptr)
{
    (void) pls;				/* pmr: make it used */
    (void) op;
    (void) ptr;
}

#else
int 
pldummy_null(void)
{
    return 0;
}

#endif				/* PLD_nulldev */

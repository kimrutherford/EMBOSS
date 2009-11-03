/* 
	PLplot Staden Datafile device driver.  pmr: derived from the
	null device file because we don't neeed plplot to plot
	anything.
*/
#include "plDevs.h"

#ifdef PLD_data

#include "plplotP.h"
#include "drivers.h"

/* Device info */
const char* plD_DEVICE_INFO_data = "data:Staden datafile:-1:data:99:data";

/* pmr: defined in drivers.h */
/*void plD_dispatch_init_data	( PLDispatchTable *pdt );*/

void plD_init_data		(PLStream *);
void plD_line_data		(PLStream *, short, short, short, short);
void plD_polyline_data		(PLStream *, short *, short *, PLINT);
void plD_eop_data		(PLStream *);
void plD_bop_data		(PLStream *);
void plD_tidy_data		(PLStream *);
void plD_state_data		(PLStream *, PLINT);
void plD_esc_data		(PLStream *, PLINT, void *);

void plD_dispatch_init_data( PLDispatchTable *pdt )
{
#ifndef ENABLE_DYNDRIVERS
    pdt->pl_MenuStr  = "Staden datafile";
    pdt->pl_DevName  = "data";
#endif
    pdt->pl_type     = plDevType_Null;
    pdt->pl_seq      = 99;
    pdt->pl_init     = (plD_init_fp)     plD_init_data;
    pdt->pl_line     = (plD_line_fp)     plD_line_data;
    pdt->pl_polyline = (plD_polyline_fp) plD_polyline_data;
    pdt->pl_eop      = (plD_eop_fp)      plD_eop_data;
    pdt->pl_bop      = (plD_bop_fp)      plD_bop_data;
    pdt->pl_tidy     = (plD_tidy_fp)     plD_tidy_data;
    pdt->pl_state    = (plD_state_fp)    plD_state_data;
    pdt->pl_esc      = (plD_esc_fp)      plD_esc_data;
}

/*--------------------------------------------------------------------------*\
 * plD_init_data()
 *
 * Initialize device (terminal).
\*--------------------------------------------------------------------------*/

void
plD_init_data(PLStream *pls)
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
 * The remaining driver functions are all data.
\*--------------------------------------------------------------------------*/

void
plD_line_data(PLStream *pls, short x1a, short y1a, short x2a, short y2a)
{
    (void) pls;				/* pmr: make it used */
    (void) x1a;
    (void) y1a;
    (void) x2a;
    (void) y2a;
}

void
plD_polyline_data(PLStream *pls, short *xa, short *ya, PLINT npts)
{
    (void) pls;				/* pmr: make it used */
    (void) xa;
    (void) ya;
    (void) npts;
}

void
plD_eop_data(PLStream *pls)
{
    (void) pls;				/* pmr: make it used */
}

void
plD_bop_data(PLStream *pls)
{
    (void) pls;				/* pmr: make it used */
}

void
plD_tidy_data(PLStream *pls)
{
    (void) pls;				/* pmr: make it used */
}

void 
plD_state_data(PLStream *pls, PLINT op)
{
    (void) pls;				/* pmr: make it used */
    (void) op;
}

void
plD_esc_data(PLStream *pls, PLINT op, void *ptr)
{
    (void) pls;				/* pmr: make it used */
    (void) op;
    (void) ptr;
}

#else
int 
pldummy_data(void)
{
    return 0;
}

#endif				/* PLD_datadev */

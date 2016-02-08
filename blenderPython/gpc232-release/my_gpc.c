#include "gpc.h"
#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include <math.h>


/*
===========================================================================
                                Constants
===========================================================================
*/

#ifndef TRUE
#define FALSE              0
#define TRUE               1
#endif

#define LEFT               0
#define RIGHT              1

#define ABOVE              0
#define BELOW              1

#define CLIP               0
#define SUBJ               1

#define INVERT_TRISTRIPS   FALSE


/*
===========================================================================
                                 Macros 
===========================================================================
*/

#define EQ(a, b)           (fabs((a) - (b)) <= GPC_EPSILON)

#define PREV_INDEX(i, n)   ((i - 1 + n) % n)
#define NEXT_INDEX(i, n)   ((i + 1    ) % n)

#define OPTIMAL(v, i, n)   ((v[PREV_INDEX(i, n)].y != v[i].y) || \
                            (v[NEXT_INDEX(i, n)].y != v[i].y))

#define FWD_MIN(v, i, n)   ((v[PREV_INDEX(i, n)].vertex.y >= v[i].vertex.y) \
                         && (v[NEXT_INDEX(i, n)].vertex.y > v[i].vertex.y))

#define NOT_FMAX(v, i, n)   (v[NEXT_INDEX(i, n)].vertex.y > v[i].vertex.y)

#define REV_MIN(v, i, n)   ((v[PREV_INDEX(i, n)].vertex.y > v[i].vertex.y) \
                         && (v[NEXT_INDEX(i, n)].vertex.y >= v[i].vertex.y))

#define NOT_RMAX(v, i, n)   (v[PREV_INDEX(i, n)].vertex.y > v[i].vertex.y)

#define VERTEX(e,p,s,x,y)  {add_vertex(&((e)->outp[(p)]->v[(s)]), x, y); \
                            (e)->outp[(p)]->active++;}

#define P_EDGE(d,e,p,i,j)  {(d)= (e); \
                            do {(d)= (d)->prev;} while (!(d)->outp[(p)]); \
                            (i)= (d)->bot.x + (d)->dx * ((j)-(d)->bot.y);}

#define N_EDGE(d,e,p,i,j)  {(d)= (e); \
                            do {(d)= (d)->next;} while (!(d)->outp[(p)]); \
                            (i)= (d)->bot.x + (d)->dx * ((j)-(d)->bot.y);}

#define MALLOC(p, b, s, t) {if ((b) > 0) { \
                            p= (t*)malloc(b); if (!(p)) { \
                            fprintf(stderr, "gpc malloc failure: %s\n", s); \
                            exit(0);}} else p= NULL;}

#define FREE(p)            {if (p) {free(p); (p)= NULL;}}


/*
===========================================================================
                            Private Data Types
===========================================================================
*/

typedef enum                        /* Edge intersection classes         */
{
  NUL,                              /* Empty non-intersection            */
  EMX,                              /* External maximum                  */
  ELI,                              /* External left intermediate        */
  TED,                              /* Top edge                          */
  ERI,                              /* External right intermediate       */
  RED,                              /* Right edge                        */
  IMM,                              /* Internal maximum and minimum      */
  IMN,                              /* Internal minimum                  */
  EMN,                              /* External minimum                  */
  EMM,                              /* External maximum and minimum      */
  LED,                              /* Left edge                         */
  ILI,                              /* Internal left intermediate        */
  BED,                              /* Bottom edge                       */
  IRI,                              /* Internal right intermediate       */
  IMX,                              /* Internal maximum                  */
  FUL                               /* Full non-intersection             */
} vertex_type;

typedef enum                        /* Horizontal edge states            */
{
  NH,                               /* No horizontal edge                */
  BH,                               /* Bottom horizontal edge            */
  TH                                /* Top horizontal edge               */
} h_state;

typedef enum                        /* Edge bundle state                 */
{
  UNBUNDLED,                        /* Isolated edge not within a bundle */
  BUNDLE_HEAD,                      /* Bundle head node                  */
  BUNDLE_TAIL                       /* Passive bundle tail node          */
} bundle_state;

typedef struct v_shape              /* Internal vertex list datatype     */
{
  double              x;            /* X coordinate component            */
  double              y;            /* Y coordinate component            */
  struct v_shape     *next;         /* Pointer to next vertex in list    */
} vertex_node;

typedef struct p_shape              /* Internal contour / tristrip type  */
{
  int                 active;       /* Active flag / vertex count        */
  int                 hole;         /* Hole / external contour flag      */
  vertex_node        *v[2];         /* Left and right vertex list ptrs   */
  struct p_shape     *next;         /* Pointer to next polygon contour   */
  struct p_shape     *proxy;        /* Pointer to actual structure used  */
} polygon_node;

typedef struct edge_shape
{
  gpc_vertex          vertex;       /* Piggy-backed contour vertex data  */
  gpc_vertex          bot;          /* Edge lower (x, y) coordinate      */
  gpc_vertex          top;          /* Edge upper (x, y) coordinate      */
  double              xb;           /* Scanbeam bottom x coordinate      */
  double              xt;           /* Scanbeam top x coordinate         */
  double              dx;           /* Change in x for a unit y increase */
  int                 type;         /* Clip / subject edge flag          */
  int                 bundle[2][2]; /* Bundle edge flags                 */
  int                 bside[2];     /* Bundle left / right indicators    */
  bundle_state        bstate[2];    /* Edge bundle state                 */
  polygon_node       *outp[2];      /* Output polygon / tristrip pointer */
  struct edge_shape  *prev;         /* Previous edge in the AET          */
  struct edge_shape  *next;         /* Next edge in the AET              */
  struct edge_shape  *pred;         /* Edge connected at the lower end   */
  struct edge_shape  *succ;         /* Edge connected at the upper end   */
  struct edge_shape  *next_bound;   /* Pointer to next bound in LMT      */
} edge_node;

typedef struct lmt_shape            /* Local minima table                */
{
  double              y;            /* Y coordinate at local minimum     */
  edge_node          *first_bound;  /* Pointer to bound list             */
  struct lmt_shape   *next;         /* Pointer to next local minimum     */
} lmt_node;

typedef struct sbt_t_shape          /* Scanbeam tree                     */
{
  double              y;            /* Scanbeam node y value             */
  struct sbt_t_shape *less;         /* Pointer to nodes with lower y     */
  struct sbt_t_shape *more;         /* Pointer to nodes with higher y    */
} sb_tree;

typedef struct it_shape             /* Intersection table                */
{
  edge_node          *ie[2];        /* Intersecting edge (bundle) pair   */
  gpc_vertex          point;        /* Point of intersection             */
  struct it_shape    *next;         /* The next intersection table node  */
} it_node;

typedef struct st_shape             /* Sorted edge table                 */
{
  edge_node          *edge;         /* Pointer to AET edge               */
  double              xb;           /* Scanbeam bottom x coordinate      */
  double              xt;           /* Scanbeam top x coordinate         */
  double              dx;           /* Change in x for a unit y increase */
  struct st_shape    *prev;         /* Previous edge in sorted list      */
} st_node;

typedef struct bbox_shape           /* Contour axis-aligned bounding box */
{
  double             xmin;          /* Minimum x coordinate              */
  double             ymin;          /* Minimum y coordinate              */
  double             xmax;          /* Maximum x coordinate              */
  double             ymax;          /* Maximum y coordinate              */
} bbox;


/*
===========================================================================
                               Global Data
===========================================================================
*/
FILE *fp;
int num_polygons;
int generated_polygons;
int read_hole_flags;

void gpc_read_init(FILE *filepointer, int will_read_hole_flags){
  fp = filepointer;
  fscanf(fp, "%d", &(num_polygons));
  
  printf("%d\n", num_polygons);
  generated_polygons = 0;
  read_hole_flags = will_read_hole_flags;
}

int gpc_get_num_polygons() {
	return num_polygons;
}

gpc_polygon *gpc_read_leaf_gen(){
  gpc_polygon* poly;
  int v;

  if (generated_polygons >= num_polygons) return NULL;

  MALLOC(poly, sizeof(gpc_polygon), "allocating accumulated gpc", gpc_polygon);
  poly->num_contours = 1;
  MALLOC(poly->hole, poly->num_contours * sizeof(int),
         "hole flag array creation", int);
  MALLOC(poly->contour, poly->num_contours
         * sizeof(gpc_vertex_list), "contour creation", gpc_vertex_list);
  fscanf(fp, "%d", &(poly->contour[0].num_vertices));
  // printf("%d\n", poly->contour[0].num_vertices);

  if (read_hole_flags)
    fscanf(fp, "%d", &(poly->hole[0]));
  else
    poly->hole[0]= FALSE; /* Assume all contours to be external */

  MALLOC(poly->contour[0].vertex, poly->contour[0].num_vertices
         * sizeof(gpc_vertex), "vertex creation", gpc_vertex);
  for (v= 0; v < poly->contour[0].num_vertices; v++)
    fscanf(fp, "%lf %lf", &(poly->contour[0].vertex[v].x),
                          &(poly->contour[0].vertex[v].y));
  generated_polygons = 0;
  return poly;
}

// start inclusive, end exclusive
gpc_polygon *gpc_read_all_polygon_recursive(int start, int end)
{
  int split;
  gpc_polygon *first;
  gpc_polygon * second;
  gpc_polygon * merged;
  MALLOC(merged, sizeof(gpc_polygon), "allocating accumulated gpc", gpc_polygon);

  if (end - start == 2) {
  	first = gpc_read_leaf_gen();
  	second = gpc_read_leaf_gen();
  	gpc_polygon_clip(GPC_UNION, first, second, merged);
  	gpc_free_polygon(first);
    gpc_free_polygon(second);
    FREE(first);
    FREE(second);
  	return merged;
  } else if (end - start == 1){
  	return gpc_read_leaf_gen();
  } else {
  	split = (start + end) / 2;
  	first = gpc_read_all_polygon_recursive(start, split);
  	second = gpc_read_all_polygon_recursive(split, end);
  	gpc_polygon_clip(GPC_UNION, first, second, merged);
  	gpc_free_polygon(first);
    gpc_free_polygon(second);
    FREE(first);
    FREE(second);
  	return merged;
  }
}

void constructRect(gpc_polygon* p, double width, double length) {
  p->num_contours = 1;
  MALLOC(p->hole, p->num_contours * sizeof(int),
         "hole flag array creation", int);
  MALLOC(p->contour, p->num_contours
         * sizeof(gpc_vertex_list), "contour creation", gpc_vertex_list);
  p->contour[0].num_vertices = 4;
  MALLOC(p->contour[0].vertex, p->contour[0].num_vertices
         * sizeof(gpc_vertex), "vertex creation", gpc_vertex);
  p->contour[0].vertex[0].x = 0;
  p->contour[0].vertex[0].y = 0;
  p->contour[0].vertex[1].x = width;
  p->contour[0].vertex[1].y = 0;
  p->contour[0].vertex[2].x = width;
  p->contour[0].vertex[2].y = length;
  p->contour[0].vertex[3].x = 0;
  p->contour[0].vertex[3].y = length;
}
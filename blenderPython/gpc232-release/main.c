#include <stdio.h>
#include <stdlib.h>
#include "gpc.h"
#include <float.h>
#include <math.h>

#define FREE(p)            {if (p) {free(p); (p)= NULL;}}

int main(void)
{
  gpc_polygon* union_result;
  gpc_polygon raw_material, result;
  FILE *sfp, *ofp;
  double length = 1023.98;
  double width = 18.73;

  // Get Union results of the support materials
  sfp= fopen("blender_uv_output.out.out", "r");
  gpc_read_init(sfp, 1);
  union_result = gpc_read_all_polygon_recursive(0, gpc_get_num_polygons());

  // Construct the shape of the raw band material
  constructRect(&raw_material, width, length);

  gpc_polygon_clip(GPC_DIFF, &raw_material, union_result, &result);
  ofp= fopen("outfile", "w");
  gpc_write_polygon(ofp, 1, union_result);

  gpc_free_polygon(union_result);
  gpc_free_polygon(&result);
  FREE(union_result);
  fclose(sfp);
  fclose(ofp);
  return 0;
}

// int main(void)
// {
//   gpc_polygon* result;
//   // gpc_polygon raw_material;
//   FILE *sfp, *ofp;

//   sfp= fopen("BlueFeltRawPython.out", "r");
//   result = gpc_read_all_polygon(sfp);

//   ofp= fopen("outfile", "w");
//   gpc_write_polygon(ofp, 1, result);

//   // raw_material.num_contours = 1;
//   // MALLOC(raw_material.hole, raw_material.num_contours * sizeof(int),
//   //        "hole flag array creation", int);
//   // MALLOC(raw_material.contour, raw_material.num_contours
//   //        * sizeof(gpc_vertex_list), "contour creation", gpc_vertex_list);
//   // raw_material.contour[c].num_vertices = 4;

//   // MALLOC(raw_material.contour[c].vertex, raw_material.contour[c].num_vertices
//   //        * sizeof(gpc_vertex), "vertex creation", gpc_vertex);
//   // for (v= 0; v < raw_material.contour[c].num_vertices; v++)
//   //   fscanf(fp, "%lf %lf", &(raw_material.contour[c].vertex[v].x),
//   //                         &(raw_material.contour[c].vertex[v].y));


//   gpc_free_polygon(result);
//   FREE(result);
//   fclose(sfp);
//   fclose(ofp);
//   return 0;
// }

// int main(void)
// {
//   gpc_polygon subject, clip, result;
//   FILE *sfp, *cfp, *ofp;

//   sfp= fopen("subjfile", "r");
//   cfp= fopen("clipfile", "r");
//   gpc_read_polygon(sfp, 0, &subject);
//   gpc_read_polygon(cfp, 0, &clip);

//   gpc_polygon_clip(GPC_UNION, &subject, &clip, &result);

//   ofp= fopen("outfile", "w");
//   gpc_write_polygon(ofp, 0, &result);

//   gpc_free_polygon(&subject);
//   gpc_free_polygon(&clip);
//   gpc_free_polygon(&result);

//   fclose(sfp);
//   fclose(cfp);
//   fclose(ofp);
//   return 0;
// }

#ifndef __DRAW_H__
#define __DRAW_H__

#include "random.h"

typedef struct point {
      float x;
      float y;
} point;

typedef struct branch {
      point left;
      point right;
} branch;

#define NUM_PARAMS (sizeof(tree_params) / sizeof(float))

typedef struct tree_params {
   float height;
   float trunk_height;
   float width;
   float trunk_width;
   float fork;
   float fork_var;
   float straight_prob;
   float angles;
   float angles_var;
   float angle_up;
   float angle_down;
   float angle_hor;
   float branch_angle_correlation;
   float branch_fork_correlation;
   float branch_endlen_correlation;
   float leaf_distr;
   float leaf_length;
   float leaf_width;
   float leaf_angle_mult;
   float leaf_shape;
   float leaf_var;
} tree_params;

static char *tree_param_labels[] = {
   "height",
   "trunk_height",
   "width",
   "trunk_width",
   "fork",
   "fork_var",
   "straight_prob",
   "angles",
   "angles_var",
   "angle_up",
   "angle_down",
   "angle_hor",
   "branch_angle_correlation",
   "branch_fork_correlation",
   "branch_endlen_correlation",
   "leaf_distr",
   "leaf_length",
   "leaf_width",
   "leaf_angle_mult",
   "leaf_shape",
   "leaf_var",
};

void drawing_init();
void render_frame();
branch draw_tree (long, float x, float y, float angle, float);
void toggle_genome_bit(int bit_num);
void set_param(float);

tree_params *tree_params_p;
char *get_params_str();

long tree_seed;

#endif /*__DRAW_H__*/



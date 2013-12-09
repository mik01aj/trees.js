#include <GL/gl.h>
#include <math.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#include "draw.h"
#include "random.h"

#define PI 3.1415


void drawing_init() {
   tree_params_p = calloc(sizeof(tree_params)/sizeof(float), sizeof(float));
   tree_params_p->height                  = 0.4;
   tree_params_p->trunk_height            = 0.1;
   tree_params_p->width                   = 0.1;
   tree_params_p->angles                  = 0.5;
   tree_params_p->fork        = 0.2;
   tree_params_p->branch_angle_correlation = 0.5;
   tree_params_p->leaf_distr              = 0.5;
   tree_params_p->leaf_length             = 0.3;
   tree_params_p->leaf_width              = 0.7;
   tree_params_p->leaf_shape              = 0.5;
}

char *get_params_str() {
   static char* buf;
   if (buf == NULL)
      buf = malloc(256);
      float* params = (float*) tree_params_p;
   int i, buf_i = 2;
   buf[0] = '{';
   buf[1] = ' ';
   for (i=0; i<sizeof(tree_params)/sizeof(float); i++)
      buf_i += snprintf(buf+buf_i, 256-buf_i, "%.3g, ", params[i]);
   buf[buf_i-2] = ' ';
   buf[buf_i-1] = '}';
   return buf;
}

void render_frame ()
{
   glClearColor(0, 0, 0, 1);
   glClear(GL_COLOR_BUFFER_BIT);

//   draw_tree(tree_seed-10000, 25, 95, 0, 1.0);
   draw_tree(tree_seed, 50, 95, 0, 1.0);
//   draw_tree(tree_seed+10000, 75, 95, 0, 1.0);
}

float interpolate(float a, float b, float alpha) {
   return a*(1-alpha) + b*alpha;
}

branch draw_tree(long r_seed, float x, float y, float angle, float width)
{
   float ay, ax, len, w, fork_proportion;
   int i;
   branch my_branch, c1, c2;
   static int depth, iters;

   assert(width > 0);

   seed(r_seed);

   len = 30 * tree_params_p->height
            * interpolate(0.1+width, -3*width*(width-1),
                          tree_params_p->branch_endlen_correlation);

   if (width == 1) {
      depth = 0;
      iters = 0;
      len = 100 * tree_params_p->height * tree_params_p->trunk_height;
   }
   else {
      assert(iters < 50000 && depth < 1000);
   }
   w = width * tree_params_p->width * 10;

   ay = sin(angle);
   ax = -cos(angle);
   {
      float w2 = (width == 1 ? w + tree_params_p->trunk_width*10 : w);
      my_branch.left.x = x+ax*w2;
      my_branch.left.y = y-ay*w2;
      my_branch.right.x = x-ax*w2;
      my_branch.right.y = y+ay*w2;
   }

   if (width < 0.001)
      return my_branch;

   // moving x and y to new position
   x += ay * len;
   y += ax * len;

   {
      float mean = tree_params_p->fork*5
                 * ((rand_uniform() > 0.5) ? 1 : -1);
      float var = tree_params_p->fork_var*5;
      float mult = interpolate(1, width, tree_params_p->branch_fork_correlation);
      fork_proportion = atan(normalvariate(mean, var)*mult)/PI + 0.5;
   }

   int straight = (rand_uniform() < tree_params_p->straight_prob);

   for (i=0; i<2; i++) {
      int side = straight ? 0 : i*2 - 1; // 1 or -1
      float other_branch_p = straight ? 0.1 :
         (side==1 ? fork_proportion : 1-fork_proportion);
      float new_width = width * (1-other_branch_p);
      float angle_mult = pow(1-new_width, pow(tree_params_p->branch_angle_correlation, 3)*10);
      float angle_mean = side * tree_params_p->angles;
      float angle_diff = normalvariate(angle_mean,
                                       tree_params_p->angles_var);
      float new_angle = angle + atan(angle_mult * angle_diff);
      float new_seed;

      new_angle += tree_params_p->angle_down * sin(new_angle)   * pow(1-width, 5);
      new_angle -= tree_params_p->angle_up   * sin(new_angle)   * pow(1-width, 10);
      new_angle += tree_params_p->angle_hor  * sin(new_angle*2) * pow(1-width, 15);

      seed(r_seed);
      chseed(i*12345);
      new_seed = rand_uniform();

      depth++;
      if (i==0) {
         c1 = draw_tree(*(long*)&new_seed,
                        x + ax*w*other_branch_p,
                        y - ay*w*other_branch_p,
                        new_angle,
                        new_width);
         if (straight)
            break;
      }
      else
         c2 = draw_tree(*(long*)&new_seed,
                        x - ax*w*other_branch_p,
                        y + ay*w*other_branch_p,
                        new_angle,
                        new_width);
      depth--;

      iters++;
   }

   {
      float v = 1;// - (float)depth/20;
      glColor3f(v, v, v);
   }
   glBegin(w>0.08 ? GL_POLYGON : GL_LINE_LOOP);
      glVertex2f(my_branch.right.x, my_branch.right.y);
      glVertex2f(my_branch.left.x, my_branch.left.y);
      glVertex2f(c1.left.x, c1.left.y);
      glVertex2f(c1.right.x, c1.right.y);
      if (!straight) {
         glVertex2f(c2.left.x, c2.left.y);
         glVertex2f(c2.right.x, c2.right.y);
      }
   glEnd();




   seed(r_seed);
   chseed(1234);

   if (width + rand_uniform() < tree_params_p->leaf_distr) {
      float leaf_dist = 1/(0.01+tree_params_p->leaf_distr);
      for (i=0; i<1; i++) {
         int side = ((i)%2)*2 - 1;
         float lx = x - ay*i*leaf_dist + side*ax*w;
         float ly = y - ax*i*leaf_dist - side*ay*w;
         float la = angle + side*0.4 * tree_params_p->leaf_angle_mult;
         float lax = sin(la);
         float lay = -cos(la);
         float ll = tree_params_p->leaf_length*5.0;
         float lw = normalvariate(tree_params_p->leaf_width, tree_params_p->leaf_var)*ll/2;
         float ls = normalvariate(tree_params_p->leaf_shape, tree_params_p->leaf_var);
         assert(side == 1 || side == -1);

         glColor3f(normalvariate(0.1, 0.07),
                   normalvariate(0.7, 0.07),
                   normalvariate(0.2, 0.07));
//         glColor3f(1,1,1);
         glBegin(GL_QUADS);
            glVertex2f(lx,                      ly);
            glVertex2f(lx + lax*ll*ls + lay*lw, ly + lay*ll*ls - lax*lw);
            glVertex2f(lx + lax*ll,             ly + lay*ll);
            glVertex2f(lx + lax*ll*ls - lay*lw, ly + lay*ll*ls + lax*lw);
         glEnd();

         iters++;
      }
   }

   return my_branch;
}

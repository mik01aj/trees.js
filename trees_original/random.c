#include <math.h>
#include <assert.h>

#include "random.h"

// most of the code here was translated from python module random

// 4 * _exp(-0.5)/_sqrt(2.0)
#define NV_MAGICCONST 1.7155277699214135

unsigned long seed_x=0, seed_y=0, seed_z=0;

// Normal distribution.
// mu = mean, sigma = standard deviation
float normalvariate(float mu, float sigma) {
   // Uses Kinderman and Monahan method. Reference: Kinderman,
   // A.J. and Monahan, J.F., "Computer generation of random
   // variables using the ratio of uniform deviates", ACM Trans
   // Math Software, 3, (1977), pp257-260.
   float u1, u2, z, zz;

   do {
      u1 = rand_uniform();
      u2 = 1.0 - rand_uniform();
      z = NV_MAGICCONST*(u1-0.5)/u2;
      zz = z*z/4.0;
   }
   while (zz > -log(u2));

   return mu + z*sigma;
}

float normalvariate2(distr_params params) {
   return normalvariate(params.mean, params.var);
}


//  Get the next random number in the range [0.0, 1.0).
float rand_uniform() {
   assert(seed_x != 0);
   assert(seed_y != 0);
   assert(seed_z != 0);
   // Wichman-Hill random number generator.
   //
   // Wichmann, B. A. & Hill, I. D. (1982)
   // Algorithm AS 183:
   // An efficient and portable pseudo-random number generator
   // Applied Statistics 31 (1982) 188-190
   //
   // see also:
   //        Correction to Algorithm AS 183
   //        Applied Statistics 33 (1984) 123
   //
   //        McLeod, A. I. (1985)
   //        A remark on Algorithm AS 183
   //        Applied Statistics 34 (1985),198-200

   // This part is thread-unsafe:
   // BEGIN CRITICAL SECTION
   seed_x = (171 * seed_x) % 30269;
   seed_y = (172 * seed_y) % 30307;
   seed_z = (170 * seed_z) % 30323;
   // END CRITICAL SECTION

   // Note: on a platform using IEEE-754 double arithmetic, this can
   // never return 0.0 (asserted by Tim; proof too long for a comment).
   return fmod(seed_x/30269.0 + seed_y/30307.0 + seed_z/30323.0, 1.0);
}

// Initialize internal state. Distinct values between
// 0 and 27814431486575L inclusive are guaranteed to yield distinct
// internal states (this guarantee is specific to the default
// Wichmann-Hill generator).
void seed(unsigned long s) {
   seed_x = s%30268 + 1;
   s /= 30268;
   seed_y = s%30306 + 1;
   s /= 30306;
   seed_z = s%30322 + 1;
}


void chseed(unsigned long s) {
   seed_y = (seed_y+s)%30306 + 1;
   s /= 30306;
   seed_z = (seed_z+s)%30322 + 1;
   s /= 30322;
   seed_x = (seed_x+s)%30268 + 1;

}

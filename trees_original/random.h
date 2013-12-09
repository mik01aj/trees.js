#ifndef __RANDOM_H__
#define __RANDOM_H__

typedef struct distr_params {
      float mean;
      float var;
} distr_params;

float normalvariate(float mu, float sigma);
float normalvariate2(distr_params params);
float rand_uniform();
void seed(unsigned long s);
void chseed(unsigned long s);

#endif /*__RANDOM_H__*/

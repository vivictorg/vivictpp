#include "Resolution.hh"

bool operator== (const Resolution &r1, const Resolution &r2) {
  return r1.w == r2.w && r1.h == r2.h;
}

bool operator!= (const Resolution &r1, const Resolution &r2) {
  return r1.w != r2.w || r1.h != r2.h;
}

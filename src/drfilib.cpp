#include "SalDRFI.h"

SalDRFI drfi_instance;
void drfiinit(CStr & data) {
  drfi_instance.load(data);
}
Mat drfiprocessImg(Mat& img) {
  Mat sal1f = drfi_instance.getSalMap(img);
  return sal1f;
}
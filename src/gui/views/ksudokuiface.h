

#ifndef _KSUDOKUIFACE_H_
#define _KSUDOKUIFACE_H_

#include <dcopobject.h>

class ksudokuIface : virtual public DCOPObject
{
  K_DCOP
public:

k_dcop:
  virtual void openURL(QString url) = 0;
};

#endif // _KSUDOKUIFACE_H_

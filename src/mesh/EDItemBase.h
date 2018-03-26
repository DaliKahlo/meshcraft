//$c1   XRL 10/7/2011 Added.
//========================================================================//
//              Copyright 2011 (Unpublished Material)                     //
//                  MSC Software Corp.                                    //
//========================================================================//
/*
  EDItemBase.h

  Base class for embedded doubly-linked lists 
  Separated from EDList.h. A mesh entity is dervived from
  EdItemBase, and since that include file is used all over
  making it as small as possible improves performance.


  $Id

*/

#ifndef H_EDItemBase
#define H_EDItemBase


class EDListBase;
class EDListIterBase;

class EDItemBase {
  friend class EDListBase;
  friend class EDListIterBase;
public:
  EDItemBase() { prev=0; next=0; }
  virtual ~EDItemBase() {}
private:
  EDItemBase *prev;
  EDItemBase *next;
};


#endif

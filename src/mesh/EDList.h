//$c1   XRL 10/7/2011 Added.
//========================================================================//
//              Copyright 2011 (Unpublished Material)                     //
//                  MSC Software Corp.                                    //
//========================================================================//
/*
  EDList.h

  List class for embedded doubly-linked lists - items which are
  used are derived from the EDItemBase class

  Created Wed Nov 26 11:45:33 EST 1997, JDT

  $Id: EDList.h,v 1.5 2000/08/05 19:10:07 oklaas Exp $

  Modification History:
  05/03/2000 JDT  Added list merge operation
*/

#ifndef H_EDList
#define H_EDList

#include "EDItemBase.h"


class EDListIterBase;

/** List class for embedded doubly-linked lists. Items in list must
  be derived from the EDItemBase class. The list owns everything in
  it, so when the list is deleted so is everything in the list */
class EDListBase {
  friend class EDItemBase;
  friend class EDListIterBase;
public:
  void append(EDItemBase *item);
  void remove(EDItemBase *item);
  EDItemBase *nth(int n) const;
  inline int size() const { return list_size; }
  void mergeList(EDListBase *other);
protected:
  EDListBase();
  ~EDListBase();

private:
  void addIter(EDListIterBase *iter);
  void removeIter(EDListIterBase *iter);

  EDItemBase *first;
  EDItemBase *last;
  int list_size;
  EDListIterBase **activeIters;
  int nActive;
  int nAlloc;
};

template<class T> 
class EDList : private EDListBase {
public:
  EDList()
    : EDListBase() {}
  ~EDList() {}

  void append(T *item)
  { EDListBase::append(item); }
  void remove(T *item)
  { EDListBase::remove(item); }
  T * nth(int n) const
  { return (T*)EDListBase::nth(n); }
  int size() const
  { return EDListBase::size(); }
  void mergeList(EDList<T> *other)
  { EDListBase::mergeList(other); }
};

class EDListIterBase {
  friend class EDListBase;
  friend class EDItemBase;
public:

  void operator=(const EDListIterBase &iter);
  int operator()(EDItemBase *&);
  void reset();
  EDItemBase *next();
  EDItemBase *current() const;
  EDListBase * list() const;
protected:
  EDListIterBase();
  EDListIterBase(const EDListBase *list);
  EDListIterBase(const EDListIterBase & iter);
  ~EDListIterBase();

private:
  const EDListBase *d_list;
  EDItemBase *d_current;
};

template<class T>
class EDListIter : private EDListIterBase {
public:
  EDListIter() : EDListIterBase() {};
  EDListIter(const EDList<T> *list)
    : EDListIterBase( (EDListBase*)list ) {}
  EDListIter(const EDListIter<T> & iter)
    : EDListIterBase(iter) {}
  ~EDListIter() {}

  void operator=(const EDListIter<T> & iter)
  { EDListIterBase::operator=(iter); }

  int operator()(T *&item)
  { return EDListIterBase::operator()(((EDItemBase*&)item)); }
  
  T * next()
  { return (T*)EDListIterBase::next(); }

  T * current() const
  { return (T*)EDListIterBase::current(); }

  EDList<T> * list() const
  { return (EDList<T>*)EDListIterBase::list(); }

  void reset()
  { EDListIterBase::reset(); }
};


#endif

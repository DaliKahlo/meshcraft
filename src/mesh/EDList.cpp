//$c1   XRL 10/7/2011 Added.
//========================================================================//
//              Copyright 2011 (Unpublished Material)                     //
//                  MSC Software Corp.                                    //
//========================================================================//
/*
  EDListBase.cc

  implementation of "embedded doubly-linked" lists of EDItemBase
  derived class items

  Created Wed Nov 26 11:47:32 EST 1997

  $Id: EDList.cc,v 1.9 2000/08/05 19:23:06 oklaas Exp $ 

  Modification History:
  05/03/2000 JDT  Added merge list operator
*/

#include "EDList.h"
//#include "MessageOut.h"


const int activeIterAllocSize = 10;

EDListBase::EDListBase() {

  first=0;
  last=0;
  list_size=0;
  activeIters=new EDListIterBase*[activeIterAllocSize];
  for (int i=0; i<activeIterAllocSize; i++) 
    activeIters[i]=0;
  nActive=0;
  nAlloc=activeIterAllocSize;
}

EDListBase::~EDListBase() {
 if (nActive) {
#ifdef DEBUG
     Warning("EDList::~EDList: deleting list with " << nActive << " active iterators");
#endif
  }
  // do we want to delete any iterators now or assume someone else will?
  if (nAlloc && activeIters)
    delete [] activeIters;
  // now delete everything in the list
  EDItemBase *item = first;
  EDItemBase *next = first;
  while(item){
    next= item->next;
    delete item;
    item = next;
  }
}

void EDListBase::addIter(EDListIterBase *iter)
{
  int i;
  // make sure there will be room to store
  if (nAlloc == nActive) {
    // make it bigger
    EDListIterBase **newactiveIters=new EDListIterBase*[2*nAlloc];
    for (i=0; i<nAlloc; i++) {
      newactiveIters[i]=activeIters[i];
      newactiveIters[i+nAlloc]=0;
    }
    delete [] activeIters;
    activeIters=newactiveIters;
    nAlloc*=2;
  }
  // save this iter in the list of active iterators
  for (i=0; i<nAlloc; i++) {
    if (activeIters[i] == 0) {
      activeIters[i] = iter;
      nActive++;
      return;
    }
  }
  //InternalError("EDListBase::addIter: should have space to store iter but don't");
}

void EDListBase::removeIter(EDListIterBase *iter)
{

  for (int i=0; i<nAlloc; i++) {
    if (activeIters[i] == iter) {
      activeIters[i]=0;
      nActive--;
      return;
    }
  }
  
  //Error("EDListBase::removeIter: iter not found in active iter list");
}

void EDListBase::append(EDItemBase *item) {
  EDItemBase *oldlast;

  if (!first) {
    first=item;
    last=item;
  }
  else {
    oldlast=last;
    oldlast->next=item;
    item->prev=oldlast;
    last=item;
  }

  // if there are iterators, we need to make sure they know about
  // the newly added item, if necessary
  if (nActive) {
    int i;
    for (i=0; i<nAlloc; i++) {
      EDListIterBase *iter=activeIters[i];
      if (iter && !iter->d_current) {
	// we appended something to the end of the list being traversed
	// by an iterator which has expired but not yet been deleted
	// need to "unexpire".  If more is appended it will be after this
	// item and we don't need any special consideration
	iter->d_current=item;
      }
    }
  }
  list_size++;
}

void EDListBase::remove(EDItemBase *item) {
  int i;
  EDListIterBase *iter;
  EDItemBase *oldprev = item->prev;
  EDItemBase *oldnext = item->next;

  // advance iterators
  if (nActive) {
    for (i=0; i<nAlloc; i++) {
      iter=activeIters[i];
      // check now if this iterator is pointing to the thing we're deleting
      if (iter && iter->d_current == item) {
	// advance the iterator, if there is a next, if not
	// move it back to the new "last"
	if (oldnext)
	  iter->d_current = oldnext;
	else
	  iter->d_current = oldprev;
      }
    }
  }

  // now just delete the item from the list
  if (oldprev)
    oldprev->next=oldnext;
  else
    first=oldnext;

  if (oldnext)
    oldnext->prev=oldprev;
  else
    last=oldprev;

  list_size--;

  // clear item's pointers for reuse in a different list
  item->prev=0;
  item->next=0;
 
}

EDItemBase *EDListBase::nth(int n) const
{
  int i;
  EDItemBase *item;

  if (n>=list_size) {
    // Error("EDListBase::nth: requested item " << n << " from list containing " << list_size << " items");
    return 0;
  }

  i=0;
  item=first;
  while (i<n) {
    i++;
    item=item->next;
  }
  return item;
}

void EDListBase::mergeList(EDListBase *other) {

  // make sure there are no active iterators on the lists
  if (other->nActive) {
    //Error("cannot merge a list with active iterators, have " << other->nActive);
  }
  // we can probably do this, but we shouldn't unless needed
  if (nActive) {
    //Error("cannot merge a list with active iterators");
  }

  // if other is empty, we do nothing
  if (!other->first) return;

  // hook up the first item in other to the last item in this
  if (!first) {
    // empty this
    first=other->first;
    last=other->last;
  }
  else {
    // neither list is empty
    last->next=other->first;
    other->first->prev=last;
    last=other->last;
  }  
  list_size+=other->list_size;
  other->first=0;
  other->last=0;
  other->list_size=0;
}

EDListIterBase::EDListIterBase()
  : d_list(0), d_current(0)
{}

EDListIterBase::EDListIterBase(const EDListBase *the_list) {

  d_list=the_list;
  d_current=d_list->first;

  ((EDListBase*)d_list)->addIter(this);
}

EDListIterBase::EDListIterBase(const EDListIterBase & iter)
  : d_list(iter.d_list), d_current(iter.d_current)
{
  ((EDListBase*)d_list)->addIter(this);
}

EDListIterBase::~EDListIterBase() 
{
  ((EDListBase*)d_list)->removeIter(this);
}

void EDListIterBase::operator=(const EDListIterBase &iter)
{
  if(d_list)
    ((EDListBase*)d_list)->removeIter(this);
  d_list = iter.d_list;
  ((EDListBase*)d_list)->addIter(this);
  d_current = iter.d_current;
}

int EDListIterBase::operator() (EDItemBase *&item) {

  item = d_current;
  if (item) {
    d_current=d_current->next;
    return 1;
  }
  return 0;
}

void EDListIterBase::reset() {

  d_current=d_list->first;
}

EDItemBase * EDListIterBase::next()
{
  EDItemBase *item = d_current;
  if(item)
    d_current = d_current->next;
  return item;
}

EDItemBase *EDListIterBase::current() const
{ return d_current; }

EDListBase * EDListIterBase::list() const
{ return (EDListBase*)d_list; }




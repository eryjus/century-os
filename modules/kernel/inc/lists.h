//===================================================================================================================
//
//  lists.h -- Standard list for the entire kernel
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  The list structures in this file are standard for all lists in the entire Century-OS implementaiton.  All lists
//  will make use of these structures.
//
//  The inspiration for this list structure and implementation is taken from the Linux list implementation.
//  References can be found in the Linux Kernel Development book, chapter 6 and the linux source file at
//  http://www.cs.fsu.edu/~baker/devices/lxr/http/source/linux/include/linux/list.h
//
//  In short, the list implementation is a circular doubly linked list.  As such there is no specific head and tail.
//
// ------------------------------------------------------------------------------------------------------------------
//
//  IMPORTANT PROGRAMMING NOTE:
//  These functions are not atomic.  This means that all calling functions will also need to mantain locks on the
//  structures and lists before maintaining the list.  Put another way, the caller is required to ensure that
//  nothing else changes the list while these functions are being executed.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2016-Sep-12  Initial   0.0.0   ADCL  Initial version
//  2018-May-24  Initial   0.1.0   ADCL  Copy this file from century to century-os
//  2018-Nov-09            0.1.0   ADCL  Reformat the list structures to have a list head, creating a separate type.
//  2019-Feb-08  Initial   0.3.0   ADCL  Relocated
//  2019-Mar-10  Initial   0.3.1   ADCL  Establish QueueHead_t and StackHead_t types
//
//===================================================================================================================


#ifndef __LISTS_H__
#define __LISTS_H__


#include "types.h"
#include "spinlock.h"


//
// -- This macro determines the offset of a member of a structure.  The 'magic' of this macro is the calculation
//    of an address as a offset from the address 0x00000000.
//    ----------------------------------------------------------------------------------------------------------
#define MEMBER_OFFSET(type,member)  ((uint32_t)(&((type *)0x00000000)->member))


//
// -- This macro determines the address of the parent of a member structure.
//    Usage: FIND_PARENT(list->next, Process, global);
//    ----------------------------------------------------------------------
#define FIND_PARENT(ptr,type,member) ({									\
		const typeof(((type *)0x00000000)->member) *__mptr = (ptr);		\
		(type *)((char *)__mptr - MEMBER_OFFSET(type,member));  })


//
// -- This is the header of the list.
//    -------------------------------
typedef struct ListHead_t {
	typedef struct List_t {
		struct List_t *prev;
		struct List_t *next;
	} List_t;

	List_t list;
	Spinlock_t lock;            // -- this or a "bigger" lock must be obtained to change the list contents
    size_t count;               // -- this is available for use by software; not used by `lists.h`
} ListHead_t;


//
// -- Declare and initialize a new List not in a structure
//    ----------------------------------------------------
#define NEW_LIST(name) ListHead_t name = { { &(name.list), &(name.list) }, {0, 0}, 0 };


//
// -- Initialize a list to point to itself
//    ------------------------------------
inline void ListInit(ListHead_t::List_t * const list) { list->next = list->prev = list; }


//
// -- Low-level function to add a node to a list
//    ------------------------------------------
inline void __list_add(ListHead_t::List_t * const nw, ListHead_t::List_t * const pv, ListHead_t::List_t * const nx) {
	nx->prev = nw; nw->next = nx; nw->prev = pv; pv->next = nw;
}


//
// -- Low-level function to delete a node from a list
//    -----------------------------------------------
inline void __list_del(ListHead_t::List_t * const pv, ListHead_t::List_t * const nx) {
	nx->prev = pv; pv->next = nx;
}


//
// -- Add a new node to a list (which is right ahead of the head)
//    -----------------------------------------------------------
inline void ListAdd(ListHead_t * const head, ListHead_t::List_t * const nw) {
	__list_add(nw, &head->list, head->list.next);
}


//
// -- Add a new node to a list (which will be right behind the tail)
//    --------------------------------------------------------------
inline void ListAddTail(ListHead_t * const head, ListHead_t::List_t * const nw) {
	__list_add(nw, head->list.prev, &head->list);
}


//
// -- Delete a node from a list (and clear the node's pointers to NULL)
//    -----------------------------------------------------------------
inline void ListRemove(ListHead_t::List_t * const entry) {
	__list_del(entry->prev, entry->next); entry->next = entry->prev = 0;
}


//
// -- Delete a node from a list (and and initialize the node to be properly empty)
//    ----------------------------------------------------------------------------
inline void ListRemoveInit(ListHead_t::List_t * const entry) {
	__list_del(entry->prev, entry->next); ListInit(entry);
}


//
// -- Is this list empty or not?  Notice that both the address and the contents are constant
//    --------------------------------------------------------------------------------------
inline bool IsListEmpty(const ListHead_t * const head) {
	return (head->list.next == &head->list);
}


//
// -- Is this entry last in the list?  Notice that both the address and the contents are constant
//    -------------------------------------------------------------------------------------------
inline bool IsLastInList(const ListHead_t * const head, const ListHead_t::List_t * const entry) {
	return entry->next == &head->list;
}


//
// -- Move an entry from one list to another (in front of the head)
//    -------------------------------------------------------------
inline void ListMove(ListHead_t * const head, ListHead_t::List_t * const entry) {
	__list_del(entry->prev, entry->next); ListAdd(head, entry);
}


//
// -- Move an entry from one list to another (after the tail)
//    -------------------------------------------------------
inline void ListMoveTail(ListHead_t * const head, ListHead_t::List_t * const entry) {
	__list_del(entry->prev, entry->next); ListAddTail(head, entry);
}


//
// -- This is a queue; the next thing to operate on is at head
//    --------------------------------------------------------
typedef ListHead_t QueueHead_t;


//
// -- Enqueue a node onto a queue
//    ---------------------------
#define Enqueue ListAddTail


//
// -- This is a stack; the next thing to operate on is at head
//    --------------------------------------------------------
typedef ListHead_t StackHead_t;


//
// -- Push a node onto a stack
//    ------------------------
#define Push ListAdd


#endif

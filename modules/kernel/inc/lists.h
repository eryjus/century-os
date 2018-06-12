//===================================================================================================================
//
// inc/lists.h -- Standard list for the entire kernel
//
// The list structures in this file are standard for all lists in the entire Century-OS implementaiton.  All lists 
// will make use of these structures.
// 
// The inspiration for this list structure and implementation is taken from the Linux list implementation.  
// References can be found in the Linux Kernel Development book, chapter 6 and the linux source file at 
// http://www.cs.fsu.edu/~baker/devices/lxr/http/source/linux/include/linux/list.h
// 
// In short, the list implementation is a circular doubly linked list.  As such there is no specific head and tail.
//
// ------------------------------------------------------------------------------------------------------------------
//
// IMPORTANT PROGRAMMING NOTE:
// These functions are not atomic.  This means that all calling functions will also need to mantain locks on the
// structures and lists before maintaining the list.  Put another way, the caller is required to ensure that 
// nothing else changes the list while these functions are being executed.
// 
// ------------------------------------------------------------------------------------------------------------------
//                                                                                                                 
//     Date     Tracker  Version  Pgmr  Description                                                                         
//  ----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2016-09-12  Initial   0.0.0   ADCL  Initial version
//  2018-05-24  Initial   0.1.0   ADCL  Copy this file from century to century-os
//
//===================================================================================================================


#ifndef __LISTS_H__
#define __LISTS_H__


#include "types.h"


//
// -- This macro determines the offset of a member of a structure.  The 'magic' of this macro is the calculation 
//    of an address as a offset from the address 0x00000000.
//    ----------------------------------------------------------------------------------------------------------
#define MEMBER_OFFSET(type,member)  ((uint32)(&((type *)0x00000000)->member))


//
// -- This macro determines the address of the parent of a member structure. 
//    Usage: FIND_PARENT(list->next, Process, global);
//    ----------------------------------------------------------------------
#define FIND_PARENT(ptr,type,member) ({									\
		const typeof(((type *)0x00000000)->member) *__mptr = (ptr);		\
		(type *)((char *)__mptr - MEMBER_OFFSET(type,member));  })


//
// -- This is an entry in a list.  It will be included as an element (not a pointer) in a structure to be "listed".
//    -------------------------------------------------------------------------------------------------------------
typedef struct List {
	struct List *prev;
	struct List *next;
} List;


//
// -- Declare and initialize a new List not in a structure
//    ----------------------------------------------------
#define NEW_LIST(name) List name = { &(name), &(name) }


//
// -- Initialize a ListHead structure to point to itself; notice that `l` is a constant address
//    -----------------------------------------------------------------------------------------
inline void ListInit(List * const l) { l->prev = l->next = l; }


//
// -- Low-level function to add a node to a list
//    ------------------------------------------
inline void __list_add(List * const nw, List * const pv, List * const nx) 
	{ nx->prev = nw; nw->next = nx; nw->prev = pv; pv->next = nw; }


//
// -- Low-level function to delete a node from a list
//    -----------------------------------------------
inline void __list_del(List * const pv, List * const nx) { nx->prev = pv; pv->next = nx; }


//
// -- Add a new node to a list (which is right ahead of the head)
//    -----------------------------------------------------------
void ListAdd(List * const nw, List * const head) { __list_add(nw, head, head->next); }


//
// -- Add a new node to a list (which will be right behind the tail)
//    --------------------------------------------------------------
void ListAddTail(List * const nw, List * const head) { __list_add(nw, head->prev, head); }


//
// -- Delete a node from a list (and clear the node's pointers to NULL)
//    -----------------------------------------------------------------
void ListDel(List * const entry) { __list_del(entry->prev, entry->next); entry->next = entry->prev = 0; }


//
// -- Delete a node from a list (and and initialize the node to be properly empty)
//    ----------------------------------------------------------------------------
void ListDelInit(List * const entry) { __list_del(entry->prev, entry->next); ListInit(entry); }


//
// -- Is this list empty or not?  Notice that both the address and the contents are constant
//    --------------------------------------------------------------------------------------
bool IsListEmpty(const List * const head) { return (head->next == head); }


//
// -- Is this entry last in the list?  Notice that both the address and the contents are constant
//    -------------------------------------------------------------------------------------------
bool IsLastInList(const List * const list, const List * const head) { return list->next == head; }


//
// -- Move an entry from one list to another (in front of the head)
//    -------------------------------------------------------------
void ListMove(List * const entry, List * const head) 
	{ __list_del(entry->prev, entry->next); ListAdd(entry, head); }


//
// -- Move an entry from one list to another (after the tail)
//    -------------------------------------------------------
void ListMoveTail(List * const entry, List * const head) 
	{ __list_del(entry->prev, entry->next); ListAddTail(entry, head); }


#endif

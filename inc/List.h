/* OeyEnc - 
 *
 * Copyright (C) 2004-2009 Jeremy Boschen. All rights reserved.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software. 
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software. If you use this software in
 * a product, an acknowledgment in the product documentation would be
 * appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not
 * be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 */

/* List.h
 *   Singly- and doubly-linked list implementation
 *
 * Copyright (C) 2004-2009 Jeremy Boschen
 */

#pragma once

#include <winnt.h>

/**********************************************************************
	
	Linked Lists Manipulation

 **********************************************************************/

FORCEINLINE
VOID
InitializeListHead(
    IN PLIST_ENTRY ListHead
)
{
   ListHead->Flink = ListHead->Blink = ListHead;
}

BOOLEAN
FORCEINLINE
IsListEmpty(
    IN const LIST_ENTRY * ListHead
)
{
   return (BOOLEAN)(ListHead->Flink == ListHead);
}

FORCEINLINE
BOOLEAN
RemoveEntryList(
    IN PLIST_ENTRY Entry
)
{
   PLIST_ENTRY Blink;
   PLIST_ENTRY Flink;

   Flink = Entry->Flink;
   Blink = Entry->Blink;
   Blink->Flink = Flink;
   Flink->Blink = Blink;
   return (BOOLEAN)(Flink == Blink);
}

FORCEINLINE
PLIST_ENTRY
RemoveHeadList(
    IN PLIST_ENTRY ListHead
)
{
   PLIST_ENTRY Flink;
   PLIST_ENTRY Entry;

   Entry = ListHead->Flink;
   Flink = Entry->Flink;
   ListHead->Flink = Flink;
   Flink->Blink = ListHead;
   return Entry;
}

FORCEINLINE
PLIST_ENTRY
RemoveTailList(
    IN PLIST_ENTRY ListHead
)
{
   PLIST_ENTRY Blink;
   PLIST_ENTRY Entry;

   Entry = ListHead->Blink;
   Blink = Entry->Blink;
   ListHead->Blink = Blink;
   Blink->Flink = ListHead;
   return Entry;
}

FORCEINLINE
VOID
InsertTailList(
    IN PLIST_ENTRY ListHead,
    IN PLIST_ENTRY Entry
)
{
   PLIST_ENTRY Blink;

   Blink = ListHead->Blink;
   Entry->Flink = ListHead;
   Entry->Blink = Blink;
   Blink->Flink = Entry;
   ListHead->Blink = Entry;
}

FORCEINLINE
VOID
InsertHeadList(
    IN PLIST_ENTRY ListHead,
    IN PLIST_ENTRY Entry
)
{
   PLIST_ENTRY Flink;

   Flink = ListHead->Flink;
   Entry->Flink = Flink;
   Entry->Blink = ListHead;
   Flink->Blink = Entry;
   ListHead->Flink = Entry;
}

/* Merge two lists by inserting at the tail of ListHead */
FORCEINLINE
VOID
AppendTailList(
    IN PLIST_ENTRY ListHead,
    IN PLIST_ENTRY ListToAppend
)
{
   PLIST_ENTRY ListEnd = ListHead->Blink;

   ListHead->Blink->Flink = ListToAppend;
   ListHead->Blink = ListToAppend->Blink;
   ListToAppend->Blink->Flink = ListHead;
   ListToAppend->Blink = ListEnd;
}

/* Merge two lists by inserting at the head of ListHead. */
FORCEINLINE
VOID
PrependHeadList(
    IN PLIST_ENTRY ListHead,
    IN PLIST_ENTRY ListToPrepend
)
{
   PLIST_ENTRY ListEnd;

   ListEnd = ListToPrepend->Blink;

   ListEnd->Flink = ListHead->Flink;
   ListHead->Flink->Blink = ListEnd;
   ListHead->Flink = ListToPrepend;
   ListToPrepend->Blink = ListHead;
}

FORCEINLINE
PLIST_ENTRY
FlushList(
   __in PLIST_ENTRY ListHead
)
{
   PLIST_ENTRY Entry;
   PLIST_ENTRY Blink;

   Entry = ListHead->Flink;
   Blink = ListHead->Blink;
   Entry->Blink = Blink;
   Blink->Flink = Entry;
   InitializeListHead(ListHead);

   return ( Entry );
}

FORCEINLINE
VOID
InitializeListHead(
   OUT PSINGLE_LIST_ENTRY ListHead
)
{
   ListHead->Next = NULL;
}

FORCEINLINE
BOOLEAN
IsListEmpty(
   IN const PSINGLE_LIST_ENTRY ListHead
)
{
   return ( (BOOLEAN)(NULL == ListHead->Next) );
}

FORCEINLINE
PSINGLE_LIST_ENTRY
PopEntryList(
    PSINGLE_LIST_ENTRY ListHead
)
{
   PSINGLE_LIST_ENTRY FirstEntry;
   FirstEntry = ListHead->Next;
   if (FirstEntry != NULL) {
      ListHead->Next = FirstEntry->Next;
   }

   return FirstEntry;
}

FORCEINLINE
VOID
PushEntryList(
    PSINGLE_LIST_ENTRY ListHead,
    PSINGLE_LIST_ENTRY Entry
)
{
   Entry->Next = ListHead->Next;
   ListHead->Next = Entry;
}

FORCEINLINE
VOID
AppendEntryList(
   IN PSINGLE_LIST_ENTRY ListHead,
   IN PSINGLE_LIST_ENTRY Entry
)
{
   while ( ListHead->Next )
   {
      ListHead = ListHead->Next;
   }

   Entry->Next    = NULL;
   ListHead->Next = Entry;
}

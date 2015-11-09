
#ifndef IPT_H
#define IPT_H

#include "copyright.h"
#include "utility.h"

// The following class defines an entry in a IPT -- copied from TranslationEntry
// with and added filed for the owner

class IPT {
public:
	int virtualPage;  	// The page number in virtual memory.
	int physicalPage;  	// The page number in real memory (relative to the
	//  start of "mainMemory"
	bool valid;         // If this bit is set, the translation is ignored.
	// (In other words, the entry hasn't been initialized.)
	bool readOnly;	// If this bit is set, the user program is not allowed
	// to modify the contents of the page.
	bool use;           // This bit is set by the hardware every time the
	// page is referenced or modified.
	bool dirty;         // This bit is set by the hardware every time the
	// page is modified.
	AddrSpace* owner;	//To keep track of which process owns the page.
};

#endif

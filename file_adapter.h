/*
 * UniversalContainer library.
 * Copyright Jason Denton, 2008,2010.
 * Made available under the new BSD license, as described in LICENSE
 *
 * Send comments and bug reports to jason.denton@gmail.com
 * http://www.greatpanic.com/code.html
 */

#ifndef __file_adapater_h__
#define __file_adapater_h__

#include <stdio.h>
#include "ucontainer.h"

namespace JAD {

	struct FILEAdapter : public UniversalContainerTypeAdapter
	{
	    virtual void* clone(void*);
	    virtual void on_delete(void*);
	    virtual std::string to_string(void*);
	    
	    static UniversalContainerType tid;
	    static UniversalContainer assign(FILE*);
	    static FILE* cast(UniversalContainer const& uc);
	};

}
#endif

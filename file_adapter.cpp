
/*
 * UniversalContainer library.
 * Copyright Jason Denton, 2008,2010.
 * Made available under the new BSD license, as described in LICENSE
 *
 * Send comments and bug reports to jason.denton@gmail.com
 * http://www.greatpanic.com/code.html
 */

#include "file_adapter.h"

namespace JAD {

//This linker hack relies on the fact that c++ will execute code to initialize global variables before
//main is invoked. You can exploit this make sure that your class is registered anytime it gets linked in.

	UniversalContainerType FILEAdapter::tid = UniversalContainer::register_new_type(new FILEAdapter);

	void* FILEAdapter::clone(void* ptr) 
	{
		//can't clone a file pointer, that's just asking for trouble.
		return NULL;
	}

	void FILEAdapter::on_delete(void* ptr)
	{
		FILE* data = (FILE*) ptr;
		fclose(data);
	}

	//For most uses there should be no need to change the code below this point.

	UniversalContainer FILEAdapter::assign(FILE* ptr)
	{
		return UniversalContainer(tid, ptr);
	}

	FILE* FILEAdapter::cast(UniversalContainer const& uc)
	{
		FILE* at = (FILE*) uc.cast_or_throw(tid);
		return at;
	}

	std::string FILEAdapter::to_string(void* ptr)
	{	
		return std::string("<< FILE* >>");
	}
}


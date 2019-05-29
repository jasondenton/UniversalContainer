/*
 * UniversalContainer library.
 * Copyright Jason Denton, 2008,2010.
 * Made available under the new BSD license, as described in LICENSE
 *
 * Send comments and bug reports to jason.denton@gmail.com
 * http://www.greatpanic.com/code.html
 */

#include <cstring>
#include <stdio.h>
#include "buffer_adapter.h"

using namespace std;

namespace JAD {

	UniversalContainerType BufferAdapter::tid = UniversalContainer::register_new_type(new BufferAdapter);

	void* BufferAdapter::clone(void* ptr) 
	{
		Buffer* data = (Buffer*) ptr;
		Buffer* clone;
		if (!data->own_data) {
			clone = new Buffer(data->data, data->length);
		}
		else {
			clone = new Buffer(data->size);
			memcpy(clone->data, data->data, data->length);
			clone->length = data->length;
		}
		clone->rpos = data->rpos;
		clone->wpos = data->wpos;
		return clone;
	}

	void BufferAdapter::on_delete(void* ptr)
	{
		Buffer* buf = (Buffer*) ptr;
		delete buf;
	}

	std::string BufferAdapter::to_string(void* ptr)
	{
		char str[128];
		Buffer* buf = (Buffer*) ptr;
		
		snprintf(str, 128, "<< Buffer Object == size: %lu length: %lu rpos: %lu wpos: %lu",
			buf->size, buf->length, buf->rpos, buf->wpos);
		return std::string(str);
	}

	//For most uses there should be no need to change the code below this point.

	UniversalContainer BufferAdapter::assign(Buffer* ptr)
	{
		return UniversalContainer(tid, ptr);
	}

	Buffer* BufferAdapter::cast(UniversalContainer const& uc)
	{
		Buffer* at = (Buffer*) uc.cast_or_throw(tid);
		return at;
	}
}



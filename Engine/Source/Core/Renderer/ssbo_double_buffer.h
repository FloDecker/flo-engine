#pragma once
#include "ssbo.h"

template <typename T>
class ssbo_double_buffer {
public:
	bool init(unsigned int size, unsigned int bindingA, unsigned int bindingB) {
		bool okA = buffers_[0].init_buffer_object(size, bindingA);
		bool okB = buffers_[1].init_buffer_object(size, bindingB);
		return okA && okB;
	}

	ssbo<T>& front() { return buffers_[read_index_]; }
	ssbo<T>& back()  { return buffers_[1 - read_index_]; }

	void swap() { read_index_ = 1 - read_index_; }

	void bind_front(unsigned int binding) { front().bind_to_base(binding); }
	void bind_back(unsigned int binding)  { back().bind_to_base(binding); }

private:
	ssbo<T> buffers_[2];
	int read_index_ = 0;
};

#pragma once
#include "ssbo.h"

template <typename T>
class ssbo_double_buffer {
public:
	bool init(unsigned int size, unsigned int binding_back, unsigned int binding_front) {
		binding_front_ = binding_front;
		binding_back_ = binding_back;
		bool okA = buffers_[0].init_buffer_object(size, binding_front);
		bool okB = buffers_[1].init_buffer_object(size, binding_back);
		return okA && okB;
	}

	ssbo<T>& front() { return buffers_[read_index_]; }
	ssbo<T>& back()  { return buffers_[1 - read_index_]; }

	void swap()
	{
		read_index_ = 1 - read_index_;
		bind_front(binding_front_);
		bind_back(binding_back_);
	}

	void bind_front(unsigned int binding) { front().bind_to_base(binding); }
	void bind_back(unsigned int binding)  { back().bind_to_base(binding); }

	void clear()
	{
		read_index_ = 0;
		buffers_[0].clear_data();
		buffers_[1].clear_data();
	}

private:
	ssbo<T> buffers_[2];
	int read_index_ = 0;
	int binding_front_ = 0;
	int binding_back_ = 0;
};

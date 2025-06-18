#pragma once
#include <iostream>
#include <vector>
#include <GL/glew.h>

template <typename T>
class ssbo
{
public:
	bool init_buffer_object(unsigned int size, unsigned int binding)
	{
		if (initialized_)
		{
			std::cout << "ssbo already initialized" << '\n';
			return false;
		}
		size_ = size;
		glGenBuffers(1, &ssbo_id_);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_id_);
		std::vector<T> data(size_, {});
		glBufferData(GL_SHADER_STORAGE_BUFFER, size_ * sizeof(T), data.data(), GL_DYNAMIC_DRAW);
		initialized_ = true;
		bind_to_base(binding);
		return true;
	}

	void bind_to_base(unsigned int binding) const
	{
		if (!initialized_)
		{
			std::cout << "ssbo not initialized" << '\n';
			return;
		}

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, ssbo_id_);
	}

	bool insert_data(const T* data, unsigned int at, unsigned int data_length = 1)
	{
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_id_);
		void* ptr = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, sizeof(T)*at, sizeof(T) * data_length,
	                             GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
		memcpy(ptr, data, sizeof(T));
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		return true;
	}

	bool move_data(unsigned int from, unsigned int to, unsigned int length) const
	{
		GLuint tempBuffer;
		glCreateBuffers(1, &tempBuffer);
		glNamedBufferData(tempBuffer, length * sizeof(T), nullptr, GL_DYNAMIC_COPY);

		// Copy to temp
		glCopyNamedBufferSubData(ssbo_id_, tempBuffer, from * sizeof(T), 0, length * sizeof(T));

		// Copy from temp to destination range
		glCopyNamedBufferSubData(tempBuffer, ssbo_id_, 0, to * sizeof(T), length * sizeof(T));

		glDeleteBuffers(1, &tempBuffer);
		return true;
	}

private:
	unsigned int size_ = 0;
	unsigned int ssbo_id_ = 0;
	bool initialized_ = false;
};

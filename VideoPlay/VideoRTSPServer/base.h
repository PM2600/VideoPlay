#pragma once
#include <string>

typedef unsigned char BYTE;

class EBuffer : public std::string {
public:
	EBuffer(const char* str) {
		resize(strlen(str));
		memcpy((void*)c_str(), str, size());
	}
	EBuffer(size_t size = 0) : std::string() {
		if (size > 0) {
			resize(size);
			memset(*this, 0, this->size());
		}
	}
	EBuffer(void* buffer, size_t size) :std::string() {
		resize(size);
		memcpy((void*)c_str(), buffer, size);
	}
	~EBuffer() {
		std::string::~basic_string();
	}
	operator char* () const { return (char*)c_str(); };
	operator const char* () const { return c_str(); };
	operator BYTE*() const { return (BYTE*)c_str(); };
	operator void* () const { return (void*)c_str(); };
	void Update(void* buffer, size_t size) {
		resize(size);
		memcpy((void*)c_str(), buffer, size);
	}
	void Zero() {
		if (size() > 0)
			memset((void*)c_str(), 0, size());
	}

	EBuffer& operator<<(const EBuffer& str) {
		if (this != str) {
			*this += str;
		}
		else {
			EBuffer tmp = str;
			*this += tmp;
		}
		return *this;
	}
	EBuffer& operator<<(const std::string& str) {
		*this += str;
		return *this;
	}
	EBuffer& operator<<(const char* str) {
		*this += str;
		return *this;
	}
	EBuffer& operator<<(int data) {
		char s[16] = "";
		snprintf(s, sizeof(s), "%d", data);
		*this += s;
		return *this;
	}
	const EBuffer& operator>>(int& data) const {
		data = atoi(c_str());
		return *this;
	}
	const EBuffer& operator>>(short& data) const {
		data = (short)atoi(c_str());
		return *this;
	}
};

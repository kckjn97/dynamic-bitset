#ifndef SAGE_BITMAP_H
#define SAGE_BITMAP_H

#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <vector>
#include <iostream>

typedef uint64_t Bit64;
class BaseBitset{
protected:
	static inline bool _GetBit(Bit64 x, unsigned int n){
		return ((x >> n) & 0x1UL);
	}

	static inline void _SetBit(Bit64& x, unsigned int n){
		x |= ( (0x1UL << (n)));
	}

	static inline void _ClearBit(Bit64& x, unsigned int n){
		x &= (~(0x1UL << (n)));
	}

public:
	static Bit64* Allocate(size_t num_bit){
		size_t array_size = GetArraySize(num_bit);
		Bit64* data = (Bit64*) malloc(array_size * sizeof(Bit64));
		memset(data, 0x00, array_size*sizeof(Bit64));
		return data;
	}

	static size_t GetSetCount(Bit64* data, size_t num_bit){
		size_t array_size = GetArraySize(num_bit);
		size_t ret = 0;
		for(size_t i=0; i<array_size; i++){
			if(data[i]){
				Bit64 bit64 = data[i];
				int idx = 0;
				while(bit64){
					if((i<<6) + idx >= num_bit )
						break;
					if(bit64 & 0x01UL)
						ret++;
					bit64 >>= 1;
					idx++;
				}
			}
		}
		return ret;
	}

	static inline size_t GetArraySize(size_t num_bit){
		return (num_bit >> 6) + ((num_bit & 0x3F) > 0);
	}

	static inline void SetAll(Bit64* data, size_t num_bit){
		size_t array_size = GetArraySize(num_bit);
		memset(data, 0xff, (array_size-1) * sizeof(Bit64));
		for(size_t i=(array_size-1)*sizeof(Bit64)*8; i<num_bit; i++)
			Set(data, i);
	}

	static inline void ClearAll(Bit64* data, size_t num_bit){
		size_t array_size = GetArraySize(num_bit);
		memset(data, 00, array_size * sizeof(Bit64));
	}

	static inline void Set(Bit64* data, unsigned int key){
		_SetBit(data[key >> 6], key & 0x3F);
	}

	static inline bool Get(Bit64* data, unsigned int key){
		return _GetBit(data[key >> 6], key & 0x3F);
	};

	static inline void Clear(Bit64* data, unsigned int key){
		_ClearBit(data[key >> 6], key & 0x3F);
	};

	static inline bool IsAllSet(Bit64* data, size_t num_bit){
		size_t array_size = GetArraySize(num_bit);

		unsigned int offset = 0;
		for(size_t i=offset; i<array_size-1; i++) {
			if (!data[i])
				return false;
		}
		for(size_t i=array_size*sizeof(Bit64)*8; i<num_bit; i++){
			if(!Get(data, i))
				return false;
		}
		return true;
	}

	static inline bool IsAllClear(Bit64* data, size_t num_bit){
		size_t array_size = GetArraySize(num_bit);

		for(unsigned int i=0; i<array_size; i++) {
			if (data[i])
				return false;
		}
		return true;
	}

	static std::vector<off_t> GetKeys(Bit64* data, size_t num_bit, unsigned int* cursor=NULL, unsigned int max_num = 0){
		size_t array_size = GetArraySize(num_bit);
		std::vector<off_t> res;

		unsigned int offset = 0;
		unsigned int _offset = 0;

		if(cursor) {
			offset = *cursor;
			_offset = offset / (sizeof(unsigned long) << 3);
		}

		if(*cursor > num_bit)
			return res;


		unsigned int cursor_res;
		for(unsigned int i=_offset; i<array_size; i++){
			cursor_res = ((i + 1) * sizeof(unsigned long)) << 3;

			if(data[i]) {
				unsigned int start = (i * sizeof(unsigned long)) << 3;
				unsigned int end = ((i + 1) * sizeof(unsigned long)) << 3;

				if(end > num_bit)
					end = num_bit;

				if(offset > start)
					start = offset;
				for (unsigned int j = start; j < end; j++) {
					cursor_res = j+1;

					if (Get(data, j)) {
						res.push_back(j);
						if(max_num){
							if(res.size() >= max_num) {
								break;
							}
						}
					}
				}

				if(max_num) {
					if (res.size() >= max_num) {
						break;
					}
				}
			}
		}
		if(cursor)
			*cursor = cursor_res;

		return res;
	}

	static inline void DeepCopy(Bit64* dest, Bit64* src, size_t num_bit){
		size_t array_size = GetArraySize(num_bit);
		memcpy(dest, src, array_size*sizeof(Bit64));
	}

	static inline void And(Bit64* dest, Bit64* src, size_t num_bit){
		size_t array_size = GetArraySize(num_bit);

		for(int i=0; i<array_size; i++)
			dest[i] &= src[i];
	}

	static inline void Or(Bit64* dest, Bit64* src, size_t num_bit){
		size_t array_size = GetArraySize(num_bit);

		for(int i=0; i<array_size; i++)
			dest[i] |= src[i];
	}

	static inline void Exclude(Bit64* dest, Bit64* src, size_t num_bit){
		size_t array_size = GetArraySize(num_bit);
		for(int i=0; i<array_size; i++)
			dest[i] &= (~src[i]);
	}

	static inline bool IsEqual(Bit64* data1, Bit64* data2, size_t num_bit){
		size_t array_size = GetArraySize(num_bit);
		for(int i=0; i<array_size; i++) {
			if (data1[i] != data2[i])
				return false;
		}
		return true;
	}

	static void Print(Bit64* data, size_t num_bit){
		printf("%lu: ", num_bit);
		for(int i=0; i<num_bit; i++)
			printf("%d ", Get(data, i));
		printf("\n");
	}
};

class Bitset:public BaseBitset {
	size_t num_bit;
	Bit64* data;
	bool allocated;

public:
	Bitset() {
		data = NULL;
		allocated = false;
	}

	Bitset(size_t num_bit){
		Init(num_bit);
		allocated = true;
	}

	Bitset(Bit64* data, size_t num_bit){
		SetData(data, num_bit);
		allocated = false;
	}

	~Bitset() {
		if(allocated && data)
			free(data);
	}

	void Init(size_t num_bit){
		Allocate(num_bit);
	}

	Bit64* Allocate(Bit64 num_bit){
		data = BaseBitset::Allocate(num_bit);
		this->num_bit = num_bit;
		return data;
	}

	inline size_t GetSetCount(){
		return BaseBitset::GetSetCount(data, num_bit);
	}

	inline size_t GetArraySize() const{
		return BaseBitset::GetArraySize(num_bit);
	}

	inline size_t GetSize() const{
		return num_bit;
	}

	inline void SetAll(){
		BaseBitset::SetAll(data, num_bit);
	}

	inline void ClearAll(){
		BaseBitset::ClearAll(data, num_bit);
	}

	inline void Set(unsigned int key){
		BaseBitset::Set(data, key);
	}

	inline bool Get(unsigned int key){
		return BaseBitset::Get(data, key);
	};

	inline void Clear(unsigned int key){
		BaseBitset::Clear(data, key);
	};

	inline bool IsAllSet(){
		return BaseBitset::IsAllSet(data, num_bit);
	}

	inline bool IsAllClear(){
		return BaseBitset::IsAllClear(data, num_bit);
	}

	std::vector<off_t> GetKeys(unsigned int* cursor=NULL, unsigned int max_num = 0) {
		return BaseBitset::GetKeys(data, num_bit, cursor, max_num);
	}

	inline void CopyFrom(const Bitset &other){
		BaseBitset::DeepCopy(data, other.GetData(), num_bit);
	}

	inline void And(const Bitset &other){
		BaseBitset::And(data, other.GetData(), num_bit);
	}

	inline void Or(const Bitset &other){
		BaseBitset::Or(data, other.GetData(), num_bit);
	}

	inline void Exclude(const Bitset &other){
		BaseBitset::Exclude(data, other.GetData(), num_bit);
	}

	inline bool IsEqual(const Bitset &other){
		BaseBitset::IsEqual(data, other.GetData(), num_bit);
	}

	void Print(){
		BaseBitset::Print(data,num_bit);
	}

	inline Bit64* GetData() const{
		return data;
	}

	inline void SetData(Bit64* data, size_t num_bit){
		this->num_bit = num_bit;
		this->data = data;
	}
};

class BitsetArray:public BaseBitset {
	size_t single_bitset_size;
	size_t single_bitset_size;
	size_t num_bitset;
	Bit64* data;

public:
	BitsetArray() {
		data = NULL;
	}

	BitsetArray(size_t signle_bitset_size, size_t num_bitset){
		Init(single_bitset_size, num_bitset);
	}

	~BitsetArray(){
	}

	void Init(size_t signle_bitset_size, size_t num_bitset){
		Allocate((signle_bitset_size + (sizeof(Bit64) << 3) - 1), num_bitset);
		ClearAll();
	}

	Bit64* Allocate(size_t signle_bitset_size, size_t num_bitset){
		single_bitset_size = BaseBitset::GetArraySize(signle_bitset_size);
		data = (Bit64*) malloc(single_bitset_size * sizeof(Bit64) * num_bitset);
		memset(data, 0x00, single_bitset_size * sizeof(Bit64) * num_bitset);
		this->single_bitset_size = signle_bitset_size;
		this->num_bitset = num_bitset;
		return data;
	}

	void Free(){
		if(data)
			free(data);
	}

	size_t GetSetCount(off_t index){
		return BaseBitset::GetSetCount(data + index*single_bitset_size, single_bitset_size);
	}

	inline size_t GetSingleBitsetArraySize() const{
		return single_bitset_size;
	}

	inline size_t GetNumBitset() const{
		return num_bitset;
	}

	inline size_t GetSingleBitsetSize() const{
		return single_bitset_size;
	}

	inline void SetAll(){
		for(int i=0; i < num_bitset; i++)
			BaseBitset::SetAll(data + single_bitset_size * i, single_bitset_size);
	}

	inline void ClearAll(){
		memset(data, 0x00, single_bitset_size * sizeof(Bit64) * num_bitset);
	}

	inline Bit64* GetData(off_t index) const{
		return data + index * single_bitset_size;
	}

	inline void SetData(Bit64* data, size_t signle_bitset_size, size_t num_bitset){
		single_bitset_size = BaseBitset::GetArraySize(signle_bitset_size);
		this->data = data;
		this->single_bitset_size = signle_bitset_size;
		this->num_bitset = num_bitset;
	}
};

#endif //GRAPH_ENGINE_BITMAP_H

/*
  Simple CUDA GPU Vector class
  Dr. Orion Lawlor, lawlor@alaska.edu, 2017-11 (Public Domain)
*/
#ifndef __OSL_GPU_VEC_H
#define __OSL_GPU_VEC_H

#include <cuda.h>
#include <cassert>
#include <vector>
#include <iostream>

/* Lawlor NetRun CUDA utility functions: */
#define gpu_check(cudacall) { cudaError_t err=cudacall; if (err!=cudaSuccess) { std::cout<<"CUDA ERROR "<<cudaGetErrorString(err)<<" ("<<(int)err<<") at line "<<__LINE__<<": "<<#cudacall<<"\n"; exit(1); } }

/* CPU side class to represent an array in GPU memory */
template <class T>
class gpu_vec {
	T *ptr; // points to GPU memory
	unsigned long len; // number of T elements allocated
public:
	// Make a null vector
	gpu_vec() :ptr(0), len(0) {}
	// Make an empty (uninitialized) vector of this length
	gpu_vec(unsigned long size) :ptr(0),len(size) {
		gpu_check(cudaMalloc((void **)&ptr, len*sizeof(T)));
	}
	// Copy from this CPU-side vector
	gpu_vec(const std::vector<T> &vec) :ptr(0),len(vec.size()) {
		gpu_check(cudaMalloc((void **)&ptr, len*sizeof(T)));
		copy_from(vec);
	}
	// Deallocate the GPU data
	~gpu_vec() {
		if (ptr) gpu_check(cudaFree(ptr));
		ptr=0; len=0;
	}
	
	// Return the length of our vector
	unsigned long size() const { return len; }
	
	// Silently convert to a GPU T pointer.  
	//  This is useful for calling GPU kernels.
	//  It is NOT useful for accessing the data on the CPU side.
	operator T* (void) const { return ptr; }
	
	// Explicit memcpy to pull separate elements out.
	//   CAUTION: this is fairly slow; assign to a vector if you need lots of elements.
	const T operator[](unsigned long idx) const {
		T ret;
		gpu_check(cudaMemcpy(&ret,&ptr[idx],sizeof(T),cudaMemcpyDeviceToHost));
		return ret;
	}
	// Copy data from the GPU to this CPU side std::vector
	void copy_to(std::vector<T> &vec) const {
		vec.resize(len);
		gpu_check(cudaMemcpy(&vec[0],&ptr[0],len*sizeof(T),cudaMemcpyDeviceToHost));
	}
	
	// Copy data from this CPU side std::vector onto the GPU
	void copy_from(const std::vector<T> &vec) {
		assert(len==vec.size());
		gpu_check(cudaMemcpy(&ptr[0],&vec[0],len*sizeof(T),cudaMemcpyHostToDevice));
	}
	
private:
	// if gpuvec gets copied, the pointer will get freed twice, so do not allow copying.
	gpu_vec(const gpu_vec<T> &g);
	void operator=(const gpu_vec<T> &g);
};

#endif

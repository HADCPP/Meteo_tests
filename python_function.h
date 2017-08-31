#pragma once
#include<algorithm>
#include <vector>
#include <valarray>


namespace PYTHON_FUNCTION
{
	/*Return evenly spaced numbers over a specified interval.
	//
	//Returns `num` evenly spaced samples, calculated over the
	//interval[`start`, `stop`].*/
	template<typename T>
	inline void linspace(std::vector<T> *t, int start, int stop, int number)
	{

		int delta = int((stop - start) / (number - 1));
		for (int i = 0; i < number; i++)
			(*t).push_back(start + i*delta);
	}
	
	inline void arange(std::vector<int> *t, int stop, int start = 0)
	{

		for (int i = start; i < stop; i++)
			(*t).push_back(i);
	}
	template<typename T>
	inline std::valarray<T> arange(T stop, T start = 0, T step = 1)
	{
		int taille = int(ceil((stop - start) / step));
		std::valarray<T> val(taille);

		for (int j = 0; j < taille; j++, start = start + step)
			val[j] = start;
		return val;
		/*int taille = int((stop - start) / step);
		std::valarray<T> val(taille);
		int j = 0;
		for (int i = start, j = 0; i < stop; i += step, j++)
			val[j] = static_cast<T>(i);
		return val;*/
	}
	
	inline std::valarray<size_t> Arange(float stop, float start, float step)
	{
		int taille = int((stop - start) / step);
		std::valarray<size_t> val(taille);

		for (int j = 0; j < taille; j++, start = start + step)
			val[j] = int(ceil(start));
		return val;
	}
	/*Test whether each element of a 1 - D array is also present in a second array
	Returns a boolean array the same length as array 1 that is true where
	an element of array 1 is in array 2 and false otherwise
	@Docstring numpy.in1D python*/
	template<typename T,typename S>
	inline std::valarray<bool> in1D(std::valarray<T> v1, std::valarray<S> v2)
	{
		std::valarray<bool> vec(v1.size());
	
		for (int i = 0; i < v1.size(); i++)
		{
			if (std::binary_search(std::begin(v2), std::end(v2), static_cast<S>(v1[i])))
				vec[i]=true;
			else vec[i]=false;
		}
		return vec;
	}
	template<typename T>
	inline std::valarray<T> np_where(std::valarray<T> v1, T value)
	{
		std::valarray<T> vec(v1.size());
		for (int i = 0; i < v1.size(); i++)
		{
			if (v1[i] != value) vec[i]=i;
		}
		return vec;
	}
	template<typename T>
	inline std::valarray<std::size_t> npwhere(std::valarray<T> v1, T value)
	{
		std::valarray<std::size_t> vec(v1.size());
		for (size_t i = 0; i < v1.size(); i++)
		{
			if (v1[i] != value) vec[i] = i;
		}
		return vec;
	}
	template<typename T>
	inline std::vector<int> np_where_vec(std::valarray<T> v1, T value)
	{
		std::vector<int> vec;
		for (int i = 0; i < v1.size(); i++)
		{
			if (v1[i] != value) vec.push_back(i);
		}
		return vec;
	}
	template<typename T>
	inline std::vector<int> np_where(std::slice_array<T> v1, std::slice_array<T> v2)
	{
		std::vector<int> vec;
		std::valarray<T> vl1(v1.size()), vl2(v2.size());
		vl1 = v1;
		vl2 = v2;
		for (int i = 0; i < v1.size(); i++)
		{
			if (vl1[i]== vl2[i]) vec.push_back(i);
		}
		return vec;
	}
	/*
	    Return a valarray where value is deleted
		equivalent to compressed,masqued_equals here
	*/
	template<typename T>
	inline std::valarray <T> masked_values(std::valarray<T> v1, T value)
	{
		std::valarray<bool> masque(true,v1.size());
		for (int i = 0; i < v1.size(); i++)
		{
			if (v1[i] == value) masque[i]=false ;
		}
		return v1[masque];
	}
	template<typename T>
	inline std::valarray <T> histogram(std::valarray<float> data, std::valarray<float> bin)
	{
		std::valarray<float> hist(bin.size() - 1);

		bool lastbin = true;
		for (int i = 0; i < data.size(); i++)
			for (int j = 0; j < bin.size(); j++)
			{
				if (data[i] >= bin[j] && data[i] < bin[j + 1])
					hist[j]++;
				if (data[i] == bin[bin.size() - 1] && lastbin)
				{
					hist[bin.size() - 2]++;
					lastbin = false;
				}
			}
		return hist;
	}

	inline void concatenate(std::valarray<float> &val1, std::valarray<float> val2)
	{
		std::valarray<float> val(val1.size() + val2.size());
		for (size_t i = 0; i < val1.size(); i++)
			val[i] = val1[i];
		for (size_t i = val1.size(); i < val.size(); i++)
			val[i] = val2[i];
		val1.resize(val.size());
		val1 = val;
	}
}

#pragma once
#include<algorithm>
#include <vector>
#include <valarray>



class CMaskedArray
{
public:

	CMaskedArray::CMaskedArray(std::valarray<float> data)
	{
		m_fill_value = float(1e20);
		m_data = data;
		m_mask = false;
	}
		
	CMaskedArray::CMaskedArray(float data,size_t  size)
	{
		m_fill_value = float(1e20);
		m_data.resize(size);
		m_data = data;
		m_mask = false;
	}

	float fill_value(){ return m_fill_value; }
	std::valarray<float> data(){ return m_data; }
	std::valarray<bool>	mask(){ return m_mask; }

	bool nomask()
	{ 
		if (m_mask.max() == true)
			return false;
		else return true;
	}
	
	std::valarray<float> compressed(){ return m_data[!m_mask]; }

	void masked(size_t index)
	{
		m_mask[index] = true;
	}
	size_t size()
	{
		return m_data.size();
	}
protected:

	float m_fill_value;
	std::valarray<float> m_data;
	std::valarray<bool> m_mask; 
	std::vector<size_t> m_masked_indices;
	
};


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
		std::vector<size_t> index;
		for (int i = 0; i < v1.size(); i++)
		{
			if (v1[i] != value) index.push_back(i);
		}
		std::valarray<T> vec(index.size());
		std::copy(index.begin(), index.end(), std::begin(vec));
		return vec;
	}
	template<typename T>
	inline std::valarray<std::size_t> npwhere(std::valarray<T>& v1, T value,const std::string condition)
	{
		
		std::vector<size_t> index;
		switch (condition)
		{
		case "=":
			for (size_t i = 0; i < v1.size(); i++)
			{
				if (v1[i] == value) index.push_back(i);
			}
			break;
		case "!":
			for (size_t i = 0; i < v1.size(); i++)
			{
				if (v1[i] != value) index.push_back(i);
			}
			break;
		case "<=":
			for (size_t i = 0; i < v1.size(); i++)
			{
				if (v1[i] <= value) index.push_back(i);
			}
			break;
		case "<":
			for (size_t i = 0; i < v1.size(); i++)
			{
				if (v1[i] < value) index.push_back(i);
			}
			break;
		case ">":
			for (size_t i = 0; i < v1.size(); i++)
			{
				if (v1[i] > value) index.push_back(i);
			}
			break;
		case ">=":
			for (size_t i = 0; i < v1.size(); i++)
			{
				if (v1[i] >= value) index.push_back(i);
			}
			break;
		default:
			break;
		}
			std::valarray<std::size_t> vec(index.size());
			std::copy(index.begin(), index.end(), std::begin(vec));
			return vec;
		
		
	}
	template<typename T>
	inline std::valarray<std::size_t> npwhere(std::vector<T> v1, T value, const std::string condition)
	{

		std::vector<size_t> index;
		switch (condition)
		{
		case "=":
			for (size_t i = 0; i < v1.size(); i++)
			{
				if (v1[i] == value) index.push_back(i);
			}
			break;
		case "!":
			for (size_t i = 0; i < v1.size(); i++)
			{
				if (v1[i] != value) index.push_back(i);
			}
			break;
		case "<=":
			for (size_t i = 0; i < v1.size(); i++)
			{
				if (v1[i] <= value) index.push_back(i);
			}
			break;
		case "<":
			for (size_t i = 0; i < v1.size(); i++)
			{
				if (v1[i] < value) index.push_back(i);
			}
			break;
		case ">":
			for (size_t i = 0; i < v1.size(); i++)
			{
				if (v1[i] > value) index.push_back(i);
			}
			break;
		case ">=":
			for (size_t i = 0; i < v1.size(); i++)
			{
				if (v1[i] >= value) index.push_back(i);
			}
			break;
		default:
			break;
		}
		std::valarray<std::size_t> vec(index.size());
		std::copy(index.begin(), index.end(), std::begin(vec));
		return vec;


	}
	template<typename T>
	inline std::valarray<std::size_t> npwhereAnd(std::vector<T> v1, T value, T value2, const std::string condition1, const std::string condition2)
	{

		std::vector<size_t> index;
		if (condition1 == '<=')
		{
			if (condition2 == '>')
				for (size_t i = 0; i < v1.size(); i++)
				{
					if (v1[i] <= value && v1[i]> value2) index.push_back(i);
				}
		}
		if (condition1 == '<')
		{
			if (condition2 == '>')
				for (size_t i = 0; i < v1.size(); i++)
				{
					if (v1[i] < value && v1[i]> value2) index.push_back(i);
				}
		}
		if (condition1 == '<')
		{
			if (condition2 == '>=')
				for (size_t i = 0; i < v1.size(); i++)
				{
					if (v1[i] < value && v1[i] >= value2) index.push_back(i);
				}
		}
		if (condition1 == '<=')
		{
			if (condition2 == '>=')
				for (size_t i = 0; i < v1.size(); i++)
				{
					if (v1[i] < value && v1[i] >= value2) index.push_back(i);
				}
		}

		std::valarray<std::size_t> vec(index.size());
		std::copy(index.begin(), index.end(), std::begin(vec));
		return vec;


	}
	template<typename T>
	inline std::valarray<std::size_t> npwhereAnd(std::valarray<T> v1, T value, T value2, const std::string condition1, const std::string condition2)
	{

		std::vector<size_t> index;
		if (condition1 == '<=')
		{
			if (condition2 == '>')
				for (size_t i = 0; i < v1.size(); i++)
				{
					if (v1[i] <= value && v1[i]> value2) index.push_back(i);
				}
		}
		if (condition1 == '<')
		{
			if (condition2 == '>')
				for (size_t i = 0; i < v1.size(); i++)
				{
					if (v1[i] < value && v1[i]> value2) index.push_back(i);
				}
		}
		if (condition1 == '<')
		{
			if (condition2 == '>=')
				for (size_t i = 0; i < v1.size(); i++)
				{
					if (v1[i] < value && v1[i] >= value2) index.push_back(i);
				}
		}
		if (condition1 == '<=')
		{
			if (condition2 == '>=')
				for (size_t i = 0; i < v1.size(); i++)
				{
					if (v1[i] < value && v1[i] >= value2) index.push_back(i);
				}
		}
		std::valarray<std::size_t> vec(index.size());
		std::copy(index.begin(), index.end(), std::begin(vec));
		return vec;


	}
	template<typename T>
	inline std::valarray<std::size_t> npwhereOr(std::valarray<T> v1, T value, T value2, cconst std::string condition1, const std::string condition2)
	{

		std::vector<size_t> index;
		if (condition1 == '<=')
		{
			if (condition2 == '>')
				for (size_t i = 0; i < v1.size(); i++)
				{
					if (v1[i] <= value || v1[i]> value2) index.push_back(i);
				}
		}
		if (condition1 == '<')
		{
			if (condition2 == '>')
				for (size_t i = 0; i < v1.size(); i++)
				{
					if (v1[i] < value || v1[i]> value2) index.push_back(i);
				}
		}
		if (condition1 == '<')
		{
			if (condition2 == '>=')
				for (size_t i = 0; i < v1.size(); i++)
				{
					if (v1[i] < value || v1[i] >= value2) index.push_back(i);
				}
		}

		std::valarray<std::size_t> vec(index.size());
		std::copy(index.begin(), index.end(), std::begin(vec));
		return vec;


	}
	template<typename T>
	inline std::valarray<std::size_t> npwhere( CMaskedArray& v1, T value, char condition)
	{
		std::vector<size_t> index;
		

		switch (condition)
		{
		case '=':
			for (size_t i = 0; i < v1.data().size(); i++)
			{
				if (v1.data()[i] == value && v1.mask()[i]==false)
					index.push_back(i);
			
			}
			break;
		case '!':
			for (size_t i = 0; i < v1.data().size(); i++)
			{
				if (v1.data()[i] != value&& v1.mask()[i] == false)
					index.push_back(i);
			}
			break;
		case '<':
			for (size_t i = 0; i < v1.data().size(); i++)
			{
				if (v1.data()[i] < value&& v1.mask()[i] == false)
					index.push_back(i);
			}
			break;
		case '>':
			for (size_t i = 0; i < v1.data().size(); i++)
			{
				if (v1.data()[i] > value&& v1.mask()[i] == false)
					index.push_back(i);
			}
			break;
		default:
			break;
		}
		std::valarray<std::size_t> vec(index.size());
		std::copy(index.begin(), index.end(), std::begin(vec));
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

	template<typename T,typename S>
	CMaskedArray ma_masked_where(std::valarray<T> valaray1, T value, std::valarray<S> valaray2)
	{
		CMaskedArray dummy = CMaskedArray(valaray2);
		for (size_t i = 0; i < valaray1.size(); i++)
		{
			if (valaray1[i] == value)
			{
				dummy.masked(i);
			}
		}
		return dummy;
	}
	template<typename T>
	inline std::vector<int> np_where(std::slice_array<T> v1, std::slice_array<T> v2)
	{
		std::vector<int> vec;
		std::valarray<T> vl1 = v1, vl2=v2;
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
	template<typename T,typename S>
	inline std::valarray <T> histogram(std::vector<S> data, std::valarray<T> bin)
	{
		std::valarray<float> hist(bin.size() - 1);

		bool lastbin = true;
		for (int i = 0; i < data.size(); i++)
			for (int j = 0; j < bin.size(); j++)
			{
				if (T(data[i]) >= bin[j] && T(data[i]) < bin[j + 1])
					hist[j]++;
				if (T(data[i]) == bin[bin.size() - 1] && lastbin)
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

	template<typename T>
	void np_roll(std::valarray<T> &val, size_t value)
	{
		std::valarray<T> val1(val.size());

		for (size_t i = 0; i < value; i++)
			val1[i] = val[val.size() - value + i];

		for (size_t i = value; i < val.size(); i++)
			val1[i] = val[i - value];
		val = val1;
	}


	// recuperer l'indice du minimum de l'array (seule la première occurence est retournée)
	inline int np_argmin(const std::valarray<float>& val)
	{
		float min = val.min();
		int j = 0;
		for (int i = 0; i < val.size();i++)
		{
			if (val[i] == min)
			{
				j = i;
				break;
			}
		}
		return j;
	}
	// recuperer l'indice du maximun de l'array (seule la première occurence est retournée)
	inline int np_argmax(const std::valarray<float>& val)
	{
		float max= val.max();
		int j = 0;
		for (int i = 0; i < val.size(); i++)
		{
			if (val[i] == max)
			{
				j = i;
				break;
			}
		}
		return j;
	}
	
	template<typename T>
	inline std::vector<T> np_abs(std::vector<T> data)
	{
		std::vector<T> abs_value;
		for (T value : data)
			abs_value.push_back(std::abs(value));

		return abs_value;
	}

	template<typename T>
	inline std::vector<T> np_ceil(std::vector<T> data)
	{
		std::vector<T> ceil_value;
		for (T value : data)
			ceil_value.push_back(T(std::ceilf(value)));

		return ceil_value;
	}
}

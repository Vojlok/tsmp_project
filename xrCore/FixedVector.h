#ifndef FixedVectorH
#define FixedVectorH
#pragma once

template <class T, const int dim>
class svector 
{
public:
	typedef	size_t				size_type;
	typedef T					value_type;
	typedef value_type*			iterator;
	typedef const value_type*	const_iterator;
	typedef value_type&			reference;
	typedef const value_type&	const_reference;

private:
	value_type	array[dim];
	size_t			count;

public:
	svector() : count(0) 
	{}
	svector(iterator p, size_t c) 
	{ assign(p,c); }

	IC iterator	begin()						{ return array;							}
	IC iterator	end	 ()						{ return array+count;					}
	IC const_iterator	begin()	const		{ return array;							}
	IC const_iterator	end	 ()	const		{ return array+count;					}
	IC size_t		size()		const		{ return count;							}
	IC void		clear()						{ count=0;								}
	IC void		resize(size_t c)				{ VERIFY(c<=dim); count=c;				}
	IC void		reserve(int c)				{ }

	IC void		push_back(value_type e)		{ VERIFY(count<dim); array[count++]=e;	}
	IC void		pop_back()					{ VERIFY(count); count--;				}

	IC reference		operator[] (size_t id)			{ VERIFY(id<count); return array[id];	}
	IC const_reference	operator[] (size_t id)	const	{ VERIFY(id<count); return array[id];	}

	IC reference		front()				{ return array[0];						}
	IC reference		back()				{ return array[count-1];				}
	IC reference		last()				{ VERIFY(count<dim); return array[count];}
	IC const_reference	front() const		{ return array[0];						}
	IC const_reference	back()  const		{ return array[count-1];				}
	IC const_reference	last()  const		{ VERIFY(count<dim); return array[count];}
	IC void		inc	()						{ count++; }
	IC bool		empty()		const			{ return 0==count;	}

	IC void		erase(u32 id)				{
		VERIFY(id<count);
		count--;
		for (size_t i=id; i<count; i++)
			array[i] = array[i+1];
	}
	IC void		erase(iterator it)				{ erase(u32(it-begin()));	}

	IC void		insert(u32 id, reference V)
	{
		VERIFY(id<count);
		for (size_t i=count; i>int(id); i--)	array[i] = array[i-1];
		count++;
		array[id] = V;
	}
	IC void		assign(iterator p, size_t c) { VERIFY(c>0 && c<dim); CopyMemory(array,p,c*sizeof(value_type)); count=c; }
	IC BOOL		equal (const svector<value_type,dim>& base) const
	{
		if (size()!=base.size())	return FALSE;
		for (size_t cmp=0; cmp<size(); cmp++)	if ((*this)[cmp]!=base[cmp])	return FALSE;
		return TRUE;
	}
};

#endif
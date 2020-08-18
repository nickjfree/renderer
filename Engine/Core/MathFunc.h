/*
	Commen functions will used in the engine
*/
#ifndef __MATH_FUNC__
#define __MATH_FUNC__


//djb2 hash functions
constexpr inline unsigned int djb2_hash(unsigned char* str)
{
	unsigned long hash = 5381;
	int c = 0;

	while (c = *str++)
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

	return hash;
}


//sdbm hash functions
inline unsigned int sdbm_hash(unsigned char* str)
{
	unsigned long hash = 0;
	int c;

	while (c = *str++)
		hash = c + (hash << 6) + (hash << 16) - hash;

	return hash;

}

#endif
/*
	Commen functions will used in the engine
*/
#ifndef __MATH_FUNC__
#define __MATH_FUNC__


//djb2 hash functions
inline unsigned int djb2_hash(unsigned char* str)
{
	unsigned long hash = 5381;
	int c = 0;

	while (c = *str++)
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

	return hash;
}

//djb2 hash functions first
template<unsigned int N>
constexpr __forceinline unsigned int djb2_hash(const char (&str)[N])
{
	unsigned long hash = 5381;
	return djb2_hash<N - 1>(str + 1, ((hash << 5) + hash) + str[0]);
}

//djb2 hash functions interation
template<unsigned int N>
constexpr __forceinline unsigned int djb2_hash(const char* str, unsigned int hash)
{
	return djb2_hash<N - 1>(str + 1, ((hash << 5) + hash) + str[0]);
}

//djb2 hash functions end
template<>
constexpr __forceinline unsigned int djb2_hash<1>(const char* str, unsigned int hash)
{
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
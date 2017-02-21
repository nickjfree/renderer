#ifndef __PSOCACHE__
#define __PSOCACHE__

#include <d3d12.h>


class PSOCache {

private:
	// hash value
	unsigned int hash;
public:
	// dirty flag
	int Dirty;
	// ids of shaders
	int VS;
	int PS;
	int GS;
	int DS;
	int HS;
	// id of render state
	int Depth;
	int Rasterizer;
	int Blend;
	// int input element
	int InputLayout;
	// render target and format;
	int NumRTV;
	DXGI_FORMAT RTVFormat[8];
	DXGI_FORMAT DSVFormat;
public:
	// operator int
	operator int();
	bool operator = (PSOCache& rh);
	PSOCache();
	virtual ~PSOCache();
};

#endif 


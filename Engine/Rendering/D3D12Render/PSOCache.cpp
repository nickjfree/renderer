#include "PSOCache.h"


using namespace D3D12API;

PSOCache::PSOCache(): hash(-1), VS(-1), PS(-1), GS(-1), DS(-1), HS(-1), InputLayout(-1), NumRTV(0), 
Depth(-1), Rasterizer(-1), Blend(-1), Dirty(0), Top(R_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED) {
	memset(RTVFormat, -1, sizeof(DXGI_FORMAT) * 8);
	DSVFormat = DXGI_FORMAT_FORCE_UINT;
}


PSOCache::~PSOCache() {
}

PSOCache::operator int() const {
    if (!Dirty) {
        return hash;
    } else {
        hash = 0;
        hash = (hash << 5) + VS;
        hash = (hash << 5) + PS;
        hash = (hash << 5) + GS;
        hash = (hash << 5) + DS;
        hash = (hash << 5) + HS;
        hash = (hash << 5) + InputLayout;
        hash = (hash << 5) + Depth;
        hash = (hash << 5) + Rasterizer;
        hash = (hash << 5) + Blend;
        return hash;
    }
}

bool PSOCache::operator==(PSOCache& rh) {
	if (VS == rh.VS && PS == rh.PS && GS == rh.GS && DS == rh.DS && HS == rh.HS) {
		if (NumRTV == rh.NumRTV && !memcmp(RTVFormat, rh.RTVFormat, sizeof(DXGI_FORMAT) * NumRTV) && DSVFormat == rh.DSVFormat && Top == rh.Top) {
			if (Depth == rh.Depth && Rasterizer == rh.Rasterizer && Blend == rh.Blend) {
				return true;
			}
		}
	}
	return false;
}

bool PSOCache::operator!=(PSOCache& rh) {
	if (VS == rh.VS && PS == rh.PS && GS == rh.GS && DS == rh.DS && HS == rh.HS) {
		if (NumRTV == rh.NumRTV && !memcmp(RTVFormat, rh.RTVFormat, sizeof(DXGI_FORMAT) * NumRTV) && DSVFormat == rh.DSVFormat && Top == rh.Top) {
			if (Depth == rh.Depth && Rasterizer == rh.Rasterizer && Blend == rh.Blend) {
				return false;
			}
		}
	}
	return true;
}

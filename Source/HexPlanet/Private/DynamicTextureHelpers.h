#pragma once

class DynamicTextureHelpers
{
public:
	// Create a dynamic texture intended to be used for passing non-texture data
	// into materials. Defaults to 32-bit RGBA. The texture is not added to the
	// root set, so something else will need to hold a reference to it.
	static UTexture2D* CreateTransientTexture(int32 Width, int32 Height, EPixelFormat PixelFormat = PF_A32B32G32R32F);
	// Updates a region of a texture with the supplied input data. Does nothing
	// if the pixel formats do not match.
	static void UpdateTextureRegion(UTexture2D* Texture, int32 MipIndex, FUpdateTextureRegion2D Region, uint32 SrcPitch, uint32 SrcBpp, uint8* SrcData, bool bFreeData);
	// Convenience wrapper for updating a dynamic texture with an array of
	// FLinearColors.
	static void UpdateDynamicVectorTexture(const TArray<FLinearColor>& Source, UTexture2D* Texture);
	
};
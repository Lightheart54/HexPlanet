#include "HexPlanet.h"
#include "DynamicTextureHelpers.h"

void DynamicTextureHelpers::UpdateTextureRegion(UTexture2D* Texture, int32 MipIndex, FUpdateTextureRegion2D Region, uint32 SrcPitch, uint32 SrcBpp, uint8* SrcData, bool bFreeData)
{
	if (Texture->Resource)
	{
		struct FUpdateTextureRegionsData
		{
			FTexture2DResource* Texture2DResource;
			int32 MipIndex;
			FUpdateTextureRegion2D Region;
			uint32 SrcPitch;
			uint32 SrcBpp;
			uint8* SrcData;
		};

		FUpdateTextureRegionsData* RegionData = new FUpdateTextureRegionsData;

		RegionData->Texture2DResource = (FTexture2DResource*)Texture->Resource;
		RegionData->MipIndex = MipIndex;
		RegionData->Region = Region;
		RegionData->SrcPitch = SrcPitch;
		RegionData->SrcBpp = SrcBpp;
		RegionData->SrcData = SrcData;

		{

			ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
				UpdateTextureRegionsData,
				FUpdateTextureRegionsData*, RegionData, RegionData,
				bool, bFreeData, bFreeData,
				{
					int32 CurrentFirstMip = RegionData->Texture2DResource->GetCurrentFirstMip();
			if (RegionData->MipIndex >= CurrentFirstMip)
			{
				RHIUpdateTexture2D(
					RegionData->Texture2DResource->GetTexture2DRHI(),
					RegionData->MipIndex - CurrentFirstMip,
					RegionData->Region,
					RegionData->SrcPitch,
					RegionData->SrcData
					+ RegionData->Region.SrcY * RegionData->SrcPitch
					+ RegionData->Region.SrcX * RegionData->SrcBpp
					);
			}
			// TODO is this leaking if we never set this to true??
			if (bFreeData)
			{
				FMemory::Free(RegionData->SrcData);
			}
			delete RegionData;
				});

		}
	}
}

void DynamicTextureHelpers::UpdateDynamicVectorTexture(const TArray<FLinearColor>& Source, UTexture2D* Texture)
{
	// Only handles 32-bit float textures
	if (!Texture || Texture->GetPixelFormat() != PF_A32B32G32R32F) return;
	// Shouldn't do anything if there's no data
	if (Source.Num() < 1) return;

	UpdateTextureRegion(Texture, 0, FUpdateTextureRegion2D(0, 0, 0, 0, Texture->GetSizeX(), Texture->GetSizeY()), Texture->GetSizeX() * sizeof(FLinearColor), sizeof(FLinearColor), (uint8*)Source.GetData(), false);
}

UTexture2D* DynamicTextureHelpers::CreateTransientTexture(int32 Width, int32 Height, EPixelFormat PixelFormat /*= PF_A32B32G32R32F*/)
{
	UTexture2D* Texture = UTexture2D::CreateTransient(Width, Height, PixelFormat);
	if (Texture)
	{
		Texture->CompressionSettings = TextureCompressionSettings::TC_VectorDisplacementmap;
		Texture->SRGB = 0;
		Texture->UpdateResource();
	}
	return Texture;
}
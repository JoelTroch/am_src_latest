#ifndef DDS_HEADERS
#define DDS_HEADERS

struct DDCOLORKEY
{
	DWORD       dwColorSpaceLowValue;   // low boundary of color space that is to
	// be treated as Color Key, inclusive
	DWORD       dwColorSpaceHighValue;  // high boundary of color space that is
	// to be treated as Color Key, inclusive
};

//
//DDPIXELFORMAT
//
struct DDPIXELFORMAT
{
	DWORD       dwSize;                 // size of structure
	DWORD       dwFlags;                // pixel format flags
	DWORD       dwFourCC;               // (FOURCC code)
	union
	{
		DWORD   dwRGBBitCount;          // how many bits per pixel
		DWORD   dwYUVBitCount;          // how many bits per pixel
		DWORD   dwZBufferBitDepth;      // how many total bits/pixel in z buffer (including any stencil bits)
		DWORD   dwAlphaBitDepth;        // how many bits for alpha channels
		DWORD   dwLuminanceBitCount;    // how many bits per pixel
		DWORD   dwBumpBitCount;         // how many bits per "buxel", total
		DWORD   dwPrivateFormatBitCount;// Bits per pixel of private driver formats. Only valid in texture
		// format list and if DDPF_D3DFORMAT is set
	};
	union
	{
		DWORD   dwRBitMask;             // mask for red bit
		DWORD   dwYBitMask;             // mask for Y bits
		DWORD   dwStencilBitDepth;      // how many stencil bits (note: dwZBufferBitDepth-dwStencilBitDepth is total Z-only bits)
		DWORD   dwLuminanceBitMask;     // mask for luminance bits
		DWORD   dwBumpDuBitMask;        // mask for bump map U delta bits
		DWORD   dwOperations;           // DDPF_D3DFORMAT Operations
	};
	union
	{
		DWORD   dwGBitMask;             // mask for green bits
		DWORD   dwUBitMask;             // mask for U bits
		DWORD   dwZBitMask;             // mask for Z bits
		DWORD   dwBumpDvBitMask;        // mask for bump map V delta bits
		struct
		{
			WORD    wFlipMSTypes;       // Multisample methods supported via flip for this D3DFORMAT
			WORD    wBltMSTypes;        // Multisample methods supported via blt for this D3DFORMAT
		} MultiSampleCaps;

	};
	union
	{
		DWORD   dwBBitMask;             // mask for blue bits
		DWORD   dwVBitMask;             // mask for V bits
		DWORD   dwStencilBitMask;       // mask for stencil bits
		DWORD   dwBumpLuminanceBitMask; // mask for luminance in bump map
	};
	union
	{
		DWORD   dwRGBAlphaBitMask;      // mask for alpha channel
		DWORD   dwYUVAlphaBitMask;      // mask for alpha channel
		DWORD   dwLuminanceAlphaBitMask;// mask for alpha channel
		DWORD   dwRGBZBitMask;          // mask for Z channel
		DWORD   dwYUVZBitMask;          // mask for Z channel
	};
};



//
// DDSCAPS2
// 
struct DDSCAPS2
{
	DWORD       dwCaps;         // capabilities of surface wanted
	DWORD       dwCaps2;
	DWORD       dwCaps3;
	union
	{
		DWORD       dwCaps4;
		DWORD       dwVolumeDepth;
	};
};



//
// DDSURFACEDESC2
//
struct DDSURFACEDESC2
{
	DWORD               dwSize;                 // size of the DDSURFACEDESC structure
	DWORD               dwFlags;                // determines what fields are valid
	DWORD               dwHeight;               // height of surface to be created
	DWORD               dwWidth;                // width of input surface
	union
	{
		LONG            lPitch;                 // distance to start of next line (return value only)
		DWORD           dwLinearSize;           // Formless late-allocated optimized surface size
	};
	union
	{
		DWORD           dwBackBufferCount;      // number of back buffers requested
		DWORD           dwDepth;                // the depth if this is a volume texture 
	};
	union
	{
		DWORD           dwMipMapCount;          // number of mip-map levels requestde
		// dwZBufferBitDepth removed, use ddpfPixelFormat one instead
		DWORD           dwRefreshRate;          // refresh rate (used when display mode is described)
		DWORD           dwSrcVBHandle;          // The source used in VB::Optimize
	};
	DWORD               dwAlphaBitDepth;        // depth of alpha buffer requested
	DWORD               dwReserved;             // reserved
	LPVOID              lpSurface;              // pointer to the associated surface memory
	union
	{
		DDCOLORKEY      ddckCKDestOverlay;      // color key for destination overlay use
		DWORD           dwEmptyFaceColor;       // Physical color for empty cubemap faces
	};
	DDCOLORKEY          ddckCKDestBlt;          // color key for destination blt use
	DDCOLORKEY          ddckCKSrcOverlay;       // color key for source overlay use
	DDCOLORKEY          ddckCKSrcBlt;           // color key for source blt use
	union
	{
		DDPIXELFORMAT   ddpfPixelFormat;        // pixel format description of the surface
		DWORD           dwFVF;                  // vertex format description of vertex buffers
	};
	DDSCAPS2            ddsCaps;                // direct draw surface capabilities
	DWORD               dwTextureStage;         // stage in multitexture cascade
};



#define FOURCC_DXT1	MAKEFOURCC('D', 'X', 'T', '1')
#define FOURCC_DXT3	MAKEFOURCC('D', 'X', 'T', '3')
#define FOURCC_DXT5	MAKEFOURCC('D', 'X', 'T', '5')


#endif
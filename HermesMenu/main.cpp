#include "stdafx.h"
#include <xtl.h>
#include <string>
#include <math.h>
#include <stdlib.h>

#include "audioPlayer.h"
#include "component.h"
#include "context.h"
#include "drawing.h"
#include "inputManager.h"
#include "xboxinternals.h"
#include "meshUtility.h"
#include "utils.h"
#include "resources.h"
#include "stringUtility.h"
#include "theme.h"
#include "ssfn.h"
#include "driveManager.h"
#include "Scenes\sceneManager.h"
#include "stl_loader.h"

#include <xgraphics.h>

#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_TEX1)

typedef struct {
    DWORD dwWidth;
    DWORD dwHeight;
    BOOL  fProgressive;
    BOOL  fWideScreen;
	DWORD dwFreq;
} DISPLAY_MODE;

DISPLAY_MODE displayModes[] =
{
    //{   720,    480,    TRUE,   TRUE,  60 },         // 720x480 progressive 16x9
    //{   720,    480,    TRUE,   FALSE, 60 },         // 720x480 progressive 4x3
    //{   720,    480,    FALSE,  TRUE,  50 },         // 720x480 interlaced 16x9 50Hz
    //{   720,    480,    FALSE,  FALSE, 50 },         // 720x480 interlaced 4x3  50Hz
    //{   720,    480,    FALSE,  TRUE,  60 },         // 720x480 interlaced 16x9
    //{   720,    480,    FALSE,  FALSE, 60 },         // 720x480 interlaced 4x3


	// Width  Height Progressive Widescreen

	// HDTV Progressive Modes
    {  1280,    720,    TRUE,   TRUE,  60 },         // 1280x720 progressive 16x9

	// EDTV Progressive Modes
    {   720,    480,    TRUE,   TRUE,  60 },         // 720x480 progressive 16x9
    {   640,    480,    TRUE,   TRUE,  60 },         // 640x480 progressive 16x9
    {   720,    480,    TRUE,   FALSE, 60 },         // 720x480 progressive 4x3
    {   640,    480,    TRUE,   FALSE, 60 },         // 640x480 progressive 4x3

	// HDTV Interlaced Modes
	//    {  1920,   1080,    FALSE,  TRUE,  60 },         // 1920x1080 interlaced 16x9

	// SDTV PAL-50 Interlaced Modes
    {   720,    480,    FALSE,  TRUE,  50 },         // 720x480 interlaced 16x9 50Hz
    {   640,    480,    FALSE,  TRUE,  50 },         // 640x480 interlaced 16x9 50Hz
    {   720,    480,    FALSE,  FALSE, 50 },         // 720x480 interlaced 4x3  50Hz
    {   640,    480,    FALSE,  FALSE, 50 },         // 640x480 interlaced 4x3  50Hz

	// SDTV NTSC / PAL-60 Interlaced Modes
    {   720,    480,    FALSE,  TRUE,  60 },         // 720x480 interlaced 16x9
    {   640,    480,    FALSE,  TRUE,  60 },         // 640x480 interlaced 16x9
    {   720,    480,    FALSE,  FALSE, 60 },         // 720x480 interlaced 4x3
    {   640,    480,    FALSE,  FALSE, 60 },         // 640x480 interlaced 4x3
};

#define NUM_MODES (sizeof(displayModes) / sizeof(displayModes[0]))

bool supportsMode(DISPLAY_MODE mode, DWORD dwVideoStandard, DWORD dwVideoFlags)
{
    if (mode.dwFreq == 60 && !(dwVideoFlags & XC_VIDEO_FLAGS_PAL_60Hz) && (dwVideoStandard == XC_VIDEO_STANDARD_PAL_I))
	{
		return false;
	}    
    if (mode.dwFreq == 50 && (dwVideoStandard != XC_VIDEO_STANDARD_PAL_I))
	{
		return false;
	}
    if (mode.dwHeight == 480 && mode.fWideScreen && !(dwVideoFlags & XC_VIDEO_FLAGS_WIDESCREEN ))
	{
		return false;
	}
    if (mode.dwHeight == 480 && mode.fProgressive && !(dwVideoFlags & XC_VIDEO_FLAGS_HDTV_480p))
	{
		return false;
	}
    if (mode.dwHeight == 720 && !(dwVideoFlags & XC_VIDEO_FLAGS_HDTV_720p))
	{
		return false;
	}
    if (mode.dwHeight == 1080 && !(dwVideoFlags & XC_VIDEO_FLAGS_HDTV_1080i))
	{
		return false;
	}
    return true;
}

bool createDevice()
{
	uint32_t videoFlags = XGetVideoFlags();
	uint32_t videoStandard = XGetVideoStandard();
	uint32_t currentMode;
    for (currentMode = 0; currentMode < NUM_MODES-1; currentMode++)
    {
		if (supportsMode(displayModes[currentMode], videoStandard, videoFlags)) 
		{
			break;
		}
    } 

	LPDIRECT3D8 d3d = Direct3DCreate8(D3D_SDK_VERSION);
    if(d3d == NULL)
	{
		utils::debugPrint("Failed to create d3d\n");
        return false;
	}

	context::setBufferWidth(720);
	context::setBufferHeight(480);
	//context::setBufferWidth(displayModes[currentMode].dwWidth);
	//context::setBufferHeight(displayModes[currentMode].dwHeight);

	D3DPRESENT_PARAMETERS params; 
    ZeroMemory(&params, sizeof(params));
	params.BackBufferWidth = displayModes[currentMode].dwWidth;
    params.BackBufferHeight = displayModes[currentMode].dwHeight;
	params.Flags = displayModes[currentMode].fProgressive ? D3DPRESENTFLAG_PROGRESSIVE : D3DPRESENTFLAG_INTERLACED;
    params.Flags |= displayModes[currentMode].fWideScreen ? D3DPRESENTFLAG_WIDESCREEN : 0;
    params.FullScreen_RefreshRateInHz = displayModes[currentMode].dwFreq;
	params.BackBufferFormat = D3DFMT_X8R8G8B8;
    params.BackBufferCount = 1;
    params.EnableAutoDepthStencil = FALSE;
	params.SwapEffect = D3DSWAPEFFECT_DISCARD;
    params.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;

	LPDIRECT3DDEVICE8 d3dDevice;
    if (FAILED(d3d->CreateDevice(0, D3DDEVTYPE_HAL, NULL, D3DCREATE_HARDWARE_VERTEXPROCESSING, &params, &d3dDevice)))
	{
		utils::debugPrint("Failed to create device\n");
        return false;
	}
	context::setD3dDevice(d3dDevice);
    
    context::getD3dDevice()->SetRenderState(D3DRS_ZFUNC, D3DCMP_NEVER);
	context::getD3dDevice()->SetRenderState(D3DRS_LIGHTING, FALSE);
	context::getD3dDevice()->SetVertexShader(D3DFVF_CUSTOMVERTEX);
	context::getD3dDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	context::getD3dDevice()->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	context::getD3dDevice()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    // Enable fog
    context::getD3dDevice()->SetRenderState(D3DRS_FOGENABLE, TRUE);
    context::getD3dDevice()->SetRenderState(D3DRS_FOGCOLOR, 0xff000000); // Black fog
    context::getD3dDevice()->SetRenderState(D3DRS_FOGTABLEMODE, D3DFOG_LINEAR);
    float fogStart = 7.0f;
    float fogEnd = 20.0f;
    context::getD3dDevice()->SetRenderState(D3DRS_FOGSTART, *(DWORD*)&fogStart);
    context::getD3dDevice()->SetRenderState(D3DRS_FOGEND, *(DWORD*)&fogEnd);

	context::getD3dDevice()->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    context::getD3dDevice()->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
    context::getD3dDevice()->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    context::getD3dDevice()->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
    context::getD3dDevice()->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    context::getD3dDevice()->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

	context::getD3dDevice()->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
	context::getD3dDevice()->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
	context::getD3dDevice()->SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

	context::getD3dDevice()->BeginScene();
	context::getD3dDevice()->Clear(0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER|D3DCLEAR_STENCIL, 0xff000000, 1.0f, 0L);
	context::getD3dDevice()->EndScene();
	context::getD3dDevice()->Present(NULL, NULL, NULL, NULL);

	return true;
}

void render_sphere(float angle1, float angle2, float angle3, utils::dataContainer* sphereMesh)
{
    const float deg_to_rad = 0.01745329252f;

    D3DXMATRIX matWorld;
    D3DXMatrixIdentity(&matWorld);

    D3DXMATRIX tmp;
    D3DXMatrixRotationX(&tmp, angle1 * deg_to_rad);
    matWorld *= tmp;
    D3DXMatrixRotationY(&tmp, angle2 * deg_to_rad);
    matWorld *= tmp;
    D3DXMatrixRotationZ(&tmp, angle3 * deg_to_rad);
    matWorld *= tmp;

    D3DXVECTOR3 cameraPosition(0.0f, 0.1f, 9.9f);
    D3DXVECTOR3 target(0.0f, 0.0f, 7.0f);
    D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);   

    D3DXMATRIX matView;
    D3DXMatrixLookAtLH(&matView, &cameraPosition, &target, &up);

    D3DXMATRIX matProjection;
    D3DXMatrixIdentity(&matProjection);
    D3DXMatrixPerspectiveFovLH(&matProjection, 45 * deg_to_rad, (float)context::getBufferWidth() / (float)context::getBufferHeight(), 0.1f, 100.0f);
   
    context::getD3dDevice()->SetTransform( D3DTS_WORLD, &matWorld);
    context::getD3dDevice()->SetTransform(D3DTS_VIEW, &matView);
    context::getD3dDevice()->SetTransform(D3DTS_PROJECTION, &matProjection);

    context::getD3dDevice()->SetRenderState(D3DRS_TEXTUREFACTOR, 0xffff5aff);
    context::getD3dDevice()->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    context::getD3dDevice()->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID); 

    image* imageToDraw = drawing::getImage("base");
    context::getD3dDevice()->SetTexture(0, imageToDraw->texture);
    context::getD3dDevice()->DrawPrimitiveUP(D3DPT_TRIANGLELIST, (sphereMesh->size / sizeof(meshUtility::vertex)) / 3, sphereMesh->data, sizeof(meshUtility::vertex));
}

void render_logo(float angle1, float angle2, float angle3, utils::dataContainer* logoMesh)
{
    const float deg_to_rad = 0.01745329252f;

    D3DXMATRIX matWorld;
    D3DXMatrixIdentity(&matWorld);

    D3DXMATRIX tmp;
    D3DXMatrixRotationX(&tmp, angle1 * deg_to_rad);
    matWorld *= tmp;
    D3DXMatrixRotationY(&tmp, angle2 * deg_to_rad);
    matWorld *= tmp;
    D3DXMatrixRotationZ(&tmp, angle3 * deg_to_rad);
    matWorld *= tmp;

    D3DXVECTOR3 cameraPosition(0.0f, 0.1f, 9.9f);
    D3DXVECTOR3 target(0.0f, 0.0f, 7.0f);
    D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);   

    D3DXMATRIX matView;
    D3DXMatrixLookAtLH(&matView, &cameraPosition, &target, &up);

    D3DXMATRIX matProjection;
    D3DXMatrixIdentity(&matProjection);
    D3DXMatrixPerspectiveFovLH(&matProjection, 45 * deg_to_rad, (float)context::getBufferWidth() / (float)context::getBufferHeight(), 0.1f, 100.0f);
   
    context::getD3dDevice()->SetTransform( D3DTS_WORLD, &matWorld);
    context::getD3dDevice()->SetTransform(D3DTS_VIEW, &matView);
    context::getD3dDevice()->SetTransform(D3DTS_PROJECTION, &matProjection);

    context::getD3dDevice()->SetRenderState(D3DRS_TEXTUREFACTOR, 0xffff5aff);
    context::getD3dDevice()->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    context::getD3dDevice()->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID); 

    image* imageToDraw = drawing::getImage("base");
    context::getD3dDevice()->SetTexture(0, imageToDraw->texture);

    // Render in smaller batches to avoid push-buffer size warnings
    uint32_t totalTriangles = (logoMesh->size / sizeof(meshUtility::vertex)) / 3;
    uint32_t batchSize = 1000; // Render 1000 triangles per batch
    uint32_t vertexSize = sizeof(meshUtility::vertex);
    
    for (uint32_t i = 0; i < totalTriangles; i += batchSize)
    {
        uint32_t trianglesInBatch = (i + batchSize > totalTriangles) ? (totalTriangles - i) : batchSize;
        uint32_t vertexOffset = i * 3;
        char* batchData = logoMesh->data + (vertexOffset * vertexSize);
        
        context::getD3dDevice()->DrawPrimitiveUP(D3DPT_TRIANGLELIST, trianglesInBatch, batchData, vertexSize);
    }
}

void render_starfield()
{
    static float time = 0.0f;
    time += 0.5f;
    if (time > 10000.0f) time = 0.0f;

    const int numStars = 200;
    static float starX[200];
    static float starY[200];
    static float starZ[200];
    static bool initialized = false;
    
    // Initialize stars on first call
    if (!initialized) {
        for (int i = 0; i < numStars; i++) {
            starX[i] = ((float)(rand() % context::getBufferWidth()) - context::getBufferWidth() / 2.0f) / (context::getBufferWidth() / 2.0f);
            starY[i] = ((float)(rand() % context::getBufferHeight()) - context::getBufferHeight() / 2.0f) / (context::getBufferHeight() / 2.0f);
            starZ[i] = (float)(rand() % 1000) / 1000.0f;
        }
        initialized = true;
    }

    // Setup rendering state
    context::getD3dDevice()->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    context::getD3dDevice()->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
    context::getD3dDevice()->SetTexture(0, NULL);
    context::getD3dDevice()->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG2);
    context::getD3dDevice()->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
    context::getD3dDevice()->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
    context::getD3dDevice()->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);

    float centerX = context::getBufferWidth() / 2.0f;
    float centerY = context::getBufferHeight() / 2.0f;
    float speed = 0.02f;

    for (int i = 0; i < numStars; i++) {
        // Move star toward camera
        starZ[i] -= speed;
        
        // Reset star if it goes behind camera
        if (starZ[i] <= 0.0f) {
            starZ[i] = 1.0f;
            starX[i] = ((float)(rand() % context::getBufferWidth()) - context::getBufferWidth() / 2.0f) / (context::getBufferWidth() / 2.0f);
            starY[i] = ((float)(rand() % context::getBufferHeight()) - context::getBufferHeight() / 2.0f) / (context::getBufferHeight() / 2.0f);
        }
        
        // Project 3D star to 2D screen
        float scale = 1.0f / (starZ[i] + 0.1f);
        float screenX = centerX + starX[i] * centerX * scale;
        float screenY = centerY + starY[i] * centerY * scale;
        
        // Calculate star size and brightness based on distance
        float starSize = 2.0f * scale;
        if (starSize < 1.0f) starSize = 1.0f;
        if (starSize > 4.0f) starSize = 4.0f;
        
        // Retro colors - brighter stars are more colorful
        uint32_t starColor;
        float brightness = 1.0f - starZ[i];
        if (brightness > 0.7f) {
            starColor = 0xFFFFFFFF; // White for close stars
        } else if (brightness > 0.4f) {
            starColor = 0xFF00FFFF; // Cyan
        } else {
            starColor = 0xFF0080FF; // Blue
        }
        
        // Draw star as a small quad
        context::getD3dDevice()->SetRenderState(D3DRS_TEXTUREFACTOR, starColor);
        utils::dataContainer* starVertices = meshUtility::createQuadXY(
            math::vec3F(screenX - starSize / 2.0f, (float)context::getBufferHeight() - (screenY + starSize / 2.0f), 0.0f),
            math::sizeF(starSize, starSize),
            math::rectF(0.0f, 0.0f, 1.0f, 1.0f)
        );
        context::getD3dDevice()->DrawPrimitiveUP(
            D3DPT_TRIANGLELIST,
            (starVertices->size / sizeof(meshUtility::vertex)) / 3,
            starVertices->data,
            sizeof(meshUtility::vertex)
        );
        delete starVertices;
    }
    
    // Restore texture stage state
    context::getD3dDevice()->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    context::getD3dDevice()->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    context::getD3dDevice()->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
    context::getD3dDevice()->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    context::getD3dDevice()->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    context::getD3dDevice()->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
}

void __cdecl main()
{
	utils::debugPrint("Welcome to Cerbios Recovery\n");

	bool deviceCreated = createDevice();

	driveManager::mountDrive("HDD0-C");
	driveManager::mountDrive("HDD0-E");
	
	context::setImageMap(new pointerMap<image*>(true));
	theme::loadSkin();

	drawing::loadFont(&font_sfn[0]);

    bitmapFont* fontExtraSmall = drawing::generateBitmapFont("FreeSans", SSFN_STYLE_REGULAR, 16, 16, 0, 256);
	context::setBitmapFontExtraSmall(fontExtraSmall);
	bitmapFont* fontSmall = drawing::generateBitmapFont("FreeSans", SSFN_STYLE_REGULAR, 18, 18, 0, 256);
	context::setBitmapFontSmall(fontSmall);
	bitmapFont* fontMedium = drawing::generateBitmapFont("FreeSans", SSFN_STYLE_REGULAR, 25, 25, 0, 256);
	context::setBitmapFontMedium(fontMedium);
	bitmapFont* fontLarge = drawing::generateBitmapFont("FreeSans", SSFN_STYLE_REGULAR, 32, 32, 0, 512);
	context::setBitmapFontLarge(fontLarge);

	drawing::renderRoundedRect("panel-fill", 24, 24, 6, 0xffffffff, 0x00000000, 0);
	drawing::renderRoundedRect("panel-stroke", 24, 24, 6, 0x01010100, 0xffffffff, 2);

    utils::dataContainer* sphereMesh = meshUtility::createSphere();
    utils::dataContainer* logoMesh = stl_loader::loadFromBinary(logo_stl, logo_stl_len);
    drawing::loadImage((char*)base_jpg, sizeof(base_jpg), "base");

    audioPlayer::init();

    sceneManager::pushScene(sceneItemMainScene);
    
    float angle1 = 0;
    float angle2 = 0;
    float angle3 = 0;
    while (TRUE)
    {
		inputManager::processController();

		drawing::clearBackground((uint32_t)0);

        // Render Sphere Background

        render_sphere(-angle1, -angle2, -angle3, sphereMesh); 
        render_logo(0.0f, angle3, 0.0f, logoMesh);

	    // Setup Ortho Camera 

        D3DXMATRIX matIdentity;
        D3DXMatrixIdentity(&matIdentity);

        D3DXMATRIX matProjection;
	    D3DXMatrixOrthoOffCenterLH(&matProjection, 0, (float)context::getBufferWidth(), 0, (float)context::getBufferHeight(), 1.0f, 800.0f);

        context::getD3dDevice()->SetTransform(D3DTS_VIEW, &matIdentity);
        context::getD3dDevice()->SetTransform(D3DTS_WORLD, &matIdentity);
        context::getD3dDevice()->SetTransform(D3DTS_PROJECTION, &matProjection);
        context::getD3dDevice()->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
        context::getD3dDevice()->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

        // Render Retro Starfield Effect
        render_starfield();

        // Process Input / Render Menu

        sceneManager::getScene()->update();
        sceneManager::getScene()->render();

        // Present Rendered Results

		context::getD3dDevice()->EndScene();
		context::getD3dDevice()->Present(NULL, NULL, NULL, NULL);

        angle1 += 0.10f;
        if (angle1 > 360.0f)
        {
            angle1 -= 360.f;
        }

        angle2 += 0.20f;
        if (angle2 > 360.0f)
        {
            angle2 -= 360.f;
        }

        angle3 += 0.40f;
        if (angle3 > 360.0f)
        {
            angle3 -= 360.f;
        }
    }
}
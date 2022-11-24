//=============================================================================
//
// ライト処理 [light.cpp]
// Author : 
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "debugproc.h"
//*****************************************************************************
// マクロ定義
//*****************************************************************************



//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
float DtoR(float degree);
XMFLOAT3 RotZ(struct LIGHT* light);
XMFLOAT3 NorLightDir(struct LIGHT* light);
//*****************************************************************************
// グローバル変数
//*****************************************************************************
static LIGHT	g_Light[LIGHT_MAX];

static FOG		g_Fog;

static	BOOL	g_FogEnable = false;

static int		colChangeCnt;
static int		sunMoveCnt;

int lightCount = 0;
int canUpdateCount = 0;
bool canUpdate = false;
#define LIGHT_PADDING	(10)

//=============================================================================
// 初期化処理
//=============================================================================
void InitLight(void)
{

	//ライト初期化
	for (int i = 0; i < LIGHT_MAX; i++)
	{
		g_Light[i].Position  = XMFLOAT3( 0.0f, 0.0f, 0.0f );
		g_Light[i].Direction = XMFLOAT3( 0.0f, -1.0f, 0.0f );
		g_Light[i].Diffuse   = XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f );
		g_Light[i].Ambient   = XMFLOAT4( 0.0f, 0.0f, 0.0f, 0.0f );
		g_Light[i].Attenuation = 100.0f;	// 減衰距離
		g_Light[i].Type = LIGHT_TYPE_NONE;	// ライトのタイプ
		g_Light[i].Enable = false;			// ON / OFF
		SetLight(i, &g_Light[i]);
	}

	// 並行光源の設定（世界を照らす光）
	g_Light[0].Position = XMFLOAT3(0.0f, 50.0f, 0.0f);
	g_Light[0].Direction = NorLightDir(&g_Light[0]);		// 光の向き
	g_Light[0].Diffuse   = XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f );	// 光の色
	g_Light[0].Type = LIGHT_TYPE_DIRECTIONAL;					// 並行光源
	g_Light[0].Enable = true;									// このライトをON
	SetLight(0, &g_Light[0]);									// これで設定している

	// ポイントライトの設定
	//g_Light[1].Position = XMFLOAT3(0.0f, 300.0f, 0.0f);
	//g_Light[1].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	//g_Light[1].Type = LIGHT_TYPE_POINT;
	//g_Light[1].degree = DtoR(10.0f);
	//g_Light[1].Enable = true;
	//g_Light[1].Attenuation = 550.0f;
	//SetLight(1, &g_Light[1]);


	// フォグの初期化（霧の効果）
	g_Fog.FogStart = 100.0f;									// 視点からこの距離離れるとフォグがかかり始める
	g_Fog.FogEnd   = 250.0f;									// ここまで離れるとフォグの色で見えなくなる
	g_Fog.FogColor = XMFLOAT4( 0.0f, 0.0f, 0.0f, 1.0f );		// フォグの色
	SetFog(&g_Fog);
	SetFogEnable(g_FogEnable);		// 他の場所もチェックする shadow

	colChangeCnt = 0;
	sunMoveCnt = 0;
}


//=============================================================================
// 更新処理
//=============================================================================
void UpdateLight(void)
{
	canUpdateCount++;
	if (canUpdateCount >= 1080) {
		canUpdate = true;
	}
	if (canUpdate) {
		lightCount++;
		if (lightCount >= LIGHT_PADDING) {
			lightCount = 0;
			g_Light[0].degree += 0.005f;
			g_Light[0].Position = RotZ(&g_Light[0]);
			g_Light[0].Direction = NorLightDir(&g_Light[0]);
			SetLight(0, &g_Light[0]);
		}
	}
	//colChangeCnt++;

	//if (colChangeCnt % 10 == 0)
	//{

	//	if (colChangeCnt % 3600 == 1080)
	//	{
	//		g_Light[0].Diffuse = XMFLOAT4{0.2f, 0.2f, 0.2f, 1.0f};
	//		SetLight(0, &g_Light[0]);

	//		// 点光源の設定
	//		g_Light[1].Position = XMFLOAT3(-200.0f, 0.0f, 0.0f);
	//		g_Light[1].Direction = XMFLOAT3(0.0f, -1.0f, -1.0f);		// 光の向き
	//		g_Light[1].Diffuse = XMFLOAT4(1.0f, 0.8f, 0.6f, 1.0f);	// 光の色
	//		g_Light[1].Type = LIGHT_TYPE_POINT;						// 並行光源
	//		g_Light[1].Enable = true;								// このライトをON
	//		SetLight(1, &g_Light[1]);								// これで設定している
	//	}
	//	if (colChangeCnt % 3600 > 1080)
	//	{
	//		sunMoveCnt++;
	//		g_Light[0].Diffuse.x += 0.01f;	// R
	//		g_Light[0].Diffuse.y += 0.01f;	// G
	//		g_Light[0].Diffuse.z += 0.01f;	// B
	//		SetLight(0, &g_Light[0]);

	//		//g_Light[1].Position.x = cosf(sunMoveCnt * XM_PI / 180 + XM_PI) * 800;
	//		//g_Light[1].Position.y = sinf(sunMoveCnt * XM_PI / 180) * 800;
	//	}
	//}

#ifdef _DEBUG	// デバッグ情報を表示する
	PrintDebugProc("Light Direction X:%f,Y:%f,Z:%f\n",
		g_Light[0].Direction.x, g_Light[0].Direction.y, g_Light[0].Direction.z);
	PrintDebugProc("Light Position X:%f,Y:%f,Z:%f\n",
		g_Light[0].Position.x, g_Light[0].Position.y, g_Light[0].Position.z);
#endif
}


//=============================================================================
// ライトの設定
// Typeによってセットするメンバー変数が変わってくる
//=============================================================================
void SetLightData(int index, LIGHT *light)
{
	SetLight(index, light);
}


LIGHT *GetLightData(int index)
{
	return(&g_Light[index]);
}


//=============================================================================
// フォグの設定
//=============================================================================
void SetFogData(FOG *fog)
{
	SetFog(fog);
}


BOOL	GetFogEnable(void)
{
	return(g_FogEnable);
}


float DtoR(float degree) {
	return (3.14 / 180) * degree;
}


XMFLOAT3 RotZ(struct LIGHT* light) {
	XMFLOAT3 newPos;
	float radian = DtoR(light->degree);
	newPos.x = light->Position.x;
	newPos.y = cosf(radian) * light->Position.y - sinf(radian) * light->Position.z;
	newPos.z = sinf(radian) * light->Position.y + cosf(radian) * light->Position.z;
	return newPos;
}


//　並行ライトの正規化
XMFLOAT3 NorLightDir(struct LIGHT* light) {
	XMFLOAT3 dir;

	dir.x = 0.0f - light->Position.x;
	dir.y = 0.0f - light->Position.y;
	dir.z = 0.0f - light->Position.z;

	float distance = pow(pow(light->Position.x, 2) + pow(light->Position.y, 2) + pow(light->Position.z, 2), 0.5f);

	dir.x /= distance;
	dir.y /= distance;
	dir.z /= distance;
	
	return dir;
}
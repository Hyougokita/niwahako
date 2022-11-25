//=============================================================================
//
// エネミーモデル処理 [cloud.h]
// Author : 
//
//=============================================================================
#pragma once


//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define MAX_CLOUD		(7)					// エネミーの数

#define	CLOUD_SIZE		(5.0f)				// 当たり判定の大きさ


//*****************************************************************************
// 構造体定義
//*****************************************************************************
struct CLOUD
{
	XMFLOAT4X4			mtxWorld;			// ワールドマトリックス
	XMFLOAT3			pos;				// モデルの位置
	XMFLOAT3			rot;				// モデルの向き(回転)
	XMFLOAT3			scl;				// モデルの大きさ(スケール)

	bool				use;
	bool				load;
	DX11_MODEL			model;				// モデル情報
	XMFLOAT4			diffuse[MODEL_MAX_MATERIAL];	// モデルの色

	float				spd;				// 移動スピード
	float				size;				// 当たり判定の大きさ
	int					shadowIdx;			// 影のインデックス番号

	int					count;				// 煙の変化のカウンター
	XMFLOAT4			diff;				// RBGA

	// 線形補間
	float				time;
	int					tblNo;
	int					tblMax;

};

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitCloud(void);
void UninitCloud(void);
void UpdateCloud(void);
void DrawCloud(void);

CLOUD *GetCloud(void);


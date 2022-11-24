//=============================================================================
//
// カメラ処理 [camera.h]
// Author : 
//
//=============================================================================
#pragma once


//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "renderer.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
struct CAMERA
{
	XMFLOAT4X4			mtxView;		// ビューマトリックス
	XMFLOAT4X4			mtxInvView;		// ビューマトリックス
	XMFLOAT4X4			mtxProjection;	// プロジェクションマトリックス
	
	XMFLOAT3			pos;			// カメラの視点(位置)
	XMFLOAT3			at;				// カメラの注視点
	XMFLOAT3			up;				// カメラの上方向ベクトル
	XMFLOAT3			rot;			// カメラの回転
	
	float				len;			// カメラの視点と注視点の距離

	float				time;			// 線形補間用
	int					tblNo;			// 行動データのテーブル番号
	int					tblMax;			// そのテーブルのデータ数

	bool				canMove;		// 動けるかのフラグ

};

struct CAM_AT
{
	XMFLOAT3			pos;			// カメラの注視点(位置)

	float				time;			// 線形補間用
	int					tblNo;			// 行動データのテーブル番号
	int					tblMax;			// そのテーブルのデータ数

	bool				use;			// カメラ注視点の使用フラグ
};


enum {
	TYPE_FULL_SCREEN,
	TYPE_LEFT_HALF_SCREEN,
	TYPE_RIGHT_HALF_SCREEN,
	TYPE_UP_HALF_SCREEN,
	TYPE_DOWN_HALF_SCREEN,
	TYPE_UP_LEFT_SCREEN,
	TYPE_UP_RIGHT_SCREEN,
	TYPE_DOWN_LEFT_SCREEN,
	TYPE_DOWN_RIGHT_SCREEN,
	TYPE_NONE,

};


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
void InitCamera(void);
void UninitCamera(void);
void UpdateCamera(void);
void SetCamera(void);

CAMERA* GetCamera(void);
CAM_AT* GetCamAT(void);

void SetViewPort(int type);
int GetViewPortType(void);

void SetCameraAT(XMFLOAT3 pos);
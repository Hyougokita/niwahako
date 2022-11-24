//=============================================================================
//
// カメラ処理 [camera.cpp]
// Author : 
//
//=============================================================================
#include "main.h"
#include "input.h"
#include "camera.h"
#include "debugproc.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define	POS_X_CAM			(-150.0f)			// カメラの初期位置(X座標)
#define	POS_Y_CAM			(70.0f)			// カメラの初期位置(Y座標)
#define	POS_Z_CAM			(-150.0f)		// カメラの初期位置(Z座標)

//#define	POS_X_CAM		(0.0f)			// カメラの初期位置(X座標)
//#define	POS_Y_CAM		(200.0f)		// カメラの初期位置(Y座標)
//#define	POS_Z_CAM		(-400.0f)		// カメラの初期位置(Z座標)


#define	VIEW_ANGLE		(XMConvertToRadians(45.0f))						// ビュー平面の視野角
#define	VIEW_ASPECT		((float)SCREEN_WIDTH / (float)SCREEN_HEIGHT)	// ビュー平面のアスペクト比	
#define	VIEW_NEAR_Z		(10.0f)											// ビュー平面のNearZ値
#define	VIEW_FAR_Z		(10000.0f)										// ビュー平面のFarZ値

#define	VALUE_MOVE_CAMERA	(2.0f)										// カメラの移動量
#define	VALUE_ROTATE_CAMERA	(XM_PI * 0.01f)								// カメラの回転量

//*****************************************************************************
// グローバル変数
//*****************************************************************************
static CAMERA			g_Camera;		// カメラデータ
static CAM_AT			g_CamAT;		// カメラ注視点データ

static int				g_ViewPortType = TYPE_FULL_SCREEN;

// カメラ用線形補間移動テーブル
static INTERPOLATION_DATA g_MoveTbl0[] = {
	//座標									回転率							拡大率							時間

	// 90°俯瞰　線形補間じゃなくあとのスイッチ文で動かす
	{ XMFLOAT3(-50.0f, 30.0f, -50.0f),	XMFLOAT3(0.0f, 0.0f, 0.0f),	XMFLOAT3(2.0f, 2.0f, 1.0f),	360 },
	{ XMFLOAT3(-200.0f, 30.0f, -200.0f),	XMFLOAT3(0.0f, 0.0f, 0.0f),	XMFLOAT3(2.0f, 2.0f, 1.0f),	200 },
	{ XMFLOAT3(300.0f, 80.0f, 300.0f),	XMFLOAT3(0.0f, 0.0f, 0.0f),	XMFLOAT3(2.0f, 2.0f, 1.0f),	300 },
	
	// 川俯瞰
	{ XMFLOAT3(-150.0f, 50.0f, -250.0f),	XMFLOAT3(0.0f, 0.0f, 0.0f),	XMFLOAT3(0.0f, 0.0f, 1.0f),	360 },
	{ XMFLOAT3(100.0f, 10.0f, 0.0f),	XMFLOAT3(0.0f, 0.0f, 0.0f),	XMFLOAT3(2.0f, 2.0f, 1.0f),	200 },

};

// カメラ注視点用線形補間移動テーブル
static INTERPOLATION_DATA g_MoveTbl1[] = {
	//座標									回転率							拡大率							時間

	{ XMFLOAT3(0.0f, 30.0f, 0.0f),	XMFLOAT3(0.0f, 0.0f, 0.0f),		XMFLOAT3(2.0f, 2.0f, 1.0f),	360 },
	{ XMFLOAT3(0.0f, 30.0f, 0.0f),	XMFLOAT3(0.0f, 0.0f, 0.0f),		XMFLOAT3(2.0f, 2.0f, 1.0f),	200 },
	{ XMFLOAT3(325.0f, 50.0f, 300.0f),	XMFLOAT3(0.0f, 0.0f, 0.0f),		XMFLOAT3(2.0f, 2.0f, 1.0f),	200 },

	// 川俯瞰
	{ XMFLOAT3(0.0f, -20.0f, -100.0f),		XMFLOAT3(0.0f, 0.0f, 0.0f),		XMFLOAT3(0.0f, 0.0f, 1.0f),	360 },
	{ XMFLOAT3(350.0f, -20.0f, 250.0f),	XMFLOAT3(0.0f, 0.0f, 0.0f),		XMFLOAT3(2.0f, 2.0f, 1.0f),	200 },

};

static INTERPOLATION_DATA* g_MoveTblAdr[] =
{
	g_MoveTbl0,
	g_MoveTbl1,

};


//=============================================================================
// 初期化処理
//=============================================================================
void InitCamera(void)
{
	g_Camera.pos = { POS_X_CAM, POS_Y_CAM, POS_Z_CAM };
	g_CamAT.pos = { 0.0f, 50.0f, 0.0f };
	g_Camera.at  = { 0.0f, 0.0f, 0.0f };
	g_Camera.up  = { 0.0f, 1.0f, 0.0f };
	g_Camera.rot = { 0.0f, 0.0f, 0.0f };
	g_CamAT.use = true;

	// 視点と注視点の距離を計算
	float vx, vz;
	vx = g_Camera.pos.x - g_Camera.at.x;
	vz = g_Camera.pos.z - g_Camera.at.z;
	g_Camera.len = sqrtf(vx * vx + vz * vz);

	// 線形補間で動かしてみる
	g_Camera.time = 0.0f;		// 線形補間用のタイマーをクリア
	g_Camera.tblNo = 0;		// 再生するアニメデータの先頭アドレスをセット
	g_Camera.tblMax = sizeof(g_MoveTbl0) / sizeof(INTERPOLATION_DATA);	// 再生するアニメデータのレコード数をセット

	g_CamAT.time = 0.0f;		// 線形補間用のタイマーをクリア
	g_CamAT.tblNo = 1;		// 再生するアニメデータの先頭アドレスをセット
	g_CamAT.tblMax = sizeof(g_MoveTbl1) / sizeof(INTERPOLATION_DATA);	// 再生するアニメデータのレコード数をセット

	// ビューポートタイプの初期化
	g_ViewPortType = TYPE_FULL_SCREEN;
}


//=============================================================================
// カメラの終了処理
//=============================================================================
void UninitCamera(void)
{

}


//=============================================================================
// カメラの更新処理
//=============================================================================
void UpdateCamera(void)
{

#ifdef _DEBUG

	//// 移動処理
	//{	// 線形補間の処理
	//	int nowNo = (int)g_CamAT.time;			// 整数分であるテーブル番号を取り出している
	//	int maxNo = g_CamAT.tblMax;				// 登録テーブル数を数えている
	//	int nextNo = (nowNo + 1) % maxNo;			// 移動先テーブルの番号を求めている
	//	INTERPOLATION_DATA* tbl = g_MoveTblAdr[g_CamAT.tblNo];	// 行動テーブルのアドレスを取得

	//	XMVECTOR nowPos = XMLoadFloat3(&tbl[nowNo].pos);	// XMVECTORへ変換
	//	XMVECTOR nowRot = XMLoadFloat3(&tbl[nowNo].rot);	// XMVECTORへ変換
	//	XMVECTOR nowScl = XMLoadFloat3(&tbl[nowNo].scl);	// XMVECTORへ変換

	//	XMVECTOR Pos = XMLoadFloat3(&tbl[nextNo].pos) - nowPos;	// XYZ移動量を計算している
	//	XMVECTOR Rot = XMLoadFloat3(&tbl[nextNo].rot) - nowRot;	// XYZ回転量を計算している
	//	XMVECTOR Scl = XMLoadFloat3(&tbl[nextNo].scl) - nowScl;	// XYZ拡大率を計算している

	//	float nowTime = g_CamAT.time - nowNo;	// 時間部分である少数を取り出している

	//	Pos *= nowTime;								// 現在の移動量を計算している
	//	Rot *= nowTime;								// 現在の回転量を計算している
	//	Scl *= nowTime;								// 現在の拡大率を計算している

	//	// 計算して求めた移動量を現在の移動テーブルXYZに足している＝表示座標を求めている
	//	XMStoreFloat3(&g_CamAT.pos, nowPos + Pos);

	//	//// 計算して求めた回転量を現在の移動テーブルに足している
	//	//XMStoreFloat3(&g_CamAT.rot, nowRot + Rot);

	//	//// 計算して求めた拡大率を現在の移動テーブルに足している
	//	//XMStoreFloat3(&g_CamAT.scl, nowScl + Scl);
	//	//g_CamAT.w = TEXTURE_WIDTH * g_CamAT.scl.x;
	//	//g_CamAT.h = TEXTURE_HEIGHT * g_CamAT.scl.y;

	//	// frameを使て時間経過処理をする
	//	g_CamAT.time += 1.0f / tbl[nowNo].frame;	// 時間を進めている
	//	if ((int)g_CamAT.time >= maxNo)			// 登録テーブル最後まで移動したか？
	//	{
	//		g_CamAT.time -= maxNo;				// ０番目にリセットしつつも小数部分を引き継いでいる
	//	}
	//}

	//{	// 線形補間の処理
	//	int nowNo = (int)g_Camera.time;			// 整数分であるテーブル番号を取り出している
	//	int maxNo = g_Camera.tblMax;				// 登録テーブル数を数えている
	//	int nextNo = (nowNo + 1) % maxNo;			// 移動先テーブルの番号を求めている
	//	INTERPOLATION_DATA* tbl = g_MoveTblAdr[g_Camera.tblNo];	// 行動テーブルのアドレスを取得

	//	XMVECTOR nowPos = XMLoadFloat3(&tbl[nowNo].pos);	// XMVECTORへ変換
	//	XMVECTOR nowRot = XMLoadFloat3(&tbl[nowNo].rot);	// XMVECTORへ変換
	//	XMVECTOR nowScl = XMLoadFloat3(&tbl[nowNo].scl);	// XMVECTORへ変換

	//	XMVECTOR Pos = XMLoadFloat3(&tbl[nextNo].pos) - nowPos;	// XYZ移動量を計算している
	//	XMVECTOR Rot = XMLoadFloat3(&tbl[nextNo].rot) - nowRot;	// XYZ回転量を計算している
	//	XMVECTOR Scl = XMLoadFloat3(&tbl[nextNo].scl) - nowScl;	// XYZ拡大率を計算している

	//	float nowTime = g_Camera.time - nowNo;	// 時間部分である少数を取り出している

	//	Pos *= nowTime;								// 現在の移動量を計算している
	//	Rot *= nowTime;								// 現在の回転量を計算している
	//	Scl *= nowTime;								// 現在の拡大率を計算している

	//	// 計算して求めた移動量を現在の移動テーブルXYZに足している＝表示座標を求めている
	//	XMStoreFloat3(&g_Camera.pos, nowPos + Pos);

	//	// 計算して求めた回転量を現在の移動テーブルに足している
	//	XMStoreFloat3(&g_Camera.rot, nowRot + Rot);

	//	//// 計算して求めた拡大率を現在の移動テーブルに足している
	//	//XMStoreFloat3(&g_Camera.scl, nowScl + Scl);
	//	//g_Camera.w = TEXTURE_WIDTH * g_Camera.scl.x;
	//	//g_Camera.h = TEXTURE_HEIGHT * g_Camera.scl.y;


	//	// frameを使て時間経過処理をする
	//	g_Camera.time += 1.0f / tbl[nowNo].frame;	// 時間を進めている
	//	if ((int)g_Camera.time >= maxNo)			// 登録テーブル最後まで移動したか？
	//	{
	//		g_Camera.time -= maxNo;				// ０番目にリセットしつつも小数部分を引き継いでいる
	//	}

	//	// 俯瞰回転とか　線形以外用スイッチ文
	//	switch ((int)g_CamAT.time)
	//	{



	//	}
	//}

	if (GetKeyboardPress(DIK_Z))
	{// 視点旋回「左」
		g_Camera.rot.y += VALUE_ROTATE_CAMERA;
		if (g_Camera.rot.y > XM_PI)
		{
			g_Camera.rot.y -= XM_PI * 2.0f;
		}

		g_Camera.pos.x = g_Camera.at.x - sinf(g_Camera.rot.y) * g_Camera.len;
		g_Camera.pos.z = g_Camera.at.z - cosf(g_Camera.rot.y) * g_Camera.len;
	}

	if (GetKeyboardPress(DIK_C))
	{// 視点旋回「右」
		g_Camera.rot.y -= VALUE_ROTATE_CAMERA;
		if (g_Camera.rot.y < -XM_PI)
		{
			g_Camera.rot.y += XM_PI * 2.0f;
		}

		g_Camera.pos.x = g_Camera.at.x - sinf(g_Camera.rot.y) * g_Camera.len;
		g_Camera.pos.z = g_Camera.at.z - cosf(g_Camera.rot.y) * g_Camera.len;
	}

	if (GetKeyboardPress(DIK_Y))
	{// 視点移動「上」
		g_Camera.pos.y += VALUE_MOVE_CAMERA;
	}

	if (GetKeyboardPress(DIK_N))
	{// 視点移動「下」
		g_Camera.pos.y -= VALUE_MOVE_CAMERA;
	}

	if (GetKeyboardPress(DIK_Q))
	{// 注視点旋回「左」
		g_Camera.rot.y -= VALUE_ROTATE_CAMERA;
		if (g_Camera.rot.y < -XM_PI)
		{
			g_Camera.rot.y += XM_PI * 2.0f;
		}

		g_Camera.at.x = g_Camera.pos.x + sinf(g_Camera.rot.y) * g_Camera.len;
		g_Camera.at.z = g_Camera.pos.z + cosf(g_Camera.rot.y) * g_Camera.len;
	}

	if (GetKeyboardPress(DIK_E))
	{// 注視点旋回「右」
		g_Camera.rot.y += VALUE_ROTATE_CAMERA;
		if (g_Camera.rot.y > XM_PI)
		{
			g_Camera.rot.y -= XM_PI * 2.0f;
		}

		g_Camera.at.x = g_Camera.pos.x + sinf(g_Camera.rot.y) * g_Camera.len;
		g_Camera.at.z = g_Camera.pos.z + cosf(g_Camera.rot.y) * g_Camera.len;
	}

	if (GetKeyboardPress(DIK_T))
	{// 注視点移動「上」
		g_Camera.at.y += VALUE_MOVE_CAMERA;
	}

	if (GetKeyboardPress(DIK_B))
	{// 注視点移動「下」
		g_Camera.at.y -= VALUE_MOVE_CAMERA;
	}

	if (GetKeyboardPress(DIK_U))
	{// 近づく
		g_Camera.len -= VALUE_MOVE_CAMERA;
		g_Camera.pos.x = g_Camera.at.x - sinf(g_Camera.rot.y) * g_Camera.len;
		g_Camera.pos.z = g_Camera.at.z - cosf(g_Camera.rot.y) * g_Camera.len;
	}

	if (GetKeyboardPress(DIK_M))
	{// 離れる
		g_Camera.len += VALUE_MOVE_CAMERA;
		g_Camera.pos.x = g_Camera.at.x - sinf(g_Camera.rot.y) * g_Camera.len;
		g_Camera.pos.z = g_Camera.at.z - cosf(g_Camera.rot.y) * g_Camera.len;
	}

	// カメラを初期に戻す
	if (GetKeyboardPress(DIK_R))
	{
		UninitCamera();
		InitCamera();
	}

#endif



#ifdef _DEBUG	// デバッグ情報を表示する
	PrintDebugProc("Camera:ZC QE TB YN UM R\n");

	if (GetKeyboardPress(DIK_LEFT))
	{	// 左へ移動
		g_Camera.pos.x -= 2.0f;
	}
	if (GetKeyboardPress(DIK_RIGHT))
	{	// 左へ移動
		g_Camera.pos.x += 2.0f;
	}
	if (GetKeyboardPress(DIK_UP))
	{	// 左へ移動
		g_Camera.pos.z += 2.0f;
	}
	if (GetKeyboardPress(DIK_DOWN))
	{	// 左へ移動
		g_Camera.pos.z -= 2.0f;
	}

	if (GetKeyboardPress(DIK_A))
	{	// 左へ移動
		g_Camera.at.x -= 2.0f;
	}
	if (GetKeyboardPress(DIK_D))
	{	// 左へ移動
		g_Camera.at.x += 2.0f;
	}
	if (GetKeyboardPress(DIK_W))
	{	// 左へ移動
		g_Camera.at.z += 2.0f;
	}
	if (GetKeyboardPress(DIK_S))
	{	// 左へ移動
		g_Camera.at.z -= 2.0f;
	}


#endif
}


//=============================================================================
// カメラの更新
//=============================================================================
void SetCamera(void) 
{
	// ビューマトリックス設定
	XMMATRIX mtxView;
	mtxView = XMMatrixLookAtLH(XMLoadFloat3(&g_Camera.pos), XMLoadFloat3(&g_Camera.at), XMLoadFloat3(&g_Camera.up));
	SetViewMatrix(&mtxView);
	XMStoreFloat4x4(&g_Camera.mtxView, mtxView);

	XMMATRIX mtxInvView;
	mtxInvView = XMMatrixInverse(nullptr, mtxView);
	XMStoreFloat4x4(&g_Camera.mtxInvView, mtxInvView);


	// プロジェクションマトリックス設定
	XMMATRIX mtxProjection;
	mtxProjection = XMMatrixPerspectiveFovLH(VIEW_ANGLE, VIEW_ASPECT, VIEW_NEAR_Z, VIEW_FAR_Z);

	SetProjectionMatrix(&mtxProjection);
	XMStoreFloat4x4(&g_Camera.mtxProjection, mtxProjection);

	SetShaderCamera(g_Camera.pos);
}


//=============================================================================
// カメラの取得
//=============================================================================
CAMERA *GetCamera(void) 
{
	return &g_Camera;
}

//=============================================================================
// カメラ注視点の取得
//=============================================================================
CAM_AT *GetCamAT(void) 
{
	return &g_CamAT;
}

//=============================================================================
// ビューポートの設定
//=============================================================================
void SetViewPort(int type)
{
	ID3D11DeviceContext *g_ImmediateContext = GetDeviceContext();
	D3D11_VIEWPORT vp;

	g_ViewPortType = type;

	// ビューポート設定
	switch (g_ViewPortType)
	{
	case TYPE_FULL_SCREEN:
		vp.Width = (FLOAT)SCREEN_WIDTH;
		vp.Height = (FLOAT)SCREEN_HEIGHT;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		break;

	case TYPE_LEFT_HALF_SCREEN:
		vp.Width = (FLOAT)SCREEN_WIDTH / 2;
		vp.Height = (FLOAT)SCREEN_HEIGHT;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		break;

	case TYPE_RIGHT_HALF_SCREEN:
		vp.Width = (FLOAT)SCREEN_WIDTH / 2;
		vp.Height = (FLOAT)SCREEN_HEIGHT;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = (FLOAT)SCREEN_WIDTH / 2;
		vp.TopLeftY = 0;
		break;

	case TYPE_UP_HALF_SCREEN:
		vp.Width = (FLOAT)SCREEN_WIDTH;
		vp.Height = (FLOAT)SCREEN_HEIGHT / 2;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		break;

	case TYPE_DOWN_HALF_SCREEN:
		vp.Width = (FLOAT)SCREEN_WIDTH;
		vp.Height = (FLOAT)SCREEN_HEIGHT / 2;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = (FLOAT)SCREEN_HEIGHT / 2;
		break;

	case TYPE_UP_LEFT_SCREEN:
		vp.Width = (FLOAT)SCREEN_WIDTH / 2;
		vp.Height = (FLOAT)SCREEN_HEIGHT / 2;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		break;

	case TYPE_UP_RIGHT_SCREEN:
		vp.Width = (FLOAT)SCREEN_WIDTH / 2;
		vp.Height = (FLOAT)SCREEN_HEIGHT / 2;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = (FLOAT)SCREEN_WIDTH / 2;
		vp.TopLeftY = 0;
		break;

	case TYPE_DOWN_LEFT_SCREEN:
		vp.Width = (FLOAT)SCREEN_WIDTH / 2;
		vp.Height = (FLOAT)SCREEN_HEIGHT / 2;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = (FLOAT)SCREEN_HEIGHT / 2;
		break;

	case TYPE_DOWN_RIGHT_SCREEN:
		vp.Width = (FLOAT)SCREEN_WIDTH / 2;
		vp.Height = (FLOAT)SCREEN_HEIGHT / 2;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = (FLOAT)SCREEN_WIDTH / 2;
		vp.TopLeftY = (FLOAT)SCREEN_HEIGHT / 2;
		break;
	}
	g_ImmediateContext->RSSetViewports(1, &vp);

}


int GetViewPortType(void)
{
	return g_ViewPortType;
}



// カメラの視点と注視点をセット
void SetCameraAT(XMFLOAT3 pos)
{
	// カメラの注視点を設定
	g_Camera.at = pos;

	// カメラの視点をカメラのY軸回転に対応させている
	g_Camera.pos.x = g_Camera.at.x - sinf(g_Camera.rot.y) * g_Camera.len;
	g_Camera.pos.z = g_Camera.at.z - cosf(g_Camera.rot.y) * g_Camera.len;

}

//=============================================================================
//
// エネミーモデル処理 [cloud.cpp]
// Author : 
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "model.h"
#include "input.h"
#include "debugproc.h"
#include "cloud.h"
#include "shadow.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define	MODEL_CLOUD			"data/MODEL/cloud.obj"		// 読み込むモデル名
#define	MODEL_HEAD			"data/MODEL/bear_head.obj"
#define	MODEL_BODY			"data/MODEL/bear_body.obj"
#define	MODEL_LEFTHAND		"data/MODEL/bear_lefthand.obj"
#define	MODEL_RIGHTHAND		"data/MODEL/bear_righthand.obj"
#define	MODEL_LEFTLEG		"data/MODEL/bear_leftleg.obj"
#define	MODEL_RIGHTLEG		"data/MODEL/bear_rightleg.obj"
#define	MODEL_HANDGUN		"data/MODEL/handgun.obj"
#define MODEL_GRASS			"data/MODEL/garss.obj"
#define MODE_HOUSE			"data/MODEL/newhome.obj"
#define MODE_CLOUD			"data/MODEL/cloud.obj"
#define MODE_DOOR			"data/MODEL/door.obj"

#define	VALUE_MOVE			(5.0f)						// 移動量
#define	VALUE_ROTATE		(XM_PI * 0.02f)				// 回転量

#define CLOUD_SHADOW_SIZE	(0.4f)						// 影の大きさ
#define CLOUD_OFFSET_Y		(7.0f)						// エネミーの足元をあわせる

#define CLOUD_SCALE		(1.0f)

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
void SetCloud();

//*****************************************************************************
// グローバル変数
//*****************************************************************************
static CLOUD			g_Cloud[MAX_CLOUD];				// エネミー
static CLOUD			g_Door;

int g_Cloud_load = 0;
int g_Door_load = 0;

bool canOpenDoor = false;
int canOpenDoorCount = 0;

int makeCloudCount = 0;
#define MAKE_CLOUD_PADDING	(100)

int makeDoorCount = 0;

//	煙突の位置
XMFLOAT3 chimneyPos = XMFLOAT3(0.0f,0.0f,0.0f);

//	目的の座標
XMFLOAT3 finalCloudPos = XMFLOAT3(chimneyPos.x + 100.0f, chimneyPos.y + 300.0f, chimneyPos.z);


//	ドアの位置
XMFLOAT3 doorPos = XMFLOAT3(50.0f, 0.0f, 0.0f);

//


//　線形補間(曇)
static INTERPOLATION_DATA_CLOUD g_MoveTbl0[] = {
	//座標									回転率							拡大率						RGBA						時間
	{ chimneyPos,						XMFLOAT3(0.0f, 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f),		XMFLOAT4(1.0f,1.0f,1.0f,1.0f),				240 },
	{ finalCloudPos,					XMFLOAT3(0.0f, 0.0f, 0.0f),		XMFLOAT3(10.0f, 10.0f, 10.0f),	XMFLOAT4(1.0f,1.0f,1.0f,0.0f),				60 },
};


static INTERPOLATION_DATA_CLOUD* g_MoveTblAdr[] =
{
	g_MoveTbl0,
};


static INTERPOLATION_DATA g_MoveTbl1[] = {
	//座標									回転率							拡大率							時間
	{doorPos,	XMFLOAT3(0.0f, 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f),	60 },
	{doorPos,	XMFLOAT3(0.0f, -1.57f, 0.0f),	XMFLOAT3(1.0f, 1.0f, 1.0f),	60 },

};

static INTERPOLATION_DATA* g_MoveTblAdr1[] =
{
	g_MoveTbl1,
};


//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitCloud(void)
{
	for (int i = 0; i < MAX_CLOUD; i++)
	{
		LoadModel(MODEL_CLOUD, &g_Cloud[i].model);
		g_Cloud[i].load = true;

		g_Cloud[i].pos = XMFLOAT3(-50.0f , CLOUD_OFFSET_Y, 20.0f);
		g_Cloud[i].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Cloud[i].scl = XMFLOAT3(CLOUD_SCALE, CLOUD_SCALE, CLOUD_SCALE);

		g_Cloud[i].spd  = 0.0f;			// 移動スピードクリア
		g_Cloud[i].size = CLOUD_SIZE;	// 当たり判定の大きさ
		g_Cloud[i].count = 0;
		g_Cloud[i].diff = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

		// モデルのディフューズを保存しておく。色変え対応の為。
		GetModelDiffuse(&g_Cloud[i].model, &g_Cloud[i].diffuse[i]);

		XMFLOAT3 pos = g_Cloud[i].pos;
		pos.y -= (CLOUD_OFFSET_Y - 0.1f);
		g_Cloud[i].shadowIdx = CreateShadow(pos, CLOUD_SHADOW_SIZE, CLOUD_SHADOW_SIZE);
		
		g_Cloud[i].use = false;		// true:生きてる

	}

	//	ドア
	LoadModel(MODE_DOOR, &g_Door.model);
	g_Door.load = true;
	g_Door.pos = doorPos;
	g_Door.rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_Door.scl = XMFLOAT3(1.0f, 1.0f, 1.0f);

	g_Door.time = 0.0f;			// 線形補間用のタイマーをクリア
	g_Door.tblNo = 0;			// 再生するアニメデータの先頭アドレスをセット
	g_Door.tblMax = sizeof(g_MoveTbl1) / sizeof(INTERPOLATION_DATA);	// 再生するアニメデータのレコード数をセット
	
	//g_Cloud[0].time = 0.0f;			// 線形補間用のタイマーをクリア
	//g_Cloud[0].tblNo = 0;			// 再生するアニメデータの先頭アドレスをセット
	//g_Cloud[0].tblMax = sizeof(g_MoveTbl0) / sizeof(INTERPOLATION_DATA_CLOUD);	// 再生するアニメデータのレコード数をセット

	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitCloud(void)
{

	for (int i = 0; i < MAX_CLOUD; i++)
	{
		if (g_Cloud[i].load)
		{
			UnloadModel(&g_Cloud[i].model);
			g_Cloud[i].load = false;
		}
	}

}

//=============================================================================
// 更新処理
//=============================================================================
void UpdateCloud(void)
{

	makeCloudCount++;
	if (makeCloudCount > MAKE_CLOUD_PADDING) {
		SetCloud();
		makeCloudCount = 0;
	}

	canOpenDoorCount++;
	if (canOpenDoorCount >= (35 * 60)) {
		canOpenDoor = true;
	}

		//ドアの線形補間
	if (canOpenDoor) {
		if (g_Door.tblMax > 0)	// 線形補間を実行する？
		{	// 線形補間の処理
			int nowNo = (int)g_Door.time;			// 整数分であるテーブル番号を取り出している
			int maxNo = g_Door.tblMax;				// 登録テーブル数を数えている
			int nextNo = (nowNo + 1) % maxNo;			// 移動先テーブルの番号を求めている
			INTERPOLATION_DATA* tbl = g_MoveTblAdr1[g_Door.tblNo];	// 行動テーブルのアドレスを取得

			XMVECTOR nowPos = XMLoadFloat3(&tbl[nowNo].pos);	// XMVECTORへ変換
			XMVECTOR nowRot = XMLoadFloat3(&tbl[nowNo].rot);	// XMVECTORへ変換
			XMVECTOR nowScl = XMLoadFloat3(&tbl[nowNo].scl);	// XMVECTORへ変換


			XMVECTOR Pos = XMLoadFloat3(&tbl[nextNo].pos) - nowPos;	// XYZ移動量を計算している
			XMVECTOR Rot = XMLoadFloat3(&tbl[nextNo].rot) - nowRot;	// XYZ回転量を計算している
			XMVECTOR Scl = XMLoadFloat3(&tbl[nextNo].scl) - nowScl;	// XYZ拡大率を計算している


			float nowTime = g_Door.time - nowNo;	// 時間部分である少数を取り出している

			Pos *= nowTime;								// 現在の移動量を計算している
			Rot *= nowTime;								// 現在の回転量を計算している
			Scl *= nowTime;								// 現在の拡大率を計算している


			// 計算して求めた移動量を現在の移動テーブルXYZに足している＝表示座標を求めている
			XMStoreFloat3(&g_Door.pos, nowPos + Pos);

			// 計算して求めた回転量を現在の移動テーブルに足している
			XMStoreFloat3(&g_Door.rot, nowRot + Rot);

			// 計算して求めた拡大率を現在の移動テーブルに足している
			XMStoreFloat3(&g_Door.scl, nowScl + Scl);
			//g_Door.w = TEXTURE_WIDTH * g_Door.scl.x;
			//g_Door.h = TEXTURE_HEIGHT * g_Door.scl.y;



			// frameを使て時間経過処理をする
			g_Door.time += 1.0f / tbl[nowNo].frame;	// 時間を進めている
			if ((int)g_Door.time >= maxNo - 1)			// 登録テーブル最後まで移動したか？
			{
				//g_Door.time -= maxNo;				// ０番目にリセットしつつも小数部分を引き継いでいる
				g_Door.tblMax = 0;
				g_Door.use = false;
			}

		}
	}

	// エネミーを動かす場合は、影も合わせて動かす事を忘れないようにね！
	for (int i = 0; i < MAX_CLOUD; i++)
	{
		if (g_Cloud[i].use == true)		// このエネミーが使われている？
		{								// Yes
			SetModelDiffuse(&g_Cloud[i].model, 0, g_Cloud[i].diff);
			if (g_Cloud[i].tblMax > 0)	// 線形補間を実行する？
			{	// 線形補間の処理
				int nowNo = (int)g_Cloud[i].time;			// 整数分であるテーブル番号を取り出している
				int maxNo = g_Cloud[i].tblMax;				// 登録テーブル数を数えている
				int nextNo = (nowNo + 1) % maxNo;			// 移動先テーブルの番号を求めている
				INTERPOLATION_DATA_CLOUD* tbl = g_MoveTblAdr[g_Cloud[i].tblNo];	// 行動テーブルのアドレスを取得

				XMVECTOR nowPos = XMLoadFloat3(&tbl[nowNo].pos);	// XMVECTORへ変換
				XMVECTOR nowRot = XMLoadFloat3(&tbl[nowNo].rot);	// XMVECTORへ変換
				XMVECTOR nowScl = XMLoadFloat3(&tbl[nowNo].scl);	// XMVECTORへ変換
				XMVECTOR nowDiff = XMLoadFloat4(&tbl[nowNo].diff);

				XMVECTOR Pos = XMLoadFloat3(&tbl[nextNo].pos) - nowPos;	// XYZ移動量を計算している
				XMVECTOR Rot = XMLoadFloat3(&tbl[nextNo].rot) - nowRot;	// XYZ回転量を計算している
				XMVECTOR Scl = XMLoadFloat3(&tbl[nextNo].scl) - nowScl;	// XYZ拡大率を計算している
				XMVECTOR Diff = XMLoadFloat4(&tbl[nextNo].diff) - nowDiff; // RGBA

				float nowTime = g_Cloud[i].time - nowNo;	// 時間部分である少数を取り出している

				Pos *= nowTime;								// 現在の移動量を計算している
				Rot *= nowTime;								// 現在の回転量を計算している
				Scl *= nowTime;								// 現在の拡大率を計算している
				Diff *= nowTime;

				// 計算して求めた移動量を現在の移動テーブルXYZに足している＝表示座標を求めている
				XMStoreFloat3(&g_Cloud[i].pos, nowPos + Pos);

				// 計算して求めた回転量を現在の移動テーブルに足している
				XMStoreFloat3(&g_Cloud[i].rot, nowRot + Rot);

				// 計算して求めた拡大率を現在の移動テーブルに足している
				XMStoreFloat3(&g_Cloud[i].scl, nowScl + Scl);
				//g_Cloud[i].w = TEXTURE_WIDTH * g_Cloud[i].scl.x;
				//g_Cloud[i].h = TEXTURE_HEIGHT * g_Cloud[i].scl.y;

				// RGBA
				XMStoreFloat4(&g_Cloud[i].diff, nowDiff + Diff);

				// frameを使て時間経過処理をする
				g_Cloud[i].time += 1.0f / tbl[nowNo].frame;	// 時間を進めている
				if ((int)g_Cloud[i].time >= maxNo - 1)			// 登録テーブル最後まで移動したか？
				{
					//g_Cloud[i].time -= maxNo;				// ０番目にリセットしつつも小数部分を引き継いでいる
					g_Cloud[i].tblMax = 0;
					g_Cloud[i].use = false;
				}

			}
		}
	}




#ifdef _DEBUG

	if (GetKeyboardTrigger(DIK_P))
	{
		// モデルの色を変更できるよ！半透明にもできるよ。
		for (int j = 0; j < g_Cloud[0].model.SubsetNum; j++)
		{
			SetModelDiffuse(&g_Cloud[0].model, j, XMFLOAT4(1.0f, 0.0f, 0.0f, 0.5f));
		}
	}

	if (GetKeyboardTrigger(DIK_L))
	{
		// モデルの色を元に戻している
		for (int j = 0; j < g_Cloud[0].model.SubsetNum; j++)
		{
			SetModelDiffuse(&g_Cloud[0].model, j, g_Cloud[0].diffuse[j]);
		}
	}
#endif


}

//=============================================================================
// 描画処理
//=============================================================================
void DrawCloud(void)
{
	XMMATRIX mtxScl, mtxRot, mtxTranslate, mtxWorld;

	// カリング無効
	SetCullingMode(CULL_MODE_NONE);

	for (int i = 0; i < MAX_CLOUD; i++)
	{
		if (g_Cloud[i].use == false) continue;

		// ワールドマトリックスの初期化
		mtxWorld = XMMatrixIdentity();

		// スケールを反映
		mtxScl = XMMatrixScaling(g_Cloud[i].scl.x, g_Cloud[i].scl.y, g_Cloud[i].scl.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

		// 回転を反映
		mtxRot = XMMatrixRotationRollPitchYaw(g_Cloud[i].rot.x, g_Cloud[i].rot.y + XM_PI, g_Cloud[i].rot.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

		// 移動を反映
		mtxTranslate = XMMatrixTranslation(g_Cloud[i].pos.x, g_Cloud[i].pos.y, g_Cloud[i].pos.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

		// ワールドマトリックスの設定
		SetWorldMatrix(&mtxWorld);

		XMStoreFloat4x4(&g_Cloud[i].mtxWorld, mtxWorld);


		// モデル描画
		DrawModel(&g_Cloud[i].model);
	}

	{

		// ワールドマトリックスの初期化
		mtxWorld = XMMatrixIdentity();

		// スケールを反映
		mtxScl = XMMatrixScaling(g_Door.scl.x, g_Door.scl.y, g_Door.scl.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

		// 回転を反映
		mtxRot = XMMatrixRotationRollPitchYaw(g_Door.rot.x, g_Door.rot.y + XM_PI, g_Door.rot.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

		// 移動を反映
		mtxTranslate = XMMatrixTranslation(g_Door.pos.x, g_Door.pos.y, g_Door.pos.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

		// ワールドマトリックスの設定
		SetWorldMatrix(&mtxWorld);

		XMStoreFloat4x4(&g_Door.mtxWorld, mtxWorld);


		// モデル描画
		DrawModel(&g_Door.model);
	}

	// カリング設定を戻す
	SetCullingMode(CULL_MODE_BACK);
}

//=============================================================================
// エネミーの取得
//=============================================================================
CLOUD *GetCloud()
{
	return &g_Cloud[0];
}

void SetCloud() {
	for (int i = 0; i < MAX_CLOUD; i++) {
		if (g_Cloud[i].use == false) {
			g_Cloud[i].time = 0.0f;			// 線形補間用のタイマーをクリア
			g_Cloud[i].tblNo = 0;			// 再生するアニメデータの先頭アドレスをセット
			g_Cloud[i].tblMax = sizeof(g_MoveTbl0) / sizeof(INTERPOLATION_DATA_CLOUD);	// 再生するアニメデータのレコード数をセット
			g_Cloud[i].use = true;
			return;
		}
	}
}
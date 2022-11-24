//=============================================================================
//
// ���C�g���� [light.cpp]
// Author : 
//
//=============================================================================
#include "main.h"
#include "renderer.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************



//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************


//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static LIGHT	g_Light[LIGHT_MAX];

static FOG		g_Fog;

static	BOOL	g_FogEnable = false;

static int		colChangeCnt;
static int		sunMoveCnt;


//=============================================================================
// ����������
//=============================================================================
void InitLight(void)
{

	//���C�g������
	for (int i = 0; i < LIGHT_MAX; i++)
	{
		g_Light[i].Position  = XMFLOAT3( 0.0f, 0.0f, 0.0f );
		g_Light[i].Direction = XMFLOAT3( 0.0f, -1.0f, 0.0f );
		g_Light[i].Diffuse   = XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f );
		g_Light[i].Ambient   = XMFLOAT4( 0.0f, 0.0f, 0.0f, 0.0f );
		g_Light[i].Attenuation = 100.0f;	// ��������
		g_Light[i].Type = LIGHT_TYPE_NONE;	// ���C�g�̃^�C�v
		g_Light[i].Enable = false;			// ON / OFF
		SetLight(i, &g_Light[i]);
	}

	// ���s�����̐ݒ�i���E���Ƃ炷���j
	g_Light[0].Position = XMFLOAT3(0.0f, 100.0f, 0.0f);
	g_Light[0].Direction = XMFLOAT3( 0.0f, -1.0f, 0.0f );		// ���̌���
	g_Light[0].Diffuse   = XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f );	// ���̐F
	g_Light[0].Type = LIGHT_TYPE_DIRECTIONAL;					// ���s����
	g_Light[0].Enable = true;									// ���̃��C�g��ON
	SetLight(0, &g_Light[0]);									// ����Őݒ肵�Ă���



	// �t�H�O�̏������i���̌��ʁj
	g_Fog.FogStart = 100.0f;									// ���_���炱�̋��������ƃt�H�O��������n�߂�
	g_Fog.FogEnd   = 250.0f;									// �����܂ŗ����ƃt�H�O�̐F�Ō����Ȃ��Ȃ�
	g_Fog.FogColor = XMFLOAT4( 0.0f, 0.0f, 0.0f, 1.0f );		// �t�H�O�̐F
	SetFog(&g_Fog);
	SetFogEnable(g_FogEnable);		// ���̏ꏊ���`�F�b�N���� shadow

	colChangeCnt = 0;
	sunMoveCnt = 0;
}


//=============================================================================
// �X�V����
//=============================================================================
void UpdateLight(void)
{
	//colChangeCnt++;

	//if (colChangeCnt % 10 == 0)
	//{

	//	if (colChangeCnt % 3600 == 1080)
	//	{
	//		g_Light[0].Diffuse = XMFLOAT4{0.2f, 0.2f, 0.2f, 1.0f};
	//		SetLight(0, &g_Light[0]);

	//		// �_�����̐ݒ�
	//		g_Light[1].Position = XMFLOAT3(-200.0f, 0.0f, 0.0f);
	//		g_Light[1].Direction = XMFLOAT3(0.0f, -1.0f, -1.0f);		// ���̌���
	//		g_Light[1].Diffuse = XMFLOAT4(1.0f, 0.8f, 0.6f, 1.0f);	// ���̐F
	//		g_Light[1].Type = LIGHT_TYPE_POINT;						// ���s����
	//		g_Light[1].Enable = true;								// ���̃��C�g��ON
	//		SetLight(1, &g_Light[1]);								// ����Őݒ肵�Ă���
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
}


//=============================================================================
// ���C�g�̐ݒ�
// Type�ɂ���ăZ�b�g���郁���o�[�ϐ����ς���Ă���
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
// �t�H�O�̐ݒ�
//=============================================================================
void SetFogData(FOG *fog)
{
	SetFog(fog);
}


BOOL	GetFogEnable(void)
{
	return(g_FogEnable);
}




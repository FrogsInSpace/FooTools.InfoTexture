#include "InfoTexture.h"
#include "resource.h"
#include "AboutRollup.h"

extern HINSTANCE hInstance;
extern TCHAR* GetString(int id);

enum
{
	kRef_pblock_params,
	kRef_count,
};

#define CLAMP(val, a, b)	( (val) = ((val) < (a)) ? (a) : ((val) >= (b)) ? (b) : (val) )
#define REPEAT(val)			( (val) = ((val) < 0.0f) ? (fmodf(1.0f - (fabsf((val))), 1.0f) ) : (fmodf((val), 1.0f)) )

//=================================================================================================

class InfoTextureClassDesc : public ClassDesc2
{
public:
	int 			IsPublic()					{ return 1; }
	void*			Create(BOOL loading=FALSE);
	const TCHAR*	ClassName()					{ return GetString(IDS_CLASS_NAME); }

#if MAX_VERSION_MAJOR >= 24
	const TCHAR* NonLocalizedClassName() { return ClassName(); }
#endif

	SClass_ID		SuperClassID()				{ return TEXMAP_CLASS_ID; }
	Class_ID		ClassID()					{ return kInfoTexture_ClassID; }
	const TCHAR* 	Category()					{ return TEXMAP_CAT_3D; }
	const TCHAR*	InternalName()				{ return _T("InfoTexture"); }
	HINSTANCE		HInstance()					{ return hInstance; }
};

static InfoTextureClassDesc infoTextureClassDesc;
ClassDesc2* GetInfoTextureClassDesc() { return &infoTextureClassDesc; }

//=================================================================================================

class InfoTextureAccessor : public PBAccessor
{
public:
	void Get(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t, Interval& valid)
	{
		Limit(v, owner, id, tabIndex, t);
	}

	void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)
	{
		Limit(v, owner, id, tabIndex, t);
	}

private:
	void Limit(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)
	{
		switch (id)
		{
			case kIt_info_type: CLAMP(v.i, 0, kInfoTypeCount-1); break;
			case kIt_wrap_mode: CLAMP(v.i, 0, kWrapModeCount-1); break;
			case kIt_coord_sys: CLAMP(v.i, 0, kCoordSysCount-1); break;
			case kIt_uvw_channel: CLAMP(v.i, 0, MAX_MESHMAPS-1); break;
			case kIt_data_channel:CLAMP(v.i, 1, MAX_VERTDATA); break;
		}
	}
};

static InfoTextureAccessor infoTextureAccessor;

//=================================================================================================

class InfoTextureDlgProc : public ParamMap2UserDlgProc
{
#if (MAX_RELEASE >= 9000)
	INT_PTR DlgProc(TimeValue t, IParamMap2* map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
#else
	BOOL DlgProc(TimeValue t, IParamMap2* map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
#endif
	{
		IParamBlock2* pblock = map->GetParamBlock();
		if (!pblock) return FALSE;

		switch (msg)
		{
			case WM_PAINT:
			{
				BOOL enable;
				InfoType infoType = static_cast<InfoType>(pblock->GetInt(kIt_info_type, t));
				CoordSys coordSys = static_cast<CoordSys>(pblock->GetInt(kIt_coord_sys, t));

				// Wrap Type enable
				enable =	infoType == kInfoUvwCoords ||
							infoType == kInfoVertexData;
				map->Enable(kIt_wrap_mode, enable);
				EnableWindow(GetDlgItem(hWnd, IDC_WRAP_MODE_LABEL), enable);

				// CoordSys Type enable
				enable =	infoType == kInfoNormal			||
							infoType == kInfoReflectVector	||
							infoType == kInfoRefractVector;
				map->Enable(kIt_coord_sys, enable);
				EnableWindow(GetDlgItem(hWnd, IDC_COORD_SYS_LABEL), enable);

				// Use Full Camera Normal Range enable
				enable = infoType == kInfoNormal && coordSys == kCoordSysCamera;
				map->Enable(kIt_use_full_camera_normal_range, enable);

				// UVW Channel enable
				enable = infoType == kInfoUvwCoords;
				map->Enable(kIt_uvw_channel, enable);
				EnableWindow(GetDlgItem(hWnd, IDC_UVW_CHANNEL_LABEL), enable);

				// Data Channel enable
				enable = infoType == kInfoVertexData;
				map->Enable(kIt_data_channel, enable);
				EnableWindow(GetDlgItem(hWnd, IDC_DATA_CHANNEL_LABEL), enable);

				return FALSE;
			}
		}

		return FALSE;
	}

	void DeleteThis()
	{
	}
};

static InfoTextureDlgProc infoTextureDlgProc;

//=================================================================================================


#if MAX_VERSION_MAJOR >= 15
#define end p_end
#endif

static ParamBlockDesc2 infoTexturePBlockDesc
(
	kIt_pblock_params, _T("parameters"),  0, &infoTextureClassDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, kRef_pblock_params,
	IDD_INFOTEXTURE, IDS_INFOTEXTURE, 0, 0, &infoTextureDlgProc,

	kIt_info_type, _T(""), TYPE_INT, 0, 0,
		p_accessor, &infoTextureAccessor,
		p_range, 0, (kInfoTypeCount-1),
		p_default, kInfoFaceIndex,
		p_ui, TYPE_INTLISTBOX, IDC_INFO_TYPE, kInfoTypeCount,	IDS_INFO_FACE_INDEX,
																IDS_INFO_BARY_COORDS,
																IDS_INFO_UVW_COORDS,
																IDS_INFO_NORMAL,
																IDS_INFO_REFLECT_VECTOR,
																IDS_INFO_REFRACT_VECTOR,
																IDS_INFO_OBJECT_BOUNDING_BOX,
																IDS_INFO_VERTEX_DATA,
		end,

	kIt_wrap_mode, _T(""), TYPE_INT, 0, 0,
		p_accessor, &infoTextureAccessor,
		p_range, 0, (kWrapModeCount-1),
		p_default, kWrapRepeat,
		p_ui, TYPE_INTLISTBOX, IDC_WRAP_MODE, kWrapModeCount,	IDS_WRAP_NONE,
																IDS_WRAP_CLAMP,
																IDS_WRAP_REPEAT,
		end,

	kIt_coord_sys, _T(""), TYPE_INT, 0, 0,
		p_accessor, &infoTextureAccessor,
		p_range, 0, (kCoordSysCount-1),
		p_default, kCoordSysCamera,
		p_ui, TYPE_INTLISTBOX, IDC_COORD_SYS, kCoordSysCount,	IDS_COORD_SYS_CAMERA,
																IDS_COORD_SYS_WORLD,
																IDS_COORD_SYS_OBJECT,
		end,

	kIt_uvw_channel, _T("uvwChannel"), TYPE_INT, 0, IDS_UVW_CHANNEL,
		p_accessor, &infoTextureAccessor,
		p_range, 0, (MAX_MESHMAPS-1),
		p_default, 1,
		p_ui, TYPE_SPINNER, EDITTYPE_POS_INT, IDC_UVW_CHANNEL_EDIT, IDC_UVW_CHANNEL_SPIN, 1.0f,
		end,

	kIt_use_full_camera_normal_range, _T("useFullCameraNormalRange"), TYPE_BOOL, 0, IDS_USE_FULL_CAMERA_NORMAL_RANGE,
		p_default, FALSE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_USE_FULL_CAMERA_NORMAL_RANGE,
		end,

	kIt_data_channel, _T("dataChannel"), TYPE_INT, 0, IDS_DATA_CHANNEL,
		p_accessor, &infoTextureAccessor,
		p_range, 1, MAX_VERTDATA,
		p_default, 1,
		p_ui, TYPE_SPINNER, EDITTYPE_POS_INT, IDC_DATA_CHANNEL_EDIT, IDC_DATA_CHANNEL_SPIN, 1.0f,
		end,

	end
);

//=================================================================================================

static FPInterfaceDesc infoTextureInterfaceDesc
(
	kInfoTexture_InterfaceID, _T("IInfoTexture"), IDS_CLASS_NAME, &infoTextureClassDesc, FP_MIXIN,

	properties,
		IInfoTexture::fn_get_info_type, IInfoTexture::fn_set_info_type, _T("infoType"), IDS_NO_DESC, TYPE_ENUM, IInfoTexture::enum_info_type,
		IInfoTexture::fn_get_wrap_mode, IInfoTexture::fn_set_wrap_mode, _T("wrapMode"), IDS_NO_DESC, TYPE_ENUM, IInfoTexture::enum_wrap_mode,
		IInfoTexture::fn_get_coord_sys, IInfoTexture::fn_set_coord_sys, _T("coordSys"), IDS_NO_DESC, TYPE_ENUM, IInfoTexture::enum_coord_sys,

	enums,
		IInfoTexture::enum_info_type, kInfoTypeCount,
		"FaceIndex", kInfoFaceIndex,
		"BaryCoords", kInfoBaryCoords,
		"UvwCoords", kInfoUvwCoords,
		"Normal", kInfoNormal,
		"ReflectVector", kInfoReflectVector,
		"RefractVector", kInfoRefractVector,
		"ObjectBoundingBox", kInfoObjectBoundingBox,
		"VertexData", kInfoVertexData,

		IInfoTexture::enum_wrap_mode, kWrapModeCount,
		"None", kWrapNone,
		"Clamp", kWrapClamp,
		"Repeat", kWrapRepeat,

		IInfoTexture::enum_coord_sys, kCoordSysCount,
		"Camera", kCoordSysCamera,
		"World", kCoordSysWorld,
		"Object", kCoordSysObject,

	end
);

FPInterfaceDesc* IInfoTexture::GetDesc()
{
	return &infoTextureInterfaceDesc;
}

void* InfoTextureClassDesc::Create(BOOL loading)
{
	AddInterface(&infoTextureInterfaceDesc);
	return new InfoTexture();
}

//=================================================================================================

InfoTexture::InfoTexture() :
	m_pblock		(NULL),
	m_infoType		(kInfoFaceIndex),
	m_wrapMode		(kWrapRepeat),
	m_coordSys		(kCoordSysCamera),
	m_uvwChannel	(1),
	m_useFullCameraNormalRange	(false),
	m_dataChannel	(1)
{
	infoTextureClassDesc.MakeAutoParamBlocks(this);
}

// Animatable =====================================================================================

Class_ID InfoTexture::ClassID()
{
	return kInfoTexture_ClassID;
}

SClass_ID InfoTexture::SuperClassID()
{
	return TEXMAP_CLASS_ID;
}

void InfoTexture::DeleteThis()
{
	delete this;
}

int InfoTexture::NumSubs()
{
	return kIt_pblock_count;
}

Animatable* InfoTexture::SubAnim(int i)
{
	switch (i)
	{
		case kIt_pblock_params:	return m_pblock;
		default:				return NULL;
	}
}
#if MAX_RELEASE_R24
TSTR InfoTexture::SubAnimName(int i, BOOL isLocalized)
#else
TSTR InfoTexture::SubAnimName(int i)
#endif
{
	switch (i)
	{
		case kIt_pblock_params:	return GetString(IDS_PARAMS);
		default:				return _T("");
	}
}

int InfoTexture::NumParamBlocks()
{
	return kIt_pblock_count;
}

IParamBlock2* InfoTexture::GetParamBlock(int i)
{
	switch (i)
	{
		case kIt_pblock_params:	return m_pblock;
		default:				return NULL;
	}
}

IParamBlock2* InfoTexture::GetParamBlockByID(BlockID id)
{
	switch (id)
	{
		case kIt_pblock_params:	return m_pblock;
		default:				return NULL;
	}
}

// ReferenceTarget ================================================================================

RefTargetHandle InfoTexture::Clone(RemapDir &remap)
{
	InfoTexture* newtex = new InfoTexture();

	*((MtlBase*)newtex) = *((MtlBase*)this); // copy superclass stuff
	newtex->ReplaceReference(kRef_pblock_params, m_pblock->Clone(remap));
	BaseClone(this, newtex, remap);

	newtex->m_infoType = m_infoType;
	newtex->m_wrapMode = m_wrapMode;
	newtex->m_coordSys = m_coordSys;
	newtex->m_uvwChannel = m_uvwChannel;
	newtex->m_useFullCameraNormalRange = m_useFullCameraNormalRange;
	newtex->m_dataChannel = m_dataChannel;

	return static_cast<RefTargetHandle>(newtex);
}

// ReferenceMaker =================================================================================

int InfoTexture::NumRefs()
{
	return kRef_count;
}

RefTargetHandle InfoTexture::GetReference(int i)
{
	switch (i)
	{
		case kRef_pblock_params:	return m_pblock;
		default:					return NULL;
	}
}

void InfoTexture::SetReference(int i, RefTargetHandle refTarg)
{
	switch (i)
	{
		case kRef_pblock_params: m_pblock = static_cast<IParamBlock2*>(refTarg);
	}
}

#if MAX_VERSION_MAJOR < 17 //Max 2015
RefResult InfoTexture::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget,
	PartID& partID, RefMessage message)
#else
RefResult InfoTexture::NotifyRefChanged(const Interval& changeInt, RefTargetHandle hTarget,
	PartID& partID, RefMessage message, BOOL propagate)
#endif
{
	switch (message)
	{
		case REFMSG_CHANGE:
		{
			if (hTarget == m_pblock)
			{
				ParamID changingParam = m_pblock->LastNotifyParamID();
				infoTexturePBlockDesc.InvalidateUI(changingParam);
			}
			break;
		}
	}

	return REF_SUCCEED;
}

// MtlBase ========================================================================================

ULONG InfoTexture::LocalRequirements(int subMtlNum)
{
	switch (m_infoType)
	{
		case kInfoFaceIndex:			return 0;
		case kInfoBaryCoords:			return 0;
		case kInfoUvwCoords:
		{
			switch (m_uvwChannel)
			{
				case 0: return MTLREQ_UV2;
				case 1: return MTLREQ_UV;
				default: return 0;
			}
		}
		case kInfoNormal:				return MTLREQ_VIEW_DEP | MTLREQ_PHONG | MTLREQ_XYZ | MTLREQ_WORLDCOORDS;
		case kInfoReflectVector:		return MTLREQ_VIEW_DEP | MTLREQ_PHONG;
		case kInfoRefractVector:		return MTLREQ_VIEW_DEP | MTLREQ_PHONG;
		case kInfoObjectBoundingBox:	return MTLREQ_XYZ;
		case kInfoVertexData:			return 0;
		default:
		{
			// each infoType MUST have an explicit LocalRequirements to avoid bugs
			assert(false);
			// avoid compiler warning...
			return 0;
		}
	}
}

void InfoTexture::LocalMappingsRequired(int subMtlNum, BitArray& mapreq, BitArray& bumpreq)
{
	if (m_infoType == kInfoUvwCoords)
	{
		mapreq.Set(m_uvwChannel);
	}
}

void InfoTexture::Update(TimeValue t, Interval& valid)
{
	m_infoType = static_cast<InfoType>(m_pblock->GetInt(kIt_info_type, t));
	m_wrapMode = static_cast<WrapMode>(m_pblock->GetInt(kIt_wrap_mode, t));
	m_coordSys = static_cast<CoordSys>(m_pblock->GetInt(kIt_coord_sys, t));
	m_uvwChannel = m_pblock->GetInt(kIt_uvw_channel, t);
	m_useFullCameraNormalRange = (m_pblock->GetInt(kIt_use_full_camera_normal_range, t)) ? TRUE : FALSE;
	m_dataChannel = m_pblock->GetInt(kIt_data_channel, t) - 1;	// UI is one based, internally zero based.

	valid &= Validity(t);
}

void InfoTexture::Reset()
{
	infoTextureClassDesc.Reset(this, TRUE);	// reset all pblocks
}

Interval InfoTexture::Validity(TimeValue t)
{
	Interval ivalid;
	m_pblock->GetValidity(t, ivalid);
	return ivalid;
}

#define MTL_HDR_CHUNK 0x4000

IOResult InfoTexture::Save(ISave* isave)
{
	IOResult res;

	isave->BeginChunk(MTL_HDR_CHUNK);
	res = MtlBase::Save(isave);
	if (res != IO_OK) return res;
	isave->EndChunk();

	return IO_OK;
}

IOResult InfoTexture::Load(ILoad* iload)
{
	while (IO_OK == iload->OpenChunk())
	{
		switch (iload->CurChunkID())
		{
			case MTL_HDR_CHUNK:
			{
				IOResult res = MtlBase::Load(iload);
				if (res != IO_OK) return res;
				break;
			}
		}
		iload->CloseChunk();
	}

	return IO_OK;
}

ParamDlg* InfoTexture::CreateParamDlg(HWND hwMtlEdit, IMtlParams* imp)
{
	IAutoMParamDlg* masterDlg = infoTextureClassDesc.CreateParamDlgs(hwMtlEdit, imp, this);

	imp->AddRollupPage(hInstance, MAKEINTRESOURCE(IDD_ABOUT), aboutDlgProc, _T("About"));

	return masterDlg;
}

// Texmap =========================================================================================

AColor InfoTexture::EvalColor(ShadeContext& sc)
{
	AColor color(0.0f, 0.0f, 0.0f, 1.0f);

#if _DEBUG
	IPoint2 screenCoord = sc.ScreenCoord();
	if (
		screenCoord.x == 300 &&
		screenCoord.y == 230
	) {
		int breakHere = 0;
	}
#endif

	switch (m_infoType)
	{
		case kInfoFaceIndex:
		{
			int faceNum = sc.FaceNumber();

			color.r = (float) (faceNum & 0x0000FF)      / 255.0f;
			color.g = (float)((faceNum & 0x00FF00)>>8)  / 255.0f;
			color.b = (float)((faceNum & 0xFF0000)>>16) / 255.0f;

			break;
		}
		case kInfoBaryCoords:
		{
			Point3 baryCoords = sc.BarycentricCoords();

			color.r = baryCoords.x;
			color.g = baryCoords.y;
			color.b = baryCoords.z;

			break;
		}
		case kInfoUvwCoords:
		{
			Point3 uvw = sc.UVW(m_uvwChannel);

			color.r = uvw.x;
			color.g = uvw.y;
			color.b = uvw.z;

			switch (m_wrapMode)
			{
				case kWrapNone:
				{
					break;
				}
				case kWrapClamp:
				{
					CLAMP(color.r, 0.0f, 1.0f);
					CLAMP(color.g, 0.0f, 1.0f);
					CLAMP(color.b, 0.0f, 1.0f);
					break;
				}
				case kWrapRepeat:
				{
					REPEAT(color.r);
					REPEAT(color.g);
					REPEAT(color.b);
					break;
				}
			}

			break;
		}
		case kInfoNormal:
		{
			Point3 vec;

			switch (m_coordSys)
			{
				case kCoordSysCamera:
					vec = sc.VectorToNoScale(sc.Normal(), REF_CAMERA);
					color.b = (m_useFullCameraNormalRange) ? ((vec.z + 1.0f) * 0.5f) : (vec.z);
					break;
				case kCoordSysWorld:
					vec = sc.VectorToNoScale(sc.Normal(), REF_WORLD);
					color.b = (vec.z + 1.0f) * 0.5f;
					break;
				case kCoordSysObject:
					vec = sc.VectorToNoScale(sc.Normal(), REF_OBJECT);
					color.b = (vec.z + 1.0f) * 0.5f;
					break;
			}

			color.r = (vec.x + 1.0f) * 0.5f;
			color.g = (vec.y + 1.0f) * 0.5f;

			break;
		}
		case kInfoReflectVector:
		{
			Point3 vec;

			switch (m_coordSys)
			{
				case kCoordSysCamera:	vec = sc.VectorToNoScale(sc.ReflectVector(), REF_CAMERA); break;
				case kCoordSysWorld:	vec = sc.VectorToNoScale(sc.ReflectVector(), REF_WORLD); break;
				case kCoordSysObject:	vec = sc.VectorToNoScale(sc.ReflectVector(), REF_OBJECT); break;
			}

			color.r = (vec.x + 1.0f) * 0.5f;
			color.g = (vec.y + 1.0f) * 0.5f;
			color.b = (vec.z + 1.0f) * 0.5f;

			break;
		}
		case kInfoRefractVector:
		{
			Point3 vec;

			switch (m_coordSys)
			{
				case kCoordSysCamera:	vec = sc.VectorToNoScale(sc.RefractVector(sc.GetIOR()), REF_CAMERA); break;
				case kCoordSysWorld:	vec = sc.VectorToNoScale(sc.RefractVector(sc.GetIOR()), REF_WORLD); break;
				case kCoordSysObject:	vec = sc.VectorToNoScale(sc.RefractVector(sc.GetIOR()), REF_OBJECT); break;
			}

			color.r = (vec.x + 1.0f) * 0.5f;
			color.g = (vec.y + 1.0f) * 0.5f;
			color.b = (vec.z + 1.0f) * 0.5f;

			break;
		}
		case kInfoObjectBoundingBox:
		{
			Point3 pnt = sc.PObjRelBox();

			color.r = (pnt.x + 1.0f) * 0.5f;
			color.g = (pnt.y + 1.0f) * 0.5f;
			color.b = (pnt.z + 1.0f) * 0.5f;

			break;
		}
		case kInfoVertexData:
		{
			if (sc.InMtlEditor() || sc.globContext == NULL || sc.NodeID() < 0) break;

			RenderInstance* ri = sc.globContext->GetRenderInstance(sc.NodeID());
			if (!ri) break;

			Mesh* mesh = ri->mesh;
			if (!mesh) break;

			if (!mesh->vDataSupport(m_dataChannel)) break;

			float* vertexData = mesh->vertexFloat(m_dataChannel);
			if (!vertexData) break;

			Point3 bc = sc.BarycentricCoords();
			int faceIndex = sc.FaceNumber();

			Face* face = &mesh->faces[faceIndex];
			float data =	vertexData[face->v[0]] * bc.x +
							vertexData[face->v[1]] * bc.y +
							vertexData[face->v[2]] * bc.z;

			switch (m_wrapMode)
			{
				case kWrapNone:		break;
				case kWrapClamp:	CLAMP(data, 0.0f, 1.0f); break;
				case kWrapRepeat:	REPEAT(data); break;
			}

			color.r = color.g = color.b = data;

			break;
		}
	}

	return color;
}

float InfoTexture::EvalMono(ShadeContext& sc)
{
	return Intens(EvalColor(sc));
}

Point3 InfoTexture::EvalNormalPerturb(ShadeContext& sc)
{
	return Point3(0, 0, 0);
}

// ILayerTexture //////////////////////////////////////////////////////////////////////////////////

BaseInterface* InfoTexture::GetInterface(Interface_ID id)
{
	if (id == kInfoTexture_InterfaceID) return (IInfoTexture*)this;
	return FPMixinInterface::GetInterface(id);
}

int		InfoTexture::GetInfoType()				{ return m_pblock->GetInt(kIt_info_type); }
void	InfoTexture::SetInfoType(int infoType)	{ m_pblock->SetValue(kIt_info_type, 0, infoType); }

int		InfoTexture::GetWrapMode()				{ return m_pblock->GetInt(kIt_wrap_mode); }
void	InfoTexture::SetWrapMode(int wrapMode)	{ m_pblock->SetValue(kIt_wrap_mode, 0, wrapMode); }

int		InfoTexture::GetCoordSys()				{ return m_pblock->GetInt(kIt_coord_sys); }
void	InfoTexture::SetCoordSys(int coordSys)	{ m_pblock->SetValue(kIt_coord_sys, 0, coordSys); }

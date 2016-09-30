#pragma once

#include "BaseInc.h"

class CVmpExporter : public ITreeEnumProc
{
public:
	CVmpExporter(Interface * pi, ITextOutputStream * pLog, int_32 iFrame, bool bExportSelected = false);
	int callback(INode * pNode);

	err_t Parse(bool bLocalCoord);
	err_t ExportMesh(IDataOutputStream * pDos, vertexformat_e eVertexFormat);
	void ExportAnim(IDataOutputStream * pDos, int_x iFramePerSecond);
	
private:
	TriObject * GetTriObjectFromNode(INode * pNode, bool & bDeleteMe);
	Modifier * GetModifierFromNode(INode * pNode, const Class_ID & cid);

	int_x TryAddAnimNode(INode * pBoneNode);
private:
	Interface * m_pi;
	bool m_bExportSelected;

	int_32 m_iIndex;
	int_32 m_iFrame;
	ITextOutputStream * m_pLog;
	vector<INode *> m_triNodes;
	vector<INode *> m_animNodes;

	int_x iVertexSum;
	int_x iIndexSum;
	int_x iUnitSum;

	// ��������
	//vector<vm_subset_t> m_nodes;
	//vector<vertex_t> m_vertices;
	//vector<uint_16> m_indices;

	// ��������
	vector<va_bone_t> m_bones;
	vector<va_frame_t> m_keyframes;

	textx m_info;
};


ControllerTypeE GetControllerType(Control * pController);
ControllerModeE GetPosControllerMode(Control * pController);
ControllerModeE GetRotControllerMode(Control * pController);
ControllerModeE GetSclControllerMode(Control * pController);

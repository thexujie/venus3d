#pragma once

#define _UNICODE_MODULE_FOR_MBCS_MAX
#include <Max.h>
#include <istdplug.h>
#include <iparamb2.h>
#include <iparamm2.h>
#include <iskin.h>
#include <3dsmaxport.h>
#include <modstack.h>
#include <stdmat.h>
#include <decomp.h>
#include <plugapi.h>

#ifdef GetForm
#undef GetForm
#endif

#include "../../venus/win32/win32.h"
using namespace venus;

const char_16 APP_NAME[] = L"VenusExporter";

const char_16 MAP_SLOT_NAME[][32] =
{
	L"Diffuse",
	L"Ambient",
	L"Specular",
	L"SpecularLevel",
	L"Opacity",
	L"Glossiness",
	L"SelfIllumination",
	L"Filter",
	L"Bump",
	L"Reflection",
	L"Refraction",
	L"Displacement",
	L"Unknown",
};

enum ExportModeE
{
	ExportModeMesh,
	ExportModeAnim,
};

const Class_ID Class_ID_Triobj = Class_ID(TRIOBJ_CLASS_ID, 0);
const Class_ID Class_ID_Bone = Class_ID(BONE_CLASS_ID, 0);
const Class_ID Class_ID_Helper = Class_ID(HELPER_CLASS_ID, 0);

//! √…∆§
const Class_ID Class_ID_Skin = Class_ID(0x0095C723, 0x00015666);
//! √…∆§±‰–Œ
const Class_ID Class_ID_SkinMorph = Class_ID(0x7AD09741, 0x69842453);

enum ClassIdE
{
	ClassId_Bip = 0x28bf6e8d,
	ClassId_Box = BOXOBJ_CLASS_ID,
	ClassId_Sphere = SPHERE_CLASS_ID,
	ClassId_Cylinder = CYLINDER_CLASS_ID,
	ClassId_Plane = 0x81f1dfc, //PLANE_CLASS_ID.PartA();
	ClassId_Pyramid = 0x76bf318a, //PYRAMID_CLASS_ID.PartA();

	ClassId_GeomObject = ClassId_Box, // GEOMOBJECT_CLASS_ID,
	ClassId_TriObj = TRIOBJ_CLASS_ID,
	ClassId_EditableObj = EDITTRIOBJ_CLASS_ID,
	ClassId_PolyObj = POLYOBJ_CLASS_ID,
	ClassId_EditablePoly = 0x1bf8338d,
	ClassId_IkChainObj = 0x1c706482,
};

enum ControllerTypeE
{
	ControllerTypeInvalid,
	ControllerTypeFloat,
	ControllerTypePoint3,
	ControllerTypeMatrix3,
	ControllerTypePosition,
	ControllerTypeRotation,
	ControllerTypeScale,
	ControllerTypeMorph,
};

enum ControllerModeE
{
	ControllerModeInvalid,
	ControllerModeTcb,
	ControllerModeLinear,
	ControllerModeBezier,
	ControllerModeConst,
};

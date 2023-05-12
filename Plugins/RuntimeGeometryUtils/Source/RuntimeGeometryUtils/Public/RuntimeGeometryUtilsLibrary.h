#pragma once

#include "DynamicMesh3.h"
#include "DynamicMeshAttributeSet.h"

class URuntimeGeometryUtilsLibrary
{
public:
	URuntimeGeometryUtilsLibrary();

	static void ConvertStaticMeshToDynamicMesh(UStaticMesh* Mesh, int32 LODIndex, int32 SectionIndex, FDynamicMesh3& OutMesh);
};
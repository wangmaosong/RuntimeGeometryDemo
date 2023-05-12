#include "RuntimeGeometryUtilsLibrary.h"
#include <Kismet/KismetMathLibrary.h>
#include "Misc/UObjectToken.h"

#define LOCTEXT_NAMESPACE "RuntimeGeometryUtilsLibrary"

URuntimeGeometryUtilsLibrary::URuntimeGeometryUtilsLibrary()
{

}

void URuntimeGeometryUtilsLibrary::ConvertStaticMeshToDynamicMesh(UStaticMesh* Mesh, int32 LODIndex, int32 SectionIndex, FDynamicMesh3& OutMesh)
{
	if (Mesh == nullptr) 
	{
		return;
	}

	if (!Mesh->bAllowCPUAccess)
	{
		FMessageLog("PIE").Warning()
			->AddToken(FTextToken::Create(LOCTEXT("ConvertStaticMeshToDynamicMeshStart", "Calling ConvertStaticMeshToDynamicMesh on")))
			->AddToken(FUObjectToken::Create(Mesh))
			->AddToken(FTextToken::Create(LOCTEXT("ConvertStaticMeshToDynamicMeshEnd", "but 'Allow CPU Access' is not enabled. This is required for converting StaticMesh to FDynamicMesh3 in cooked builds.")));
		return;
	}

	if (Mesh->RenderData == nullptr || !Mesh->RenderData->LODResources.IsValidIndex(LODIndex))
	{
		return;
	}

	const FStaticMeshLODResources& LOD = Mesh->RenderData->LODResources[LODIndex];
	if (!LOD.Sections.IsValidIndex(SectionIndex)) 
	{ 
		return; 
	}

	OutMesh.EnableAttributes();
	OutMesh.EnableVertexColors(FVector3f(0.0f, 0.0f, 0.0f));
	OutMesh.EnableVertexNormals(FVector3f(0.0f, 0.0f, 1.0f));
	OutMesh.EnableVertexUVs(FVector2f(0.0f, 0.0f));

	FDynamicMeshNormalOverlay* Normals = OutMesh.Attributes()->PrimaryNormals();
	FDynamicMeshUVOverlay* UVs = OutMesh.HasAttributes() ? OutMesh.Attributes()->PrimaryUV() : nullptr;

	TMap<int32, int32> MeshToSectionVertMap = {};
	uint32 Index = 0;
	uint32 SectionsFirstIndex = LOD.Sections[SectionIndex].FirstIndex;
	uint32 SectionsLastIndex = LOD.Sections[SectionIndex].FirstIndex + LOD.Sections[SectionIndex].NumTriangles * 3;
	FIndexArrayView Indices = LOD.IndexBuffer.GetArrayView();
	bool hasColors = LOD.VertexBuffers.ColorVertexBuffer.GetNumVertices() >= LOD.VertexBuffers.PositionVertexBuffer.GetNumVertices();

	int32 VertexCount = 0, CurrentVertexIndex = 0, VertexIndex = 0;
	int32* NewIndexPtr = nullptr;
	FIndex3i Tris = {};
	for (Index = SectionsFirstIndex; Index < SectionsLastIndex; ++Index) 
	{
		VertexIndex = Indices[Index];
		NewIndexPtr = MeshToSectionVertMap.Find(VertexIndex);
		if (NewIndexPtr != nullptr) 
		{ 
			CurrentVertexIndex = *NewIndexPtr; 
		}
		else 
		{
			// add vertex, normal, uv, vertex color, depend vertex count
			OutMesh.AppendVertex(LOD.VertexBuffers.PositionVertexBuffer.VertexPosition(VertexIndex));
			FVector Normal = LOD.VertexBuffers.StaticMeshVertexBuffer.VertexTangentZ(VertexIndex);
			OutMesh.SetVertexNormal(VertexCount, Normal);
			FVector2D UVCoords = LOD.VertexBuffers.StaticMeshVertexBuffer.GetVertexUV(VertexIndex, 0);
			OutMesh.SetVertexUV(VertexCount, UVCoords);
			UVs->AppendElement(UVCoords);
			if (hasColors) 
			{ 
				FColor VertexColor = LOD.VertexBuffers.ColorVertexBuffer.VertexColor(VertexIndex);
				OutMesh.SetVertexColor(VertexCount, FVector3f(VertexColor.R, VertexColor.G, VertexColor.B));
			}
			CurrentVertexIndex = VertexCount;
			MeshToSectionVertMap.Add(VertexIndex, VertexCount);
			VertexCount++;
		}

		int32 TrisIndex = (Index-SectionsFirstIndex) % 3;
		Tris[TrisIndex] = CurrentVertexIndex;
		if (Index != SectionsFirstIndex && TrisIndex == 2)
		{
			// add triangle
			int tid = OutMesh.AppendTriangle(Tris);
			UVs->SetTriangle(tid, Tris);
		}
	}
}

#undef LOCTEXT_NAMESPACE
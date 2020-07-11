#pragma once
class ResourceManager
{
public:
	template<class _VertTy>
	static HRESULT AllocModelBuffer(ID3D11Device* Device, MeshBuffer** pBuffer,  Mesh<_VertTy>* InputMesh);

	template<class _Ty>
	static HRESULT AllocCustomConstBuffer(ID3D11Device* Device, MeshBuffer** pOutBuffer);

	template<class _Ty>
	static HRESULT AllocCustomConstBuffer(ID3D11Device* Device, ID3D11Buffer** pOutBuffer);

	template<class _Ty>
	static HRESULT AllocCustomBuffer(ID3D11Device* Device, ID3D11Buffer** pOutBuffer, size_t Count, D3D11_BIND_FLAG Bind, D3D11_USAGE Usage, UINT MiscFlag, void** InitialData = nullptr);

	static HRESULT CreateTexture2D(ID3D11Device* Device, DXGI_FORMAT Format, UINT Width, UINT Height, RenderTarget2D* pOutTexture);

	template<class _Ty>
	static HRESULT GenerateTriangleBuffer(ID3D11Device* Device, Mesh<_Ty>* Object, ID3D11Buffer** pOutBuffer, ID3D11ShaderResourceView** pOutSRV);

	static HRESULT CreateRenderTarget2D(ID3D11Device* Device, RenderTarget2D** pRT, DXGI_FORMAT Format, UINT BindFlags, UINT Width, UINT Height, UINT Usage,UINT CPUAccessFlag = 0, UINT ArraySize = 1);
	static HRESULT CreateComputeTexture2D(ID3D11Device* Device, ComputeTexture2D** pRT, DXGI_FORMAT Format, UINT BindFlags, UINT Width, UINT Height, UINT Usage,UINT CPUAccessFlag = 0, UINT ArraySize = 1);

	static HRESULT CreateComputeTexture3D(ID3D11Device* Device, ComputeTexture3D** pCT, DXGI_FORMAT Format, UINT BindFlags, UINT Width, UINT Height, UINT Depth, UINT Usage, UINT CPUAccessFlag = 0, UINT ArraySize = 1);
};

template<class _VertTy>
inline HRESULT ResourceManager::AllocModelBuffer(ID3D11Device* Device, MeshBuffer** pBuffer, Mesh<_VertTy>* InputMesh)
{
	HRESULT Result;
	D3D11_BUFFER_DESC VDesc{}, IDesc{};
	D3D11_SUBRESOURCE_DATA SubData{};

	*pBuffer = new MeshBuffer();

	VDesc.ByteWidth = sizeof(_VertTy) * InputMesh->Vertices.size();
	VDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	VDesc.Usage = D3D11_USAGE_DEFAULT;
	
	IDesc.ByteWidth = sizeof(Index) * InputMesh->Indices.size();
	IDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	IDesc.Usage = D3D11_USAGE_DEFAULT;

	SubData.pSysMem = InputMesh->Vertices.data();
	Result = Device->CreateBuffer(&VDesc, &SubData, &pBuffer[0]->VertexBuffer);
	assert(Result == S_OK && "Failed to create a vertex buffer");

	SubData.pSysMem = InputMesh->Indices.data();
	Result = Device->CreateBuffer(&IDesc, &SubData, &pBuffer[0]->IndexBuffer);
	assert(Result == S_OK && "Failed to create a index buffer");

	return S_OK;
}

template<class _Ty>
inline HRESULT ResourceManager::AllocCustomConstBuffer(ID3D11Device* Device, MeshBuffer** pOutBuffer)
{
	
	HRESULT Result;
	D3D11_BUFFER_DESC Desc{};

	if (*pOutBuffer == nullptr)
		*pOutBuffer = new MeshBuffer();

	Desc.ByteWidth = sizeof(_Ty);
	Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	Desc.Usage = D3D11_USAGE_DEFAULT;

	Result = Device->CreateBuffer(&Desc, nullptr, &pOutBuffer[0]->ConstBuffer);
	assert(Result == S_OK && "Failed to create const buffer");

	return S_OK;
}

template<class _Ty>
inline HRESULT ResourceManager::AllocCustomConstBuffer(ID3D11Device* Device, ID3D11Buffer** pOutBuffer)
{

	HRESULT Result;
	D3D11_BUFFER_DESC Desc{};

	Desc.ByteWidth = sizeof(_Ty);
	Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	Desc.Usage = D3D11_USAGE_DEFAULT;

	Result = Device->CreateBuffer(&Desc, nullptr, &pOutBuffer[0]);
	assert(Result == S_OK && "Failed to create const buffer");

	return S_OK;
}


template<class _Ty>
inline HRESULT ResourceManager::AllocCustomBuffer(ID3D11Device* Device, ID3D11Buffer** pOutBuffer, size_t Count, D3D11_BIND_FLAG Bind, D3D11_USAGE Usage,UINT MiscFlag, void** InitialData)
{

	HRESULT Result;
	D3D11_BUFFER_DESC Desc{};
	D3D11_SUBRESOURCE_DATA SubData{};

	
	Desc.ByteWidth = sizeof(_Ty)* Count;
	Desc.BindFlags = Bind;
	Desc.Usage = Usage;
	Desc.MiscFlags = MiscFlag;
	
	if (MiscFlag != 0)
		Desc.StructureByteStride = sizeof(_Ty);
	
	if (InitialData)
	{
		SubData.pSysMem = InitialData;

		Result = Device->CreateBuffer(&Desc, &SubData, pOutBuffer);
	}
	else
	{
		Result = Device->CreateBuffer(&Desc, nullptr, pOutBuffer);

	}

	assert(Result == S_OK && "Failed to create custom buffer");

	return S_OK;
}

template<class _Ty>
inline HRESULT ResourceManager::GenerateTriangleBuffer(ID3D11Device* Device, Mesh<_Ty>* Object, ID3D11Buffer** pOutBuffer, ID3D11ShaderResourceView** pOutSRV)
{
	HRESULT Result;
	vector<Triangle<_Ty>> TriList;
	D3D11_BUFFER_DESC Desc{};
	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc{};
	D3D11_SUBRESOURCE_DATA SubData{};


	for (int I=0;I<Object->Indices.size();I+=3)
	{
		Triangle<_Ty> Tri;

		if (Object->Indices[I] < Object->Vertices.size() && Object->Indices[I] >= 0)
		{
			Index Idx1 = Object->Indices[I];
			Index Idx2 = Object->Indices[I + 1];
			Index Idx3 = Object->Indices[I + 2];



			Tri.Tri[0] = Object->Vertices[Idx1];
			Tri.Tri[1] = Object->Vertices[Idx2];
			Tri.Tri[2] = Object->Vertices[Idx3];
		}
		TriList.push_back(Tri);
	}

	SubData.pSysMem = TriList.data();

	SRVDesc.Format = DXGI_FORMAT_UNKNOWN;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	SRVDesc.Buffer.NumElements = TriList.size();

	Desc.ByteWidth = sizeof(Triangle<_Ty>) * TriList.size();
	Desc.BindFlags = D3D11_BIND_SHADER_RESOURCE|D3D11_BIND_UNORDERED_ACCESS;
	Desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	Desc.StructureByteStride = sizeof(Triangle<_Ty>);
	Desc.Usage = D3D11_USAGE_DEFAULT;

	Object->TriangleCount = TriList.size();

	Result = Device->CreateBuffer(&Desc, &SubData, pOutBuffer);
	assert(Result == S_OK);

	Result = Device->CreateShaderResourceView(*pOutBuffer, &SRVDesc, pOutSRV);
	assert(Result == S_OK);

	return S_OK;
}

//static HRESULT LoadObjFromFile(ID3D11Device* Device, string Path, ModelArray<Vertex>* pOutModel)
//{
//    HRESULT hr = 0;
//
//    std::wifstream fileIn(Path.c_str());    //Open file
//    std::wstring meshMatLib;                    //String to hold our obj material library filename
//
//    //Arrays to store our model's information
//    std::vector<DWORD> indices;
//    std::vector<XMFLOAT4> vertPos;
//    std::vector<XMFLOAT3> vertNorm;
//    std::vector<XMFLOAT2> vertTexCoord;
//    std::vector<std::wstring> meshMaterials;
//
//    //Vertex definition indices
//    std::vector<int> vertPosIndex;
//    std::vector<int> vertNormIndex;
//    std::vector<int> vertTCIndex;
//
//    //Make sure we have a default if no tex coords or normals are defined
//    bool hasTexCoord = false;
//    bool hasNorm = false;
//
//    //Temp variables to store into vectors
//    std::wstring meshMaterialsTemp;
//    int vertPosIndexTemp;
//    int vertNormIndexTemp;
//    int vertTCIndexTemp;
//
//    wchar_t checkChar;        //The variable we will use to store one char from file at a time
//    std::wstring face;        //Holds the string containing our face vertices
//    int vIndex = 0;            //Keep track of our vertex index count
//    int triangleCount = 0;    //Total Triangles
//    int totalVerts = 0;
//    int meshTriangles = 0;
//
//    //Check to see if the file was opened
//    if (fileIn)
//    {
//        while (fileIn)
//        {
//            checkChar = fileIn.get();    //Get next char
//
//            switch (checkChar)
//            {
//                case '#':
//                    checkChar = fileIn.get();
//                    while (checkChar != '\n')
//                        checkChar = fileIn.get();
//                    break;
//                case 'v':    //Get Vertex Descriptions
//                    checkChar = fileIn.get();
//                    if (checkChar == ' ')    //v - vert position
//                    {
//                        float vz, vy, vx;
//                        fileIn >> vx >> vy >> vz;    //Store the next three types
//
//                        vertPos.push_back(XMFLOAT4(vx, vy, vz,1.0f));
//                    }
//                    if (checkChar == 't')    //vt - vert tex coords
//                    {
//                        float vtcu, vtcv;
//                        fileIn >> vtcu >> vtcv;        //Store next two types
//
//                        vertTexCoord.push_back(XMFLOAT2(vtcu, vtcv));
//
//                        hasTexCoord = true;    //We know the model uses texture coords
//                    }
//                    //Since we compute the normals later, we don't need to check for normals
//                    //In the file, but i'll do it here anyway
//                    if (checkChar == 'n')    //vn - vert normal
//                    {
//                        float vnx, vny, vnz;
//                        fileIn >> vnx >> vny >> vnz;    //Store next three types
//
//                        vertNorm.push_back(XMFLOAT3(vnx, vny, vnz));
//
//                        hasNorm = true;    //We know the model defines normals
//                    }
//                    break;
//
//                    //New group (Subset)
//                case 'g':    //g - defines a group
//                    checkChar = fileIn.get();
//                    if (checkChar == ' ')
//                    {
//                        subsetIndexStart.push_back(vIndex);        //Start index for this subset
//                        subsetCount++;
//                    }
//                    break;
//
//                    //Get Face Index
//                case 'f':    //f - defines the faces
//                    checkChar = fileIn.get();
//                    if (checkChar == ' ')
//                    {
//                        face = L"";
//                        std::wstring VertDef;    //Holds one vertex definition at a time
//                        triangleCount = 0;
//
//                        checkChar = fileIn.get();
//                        while (checkChar != '\n')
//                        {
//                            face += checkChar;            //Add the char to our face string
//                            checkChar = fileIn.get();    //Get the next Character
//                            if (checkChar == ' ')        //If its a space...
//                                triangleCount++;        //Increase our triangle count
//                        }
//
//                        //Check for space at the end of our face string
//                        if (face[face.length() - 1] == ' ')
//                            triangleCount--;    //Each space adds to our triangle count
//
//                        triangleCount -= 1;        //Ever vertex in the face AFTER the first two are new faces
//
//                        std::wstringstream ss(face);
//
//                        if (face.length() > 0)
//                        {
//                            int firstVIndex, lastVIndex;    //Holds the first and last vertice's index
//
//                            for (int i = 0; i < 3; ++i)        //First three vertices (first triangle)
//                            {
//                                ss >> VertDef;    //Get vertex definition (vPos/vTexCoord/vNorm)
//
//                                std::wstring vertPart;
//                                int whichPart = 0;        //(vPos, vTexCoord, or vNorm)
//
//                                //Parse this string
//                                for (int j = 0; j < VertDef.length(); ++j)
//                                {
//                                    if (VertDef[j] != '/')    //If there is no divider "/", add a char to our vertPart
//                                        vertPart += VertDef[j];
//
//                                    //If the current char is a divider "/", or its the last character in the string
//                                    if (VertDef[j] == '/' || j == VertDef.length() - 1)
//                                    {
//                                        std::wistringstream wstringToInt(vertPart);    //Used to convert wstring to int
//
//                                        if (whichPart == 0)    //If vPos
//                                        {
//                                            wstringToInt >> vertPosIndexTemp;
//                                            vertPosIndexTemp -= 1;        //subtract one since c++ arrays start with 0, and obj start with 1
//
//                                            //Check to see if the vert pos was the only thing specified
//                                            if (j == VertDef.length() - 1)
//                                            {
//                                                vertNormIndexTemp = 0;
//                                                vertTCIndexTemp = 0;
//                                            }
//                                        }
//
//                                        else if (whichPart == 1)    //If vTexCoord
//                                        {
//                                            if (vertPart != L"")    //Check to see if there even is a tex coord
//                                            {
//                                                wstringToInt >> vertTCIndexTemp;
//                                                vertTCIndexTemp -= 1;    //subtract one since c++ arrays start with 0, and obj start with 1
//                                            }
//                                            else    //If there is no tex coord, make a default
//                                                vertTCIndexTemp = 0;
//
//                                            //If the cur. char is the second to last in the string, then
//                                            //there must be no normal, so set a default normal
//                                            if (j == VertDef.length() - 1)
//                                                vertNormIndexTemp = 0;
//
//                                        }
//                                        else if (whichPart == 2)    //If vNorm
//                                        {
//                                            std::wistringstream wstringToInt(vertPart);
//
//                                            wstringToInt >> vertNormIndexTemp;
//                                            vertNormIndexTemp -= 1;        //subtract one since c++ arrays start with 0, and obj start with 1
//                                        }
//
//                                        vertPart = L"";    //Get ready for next vertex part
//                                        whichPart++;    //Move on to next vertex part                    
//                                    }
//                                }
//
//                                //Check to make sure there is at least one subset
//                                if (subsetCount == 0)
//                                {
//                                    subsetIndexStart.push_back(vIndex);        //Start index for this subset
//                                    subsetCount++;
//                                }
//
//                                //Avoid duplicate vertices
//                                bool vertAlreadyExists = false;
//                                if (totalVerts >= 3)    //Make sure we at least have one triangle to check
//                                {
//                                    //Loop through all the vertices
//                                    for (int iCheck = 0; iCheck < totalVerts; ++iCheck)
//                                    {
//                                        //If the vertex position and texture coordinate in memory are the same
//                                        //As the vertex position and texture coordinate we just now got out
//                                        //of the obj file, we will set this faces vertex index to the vertex's
//                                        //index value in memory. This makes sure we don't create duplicate vertices
//                                        if (vertPosIndexTemp == vertPosIndex[iCheck] && !vertAlreadyExists)
//                                        {
//                                            if (vertTCIndexTemp == vertTCIndex[iCheck])
//                                            {
//                                                indices.push_back(iCheck);        //Set index for this vertex
//                                                vertAlreadyExists = true;        //If we've made it here, the vertex already exists
//                                            }
//                                        }
//                                    }
//                                }
//
//                                //If this vertex is not already in our vertex arrays, put it there
//                                if (!vertAlreadyExists)
//                                {
//                                    vertPosIndex.push_back(vertPosIndexTemp);
//                                    vertTCIndex.push_back(vertTCIndexTemp);
//                                    vertNormIndex.push_back(vertNormIndexTemp);
//                                    totalVerts++;    //We created a new vertex
//                                    indices.push_back(totalVerts - 1);    //Set index for this vertex
//                                }
//
//                                //If this is the very first vertex in the face, we need to
//                                //make sure the rest of the triangles use this vertex
//                                if (i == 0)
//                                {
//                                    firstVIndex = indices[vIndex];    //The first vertex index of this FACE
//
//                                }
//
//                                //If this was the last vertex in the first triangle, we will make sure
//                                //the next triangle uses this one (eg. tri1(1,2,3) tri2(1,3,4) tri3(1,4,5))
//                                if (i == 2)
//                                {
//                                    lastVIndex = indices[vIndex];    //The last vertex index of this TRIANGLE
//                                }
//                                vIndex++;    //Increment index count
//                            }
//
//                            meshTriangles++;    //One triangle down
//
//                            //If there are more than three vertices in the face definition, we need to make sure
//                            //we convert the face to triangles. We created our first triangle above, now we will
//                            //create a new triangle for every new vertex in the face, using the very first vertex
//                            //of the face, and the last vertex from the triangle before the current triangle
//                            for (int l = 0; l < triangleCount - 1; ++l)    //Loop through the next vertices to create new triangles
//                            {
//                                //First vertex of this triangle (the very first vertex of the face too)
//                                indices.push_back(firstVIndex);            //Set index for this vertex
//                                vIndex++;
//
//                                //Second Vertex of this triangle (the last vertex used in the tri before this one)
//                                indices.push_back(lastVIndex);            //Set index for this vertex
//                                vIndex++;
//
//                                //Get the third vertex for this triangle
//                                ss >> VertDef;
//
//                                std::wstring vertPart;
//                                int whichPart = 0;
//
//                                //Parse this string (same as above)
//                                for (int j = 0; j < VertDef.length(); ++j)
//                                {
//                                    if (VertDef[j] != '/')
//                                        vertPart += VertDef[j];
//                                    if (VertDef[j] == '/' || j == VertDef.length() - 1)
//                                    {
//                                        std::wistringstream wstringToInt(vertPart);
//
//                                        if (whichPart == 0)
//                                        {
//                                            wstringToInt >> vertPosIndexTemp;
//                                            vertPosIndexTemp -= 1;
//
//                                            //Check to see if the vert pos was the only thing specified
//                                            if (j == VertDef.length() - 1)
//                                            {
//                                                vertTCIndexTemp = 0;
//                                                vertNormIndexTemp = 0;
//                                            }
//                                        }
//                                        else if (whichPart == 1)
//                                        {
//                                            if (vertPart != L"")
//                                            {
//                                                wstringToInt >> vertTCIndexTemp;
//                                                vertTCIndexTemp -= 1;
//                                            }
//                                            else
//                                                vertTCIndexTemp = 0;
//                                            if (j == VertDef.length() - 1)
//                                                vertNormIndexTemp = 0;
//
//                                        }
//                                        else if (whichPart == 2)
//                                        {
//                                            std::wistringstream wstringToInt(vertPart);
//
//                                            wstringToInt >> vertNormIndexTemp;
//                                            vertNormIndexTemp -= 1;
//                                        }
//
//                                        vertPart = L"";
//                                        whichPart++;
//                                    }
//                                }
//
//                                //Check for duplicate vertices
//                                bool vertAlreadyExists = false;
//                                if (totalVerts >= 3)    //Make sure we at least have one triangle to check
//                                {
//                                    for (int iCheck = 0; iCheck < totalVerts; ++iCheck)
//                                    {
//                                        if (vertPosIndexTemp == vertPosIndex[iCheck] && !vertAlreadyExists)
//                                        {
//                                            if (vertTCIndexTemp == vertTCIndex[iCheck])
//                                            {
//                                                indices.push_back(iCheck);            //Set index for this vertex
//                                                vertAlreadyExists = true;        //If we've made it here, the vertex already exists
//                                            }
//                                        }
//                                    }
//                                }
//
//                                if (!vertAlreadyExists)
//                                {
//                                    vertPosIndex.push_back(vertPosIndexTemp);
//                                    vertTCIndex.push_back(vertTCIndexTemp);
//                                    vertNormIndex.push_back(vertNormIndexTemp);
//                                    totalVerts++;                    //New vertex created, add to total verts
//                                    indices.push_back(totalVerts - 1);        //Set index for this vertex
//                                }
//
//                                //Set the second vertex for the next triangle to the last vertex we got        
//                                lastVIndex = indices[vIndex];    //The last vertex index of this TRIANGLE
//
//                                meshTriangles++;    //New triangle defined
//                                vIndex++;
//                            }
//                        }
//                    }
//                    break;
//
//                case 'm':    //mtllib - material library filename
//                    checkChar = fileIn.get();
//                    if (checkChar == 't')
//                    {
//                        checkChar = fileIn.get();
//                        if (checkChar == 'l')
//                        {
//                            checkChar = fileIn.get();
//                            if (checkChar == 'l')
//                            {
//                                checkChar = fileIn.get();
//                                if (checkChar == 'i')
//                                {
//                                    checkChar = fileIn.get();
//                                    if (checkChar == 'b')
//                                    {
//                                        checkChar = fileIn.get();
//                                        if (checkChar == ' ')
//                                        {
//                                            //Store the material libraries file name
//                                            fileIn >> meshMatLib;
//                                        }
//                                    }
//                                }
//                            }
//                        }
//                    }
//
//                    break;
//
//                case 'u':    //usemtl - which material to use
//                    checkChar = fileIn.get();
//                    if (checkChar == 's')
//                    {
//                        checkChar = fileIn.get();
//                        if (checkChar == 'e')
//                        {
//                            checkChar = fileIn.get();
//                            if (checkChar == 'm')
//                            {
//                                checkChar = fileIn.get();
//                                if (checkChar == 't')
//                                {
//                                    checkChar = fileIn.get();
//                                    if (checkChar == 'l')
//                                    {
//                                        checkChar = fileIn.get();
//                                        if (checkChar == ' ')
//                                        {
//                                            meshMaterialsTemp = L"";    //Make sure this is cleared
//
//                                            fileIn >> meshMaterialsTemp; //Get next type (string)
//
//                                            meshMaterials.push_back(meshMaterialsTemp);
//                                        }
//                                    }
//                                }
//                            }
//                        }
//                    }
//                    break;
//
//                default:
//                    break;
//            }
//        }
//    }
//    else    //If we could not open the file
//    {
//       // SwapChain->SetFullscreenState(false, NULL);    //Make sure we are out of fullscreen
//
//        //create message
//        std::wstring message = L"Could not open: ";
//     //   message += filename;
//        assert(false && "Could not open this .obj file.");
//     //   MessageBox(0, message.c_str(),    //display message
//     //              L"Error", MB_OK);
//
//        return false;
//    }
//
//    subsetIndexStart.push_back(vIndex); //There won't be another index start after our last subset, so set it here
//
//    //sometimes "g" is defined at the very top of the file, then again before the first group of faces.
//    //This makes sure the first subset does not conatain "0" indices.
//    if (subsetIndexStart[1] == 0)
//    {
//        subsetIndexStart.erase(subsetIndexStart.begin() + 1);
//        meshSubsets--;
//    }
//
//    //Make sure we have a default for the tex coord and normal
//    //if one or both are not specified
//    if (!hasNorm)
//        vertNorm.push_back(XMFLOAT3(0.0f, 0.0f, 0.0f));
//    if (!hasTexCoord)
//        vertTexCoord.push_back(XMFLOAT2(0.0f, 0.0f));
//
//    //Close the obj file, and open the mtl file
//    fileIn.close();
//    fileIn.open(meshMatLib.c_str());
//
//    std::wstring lastStringRead;
//    int matCount = material.size();    //total materials
//
//    //kdset - If our diffuse color was not set, we can use the ambient color (which is usually the same)
//    //If the diffuse color WAS set, then we don't need to set our diffuse color to ambient
//    bool kdset = false;
//
//    if (fileIn)
//    {
//        while (fileIn)
//        {
//            checkChar = fileIn.get();    //Get next char
//
//            switch (checkChar)
//            {
//                //Check for comment
//                case '#':
//                    checkChar = fileIn.get();
//                    while (checkChar != '\n')
//                        checkChar = fileIn.get();
//                    break;
//
//                    //Set diffuse color
//                case 'K':
//                    checkChar = fileIn.get();
//                    if (checkChar == 'd')    //Diffuse Color
//                    {
//                        checkChar = fileIn.get();    //remove space
//
//                        fileIn >> material[matCount - 1].difColor.x;
//                        fileIn >> material[matCount - 1].difColor.y;
//                        fileIn >> material[matCount - 1].difColor.z;
//
//                        kdset = true;
//                    }
//
//                    //Ambient Color (We'll store it in diffuse if there isn't a diffuse already)
//                    if (checkChar == 'a')
//                    {
//                        checkChar = fileIn.get();    //remove space
//                        if (!kdset)
//                        {
//                            fileIn >> material[matCount - 1].difColor.x;
//                            fileIn >> material[matCount - 1].difColor.y;
//                            fileIn >> material[matCount - 1].difColor.z;
//                        }
//                    }
//                    break;
//
//                    //Check for transparency
//                case 'T':
//                    checkChar = fileIn.get();
//                    if (checkChar == 'r')
//                    {
//                        checkChar = fileIn.get();    //remove space
//                        float Transparency;
//                        fileIn >> Transparency;
//
//                        material[matCount - 1].difColor.w = Transparency;
//
//                        if (Transparency > 0.0f)
//                            material[matCount - 1].transparent = true;
//                    }
//                    break;
//
//                    //Some obj files specify d for transparency
//                case 'd':
//                    checkChar = fileIn.get();
//                    if (checkChar == ' ')
//                    {
//                        float Transparency;
//                        fileIn >> Transparency;
//
//                        //'d' - 0 being most transparent, and 1 being opaque, opposite of Tr
//                        Transparency = 1.0f - Transparency;
//
//                        material[matCount - 1].difColor.w = Transparency;
//
//                        if (Transparency > 0.0f)
//                            material[matCount - 1].transparent = true;
//                    }
//                    break;
//
//                    //Get the diffuse map (texture)
//                case 'm':
//                    checkChar = fileIn.get();
//                    if (checkChar == 'a')
//                    {
//                        checkChar = fileIn.get();
//                        if (checkChar == 'p')
//                        {
//                            checkChar = fileIn.get();
//                            if (checkChar == '_')
//                            {
//                                //map_Kd - Diffuse map
//                                checkChar = fileIn.get();
//                                if (checkChar == 'K')
//                                {
//                                    checkChar = fileIn.get();
//                                    if (checkChar == 'd')
//                                    {
//                                        std::wstring fileNamePath;
//
//                                        fileIn.get();    //Remove whitespace between map_Kd and file
//
//                                        //Get the file path - We read the pathname char by char since
//                                        //pathnames can sometimes contain spaces, so we will read until
//                                        //we find the file extension
//                                        bool texFilePathEnd = false;
//                                        while (!texFilePathEnd)
//                                        {
//                                            checkChar = fileIn.get();
//
//                                            fileNamePath += checkChar;
//
//                                            if (checkChar == '.')
//                                            {
//                                                for (int i = 0; i < 3; ++i)
//                                                    fileNamePath += fileIn.get();
//
//                                                texFilePathEnd = true;
//                                            }
//                                        }
//
//                                        //check if this texture has already been loaded
//                                        bool alreadyLoaded = false;
//                                        for (int i = 0; i < textureNameArray.size(); ++i)
//                                        {
//                                            if (fileNamePath == textureNameArray[i])
//                                            {
//                                                alreadyLoaded = true;
//                                                material[matCount - 1].texArrayIndex = i;
//                                                material[matCount - 1].hasTexture = true;
//                                            }
//                                        }
//
//                                        //if the texture is not already loaded, load it now
//                                        if (!alreadyLoaded)
//                                        {
//                                            ID3D11ShaderResourceView* tempMeshSRV;
//                                            hr = D3DX11CreateShaderResourceViewFromFile(d3d11Device, fileNamePath.c_str(),
//                                                                                        NULL, NULL, &tempMeshSRV, NULL);
//                                            if (SUCCEEDED(hr))
//                                            {
//                                                textureNameArray.push_back(fileNamePath.c_str());
//                                                material[matCount - 1].texArrayIndex = meshSRV.size();
//                                                meshSRV.push_back(tempMeshSRV);
//                                                material[matCount - 1].hasTexture = true;
//                                            }
//                                        }
//                                    }
//                                }
//                                //map_d - alpha map
//                                else if (checkChar == 'd')
//                                {
//                                    //Alpha maps are usually the same as the diffuse map
//                                    //So we will assume that for now by only enabling
//                                    //transparency for this material, as we will already
//                                    //be using the alpha channel in the diffuse map
//                                    material[matCount - 1].transparent = true;
//                                }
//                            }
//                        }
//                    }
//                    break;
//
//                case 'n':    //newmtl - Declare new material
//                    checkChar = fileIn.get();
//                    if (checkChar == 'e')
//                    {
//                        checkChar = fileIn.get();
//                        if (checkChar == 'w')
//                        {
//                            checkChar = fileIn.get();
//                            if (checkChar == 'm')
//                            {
//                                checkChar = fileIn.get();
//                                if (checkChar == 't')
//                                {
//                                    checkChar = fileIn.get();
//                                    if (checkChar == 'l')
//                                    {
//                                        checkChar = fileIn.get();
//                                        if (checkChar == ' ')
//                                        {
//                                            //New material, set its defaults
//                                            SurfaceMaterial tempMat;
//                                            material.push_back(tempMat);
//                                            fileIn >> material[matCount].matName;
//                                            material[matCount].transparent = false;
//                                            material[matCount].hasTexture = false;
//                                            material[matCount].texArrayIndex = 0;
//                                            matCount++;
//                                            kdset = false;
//                                        }
//                                    }
//                                }
//                            }
//                        }
//                    }
//                    break;
//
//                default:
//                    break;
//            }
//        }
//    }
//    else
//    {
//        SwapChain->SetFullscreenState(false, NULL);    //Make sure we are out of fullscreen
//
//        std::wstring message = L"Could not open: ";
//        message += meshMatLib;
//
//        MessageBox(0, message.c_str(),
//                   L"Error", MB_OK);
//
//        return false;
//    }
//
//    //Set the subsets material to the index value
//    //of the its material in our material array
//    for (int i = 0; i < meshSubsets; ++i)
//    {
//        bool hasMat = false;
//        for (int j = 0; j < material.size(); ++j)
//        {
//            if (meshMaterials[i] == material[j].matName)
//            {
//                subsetMaterialArray.push_back(j);
//                hasMat = true;
//            }
//        }
//        if (!hasMat)
//            subsetMaterialArray.push_back(0); //Use first material in array
//    }
//
//    std::vector<Vertex> vertices;
//    Vertex tempVert;
//
//    //Create our vertices using the information we got 
//    //from the file and store them in a vector
//    for (int j = 0; j < totalVerts; ++j)
//    {
//        tempVert.Position = vertPos[vertPosIndex[j]];
//        tempVert.Normal = vertNorm[vertNormIndex[j]];
//        tempVert.UV = vertTexCoord[vertTCIndex[j]];
//
//        vertices.push_back(tempVert);
//    }
//
//    //////////////////////Compute Normals///////////////////////////
//    //If computeNormals was set to true then we will create our own
//    //normals, if it was set to false we will use the obj files normals
//    if (computeNormals)
//    {
//        std::vector<XMFLOAT3> tempNormal;
//
//        //normalized and unnormalized normals
//        XMFLOAT3 unnormalized = XMFLOAT3(0.0f, 0.0f, 0.0f);
//
//        //Used to get vectors (sides) from the position of the verts
//        float vecX, vecY, vecZ;
//
//        //Two edges of our triangle
//        XMVECTOR edge1 = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
//        XMVECTOR edge2 = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
//
//        //Compute face normals
//        for (int i = 0; i < meshTriangles; ++i)
//        {
//            //Get the vector describing one edge of our triangle (edge 0,2)
//            vecX = vertices[indices[(i * 3)]].Position.x - vertices[indices[(i * 3) + 2]].Position.x;
//            vecY = vertices[indices[(i * 3)]].Position.y - vertices[indices[(i * 3) + 2]].Position.y;
//            vecZ = vertices[indices[(i * 3)]].Position.z - vertices[indices[(i * 3) + 2]].Position.z;
//            edge1 = XMVectorSet(vecX, vecY, vecZ, 0.0f);    //Create our first edge
//
//            //Get the vector describing another edge of our triangle (edge 2,1)
//            vecX = vertices[indices[(i * 3) + 2]].Position.x - vertices[indices[(i * 3) + 1]].Position.x;
//            vecY = vertices[indices[(i * 3) + 2]].Position.y - vertices[indices[(i * 3) + 1]].Position.y;
//            vecZ = vertices[indices[(i * 3) + 2]].Position.z - vertices[indices[(i * 3) + 1]].Position.z;
//            edge2 = XMVectorSet(vecX, vecY, vecZ, 0.0f);    //Create our second edge
//
//            //Cross multiply the two edge vectors to get the un-normalized face normal
//            XMStoreFloat3(&unnormalized, XMVector3Cross(edge1, edge2));
//            tempNormal.push_back(unnormalized);            //Save unormalized normal (for normal averaging)
//        }
//
//        //Compute vertex normals (normal Averaging)
//        XMVECTOR normalSum = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
//        int facesUsing = 0;
//        float tX;
//        float tY;
//        float tZ;
//
//        //Go through each vertex
//        for (int i = 0; i < totalVerts; ++i)
//        {
//            //Check which triangles use this vertex
//            for (int j = 0; j < meshTriangles; ++j)
//            {
//                if (indices[j * 3] == i ||
//                    indices[(j * 3) + 1] == i ||
//                    indices[(j * 3) + 2] == i)
//                {
//                    tX = XMVectorGetX(normalSum) + tempNormal[j].x;
//                    tY = XMVectorGetY(normalSum) + tempNormal[j].y;
//                    tZ = XMVectorGetZ(normalSum) + tempNormal[j].z;
//
//                    normalSum = XMVectorSet(tX, tY, tZ, 0.0f);    //If a face is using the vertex, add the unormalized face normal to the normalSum
//                    facesUsing++;
//                }
//            }
//
//            //Get the actual normal by dividing the normalSum by the number of faces sharing the vertex
//            normalSum = normalSum / facesUsing;
//
//            //Normalize the normalSum vector
//            normalSum = XMVector3Normalize(normalSum);
//
//            //Store the normal in our current vertex
//            vertices[i].Normal.x = XMVectorGetX(normalSum);
//            vertices[i].Normal.y = XMVectorGetY(normalSum);
//            vertices[i].Normal.z = XMVectorGetZ(normalSum);
//
//            //Clear normalSum and facesUsing for next vertex
//            normalSum = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
//            facesUsing = 0;
//
//        }
//    }
//
//    ////Create index buffer
//    //D3D11_BUFFER_DESC indexBufferDesc;
//    //ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));
//
//    //indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
//    //indexBufferDesc.ByteWidth = sizeof(DWORD) * meshTriangles * 3;
//    //indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
//    //indexBufferDesc.CPUAccessFlags = 0;
//    //indexBufferDesc.MiscFlags = 0;
//
//    D3D11_SUBRESOURCE_DATA iinitData;
//
//  //  iinitData.pSysMem = &indices[0];
//  //  Device->CreateBuffer(&indexBufferDesc, &iinitData, indexBuff);
//
//    //Create Vertex Buffer
//  //  D3D11_BUFFER_DESC vertexBufferDesc;
//  //  ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));
//
// //   vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
//  //  vertexBufferDesc.ByteWidth = sizeof(Vertex) * totalVerts;
// //   vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
// //   vertexBufferDesc.CPUAccessFlags = 0;
// //   vertexBufferDesc.MiscFlags = 0;
//
//    //D3D11_SUBRESOURCE_DATA vertexBufferData;
//
//    //ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
//    //vertexBufferData.pSysMem = &vertices[0];
//    //hr = d3d11Device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, vertBuff);
//
//    return true;
//}
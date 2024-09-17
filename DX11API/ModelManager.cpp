
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "DX11Utility.h"
#include "ModelManager.h"
using namespace DirectX;

using namespace Assimp;

Importer mImporter;

void Model::CreateFromFile(Model& model, ID3D11Device* device, std::string_view filename)
{
    using namespace Assimp;

    model.materials.clear();
    model.meshdatas.clear();
    model.boundingbox = BoundingBox();

    Importer importer;
    // 去掉里面的点、线图元
    importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE | aiPrimitiveType_POINT);
    auto pAssimpScene = importer.ReadFile(filename.data(),
        aiProcess_ConvertToLeftHanded |     // 转为左手系
        aiProcess_GenBoundingBoxes |        // 获取碰撞盒
        aiProcess_Triangulate |             // 将多边形拆分
        aiProcess_ImproveCacheLocality |    // 改善缓存局部性
        aiProcess_SortByPType);             // 按图元顶点数排序用于移除非三角形图元

    if (pAssimpScene && !(pAssimpScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) && pAssimpScene->HasMeshes())
    {
        model.meshdatas.resize(pAssimpScene->mNumMeshes);
        model.materials.resize(pAssimpScene->mNumMaterials);
        for (uint32_t i = 0; i < pAssimpScene->mNumMeshes; ++i)
        {
            auto& mesh = model.meshdatas[i];

            auto pAiMesh = pAssimpScene->mMeshes[i];
            uint32_t numVertices = pAiMesh->mNumVertices;

            CD3D11_BUFFER_DESC bufferDesc(0, D3D11_BIND_VERTEX_BUFFER);
            D3D11_SUBRESOURCE_DATA initData{ nullptr, 0, 0 };
            // 位置
            if (pAiMesh->mNumVertices > 0)
            {
                initData.pSysMem = pAiMesh->mVertices;
                bufferDesc.ByteWidth = numVertices * sizeof(XMFLOAT3);
                device->CreateBuffer(&bufferDesc, &initData, mesh.m_pVertices.GetAddressOf());

                BoundingBox::CreateFromPoints(mesh.m_BoundingBox, numVertices,
                    (const XMFLOAT3*)pAiMesh->mVertices, sizeof(XMFLOAT3));
                if (i == 0)
                    model.boundingbox = mesh.m_BoundingBox;
                else
                    model.boundingbox.CreateMerged(model.boundingbox, model.boundingbox, mesh.m_BoundingBox);
            }

            // 法线
            if (pAiMesh->HasNormals())
            {
                initData.pSysMem = pAiMesh->mNormals;
                bufferDesc.ByteWidth = numVertices * sizeof(XMFLOAT3);
                device->CreateBuffer(&bufferDesc, &initData, mesh.m_pNormals.GetAddressOf());
            }

            // 切线和副切线
            if (pAiMesh->HasTangentsAndBitangents())
            {
                std::vector<XMFLOAT4> tangents(numVertices, XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
                for (uint32_t i = 0; i < pAiMesh->mNumVertices; ++i)
                {
                    memcpy_s(&tangents[i], sizeof(XMFLOAT3),
                        pAiMesh->mTangents + i, sizeof(XMFLOAT3));
                }

                initData.pSysMem = tangents.data();
                bufferDesc.ByteWidth = pAiMesh->mNumVertices * sizeof(XMFLOAT4);
                device->CreateBuffer(&bufferDesc, &initData, mesh.m_pTangents.GetAddressOf());

                for (uint32_t i = 0; i < pAiMesh->mNumVertices; ++i)
                {
                    memcpy_s(&tangents[i], sizeof(XMFLOAT3),
                        pAiMesh->mBitangents + i, sizeof(XMFLOAT3));
                }
                device->CreateBuffer(&bufferDesc, &initData, mesh.m_pBitangents.GetAddressOf());
            }

            // 纹理坐标
            uint32_t numUVs = 8;
            while (numUVs && !pAiMesh->HasTextureCoords(numUVs - 1))
                numUVs--;

            if (numUVs > 0)
            {
                mesh.m_pTexcoordArrays.resize(numUVs);
                for (uint32_t i = 0; i < numUVs; ++i)
                {
                    std::vector<XMFLOAT2> uvs(numVertices);
                    for (uint32_t j = 0; j < numVertices; ++j)
                    {
                        memcpy_s(&uvs[j], sizeof(XMFLOAT2),
                            pAiMesh->mTextureCoords[i] + j, sizeof(XMFLOAT2));
                    }
                    initData.pSysMem = uvs.data();
                    bufferDesc.ByteWidth = numVertices * sizeof(XMFLOAT2);
                    device->CreateBuffer(&bufferDesc, &initData, mesh.m_pTexcoordArrays[i].GetAddressOf());
                }
            }

            // 索引
            uint32_t numFaces = pAiMesh->mNumFaces;
            uint32_t numIndices = numFaces * 3;
            if (numFaces > 0)
            {
                mesh.m_IndexCount = numIndices;
                if (numIndices < 65535)
                {
                    std::vector<uint16_t> indices(numIndices);
                    for (size_t i = 0; i < numFaces; ++i)
                    {
                        indices[i * 3] = static_cast<uint16_t>(pAiMesh->mFaces[i].mIndices[0]);
                        indices[i * 3 + 1] = static_cast<uint16_t>(pAiMesh->mFaces[i].mIndices[1]);
                        indices[i * 3 + 2] = static_cast<uint16_t>(pAiMesh->mFaces[i].mIndices[2]);
                    }
                    bufferDesc = CD3D11_BUFFER_DESC(numIndices * sizeof(uint16_t), D3D11_BIND_INDEX_BUFFER);
                    initData.pSysMem = indices.data();
                    device->CreateBuffer(&bufferDesc, &initData, mesh.m_pIndices.GetAddressOf());
                }
                else
                {
                    std::vector<uint32_t> indices(numIndices);
                    for (size_t i = 0; i < numFaces; ++i)
                    {
                        memcpy_s(indices.data() + i * 3, sizeof(uint32_t) * 3,
                            pAiMesh->mFaces[i].mIndices, sizeof(uint32_t) * 3);
                    }
                    bufferDesc = CD3D11_BUFFER_DESC(numIndices * sizeof(uint32_t), D3D11_BIND_INDEX_BUFFER);
                    initData.pSysMem = indices.data();
                    device->CreateBuffer(&bufferDesc, &initData, mesh.m_pIndices.GetAddressOf());
                }
            }

            // 材质索引
            mesh.m_MaterialIndex = pAiMesh->mMaterialIndex;
        }


        for (uint32_t i = 0; i < pAssimpScene->mNumMaterials; ++i)
        {
            auto& material = model.materials[i];

            auto pAiMaterial = pAssimpScene->mMaterials[i];
            XMFLOAT4 vec{};
            float value{};
            uint32_t boolean{};
            uint32_t num = 3;

            if (aiReturn_SUCCESS == pAiMaterial->Get(AI_MATKEY_COLOR_AMBIENT, (float*)&vec, &num))
                material.Set("$AmbientColor", vec);
            if (aiReturn_SUCCESS == pAiMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, (float*)&vec, &num))
                material.Set("$DiffuseColor", vec);
            if (aiReturn_SUCCESS == pAiMaterial->Get(AI_MATKEY_COLOR_SPECULAR, (float*)&vec, &num))
                material.Set("$SpecularColor", vec);
            if (aiReturn_SUCCESS == pAiMaterial->Get(AI_MATKEY_SPECULAR_FACTOR, value))
                material.Set("$SpecularFactor", value);
            if (aiReturn_SUCCESS == pAiMaterial->Get(AI_MATKEY_COLOR_EMISSIVE, (float*)&vec, &num))
                material.Set("$EmissiveColor", vec);
            if (aiReturn_SUCCESS == pAiMaterial->Get(AI_MATKEY_OPACITY, value))
                material.Set("$Opacity", value);
            if (aiReturn_SUCCESS == pAiMaterial->Get(AI_MATKEY_COLOR_TRANSPARENT, (float*)&vec, &num))
                material.Set("$TransparentColor", vec);
            if (aiReturn_SUCCESS == pAiMaterial->Get(AI_MATKEY_COLOR_REFLECTIVE, (float*)&vec, &num))
                material.Set("$ReflectiveColor", vec);

            aiString aiPath;
            std::string texName;

            auto TryCreateTexture = [&](aiTextureType type, std::string_view propertyName, bool genMips = false, bool forceSRGB = false) {
                if (!pAiMaterial->GetTextureCount(type))
                    return;

                pAiMaterial->GetTexture(type, 0, &aiPath);

                // 纹理已经预先加载进来
                if (aiPath.data[0] == '*')
                {
                    texName = filename;
                    texName += aiPath.C_Str();
                    char* pEndStr = nullptr;
                    aiTexture* pTex = pAssimpScene->mTextures[strtol(aiPath.data + 1, &pEndStr, 10)];
                    //TextureManager::Get().CreateFromMemory(texName, pTex->pcData, pTex->mHeight ? pTex->mWidth * pTex->mHeight : pTex->mWidth, genMips, forceSRGB);
                    material.Set(propertyName, std::string(texName));
                }
                // 纹理通过文件名索引
                else
                {
              
                }
                };

            TryCreateTexture(aiTextureType_DIFFUSE, "$Diffuse", true, true);
            TryCreateTexture(aiTextureType_NORMALS, "$Normal");
            TryCreateTexture(aiTextureType_BASE_COLOR, "$Albedo", true, true);
            TryCreateTexture(aiTextureType_NORMAL_CAMERA, "$NormalCamera");
            TryCreateTexture(aiTextureType_METALNESS, "$Metalness");
            TryCreateTexture(aiTextureType_DIFFUSE_ROUGHNESS, "$Roughness");
            TryCreateTexture(aiTextureType_AMBIENT_OCCLUSION, "$AmbientOcclusion");
        }
    }
    else
    {
        std::string warning = "[Warning]: ModelManager::CreateFromFile, failed to load \"";
        warning += filename;
        warning += "\"\n";

      
    }
}

void Model::SetDebugObjectName(std::string_view name)
{
#if (defined(DEBUG) || defined(_DEBUG)) && (GRAPHICS_DEBUGGER_OBJECT_NAME)
    std::string baseStr = name.data();
    size_t sz = meshdatas.size();
    std::string str;
    str.reserve(100);
    for (size_t i = 0; i < sz; ++i)
    {
        baseStr = name.data();
        baseStr += "[" + std::to_string(i) + "].";
 
    }
#else
    UNREFERENCED_PARAMETER(name);
#endif
}

namespace
{
    // ModelManager单例
    ModelManager* s_pInstance = nullptr;
}


ModelManager::ModelManager()
{
    if (s_pInstance)
        throw std::exception("ModelManager is a singleton!");
    s_pInstance = this;
}

ModelManager::~ModelManager()
{
}

ModelManager& ModelManager::Get()
{
    if (!s_pInstance)
        throw std::exception("ModelManager needs an instance!");
    return *s_pInstance;
}

void ModelManager::Init(ID3D11Device* device)
{
    m_pDevice = device;
    m_pDevice->GetImmediateContext(m_pDeviceContext.ReleaseAndGetAddressOf());
}

Model* ModelManager::CreateFromFile(std::string_view filename)
{
    return CreateFromFile(filename, filename);
}

Model* ModelManager::CreateFromFile(std::string_view name, std::string_view filename)
{
    XID modelID = StringToID(name);
    auto& model = m_Models[modelID];
    Model::CreateFromFile(model, m_pDevice.Get(), filename);
    return &model;
}


const Model* ModelManager::GetModel(std::string_view name) const
{
    XID nameID = StringToID(name);
    if (auto it = m_Models.find(nameID); it != m_Models.end())
        return &it->second;
    return nullptr;
}

Model* ModelManager::GetModel(std::string_view name)
{
    XID nameID = StringToID(name);
    if (m_Models.count(nameID))
        return &m_Models[nameID];
    return nullptr;
}
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <assert.h>
#include <map>
#include <vector>
#include <GL/glew.h>

#include "util.h"
#include "math_3d.h"
#include "texture.h"

struct Vertex
{
    Vector3f pos;
    Vector2f tex;
    Vector3f normal;

    Vertex() {}

    Vertex(const Vector3f& Pos, const Vector2f& Tex, const Vector3f& Normal)
    {
        pos = Pos;
        tex = Tex;
        normal = Normal;
    }
};

//an interface between assimp and opengl
class Mesh
{
public:
    Mesh() {};
    ~Mesh() {
        Clear();
    };
    bool LoadMesh(const std::string& Filename)
    {
        Clear();

        bool Ret = false;

        Assimp::Importer Importer;

        // stores the structure of mesh
        const aiScene* pScene = Importer.ReadFile(Filename.c_str(),
            aiProcess_Triangulate | aiProcess_GenSmoothNormals |
            aiProcess_FlipUVs);

        if (pScene) {
            Ret = InitFromScene(pScene, Filename);
        }
        else {
            printf("Error parsing '%s': '%s'\n", Filename.c_str(), Importer.GetErrorString());
        }

        return Ret;
    }

    void Render()
    {
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);

        for (unsigned int i = 0; i < Entries.size(); i++) {
            glBindBuffer(GL_ARRAY_BUFFER, Entries[i].VB);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)12);
            glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)20);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Entries[i].IB);

            const unsigned int MaterialIndex = Entries[i].MaterialIndex;

            if (MaterialIndex < Textures.size() && Textures[MaterialIndex]) {
                Textures[MaterialIndex]->Bind(GL_TEXTURE0);
            }

            glDrawElements(GL_TRIANGLES, Entries[i].NumIndices, GL_UNSIGNED_INT, 0);
        }

        glDisableVertexAttribArray(2);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(0);
    }

private:

    // initializations
    bool InitFromScene(const aiScene* pScene, const std::string& Filename)
    {
        Entries.resize(pScene->mNumMeshes);
        Textures.resize(pScene->mNumMaterials);

        for (unsigned int i = 0; i < Entries.size(); i++) {
            const aiMesh* paiMesh = pScene->mMeshes[i];
            InitMesh(i, paiMesh);
        }

        return InitMaterials(pScene, Filename);
    }

    void InitMesh(unsigned int Index, const aiMesh* paiMesh)
    {
        Entries[Index].MaterialIndex = paiMesh->mMaterialIndex;

        std::vector<Vertex> Vertices;
        std::vector<unsigned int> Indices;

        const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);
        for (unsigned int i = 0; i < paiMesh->mNumVertices; i++) {
            const aiVector3D* pPos = &(paiMesh->mVertices[i]);
            const aiVector3D* pNormal = &(paiMesh->mNormals[i]);
            const aiVector3D* pTexCoord = paiMesh->HasTextureCoords(0) ?
                &(paiMesh->mTextureCoords[0][i]) : &Zero3D;

            Vertex v(Vector3f(pPos->x, pPos->y, pPos->z),
                Vector2f(pTexCoord->x, pTexCoord->y),
                Vector3f(pNormal->x, pNormal->y, pNormal->z));
            Vertices.push_back(v);
        }

        for (unsigned int i = 0; i < paiMesh->mNumFaces; i++) {
            const aiFace& Face = paiMesh->mFaces[i];
            assert(Face.mNumIndices == 3);
            Indices.push_back(Face.mIndices[0]);
            Indices.push_back(Face.mIndices[1]);
            Indices.push_back(Face.mIndices[2]);
        }

        Entries[Index].Init(Vertices, Indices);
    }

    bool InitMaterials(const aiScene* pScene, const std::string& Filename)
    {
        std::string::size_type SlashIndex = Filename.find_last_of("/");
        std::string Dir;

        if (SlashIndex == std::string::npos) {
            Dir = ".";
        }
        else if (SlashIndex == 0) {
            Dir = "/";
        }
        else {
            Dir = Filename.substr(0, SlashIndex);
        }

        bool Ret = true;

        // Èíèöèàëèçèðóåì ìàòåðèàë
        for (unsigned int i = 0; i < pScene->mNumMaterials; i++) {
            const aiMaterial* pMaterial = pScene->mMaterials[i];

            Textures[i] = NULL;
            if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
                aiString Path;

                if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &Path,
                    NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
                    std::string FullPath = Dir + "/" + Path.data;
                    Textures[i] = new Texture(GL_TEXTURE_2D, FullPath.c_str());

                    if (!Textures[i]->Load()) {
                        printf("Error loading texture '%s'\n", FullPath.c_str());
                        delete Textures[i];
                        Textures[i] = NULL;
                        Ret = false;
                    }
                }
            }

            // Çàãðóæàåì áåëóþ òåêñòóðó åñëè ìîäåëü íå èìååò ñîáñòâåííîé
            // Â ýòîì óðîêå äàííûé ôóêíöèîíàë - ðóäèìåíò
            /*if (!Textures[i]){
                Textures[i] = new Texture(GL_TEXTURE_2D, "./white.png");
                Ret = Textures[i]->Load();
            }*/
        }

        return Ret;
    }
    void Clear()
    {
        for (unsigned int i = 0; i < Textures.size(); i++)
            SAFE_DELETE(Textures[i]);
    }

#define INVALID_MATERIAL 0xFFFFFFFF

    struct MeshEntry
    {
        MeshEntry()
        {
            VB = INVALID_OGL_VALUE;
            IB = INVALID_OGL_VALUE;
            NumIndices = 0;
            MaterialIndex = INVALID_MATERIAL;
        }

        ~MeshEntry()
        {
            if (VB != INVALID_OGL_VALUE) glDeleteBuffers(1, &VB);
            if (IB != INVALID_OGL_VALUE) glDeleteBuffers(1, &IB);
        }

        bool Init(const std::vector<Vertex>& Vertices, const std::vector<unsigned int>& Indices)
        {
            NumIndices = Indices.size();

            glGenBuffers(1, &VB);
            glBindBuffer(GL_ARRAY_BUFFER, VB);
            glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * Vertices.size(),
                &Vertices[0], GL_STATIC_DRAW);

            glGenBuffers(1, &IB);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IB);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * NumIndices,
                &Indices[0], GL_STATIC_DRAW);

            return true;
        }

        GLuint VB;
        GLuint IB;

        unsigned int NumIndices;
        unsigned int MaterialIndex;
    };

    std::vector<MeshEntry> Entries;
    std::vector<Texture*> Textures;
};
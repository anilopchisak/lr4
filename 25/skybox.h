#ifndef SKYBOX_H
#define	SKYBOX_H

#include "camera.h"
#include "skybox_technique.h"
#include "cubemap_texture.h"
#include "mesh.h"
#include "pipeline.h"
#include "util.h"

class SkyBox
{
public:

    SkyBox(const Camera* m_pCamera, const PersProjInfo& p)
    {
        pCamera = m_pCamera;
        persProjInfo = p;

        pSkyboxTechnique = nullptr;
        pCubemapTex = nullptr;
        pMesh = nullptr;
    }

    ~SkyBox()
    {
        SAFE_DELETE(pSkyboxTechnique);
        SAFE_DELETE(pCubemapTex);
        SAFE_DELETE(pMesh);
    }

    bool Init(const string& Directory,
        const string& PosXFilename,
        const string& NegXFilename,
        const string& PosYFilename,
        const string& NegYFilename,
        const string& PosZFilename,
        const string& NegZFilename)
    {
        pSkyboxTechnique = new SkyboxTechnique();

        if (!pSkyboxTechnique->Init()) 
        {
            printf("Error initializing the skybox technique\n");
            return false;
        }

        pSkyboxTechnique->Enable();
        pSkyboxTechnique->SetTextureUnit(0);

        pCubemapTex = new CubemapTexture(Directory,
            PosXFilename,
            NegXFilename,
            PosYFilename,
            NegYFilename,
            PosZFilename,
            NegZFilename);

        if (!pCubemapTex->Load()) 
        {
            return false;
        }

        pMesh = new Mesh();

        return pMesh->LoadMesh("C:/Content/sphere.obj");
    }


    void Render()
    {
        pSkyboxTechnique->Enable();

        GLint OldCullFaceMode;
        glGetIntegerv(GL_CULL_FACE_MODE, &OldCullFaceMode);
        GLint OldDepthFuncMode;
        glGetIntegerv(GL_DEPTH_FUNC, &OldDepthFuncMode);

        glCullFace(GL_FRONT);
        glDepthFunc(GL_LEQUAL);

        Pipeline p;
        p.Scale(20.0f, 20.0f, 20.0f);
        p.Rotate(0.0f, 0.0f, 0.0f);
        p.WorldPos(pCamera->GetPos().x, pCamera->GetPos().y, pCamera->GetPos().z);
        p.SetCamera(pCamera->GetPos(), pCamera->GetTarget(), pCamera->GetUp());
        p.SetPerspectiveProj(persProjInfo);
        pSkyboxTechnique->SetWVP(p.GetWVPTrans());
        pCubemapTex->Bind(GL_TEXTURE0);
        pMesh->Render();

        glCullFace(OldCullFaceMode);
        glDepthFunc(OldDepthFuncMode);
    }
    
private:    
    SkyboxTechnique* pSkyboxTechnique;
    const Camera* pCamera;
    CubemapTexture* pCubemapTex;
    Mesh* pMesh;
    PersProjInfo persProjInfo;
};

#endif	/* SKYBOX_H */


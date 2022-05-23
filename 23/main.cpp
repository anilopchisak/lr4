#include <math.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "pipeline.h"
#include "camera.h"
#include "texture.h"
#include "lighting_technique.h"
#include "glut_backend.h"
#include "util.h"
#include "mesh.h"
#include "shadow_map_fbo.h"
#include "shadow_map_technique.h"

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 1024;

class Main : public ICallbacks
{
private:

    LightingTechnique* pLightingEffect;
    ShadowMapTechnique* pShadowMapTech;
    Camera* pGameCamera;
    float scale;
    SpotLight spotLight;
    Mesh* pMesh;
    Mesh* pQuad;
    ShadowMapFBO shadowMapFBO;

public:

    Main()
    {
        pLightingEffect = nullptr;
        pShadowMapTech = nullptr;
        pGameCamera = nullptr;
        pMesh = nullptr;
        pQuad = nullptr;
        scale = 0.0f;

        spotLight.AmbientIntensity = 0.0f;
        spotLight.DiffuseIntensity = 0.9f;
        spotLight.Color = Vector3f(1.0f, 1.0f, 1.0f);
        spotLight.Attenuation.Linear = 0.01f;
        spotLight.Position = Vector3f(-20.0, 20.0, 5.0f);
        spotLight.Direction = Vector3f(1.0f, -1.0f, 0.0f);
        spotLight.Cutoff = 20.0f;
    }

    virtual ~Main()
    {
        SAFE_DELETE(pLightingEffect);
        SAFE_DELETE(pShadowMapTech);
        SAFE_DELETE(pGameCamera);
        SAFE_DELETE(pMesh);
        SAFE_DELETE(pQuad);
    }

    bool Init()
    {
        if (!shadowMapFBO.Init(WINDOW_WIDTH, WINDOW_HEIGHT)) return false;

        pGameCamera = new Camera(WINDOW_WIDTH, WINDOW_HEIGHT);

        pLightingEffect = new LightingTechnique();
        if (!pLightingEffect->Init())
        {
            printf("Error initializing the lighting technique\n");
            return false;
        }

        pShadowMapTech = new ShadowMapTechnique();
        if (!pShadowMapTech->Init())
        {
            printf("Error initializing the shadow map technique\n");
            return false;
        }
        pShadowMapTech->Enable();

        pQuad = new Mesh();
        if (!pQuad->LoadMesh("C:/Content/quad.obj")) return false;

        pMesh = new Mesh();
        return pMesh->LoadMesh("C:/Content/phoenix_ugv.md2");
    }

    void Run()
    {
        GLUTBackendRun(this);
    }

    virtual void RenderSceneCB()
    {
        pGameCamera->OnRender();
        scale += 0.05f;

        ShadowMapPass();
        RenderPass();

        glutSwapBuffers();
    }

    virtual void ShadowMapPass()
    {
        // all height values will go into our shadow map texture and the color will be discarded.
        shadowMapFBO.BindForWriting();

        glClear(GL_DEPTH_BUFFER_BIT);

        Pipeline p;
        p.Scale(0.2f, 0.2f, 0.2f);
        p.Rotate(0.0f, scale, 0.0f);
        p.WorldPos(0.0f, 0.0f, 5.0f);
        //the camera is updated according to the position and direction of the spotlight
        p.SetCamera(spotLight.Position, spotLight.Direction, Vector3f(0.0f, 1.0f, 0.0f));
        p.SetPerspectiveProj(60.0f, WINDOW_WIDTH, WINDOW_HEIGHT, 1.0f, 50.0f);
        pShadowMapTech->SetWVP(p.GetWVPTrans());
        pMesh->Render();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    virtual void RenderPass()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        pShadowMapTech->SetTextureUnit(0);
        shadowMapFBO.BindForReading(GL_TEXTURE0);

        Pipeline p;
        p.Scale(5.0f, 5.0f, 5.0f);
        p.WorldPos(0.0f, 0.0f, 10.0f);
        p.SetCamera(pGameCamera->GetPos(), pGameCamera->GetTarget(), pGameCamera->GetUp());
        p.SetPerspectiveProj(60.0f, WINDOW_WIDTH, WINDOW_HEIGHT, 1.0f, 50.0f);
        pShadowMapTech->SetWVP(p.GetWVPTrans());
        pQuad->Render();
    }

    virtual void IdleCB()
    {
        RenderSceneCB();
    }

    virtual void SpecialKeyboardCB(int Key, int x, int y)
    {
        pGameCamera->OnKeyboard(Key);
    }

    virtual void KeyboardCB(unsigned char Key, int x, int y)
    {
        switch (Key) {
        case 'q':
            glutLeaveMainLoop();
            break;
        }
    }

    virtual void PassiveMouseCB(int x, int y)
    {
        pGameCamera->OnMouse(x, y);
    }
};


int main(int argc, char** argv)
{
    Magick::InitializeMagick(nullptr);
    GLUTBackendInit(argc, argv);
    if (!GLUTBackendCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, 32, false, "OpenGL tutors"))
        return 1;

    Main* MainProgram = new Main();
    if (!MainProgram->Init()) return 1;

    MainProgram->Run();

    delete MainProgram;

    return 0;
}
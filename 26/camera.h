#ifndef CAMERA_H
#define	CAMERA_H

#include <GL/freeglut.h>
#include <glm/glm.hpp>

#include "math_3d.h"

const static float STEP_SCALE = 0.1f;
const static int MARGIN = 10;

class Camera
{
private:

    void Init()
    {
        Vector3f HTarget(target.x, 0.0, target.z);
        HTarget.Normalize();

        if (HTarget.z >= 0.0f)
        {
            if (HTarget.x >= 0.0f)
                AngleH = 360.0f - glm::degrees(asin(HTarget.z));
            else
                AngleH = 180.0f + glm::degrees(asin(HTarget.z));
        }
        else
        {
            if (HTarget.x >= 0.0f)
                AngleH = glm::degrees(asin(-HTarget.z));
            else
                AngleH = 90.0f + glm::degrees(asin(-HTarget.z));
        }

        AngleV = -glm::degrees(asin(target.y));

        mousePos.x = windowWidth / 2;
        mousePos.y = windowHeight / 2;

        glutWarpPointer(mousePos.x, mousePos.y);
    }

    void Update()
    {
        const Vector3f Vaxis(0.0f, 1.0f, 0.0f);

        // Rotate the view vector by the horizontal angle around the vertical axis
        Vector3f View(1.0f, 0.0f, 0.0f);
        View.Rotate(AngleH, Vaxis);
        View.Normalize();

        // Rotate the view vector by the vertical angle around the horizontal axis
        Vector3f Haxis = Vaxis.Cross(View);
        Haxis.Normalize();
        View.Rotate(AngleV, Haxis);

        target = View;
        target.Normalize();

        up = target.Cross(Haxis);
        up.Normalize();
    }

    Vector3f pos;
    Vector3f target;
    Vector3f up;

    int windowWidth;
    int windowHeight;

    float AngleH;
    float AngleV;

    Vector2i mousePos;

public:

    Camera(int WindowWidth, int WindowHeight)
    {
        windowWidth = WindowWidth;
        windowHeight = WindowHeight;
        pos = Vector3f(0.0f, 0.0f, 0.0f);
        target = Vector3f(0.0f, 0.0f, 1.0f);
        target.Normalize();
        up = Vector3f(0.0f, 1.0f, 0.0f);

        Init();
    }

    Camera(int WindowWidth, int WindowHeight, const Vector3f& Pos, const Vector3f& Target, const Vector3f& Up)
    {
        windowWidth = WindowWidth;
        windowHeight = WindowHeight;
        pos = Pos;

        target = Target;
        target.Normalize();

        up = Up;
        up.Normalize();

        Init();
    }

    bool OnKeyboard(int Key)
    {
        bool Ret = false;

        switch (Key)
        {

        case GLUT_KEY_UP:
        {
            pos += (target * STEP_SCALE);
            Ret = true;
        }
        break;

        case GLUT_KEY_DOWN:
        {
            pos -= (target * STEP_SCALE);
            Ret = true;
        }
        break;

        case GLUT_KEY_LEFT:
        {
            Vector3f Left = target.Cross(up);
            Left.Normalize();
            Left *= STEP_SCALE;
            pos += Left;
            Ret = true;
        }
        break;

        case GLUT_KEY_RIGHT:
        {
            Vector3f Right = up.Cross(target);
            Right.Normalize();
            Right *= STEP_SCALE;
            pos += Right;
            Ret = true;
        }
        break;
        }

        return Ret;
    }

    void OnMouse(int x, int y)
    {
        if ((x == mousePos.x) && (y == mousePos.y)) return;

        const int DeltaX = x - mousePos.x;
        const int DeltaY = y - mousePos.y;

        AngleH += (float)DeltaX / 20.0f;
        AngleV += (float)DeltaY / 20.0f;

        Update();
        glutWarpPointer(mousePos.x, mousePos.y);
    }

    void OnRender()
    {
        bool ShouldUpdate = false;

        if (ShouldUpdate) {
            Update();
        }
    }

    const Vector3f& GetPos() const
    {
        return pos;
    }

    const Vector3f& GetTarget() const
    {
        return target;
    }

    const Vector3f& GetUp() const
    {
        return up;
    }
};

#endif	/* CAMERA_H */

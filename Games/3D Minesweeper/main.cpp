#include <iostream>
#include <time.h>
#include "src\precomp.h"

#define RAYMATH_IMPLEMENTATION

using namespace rl;
#define FPS 1000

int main()
{
    rl::InitWindow(sst::baseX, sst::baseY, "3D exm");
    clock_t time = 0, ref = 0;

    //BASIC CAMERA REQUIREMENTS
        Camera3D camera; //Use vector3 to set position
        camera.position = Vector3{0, 5, 5}; //x y z position
        camera.target = Vector3{0, 0, 0}; //Sets where the camera points towards
        camera.up = Vector3{0,1,0}; //Camera NEEDS to know which direction is up
        /* Measured in degrees, determines the vertical viewing angle of the perspective projection
        Larger values increase the fov, making the scene more zoomed out, 
            more is visible, but objects appear smaller*/
        camera.fovy = 45.0;
        /* Determines how the 3D scene is rendered from the cameras pov
        CAMERA_PERSPECTIVE simulates how humans perceive depth (objects farther away appear smaller,
            lines that are parallel in 3D may converge to a vanishing point)*/ 
        camera.projection = CAMERA_PERSPECTIVE; 

    //MODELS
        Mesh mesh = GenMeshCube(2, 2, 2); //Every model needs a mesh
        Model model = LoadModelFromMesh(mesh); //Now we can display our mesh as a model
        Mesh mesh_cylinder = GenMeshCylinder(1, 1, 50);
        Model model_cylinder = LoadModelFromMesh(mesh_cylinder);
    
    //TEXTURE (getting our models to not be a boring color)
        Image image = GenImageGradientLinear(20, 20, 1, RED, YELLOW); //Defines a nice gradient
        Texture2D texture = LoadTextureFromImage(image); //Load it to a texture
        /* Assigns the texture to the model. Most of the time, you only use one texture, so put it in materials[0]
            Map type determines how light reflects from the object. MATERIAL_MAP_ALBEDO is most basic
            Then supply the texture just created.
        */
        SetMaterialTexture(&model_cylinder.materials[0], MATERIAL_MAP_ALBEDO, texture);

    //Movement
        Vector3 pos;


    SetTargetFPS(FPS);
    ref = clock();
    while (!WindowShouldClose())
    {
        //Frame independent (ish) movement
        time = clock();
        pos.x +=  (float) 1/ FPS;
        //TRANSFORMING THE MODEL
            //model_cylinder.transform;

        rl::BeginDrawing();
            ClearBackground(RAYWHITE);

            //3d
            BeginMode3D(camera); //Enables 3D mode, required for 3D object display
                /* Draws grid lines. Slices determines # of divisions in each direction (X,Z)
                Spacing determines the side lengths of the squares (grids). */ 
                DrawGrid(10, 1);
                DrawPoint3D(Vector3{1,0,1},RED);
                //DrawModel(model, Vector3{0,0,0}, 1, RED); //Uses center position
                /* This is not what we expected. Raylib does not really know where to exactly put the texture
                    we need a UV_map to do so and that is done using outside programs such as blender to created
                        the appropriate texture. Not really a problem as of now*/
                DrawModel(model_cylinder, pos, 1, WHITE);

            EndMode3D();


        EndDrawing();
    }
    return 0;
}
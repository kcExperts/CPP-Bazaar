#include <iostream>
#include <time.h>
#include "src\precomp.h"

#define RAYMATH_IMPLEMENTATION


#define FPS 1000

struct MapDrawInfo
{
    Mesh mesh;
    Material mat;
    size_t length;
    int selected = -1;
};

void Draw3DMapSlabs(MapDrawInfo& mapDraw);


int main()
{
    InitWindow(sst::baseX, sst::baseY, "3D exm");
    clock_t time = 0, ref = 0;

    //Basic slab map
    Image temp = GenImageColor(1,1, GRAY);
    Texture2D temptext = LoadTextureFromImage(temp);
    UnloadImage(temp);
    MapDrawInfo mapDraw;
    mapDraw.length = 19;
    mapDraw.mat = LoadMaterialDefault();
    mapDraw.mesh = GenMeshCube(mapDraw.length, mapDraw.length, 1);
    SetMaterialTexture(&mapDraw.mat, MATERIAL_MAP_ALBEDO, temptext);




    Vector3 mapTarget = Vector3{0, 0, 0};
    Vector3 playTarget = Vector3{-(float)mapDraw.length, 0, (float)mapDraw.length + 11};

    //BASIC CAMERA REQUIREMENTS
        Camera3D camera; //Use vector3 to set position
        camera.position = Vector3{-(float)mapDraw.length, (float)mapDraw.length, (float)mapDraw.length}; //x y z position
        camera.target = Vector3{0, 0, 0}; //Sets where the camera points towards
        camera.up = Vector3{0,0,1}; //Camera NEEDS to know which direction is up
        /* Measured in degrees, determines the vertical viewing angle of the perspective projection
        Larger values increase the fov, making the scene more zoomed out, 
            more is visible, but objects appear smaller*/
        camera.fovy = 45.0;
        /* Determines how the 3D scene is rendered from the cameras pov
        CAMERA_PERSPECTIVE simulates how humans perceive depth (objects farther away appear smaller,
            lines that are parallel in 3D may converge to a vanishing point)*/ 
        camera.projection = CAMERA_PERSPECTIVE; 

    //MODELS
        // Mesh mesh = GenMeshCube(2, 2, 2); //Every model needs a mesh
        // Model model = LoadModelFromMesh(mesh); //Now we can display our mesh as a model
        // Mesh mesh_cylinder = GenMeshCylinder(1, 1, 50);
        // Model model_cylinder = LoadModelFromMesh(mesh_cylinder);
    
    //TEXTURE (getting our models to not be a boring color)
        Image image = GenImageGradientLinear(20, 20, 1, RED, YELLOW); //Defines a nice gradient
        Texture2D texture = LoadTextureFromImage(image); //Load it to a texture
        /* Assigns the texture to the model. Most of the time, you only use one texture, so put it in materials[0]
            Map type determines how light reflects from the object. MATERIAL_MAP_ALBEDO is most basic
            Then supply the texture just created.
        */
        //SetMaterialTexture(&model_cylinder.materials[0], MATERIAL_MAP_ALBEDO, texture);

    //Movement
        Vector3 pos;
        float rotation = 0;
    Matrix transform = MatrixTranslate(0.0f, 0.0f, 0.0f);
    SetTargetFPS(FPS);
    ref = clock();
    while (!WindowShouldClose())
    {
        //Frame independent (ish) movement
        time = clock();
        pos.x +=  (float) 1/ FPS;
        rotation += (float) 1/ FPS;
        //TRANSFORMING THE MODEL
            //model_cylinder.transform = MatrixRotateX(rotation);

        // UpdateCamera(&camera, CAMERA_FIRST_PERSON);
        //camera.fovy = 45.0;
        // DisableCursor();

        if (IsKeyPressed(KEY_ZERO)) camera.target = mapTarget;
        if (IsKeyPressed(KEY_ONE)) camera.target = playTarget;
        float wheelMove = GetMouseWheelMove();
        if (wheelMove > 0)
        {
            mapDraw.selected++;
            if (mapDraw.selected >= mapDraw.length) mapDraw.selected = 0;
        } else if (wheelMove < 0)
        {
            mapDraw.selected--;
            if (mapDraw.selected <= -1) mapDraw.selected = mapDraw.length - 1;
        }

        BeginDrawing();
            ClearBackground(RAYWHITE);

            //3d
            BeginMode3D(camera); //Enables 3D mode, required for 3D object display
                /* Draws grid lines. Slices determines # of divisions in each direction (X,Z)
                Spacing determines the side lengths of the squares (grids). */ 
                DrawGrid(100, 1);
                DrawPoint3D(Vector3{1,0,0},RED);
                DrawPoint3D(Vector3{0,1,0},GREEN);
                DrawPoint3D(Vector3{0,0,1},BLUE);
                //DrawModel(model, Vector3{0,0,0}, 1, RED); //Uses center position
                /* This is not what we expected. Raylib does not really know where to exactly put the texture
                    we need a UV_map to do so and that is done using outside programs such as blender to created
                        the appropriate texture. Not really a problem as of now*/
                //DrawModel(model_cylinder, pos, 1, WHITE);
                Draw3DMapSlabs(mapDraw);
                //DrawMesh(mapMesh, LoadMaterialDefault(), transform);


            EndMode3D();


        EndDrawing();
    }
    return 0;
}

//Cube relic drawing
// void DrawMineMap(MapDrawInfo& mapDraw)
// {
//     size_t totalCubes = mapDraw.length * mapDraw.length * mapDraw.length;
//     float reset = (float)mapDraw.length/2 - 0.5f;
//     Matrix pos = MatrixTranslate(reset, -0.5f, reset); //translation x,y,z stored in m12, m13, m14
//     for (size_t i = 1; i <= totalCubes; i++)
//     {
//         DrawMesh(mapDraw.mesh, mapDraw.mat, pos);
//         pos.m12 -= 1;
//         if (i % mapDraw.length == 0)
//         {
//             pos.m12 = reset;
//             pos.m14 -= 1;
//         }
//         if (i % (mapDraw.length * mapDraw.length) == 0)
//         {
//             pos.m12 = reset;
//             pos.m14 = reset;
//             pos.m13 -=1;
//         }
//     }
// }

void Draw3DMapSlabs(MapDrawInfo& mapDraw)
{
    Matrix pos = MatrixTranslate(0, -(float)mapDraw.length/2, (float)mapDraw.length/2 - 0.5f);
    for (size_t i = 0; i < mapDraw.length; i++)
    {
        if (i == mapDraw.selected)
        {
            pos.m13 += mapDraw.length; //y position
            DrawMesh(mapDraw.mesh, mapDraw.mat, pos);
            pos.m13 -= mapDraw.length; //Reset
        } else {
            DrawMesh(mapDraw.mesh, mapDraw.mat, pos);
        }
        pos.m14 -= 1; // z position
    }
}
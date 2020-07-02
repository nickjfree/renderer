// LevelGenerator.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include "Resource\LevelDesc.h"
#include "windows.h"

/*
    simple level test.
    Mesh material model gameobject[component]
*/


int TestGen(char * File) {
    HANDLE hFile = CreateFileA(File, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, NULL, NULL);
    DWORD write;
    // write mesh data, Lumia, Adata usbdrive and Sphere(for point light)
    LevelHeader Header = {};
    int Num = 5;
    Header.NumEntries = Num;
    char * meshes[5] = {
        "Mesh\\Unit.pack\\ground\\0",
        "Mesh\\character.pack\\human2\\0",
        "Mesh\\Unit.pack\\sphere\\0",
        "Mesh\\Unit.pack\\sneak\\0",
        "Mesh\\BlendShapes.pack\\eyeBlink_L\\0",
    };
    MeshEntry mesh = {};
    WriteFile(hFile, &Header, sizeof(Header), &write, 0);
    while (Num--) {
        strcpy_s(mesh.Url, 256, meshes[Num]);
        WriteFile(hFile, &mesh, sizeof(mesh), &write, 0);
    }
    // write material 
    char * materials[15] = {
        "Material\\Materials\\sneak.xml\\0",
        "Material\\Materials\\sneak1.xml\\0",
        "Material\\Materials\\sneak2.xml\\0",
        "Material\\Materials\\sneak3.xml\\0",
        "Material\\Materials\\sneak4.xml\\0",
        "Material\\Materials\\sneak5.xml\\0",
        "Material\\Materials\\sneak6.xml\\0",
        "Material\\Materials\\sneak7.xml\\0",
        "Material\\Materials\\sneak8.xml\\0",
        "Material\\Materials\\sneak9.xml\\0",


        "Material\\Materials\\lightprobe.xml\\0",
        "Material\\Materials\\head.xml\\0",
        "Material\\Materials\\ground.xml\\0", 
        "Material\\Materials\\human.xml\\0",
        //"Material\\Materials\\resistor.xml\\0",
        "Material\\Materials\\light.xml\\0",
    };
    MatrialEntry material = {};
    Num = 15;
    Header.NumEntries = Num;
    WriteFile(hFile, &Header, sizeof(Header), &write, 0);
    while (Num--) {
        strcpy_s(material.Url, 256, materials[Num]);
        WriteFile(hFile, &material, sizeof(material), &write, 0);
    }
    // write models
    Num = 5;
    Header.NumEntries = Num;
    ModelEntry model = {};
    char * models[5] = { "eyeBlinkLeft", "sneak", "sphere", "human2", "plane", };
    WriteFile(hFile, &Header, sizeof(Header), &write, 0);
    while (Num--) {
        memset(model.MeshGroup, 0, sizeof(int) * 8);
        model.MeshGroup[0] = Num;
        strcpy_s(model.Name, 128, models[Num]);
        WriteFile(hFile, &model, sizeof(model), &write, 0);
    }
    // write skeletions
    Num = 1;
    Header.NumEntries = Num;
    SkeletonEntry skeleton = {};
    WriteFile(hFile, &Header, sizeof(Header), &write, 0);
    while (Num--) {
        strcpy_s(skeleton.Url, 128, "Skeleton\\skeletons\\human.hsk");
        WriteFile(hFile, &skeleton, sizeof(skeleton), &write, 0);
    }
    // write animations
    Num = 3;
    Header.NumEntries = Num;
    AnimationEntry animation = {};
    WriteFile(hFile, &Header, sizeof(Header), &write, 0);
    char * animations[3] = {
        "Animation\\keyframe\\human_walking.ha", 
        "Animation\\keyframe\\human_left_turn.ha", 
        "Animation\\keyframe\\human_right_turn.ha"
    };
    auto i = 0;
    while (i < Num) {
        strcpy_s(animation.Url, 128, animations[i++]);
        WriteFile(hFile, &animation, sizeof(animation), &write, 0);
    }
    // write blendshapes
    Num = 1;
    Header.NumEntries = Num;
    BlendShapeEntry blendshape = {};
    WriteFile(hFile, &Header, sizeof(Header), &write, 0);
    char* blendshapes[1] = {
        "BlendShape\\blendshapes\\arkit.xml"
    };
    i = 0;
    while (i < Num) {
        strcpy_s(blendshape.Url, 128, blendshapes[i++]);
        WriteFile(hFile, &blendshape, sizeof(blendshape), &write, 0);
    }
    // write gameobjects, lumia adata and some lights
    RenderEntry render = {};
    LightEntry light = {};
    PhysicsEntry physics = {};
    ObjectEntry object = {};

    Num = 7 + 100;
    Header.NumEntries = Num;
    WriteFile(hFile, &Header, sizeof(Header), &write, 0);
    Quaternion rot = Quaternion();
    object.Rotation = Quaternion();
    // human2
    strcpy_s(object.Name, "Player");
    object.NumComponents = 1;
    object.Position = Vector3(0, 0, 0);
    object.Rotation = Quaternion();
    object.Scale = Vector3(1, 1, 1);
    render.MaterialIndex = 1;
    render.ModelIndex = 1;
    strcpy_s(render.Info.TypeName, "Renderer");
    // write human2
    WriteFile(hFile, &object, sizeof(ObjectEntry), &write, 0);
    WriteFile(hFile, &render, sizeof(render), &write, 0);


    // head
    strcpy_s(object.Name, "Plane");
    object.NumComponents = 2;
    object.Position = Vector3(0.4f, 4.4, -5.5f);
    rot.RotationAxis(Vector3(1, 0, 0), 0.5f * 3.14159f);
    object.Rotation = rot;
    object.Scale = Vector3(1, 1, 1);
    render.MaterialIndex = 3;
    render.ModelIndex = 0;
    strcpy_s(render.Info.TypeName, "Renderer");
    // physics component
    strcpy_s(physics.Info.TypeName, "PhysicsObject");
    physics.Type = 0;
    // write adata
    WriteFile(hFile, &object, sizeof(ObjectEntry), &write, 0);
    WriteFile(hFile, &render, sizeof(render), &write, 0);
    WriteFile(hFile, &physics, sizeof(physics), &write, 0);


    // ground  as a big plane
    strcpy_s(object.Name, "Ground");
    object.NumComponents = 1;
    object.Position = Vector3(0.0f, 0.0f, 0.0f);
    rot.RotationAxis(Vector3(1, 0, 0), 0.5f * 3.14159f);
    object.Rotation = rot;
    object.Scale = Vector3(1, 1, 1);
    render.MaterialIndex = 2;
    render.ModelIndex = 0;
    strcpy_s(render.Info.TypeName, "Renderer");
    // write adata
    WriteFile(hFile, &object, sizeof(ObjectEntry), &write, 0);
    WriteFile(hFile, &render, sizeof(render), &write, 0);

    // light1
    strcpy_s(object.Name, "Light1");
    object.NumComponents = 1;
    object.Position = Vector3(0, 30, 0);
    object.Rotation = Quaternion();
    object.Rotation.RotationAxis(Vector3(1, 0, 0), 0.5f * 3.14159f);
    object.Scale = Vector3(1, 1, 1);
    light.MaterialIndex = 0;
    light.ModelIndex = 2;
    light.Color = Vector3(1, 1, 1);
    light.Intensity = 1;
    light.Radius = 50;
    light.Type = 0;
    strcpy_s(light.Info.TypeName, "Light");
    // write light1
    WriteFile(hFile, &object, sizeof(ObjectEntry), &write, 0);
    WriteFile(hFile, &light, sizeof(light), &write, 0);

    strcpy_s(object.Name, "Light2");
    object.NumComponents = 1;
    object.Position = Vector3(0, 1, 3);
    object.Rotation.RotationAxis(Vector3(1, 0, 0), 0.5f * 3.14159f);
    object.Scale = Vector3(1, 1, 1);
    light.MaterialIndex = 0;
    light.ModelIndex = 2;
    light.Color = Vector3(1, 1, 1);
    light.Intensity = 1;
    light.Radius = 50;
    light.Type = 1;
    light.Direction = Vector3(1.414f, -1.414f, 0.5);
    strcpy_s(light.Info.TypeName, "Light");
    // write light2
    WriteFile(hFile, &object, sizeof(ObjectEntry), &write, 0);
    WriteFile(hFile, &light, sizeof(light), &write, 0);

    // sneak
    strcpy_s(object.Name, "qianzhihe");
    object.NumComponents = 2;
    object.Position = Vector3(0, 0, 0);
    //	Quaternion rot = Quaternion();
    rot.RotationAxis(Vector3(1, 0, 0), 1.5f * 3.14159f);
    object.Rotation = rot;
    object.Rotation = Quaternion();
    object.Rotation.RotationAxis(Vector3(1, 0, 0), 3.14159f);
    object.Scale = Vector3(1, 1, 1);
    // render componemt
    render.MaterialIndex = 5;
    render.ModelIndex = 3;
    strcpy_s(render.Info.TypeName, "Renderer");
    // physics component
    strcpy_s(physics.Info.TypeName, "PhysicsObject");
    physics.Type = 0;
    // write sneak
    WriteFile(hFile, &object, sizeof(ObjectEntry), &write, 0);
    WriteFile(hFile, &render, sizeof(render), &write, 0);
    WriteFile(hFile, &physics, sizeof(physics), &write, 0);

    i = 0;
    while (i++ < 100) {
        strcpy_s(object.Name, "qianzhihe");
        object.NumComponents = 2;
        object.Position = Vector3(0 + (i / 10) * 10, 5, 0 + (i % 10) * 10);
        //	Quaternion rot = Quaternion();
        rot.RotationAxis(Vector3(1, 0, 0), 1.5f * 3.14159f);
        object.Rotation = rot;
        object.Rotation = Quaternion();
        object.Rotation.RotationAxis(Vector3(1, 0, 0), 3.14159f);
        object.Scale = Vector3(1, 1, 1);
        // renderer
        render.MaterialIndex = 5 + (i % 10);
        render.ModelIndex = 3;
        strcpy_s(render.Info.TypeName, "Renderer");
        // physics
        strcpy_s(physics.Info.TypeName, "PhysicsObject");
        physics.Type = 0;
        // write qianzhihe
        WriteFile(hFile, &object, sizeof(ObjectEntry), &write, 0);
        WriteFile(hFile, &render, sizeof(render), &write, 0);
        WriteFile(hFile, &physics, sizeof(physics), &write, 0);
    }

    // envirremont light
    strcpy_s(object.Name, "LightProb");
    object.NumComponents = 1;
    object.Position = Vector3(0, 1, 3);
    object.Rotation.RotationAxis(Vector3(1, 0, 0), 0.5f * 3.14159f);
    object.Scale = Vector3(1, 1, 1);
    light.MaterialIndex = 4;
    light.ModelIndex = 2;
    light.Color = Vector3(1, 1, 1);
    light.Intensity = 1;
    light.Radius = 50;
    light.Type = 3;
    light.Direction = Vector3(1.414f, -1.414f, 0.5);
    strcpy_s(light.Info.TypeName, "Light");
    // write light2
    WriteFile(hFile, &object, sizeof(ObjectEntry), &write, 0);
    WriteFile(hFile, &light, sizeof(light), &write, 0);


    // over
    CloseHandle(hFile);
    return 0;
}



int _tmain(int argc, _TCHAR* argv[])
{
    char * file = "F:\\proj\\Game11\\Game\\Engine\\Levels\\Desktop.level";
    TestGen(file);
    printf("finish %s\n", file);
    return 0;
}


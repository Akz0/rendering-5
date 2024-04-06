#pragma once
#ifndef MODEL_CLASS
#define MODEL_CLASS

#include <vector>
#include <iostream>
#include <fstream>
#include <iomanip>

#include <Eigen/Dense>
#include <Eigen/Sparse>

#include <glm/glm.hpp>
#include <GL/glew.h>

#include <assimp/cimport.h> 
#include <assimp/scene.h>
#include <assimp/postprocess.h> 

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "imGui/imgui.h"
#include "imGui/imgui_impl_glfw.h"
#include "imGui/imgui_impl_opengl3.h"

#include "Shader.h"
#include "Utilities.h"
#include "Texture.h"

#define NEUTRAL_LOW_RES "./src/model/neutral.obj"                                      
#define JAW_OPEN_LOW_RES "./src/low-res-model/Mery_jaw_open.obj"                       
#define KISS_LOW_RES "./src/low-res-model/Mery_kiss.obj"                                
#define L_BROW_LOWER_LOW_RES "./src/low-res-model/Mery_l_brow_lower.obj"                
#define L_BROW_NARROW_LOW_RES "./src/low-res-model/Mery_l_brow_narrow.obj"             
#define L_BROW_RAISE_LOW_RES "./src/low-res-model/Mery_l_brow_raise.obj"                
#define LEFT_CLOSED_LOW_RES "./src/low-res-model/Mery_l_eye_closed.obj"                 
#define L_LOWER_O_LOW_RES "./src/low-res-model/Mery_l_eye_lower_open.obj"               
#define L_UPPER_O_LOW_RES "./src/low-res-model/Mery_l_eye_upper_open.obj"               
#define L_NOSE_WRINKLE_LOW_RES "./src/low-res-model/Mery_l_nose_wrinkle.obj"            
#define L_PUFF_LOW_RES "./src/low-res-model/Mery_l_puff.obj"                            
#define L_SAD_LOW_RES "./src/low-res-model/Mery_l_sad.obj"                              
#define L_SMILE_LOW_RES "./src/low-res-model/Mery_l_smile.obj"                          
#define L_SUCK_LOW_RES "./src/low-res-model/Mery_l_suck.obj"                           
#define R_BROW_LOWER_LOW_RES "./src/low-res-model/Mery_r_brow_lower.obj"                
#define R_BROW_NARROW_LOW_RES "./src/low-res-model/Mery_r_brow_narrow.obj"              
#define R_BROW_RAISE_LOW_RES "./src/low-res-model/Mery_r_brow_raise.obj"                
#define R_CLOSED_LOW_RES "./src/low-res-model/Mery_r_eye_closed.obj"                     
#define R_LOWER_O_LOW_RES "./src/low-res-model/Mery_r_eye_lower_open.obj"              
#define R_UPPER_O_LOW_RES "./src/low-res-model/Mery_r_eye_upper_open.obj"               
#define R_NOSE_WRINKLE_LOW_RES "./src/low-res-model/Mery_r_nose_wrinkle.obj"            
#define R_PUFF_LOW_RES "./src/low-res-model/Mery_r_puff.obj"                            
#define R_SAD_LOW_RES "./src/low-res-model/Mery_r_sad.obj"                              
#define R_SMILE_LOW_RES "./src/low-res-model/Mery_r_smile.obj"                          
#define R_SUCK_LOW_RES "./src/low-res-model/Mery_r_suck.obj"                            

class Mesh {
public:
    size_t totalPoints = 0;
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> textureCoords;
    std::vector<glm::vec3> tangents;
    std::vector<glm::vec3> biTangents;

    std::vector<float> VertexData;

    Mesh() {};
    Mesh(const char* fileName);
    void CreateMesh(const char* fileName);
};

class Object
{
public:
    std::string name;

    Mesh mesh;
    GLuint VAO;

    Texture baseMap;
    Texture normalMap;
    Texture specularMap;

    glm::mat4 model;
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;

    Object(std::string n) :name(n) {
        position = glm::vec3(0.f, 0.0f, 0.0f);
        rotation = glm::vec3(0.f, 0.0f, 0.0f);
        scale = glm::vec3(1.f, 1.0f, 1.0f);
    };

    void CalculateModel() {
        model = glm::translate(glm::mat4(1.0f), position);
        model = glm::rotate(model, rotation.x * glm::radians(1.f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, rotation.y * glm::radians(1.f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, rotation.z * glm::radians(1.f), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, scale);
    }

    void AddMesh(const char* fileName);

    void AddBaseMap(const char* fileName);
    void AddNormalMap(const char* fileName);
    void AddSpecularMap(const char* fileName);

    void Load();
    void Display(Camera camera,Shader shader);
    void AddToUI();
};

#endif // !MODEL_CLASS


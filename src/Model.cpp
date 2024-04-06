#include "Model.h"

Mesh::Mesh(const char* fileName) {
    CreateMesh(fileName);
}

void Mesh::CreateMesh(const char* fileName) {
   
    const aiScene* scene = aiImportFile(
        fileName,
        aiProcess_Triangulate
        | aiProcess_GenSmoothNormals
        | aiProcess_PreTransformVertices
        | aiProcess_OptimizeMeshes
        | aiProcess_FlipUVs);

    if (!scene) {
        std::cerr << "Error Reading Mesh : " << fileName << std::endl;
        return ;
    }

    for (unsigned int m_i = 0; m_i < scene->mNumMeshes; m_i++) {
        const aiMesh* mesh = scene->mMeshes[m_i];
        totalPoints += mesh->mNumVertices;
        for (unsigned int v_i = 0; v_i < mesh->mNumVertices; v_i++) {
            if (mesh->HasPositions()) {
                const aiVector3D* vertex = &(mesh->mVertices[v_i]);
                vertices.push_back(glm::vec3(vertex->x, vertex->y, vertex->z));
                
                VertexData.push_back(vertex->x);
                VertexData.push_back(vertex->y);
                VertexData.push_back(vertex->z);
            }
            if (mesh->HasNormals()) {
                const aiVector3D* normal = &(mesh->mNormals[v_i]);
                normals.push_back(glm::vec3(normal->x, normal->y, normal->z));
            }
            if (mesh->HasTextureCoords(0)) {
                const aiVector3D* textureCoordinates = &(mesh->mTextureCoords[0][v_i]);
                textureCoords.push_back(glm::vec2(textureCoordinates->x, textureCoordinates->y));

            }
            if (mesh->HasTangentsAndBitangents()) {
                const aiVector3D* tangent = &(mesh->mTangents[v_i]);
                tangents.push_back(glm::vec3(tangent->x, tangent->y, tangent->z));

                const aiVector3D* biTangent = &(mesh->mBitangents[v_i]);
                biTangents.push_back(glm::vec3(biTangent->x, biTangent->y, biTangent->z));
            }
        }
    }
    
    aiReleaseImport(scene);
    return;
}

void Object::AddMesh(const char* fileName) {
    mesh.CreateMesh(fileName);
}

void Object::AddBaseMap(const char* fileName) {
    baseMap.Initialize("diffuse", fileName);
    baseMap.Generate(0,GL_UNSIGNED_BYTE);
}

void Object::AddNormalMap(const char* fileName) {
    baseMap.Initialize("normal", fileName);
    baseMap.Generate(1, GL_UNSIGNED_BYTE);
}

void Object::AddSpecularMap(const char* fileName) {
    baseMap.Initialize("specular", fileName);
    baseMap.Generate(2, GL_UNSIGNED_BYTE);
}

void Object::Load() {
    GLuint aPos = 0;
    GLuint aNormal = 1;
    GLuint aTexCords = 2;
    GLuint aTangent = 3;
    GLuint aBiTangent = 4;

    unsigned int aPosVBO = 0;
    unsigned int aTexCordsVBO = 0;
    unsigned int aNormalVBO = 0;
    unsigned int aTangentVBO = 0;
    unsigned int aBiTangentVBO = 0;

    glGenBuffers(1, &aPosVBO);
    glBindBuffer(GL_ARRAY_BUFFER, aPosVBO);
    glBufferData(GL_ARRAY_BUFFER, mesh.VertexData.size() * sizeof(float), mesh.VertexData.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &aNormalVBO);
    glBindBuffer(GL_ARRAY_BUFFER, aNormalVBO);
    glBufferData(GL_ARRAY_BUFFER, 3 * mesh.totalPoints * sizeof(float), &mesh.normals[0], GL_STATIC_DRAW);

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glEnableVertexAttribArray(aPos);
    glBindBuffer(GL_ARRAY_BUFFER, aPosVBO);
    glVertexAttribPointer(aPos, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    glEnableVertexAttribArray(aNormal);
    glBindBuffer(GL_ARRAY_BUFFER, aNormalVBO);
    glVertexAttribPointer(aNormal, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    if (mesh.textureCoords.size() > 0) {
        glGenBuffers(1, &aTexCordsVBO);
        glBindBuffer(GL_ARRAY_BUFFER, aTexCordsVBO);
        glBufferData(GL_ARRAY_BUFFER, 3 * mesh.totalPoints * sizeof(float), &mesh.textureCoords[0], GL_STATIC_DRAW);
    }
    if (mesh.tangents.size() > 0) {
        glGenBuffers(1, &aTangentVBO);
        glBindBuffer(GL_ARRAY_BUFFER, aTangentVBO);
        glBufferData(GL_ARRAY_BUFFER, 3 * mesh.totalPoints * sizeof(float), &mesh.tangents[0], GL_STATIC_DRAW);
    }

    if (mesh.biTangents.size() > 0) {
        glGenBuffers(1, &aBiTangentVBO);
        glBindBuffer(GL_ARRAY_BUFFER, aBiTangentVBO);
        glBufferData(GL_ARRAY_BUFFER, 3 * mesh.totalPoints * sizeof(float), &mesh.biTangents[0], GL_STATIC_DRAW);
    }

    if (mesh.textureCoords.size() > 0) {
        glEnableVertexAttribArray(aTexCords);
        glBindBuffer(GL_ARRAY_BUFFER, aTexCordsVBO);
        glVertexAttribPointer(aTexCords, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    }

    if (mesh.tangents.size() > 0) {
        glEnableVertexAttribArray(aTangent);
        glBindBuffer(GL_ARRAY_BUFFER, aTangentVBO);
        glVertexAttribPointer(aTangent, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    }

    if (mesh.biTangents.size() > 0) {
        glEnableVertexAttribArray(aBiTangent);
        glBindBuffer(GL_ARRAY_BUFFER, aBiTangentVBO);
        glVertexAttribPointer(aBiTangent, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    }
}

void Object::Display(Camera camera,Shader shader) {
    shader.Activate();
    CalculateModel();

    baseMap.Load(shader, "baseMap", 0);
    baseMap.Bind();

    glUniform3f(glGetUniformLocation(shader.ID, "CameraPosition"), camera.position.x, camera.position.y, camera.position.z);

    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(camera.projection));
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "view"), 1, GL_FALSE, glm::value_ptr(camera.view));

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, mesh.totalPoints);
}

void Object::AddToUI() {
    if (ImGui::CollapsingHeader(name.c_str())) {

        ImGui::Spacing();
        ImGui::Spacing();

        std::string pos = name + " Position";
        std::string sca = name + " Scale";
        std::string rot = name + " Transform";

        ImGui::SeparatorText("Transform");
        ImGui::DragFloat3(pos.c_str(), glm::value_ptr(position), 1, -1000.0f, 1000.0f);
        ImGui::DragFloat3(sca.c_str(), glm::value_ptr(scale), 0.1, -1000.f, 1000.f);
        ImGui::DragFloat3(rot.c_str(), glm::value_ptr(rotation), 1, -1000.f, 1000.f);

        ImGui::Spacing();
        ImGui::Spacing();

    }
}
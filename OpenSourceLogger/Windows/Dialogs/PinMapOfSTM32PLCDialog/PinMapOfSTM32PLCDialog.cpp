#include "PinMapOfSTM32PLCDialog.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#define GL_SILENCE_DEPRECATION
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h> // Will drag system OpenGL headers
#define STB_IMAGE_IMPLEMENTATION
#define GL_CLAMP_TO_EDGE 0x812F
#include "stb_image.h"
#include <Windows.h>
#include "../../../Resource/resource.h"

GLuint my_image_texture = 0;
int my_image_width = 0;
int my_image_height = 0;
bool isImageLoaded = false;
bool LoadTextureFromFile(GLuint* out_texture, int* out_width, int* out_height);

void showPinMapOfSTM32PLCDialog(bool* pinMapOfSTM32PLC) {
    ImGui::SetNextWindowSize(ImVec2(820.0f, 740.0f));
    ImGui::Begin("Pinmap of STM32PLC", pinMapOfSTM32PLC, ImGuiWindowFlags_NoResize);
    if (!isImageLoaded) {
        isImageLoaded = LoadTextureFromFile(&my_image_texture, &my_image_width, &my_image_height);
    }
    if (isImageLoaded) {
        ImGui::Image((void*)(intptr_t)my_image_texture, ImVec2(my_image_width, my_image_height));
    } else {
        ImGui::Text("Missing pinmap image");
    }
    ImGui::End();
}

// Simple helper function to load an image into a OpenGL texture with common settings
bool LoadTextureFromFile(GLuint* out_texture, int* out_width, int* out_height)
{

    // Locate the resource in the application's executable.
    HRSRC imageResHandle = FindResource(
        NULL,             // This component.
        MAKEINTRESOURCE(IDR_IMAGE1),   // Resource name.
        L"Image");        // Resource type.
    HRESULT hr = (imageResHandle ? S_OK : E_FAIL);

    // Load the resource to the HGLOBAL.
    HGLOBAL imageResDataHandle = NULL;
    if (SUCCEEDED(hr)) {
        imageResDataHandle = LoadResource(NULL, imageResHandle);
        hr = (imageResDataHandle ? S_OK : E_FAIL);
    }

    // Lock the resource to retrieve memory pointer.
    LPVOID void_data = NULL;
    DWORD size_of_void_data = 0;
    if (SUCCEEDED(hr)) {
        void_data = LockResource(imageResDataHandle);
        size_of_void_data = SizeofResource(NULL, imageResHandle);
        UnlockResource(imageResDataHandle);
        hr = (void_data ? S_OK : E_FAIL);
    }

    if (void_data == NULL)
        return false;

    // Load from file
    int image_width = 0;
    int image_height = 0;
    unsigned char* image_data = stbi_load_from_memory((const stbi_uc*)void_data, size_of_void_data, &image_width, &image_height, NULL, 4);
    if (image_data == NULL)
        return false;

    // Create a OpenGL texture identifier
    GLuint image_texture;
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

    // Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    stbi_image_free(image_data);

    *out_texture = image_texture;
    *out_width = image_width;
    *out_height = image_height;

    return true;
}
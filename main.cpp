// Dear ImGui: standalone example application for GLFW + OpenGL 3, using programmable pipeline
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif
static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Simple helper function to load an image into a OpenGL texture with common settings
bool LoadTextureFromFile(const char* filename, GLuint* out_texture, int* out_width, int* out_height)
{
    // Load from file
    int image_width = 0;
    int image_height = 0;
    unsigned char* image_data = stbi_load(filename, &image_width, &image_height, NULL, 4);
    if (image_data == NULL)
        return false;

    // Create a OpenGL texture identifier
    GLuint image_texture;
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
    //#glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

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



#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <set>

int main(int, char**)
{
    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char* glsl_version = "#version 100";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    // Create window with graphics context
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Tarot", NULL, NULL);
    if (window == NULL)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImFont* font2 = io.Fonts->AddFontFromFileTTF("FONT_CH.ttf", 30, NULL, io.Fonts->GetGlyphRangesChineseSimplifiedCommon());
    //bool is_CH = false;
    bool is_CH = (font2!=NULL);
    std::vector<std::string>big_arc;
    std::vector<std::string>small_arc_type0;
    std::vector<std::string>small_arc_type1;
    std::vector<std::string>small_arc;

    std::vector<std::string>big_arc_en = { "The Fool","The Magician","The High Priestess","The Empress","The Emperor","The Pope","The Lovers","The Chariot","Strength","The Hermit","The Wheel of Fortune","Justice",
                        "The Hanged Man","Death","Temperance","The Devil","The Tower","The Star","The Moon","The Sun","Judgement","The World" };
    std::vector<std::string>small_arc_type0_en = { "Wands","Pentacles","Swords","Cups" };
    std::vector<std::string>small_arc_type1_en = { "Ace","Two","Three","Four","Five","Six","Seven","Eight","Nine","Ten","King","Knight","Page","Queen" };
    std::vector<std::string>small_arc_en;
    if (is_CH)
    {
        big_arc = { u8"愚者",u8"魔术师",u8"女祭司",u8"女皇",u8"皇帝",u8"教皇",u8"恋人",u8"战车",u8"力量",u8"隐士",u8"命运之轮",u8"正义",
                        u8"倒吊人",u8"死神",u8"节制",u8"恶魔",u8"塔",u8"星星",u8"月亮",u8"太阳",u8"审判",u8"世界" };
        small_arc_type0 = { u8"权杖",u8"星币",u8"宝剑",u8"圣杯" };
        small_arc_type1 = { u8"一",u8"二",u8"三",u8"四",u8"五",u8"六",u8"七",u8"八",u8"九",u8"十",u8"国王",u8"骑士",u8"侍从",u8"王后" };
    }
    else
    {
        big_arc = big_arc_en;
        small_arc_type0 = small_arc_type0_en;
        small_arc_type1 = small_arc_type1_en;
    }
    for (auto i : small_arc_type0_en)
    {
        for (auto j : small_arc_type1_en)
        {
            small_arc_en.push_back(j + std::string(" of ") + i);
        }
    }
    for (auto i : small_arc_type0)
    {
        for (auto j : small_arc_type1)
        {
            if (is_CH)
            {
                small_arc.push_back(i + j);
            }
            else
            {
                small_arc.push_back(j + std::string(" of ") + i);
            }
        }
    }

    std::vector<std::string>all_arc = big_arc;
    all_arc.insert(all_arc.end(), small_arc.begin(), small_arc.end());
    std::vector<std::string>all_arc_en = big_arc_en;
    all_arc_en.insert(all_arc_en.end(), small_arc_en.begin(), small_arc_en.end());
    std::vector<int>all_arc_idx;
    for (int i = 0; i < all_arc.size(); ++i)
    {
        all_arc_idx.push_back(i);
    }
    std::random_shuffle(all_arc_idx.begin(), all_arc_idx.end());



    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);

    // Our state
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).

        {
            static int card_num = 1;
            static std::vector<int> card_idx(card_num,0);
            static std::vector<int> inverse_or_not(card_num);
            static std::vector<int>img_width(card_num);
            static std::vector<int>img_height(card_num);
            static std::vector<GLuint>image_textures(card_num);
            static bool show_result = false;
            static bool is_repeat = false;
            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, io.DisplaySize.y));
            ImGui::Begin(is_CH? u8"塔罗选牌" : "Tarot Picker");

            bool num_changed=ImGui::SliderInt(is_CH ? u8"请选择要选择的牌的数目（1~78）" : "How many tarot cards?(1~78):", &card_num,1,78);
            if (num_changed)
            {
                card_idx.clear();
                card_idx = std::vector<int>(card_num,0);
                inverse_or_not = std::vector<int>(card_num);
                img_width= std::vector<int>(card_num);
                img_height= std::vector<int>(card_num);
                image_textures= std::vector<GLuint>(card_num);
            }
            ImGui::Text(is_CH ? u8"塔罗牌选择":"Card picker");
            for (int i = 0; i < card_num; ++i)
            {
                ImGui::SliderInt((std::string(is_CH ? u8"塔罗牌序号选择（0~77）" : "Card index(0~77):")+std::to_string(i+1)).c_str(), &card_idx[i],0,77);
            }

            if (ImGui::Button(is_CH ? u8"我已经选择完毕" : "I've Pick all the cards"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
            {
                std::set<int>tmp_set(card_idx.begin(), card_idx.end());
                if (tmp_set.size() == card_idx.size())
                {
                    show_result = true;
                    is_repeat = false;
                    for (int i = 0; i < card_num; ++i)
                    {
                        inverse_or_not[i] = rand() % 2;
                        std::string img_file = (std::string("Tarot/") + all_arc_en[all_arc_idx[card_idx[i]]] + std::string(".jpeg"));
                        bool ret = LoadTextureFromFile(img_file.c_str(),
                            &image_textures[i], &img_width[i], &img_height[i]);
                    }
                }
                else
                {
                    is_repeat = true;
                }
            }
            ImGui::SameLine();
            if (ImGui::Button(is_CH ? u8"占卜完毕，重新洗牌" : "Done and Reshuffle"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
            {
                show_result = false;
                std::random_shuffle(all_arc_idx.begin(), all_arc_idx.end());
                card_idx = std::vector<int>(card_num, 0);
                inverse_or_not = std::vector<int>(card_num);
                img_width = std::vector<int>(card_num);
                img_height = std::vector<int>(card_num);
                for (int i = 0; i < image_textures.size(); ++i)
                {
                    glDeleteTextures(1, &image_textures[i]);
                }
                image_textures = std::vector<GLuint>(card_num);

            }
            if (is_repeat)
            {
                ImGui::Text(is_CH ? u8"存在重复牌，请重新选牌":"Duplicate cards,Please Check your choice.");
            }
            if (show_result)
            {
                ImGui::Text(is_CH ? u8"结果" : "Result:");
                if (ImGui::BeginTable("table", 3, ImGuiTableFlags_Borders))
                {
                    for (int i = 0; i < card_num; ++i)
                    {
                        std::string upright = is_CH ? u8"正位" : " -> Upright";
                        std::string reverse = is_CH ? u8"逆位" : " -> Reverse";
                        std::string card = all_arc[all_arc_idx[card_idx[i]]];

                        std::string card_order = std::to_string(i + 1);
                        card_order = is_CH ? (std::string(u8"牌") + card_order) :
                            (std::string("card ") + card_order);
                        card_order += std::string(": ");
                        ImVec2 uv0 = ImVec2(0.0f, 0.0f);

                        // Normalized coordinates of pixel (110,210) in a 256x256 texture.
                        ImVec2 uv1 = ImVec2(1.0f, 1.0f);
                        if (inverse_or_not[i])
                        {
                            card += reverse;
                        }
                        else
                        {
                            card += upright;
                        }
                        //ImGui::Text(card_order.c_str());

                        ImGui::TableNextColumn();
                        ImGui::Text(card.c_str());
                    }
                    ImGui::EndTable();
                }
                ImGui::NewLine();
                for (int i = 0; i < card_num; ++i)
                {
           
                    ImVec2 uv0 = ImVec2(0.0f, 0.0f);

                    ImVec2 uv1 = ImVec2(1.0f,1.0f);
                    if (inverse_or_not[i])
                    {
                        ImGui::Image((void*)(intptr_t)image_textures[i], ImVec2(img_width[i], img_height[i]),uv1,uv0);
                    }
                    else
                    {
                        ImGui::Image((void*)(intptr_t)image_textures[i], ImVec2(img_width[i], img_height[i]), uv0, uv1);
                    }
                    if (i % 3 != 2)
                    {
                        ImGui::SameLine();
                    }

                    //ImGui::EndGroup();
                }
                ImGui::NewLine();
            }
            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

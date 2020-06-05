#pragma once

#include <memory>
#include <SDL.h>
#include <imgui.h>
#include "core/ResourcePool.hpp"
#include "core/Shader.hpp"
#include "core/Texture.hpp"

struct GuiController
{
  Uint64      time                                 = 0;
  bool        mousePressed[3]                      = {false, false, false};
  SDL_Cursor* mouseCursors[ImGuiMouseCursor_COUNT] = {0};
  char*       clipboardTextData                    = nullptr;
  ImVec4      clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
  SDL_Window* window;
  Texture     fontTexture;
  std::shared_ptr<ShaderProgram> mShader;
  int attribLocationTex = 0, attribLocationProjMtx = 0;
  int attribLocationPosition = 0, attribLocationUV = 0, attribLocationColor = 0;
  unsigned int vboHandle = 0, elementsHandle = 0;

  GuiController(SDL_Window*   window,
                SDL_GLContext gl_context,
                ResourcePool* pool)
    : window(window)
  {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.BackendFlags |=
      ImGuiBackendFlags_HasMouseCursors; // We can honor GetMouseCursor() values
                                         // (optional)
    io.BackendFlags |=
      ImGuiBackendFlags_HasSetMousePos; // We can honor io.WantSetMousePos
                                        // requests (optional, rarely used)

    // Keyboard mapping. ImGui will use those indices to peek into the
    // io.KeysDown[] array.
    io.KeyMap[ImGuiKey_Tab]        = SDL_SCANCODE_TAB;
    io.KeyMap[ImGuiKey_LeftArrow]  = SDL_SCANCODE_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = SDL_SCANCODE_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow]    = SDL_SCANCODE_UP;
    io.KeyMap[ImGuiKey_DownArrow]  = SDL_SCANCODE_DOWN;
    io.KeyMap[ImGuiKey_PageUp]     = SDL_SCANCODE_PAGEUP;
    io.KeyMap[ImGuiKey_PageDown]   = SDL_SCANCODE_PAGEDOWN;
    io.KeyMap[ImGuiKey_Home]       = SDL_SCANCODE_HOME;
    io.KeyMap[ImGuiKey_End]        = SDL_SCANCODE_END;
    io.KeyMap[ImGuiKey_Insert]     = SDL_SCANCODE_INSERT;
    io.KeyMap[ImGuiKey_Delete]     = SDL_SCANCODE_DELETE;
    io.KeyMap[ImGuiKey_Backspace]  = SDL_SCANCODE_BACKSPACE;
    io.KeyMap[ImGuiKey_Space]      = SDL_SCANCODE_SPACE;
    io.KeyMap[ImGuiKey_Enter]      = SDL_SCANCODE_RETURN;
    io.KeyMap[ImGuiKey_Escape]     = SDL_SCANCODE_ESCAPE;
    io.KeyMap[ImGuiKey_A]          = SDL_SCANCODE_A;
    io.KeyMap[ImGuiKey_C]          = SDL_SCANCODE_C;
    io.KeyMap[ImGuiKey_V]          = SDL_SCANCODE_V;
    io.KeyMap[ImGuiKey_X]          = SDL_SCANCODE_X;
    io.KeyMap[ImGuiKey_Y]          = SDL_SCANCODE_Y;
    io.KeyMap[ImGuiKey_Z]          = SDL_SCANCODE_Z;

    io.SetClipboardTextFn = setClipboardText;
    io.GetClipboardTextFn = getClipboardText;
    io.ClipboardUserData  = this;

    mouseCursors[ImGuiMouseCursor_Arrow] =
      SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
    mouseCursors[ImGuiMouseCursor_TextInput] =
      SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);
    mouseCursors[ImGuiMouseCursor_ResizeAll] =
      SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEALL);
    mouseCursors[ImGuiMouseCursor_ResizeNS] =
      SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS);
    mouseCursors[ImGuiMouseCursor_ResizeEW] =
      SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE);
    mouseCursors[ImGuiMouseCursor_ResizeNESW] =
      SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENESW);
    mouseCursors[ImGuiMouseCursor_ResizeNWSE] =
      SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENWSE);
    mouseCursors[ImGuiMouseCursor_Hand] =
      SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);

#ifdef _WIN32
    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    SDL_GetWindowWMInfo(window, &wmInfo);
    io.ImeWindowHandle = wmInfo.info.win.window;
#else
    (void)window;
#endif
    ImGui::StyleColorsDark();
    mShader               = pool->getShaderProgram("imgui_330");
    attribLocationTex     = mShader->getUniformLocation("Texture");
    attribLocationProjMtx = mShader->getUniformLocation("ProjMtx");
    attribLocationPosition =
      glGetAttribLocation(mShader->shaderProgramId(), "Position");
    attribLocationUV = glGetAttribLocation(mShader->shaderProgramId(), "UV");
    attribLocationColor =
      glGetAttribLocation(mShader->shaderProgramId(), "Color");

    // Create buffers
    glGenBuffers(1, &vboHandle);
    glGenBuffers(1, &elementsHandle);

    unsigned char* pixels;
    int            width, height;
    io.Fonts->GetTexDataAsRGBA32(
      &pixels,
      &width,
      &height); // Load as RGBA 32-bits (75% of the memory is wasted, but
                // default font is so small) because it is more likely to be
                // compatible with user's existing shaders. If your ImTextureId
                // represent a higher-level concept than just a GL texture id,
                // consider calling GetTexDataAsAlpha8() instead to save on GPU
                // memory.
    fontTexture.updateTexture(pixels, width, height);
    io.Fonts->TexID = (ImTextureID)(intptr_t)fontTexture.textureId();
  }

  static const char* getClipboardText(void* userData)
  {
    GuiController* self = static_cast<GuiController*>(userData);
    if (self->clipboardTextData)
      SDL_free(self->clipboardTextData);
    self->clipboardTextData = SDL_GetClipboardText();
    return self->clipboardTextData;
  }

  static void setClipboardText(void*, const char* text)
  {
    SDL_SetClipboardText(text);
  }

  bool processEvent(const SDL_Event& event)
  {
    ImGuiIO& io = ImGui::GetIO();
    switch (event.type) {
      case SDL_MOUSEWHEEL: {
        if (event.wheel.x > 0)
          io.MouseWheelH += 1;
        if (event.wheel.x < 0)
          io.MouseWheelH -= 1;
        if (event.wheel.y > 0)
          io.MouseWheel += 1;
        if (event.wheel.y < 0)
          io.MouseWheel -= 1;
        return io.WantCaptureMouse;
      }
      case SDL_MOUSEBUTTONDOWN: {
        if (event.button.button == SDL_BUTTON_LEFT)
          mousePressed[0] = true;
        if (event.button.button == SDL_BUTTON_RIGHT)
          mousePressed[1] = true;
        if (event.button.button == SDL_BUTTON_MIDDLE)
          mousePressed[2] = true;
        return io.WantCaptureMouse;
      }
      case SDL_MOUSEBUTTONUP:
      case SDL_MOUSEMOTION:
        return io.WantCaptureMouse;
      case SDL_TEXTINPUT: {
        io.AddInputCharactersUTF8(event.text.text);
        return io.WantCaptureKeyboard;
      }
      case SDL_KEYDOWN:
      case SDL_KEYUP: {
        int key = event.key.keysym.scancode;
        IM_ASSERT(key >= 0 && key < IM_ARRAYSIZE(io.KeysDown));
        io.KeysDown[key] = (event.type == SDL_KEYDOWN);
        io.KeyShift      = ((SDL_GetModState() & KMOD_SHIFT) != 0);
        io.KeyCtrl       = ((SDL_GetModState() & KMOD_CTRL) != 0);
        io.KeyAlt        = ((SDL_GetModState() & KMOD_ALT) != 0);
        io.KeySuper      = ((SDL_GetModState() & KMOD_GUI) != 0);
        return io.WantCaptureKeyboard;
      }
    }
    return false;
  }

  void newFrame()
  {
    ImGuiIO& io = ImGui::GetIO();
    IM_ASSERT(
      io.Fonts
        ->IsBuilt()); // Font atlas needs to be built, call renderer _NewFrame()
                      // function e.g. ImGui_ImplOpenGL3_NewFrame()

    // Setup display size (every frame to accommodate for window resizing)
    int w, h;
    int display_w, display_h;
    SDL_GetWindowSize(window, &w, &h);
    SDL_GL_GetDrawableSize(window, &display_w, &display_h);
    io.DisplaySize             = ImVec2((float)w, (float)h);
    io.DisplayFramebufferScale = ImVec2(w > 0 ? ((float)display_w / w) : 0,
                                        h > 0 ? ((float)display_h / h) : 0);

    // Setup time step (we don't use SDL_GetTicks() because it is using
    // millisecond resolution)
    static Uint64 frequency    = SDL_GetPerformanceFrequency();
    Uint64        current_time = SDL_GetPerformanceCounter();
    io.DeltaTime = time > 0 ? (float)((double)(current_time - time) / frequency)
                            : (float)(1.0f / 60.0f);
    time = current_time;

    updateMousePosAndButtons();
    updateMouseCursor();

    ImGui::NewFrame();
  }

  void render()
  {
    ImGui::Render();
    auto draw_data = ImGui::GetDrawData();
    // Avoid rendering when minimized, scale coordinates for retina displays
    // (screen coordinates != framebuffer coordinates)
    ImGuiIO& io = ImGui::GetIO();
    int      fb_width =
      (int)(draw_data->DisplaySize.x * io.DisplayFramebufferScale.x);
    int fb_height =
      (int)(draw_data->DisplaySize.y * io.DisplayFramebufferScale.y);
    if (fb_width <= 0 || fb_height <= 0)
      return;
    draw_data->ScaleClipRects(io.DisplayFramebufferScale);

    // Backup GL state
    GLenum last_active_texture;
    glGetIntegerv(GL_ACTIVE_TEXTURE, (GLint*)&last_active_texture);
    glActiveTexture(GL_TEXTURE0);
#ifdef GL_SAMPLER_BINDING
    GLint last_sampler;
    glGetIntegerv(GL_SAMPLER_BINDING, &last_sampler);
#endif
    GLint last_array_buffer;
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
    GLint last_vertex_array;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);
#ifdef GL_POLYGON_MODE
    GLint last_polygon_mode[2];
    glGetIntegerv(GL_POLYGON_MODE, last_polygon_mode);
#endif
    GLint last_viewport[4];
    glGetIntegerv(GL_VIEWPORT, last_viewport);
    GLint last_scissor_box[4];
    glGetIntegerv(GL_SCISSOR_BOX, last_scissor_box);
    GLenum last_blend_src_rgb;
    glGetIntegerv(GL_BLEND_SRC_RGB, (GLint*)&last_blend_src_rgb);
    GLenum last_blend_dst_rgb;
    glGetIntegerv(GL_BLEND_DST_RGB, (GLint*)&last_blend_dst_rgb);
    GLenum last_blend_src_alpha;
    glGetIntegerv(GL_BLEND_SRC_ALPHA, (GLint*)&last_blend_src_alpha);
    GLenum last_blend_dst_alpha;
    glGetIntegerv(GL_BLEND_DST_ALPHA, (GLint*)&last_blend_dst_alpha);
    GLenum last_blend_equation_rgb;
    glGetIntegerv(GL_BLEND_EQUATION_RGB, (GLint*)&last_blend_equation_rgb);
    GLenum last_blend_equation_alpha;
    glGetIntegerv(GL_BLEND_EQUATION_ALPHA, (GLint*)&last_blend_equation_alpha);
    GLboolean last_enable_blend        = glIsEnabled(GL_BLEND);
    GLboolean last_enable_cull_face    = glIsEnabled(GL_CULL_FACE);
    GLboolean last_enable_depth_test   = glIsEnabled(GL_DEPTH_TEST);
    GLboolean last_enable_scissor_test = glIsEnabled(GL_SCISSOR_TEST);

    // Setup render state: alpha-blending enabled, no face culling, no depth
    // testing, scissor enabled, polygon fill
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);
#ifdef GL_POLYGON_MODE
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#endif

    // Setup viewport, orthographic projection matrix
    // Our visible imgui space lies from draw_data->DisplayPos (top left) to
    // draw_data->DisplayPos+data_data->DisplaySize (bottom right). DisplayMin
    // is typically (0,0) for single viewport apps.
    glViewport(0, 0, (GLsizei)fb_width, (GLsizei)fb_height);
    float       L = draw_data->DisplayPos.x;
    float       R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
    float       T = draw_data->DisplayPos.y;
    float       B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;
    const float ortho_projection[4][4] = {
      {2.0f / (R - L), 0.0f, 0.0f, 0.0f},
      {0.0f, 2.0f / (T - B), 0.0f, 0.0f},
      {0.0f, 0.0f, -1.0f, 0.0f},
      {(R + L) / (L - R), (T + B) / (B - T), 0.0f, 1.0f},
    };
    glUseProgram(mShader->shaderProgramId());
    glUniform1i(attribLocationTex, 0);
    glUniformMatrix4fv(
      attribLocationProjMtx, 1, GL_FALSE, &ortho_projection[0][0]);
#ifdef GL_SAMPLER_BINDING
    glBindSampler(0, 0); // We use combined texture/sampler state. Applications
                         // using GL 3.3 may set that otherwise.
#endif
    // Recreate the VAO every time
    // (This is to easily allow multiple GL contexts. VAO are not shared among
    // GL contexts, and we don't track creation/deletion of windows so we don't
    // have an obvious key to use to cache them.)
    GLuint vao_handle = 0;
    glGenVertexArrays(1, &vao_handle);
    glBindVertexArray(vao_handle);
    glBindBuffer(GL_ARRAY_BUFFER, vboHandle);
    glEnableVertexAttribArray(attribLocationPosition);
    glEnableVertexAttribArray(attribLocationUV);
    glEnableVertexAttribArray(attribLocationColor);
    glVertexAttribPointer(attribLocationPosition,
                          2,
                          GL_FLOAT,
                          GL_FALSE,
                          sizeof(ImDrawVert),
                          (GLvoid*)IM_OFFSETOF(ImDrawVert, pos));
    glVertexAttribPointer(attribLocationUV,
                          2,
                          GL_FLOAT,
                          GL_FALSE,
                          sizeof(ImDrawVert),
                          (GLvoid*)IM_OFFSETOF(ImDrawVert, uv));
    glVertexAttribPointer(attribLocationColor,
                          4,
                          GL_UNSIGNED_BYTE,
                          GL_TRUE,
                          sizeof(ImDrawVert),
                          (GLvoid*)IM_OFFSETOF(ImDrawVert, col));

    // Draw
    ImVec2 pos = draw_data->DisplayPos;
    for (int n = 0; n < draw_data->CmdListsCount; n++) {
      const ImDrawList* cmd_list          = draw_data->CmdLists[n];
      const ImDrawIdx*  idx_buffer_offset = 0;

      glBindBuffer(GL_ARRAY_BUFFER, vboHandle);
      glBufferData(GL_ARRAY_BUFFER,
                   (GLsizeiptr)cmd_list->VtxBuffer.Size * sizeof(ImDrawVert),
                   (const GLvoid*)cmd_list->VtxBuffer.Data,
                   GL_STREAM_DRAW);

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementsHandle);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                   (GLsizeiptr)cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx),
                   (const GLvoid*)cmd_list->IdxBuffer.Data,
                   GL_STREAM_DRAW);

      for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
        const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
        if (pcmd->UserCallback) {
          // User callback (registered via ImDrawList::AddCallback)
          pcmd->UserCallback(cmd_list, pcmd);
        } else {
          ImVec4 clip_rect = ImVec4(pcmd->ClipRect.x - pos.x,
                                    pcmd->ClipRect.y - pos.y,
                                    pcmd->ClipRect.z - pos.x,
                                    pcmd->ClipRect.w - pos.y);
          if (clip_rect.x < fb_width && clip_rect.y < fb_height &&
              clip_rect.z >= 0.0f && clip_rect.w >= 0.0f) {
            // Apply scissor/clipping rectangle
            glScissor((int)clip_rect.x,
                      (int)(fb_height - clip_rect.w),
                      (int)(clip_rect.z - clip_rect.x),
                      (int)(clip_rect.w - clip_rect.y));

            // Bind texture, Draw
            glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->TextureId);
            glDrawElements(GL_TRIANGLES,
                           (GLsizei)pcmd->ElemCount,
                           sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT
                                                  : GL_UNSIGNED_INT,
                           idx_buffer_offset);
          }
        }
        idx_buffer_offset += pcmd->ElemCount;
      }
    }
    glDeleteVertexArrays(1, &vao_handle);

    // Restore modified GL state
    glBlendEquationSeparate(last_blend_equation_rgb, last_blend_equation_alpha);
    glBlendFuncSeparate(last_blend_src_rgb,
                        last_blend_dst_rgb,
                        last_blend_src_alpha,
                        last_blend_dst_alpha);
    if (last_enable_blend)
      glEnable(GL_BLEND);
    else
      glDisable(GL_BLEND);
    if (last_enable_cull_face)
      glEnable(GL_CULL_FACE);
    else
      glDisable(GL_CULL_FACE);
    if (last_enable_depth_test)
      glEnable(GL_DEPTH_TEST);
    else
      glDisable(GL_DEPTH_TEST);
    if (last_enable_scissor_test)
      glEnable(GL_SCISSOR_TEST);
    else
      glDisable(GL_SCISSOR_TEST);
#ifdef GL_POLYGON_MODE
    glPolygonMode(GL_FRONT_AND_BACK, (GLenum)last_polygon_mode[0]);
#endif
    glViewport(last_viewport[0],
               last_viewport[1],
               (GLsizei)last_viewport[2],
               (GLsizei)last_viewport[3]);
    glScissor(last_scissor_box[0],
              last_scissor_box[1],
              (GLsizei)last_scissor_box[2],
              (GLsizei)last_scissor_box[3]);
  }

  void updateMousePosAndButtons()
  {
    ImGuiIO& io = ImGui::GetIO();

    // Set OS mouse position if requested (rarely used, only when
    // ImGuiConfigFlags_NavEnableSetMousePos is enabled by user)
    if (io.WantSetMousePos)
      SDL_WarpMouseInWindow(window, (int)io.MousePos.x, (int)io.MousePos.y);
    else
      io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);

    int    mx, my;
    Uint32 mouse_buttons = SDL_GetMouseState(&mx, &my);
    io.MouseDown[0] =
      mousePressed[0] ||
      (mouse_buttons & SDL_BUTTON(SDL_BUTTON_LEFT)) !=
        0; // If a mouse press event came, always pass it as "mouse held this
           // frame", so we don't miss click-release events that are shorter
           // than 1 frame.
    io.MouseDown[1] =
      mousePressed[1] || (mouse_buttons & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0;
    io.MouseDown[2] =
      mousePressed[2] || (mouse_buttons & SDL_BUTTON(SDL_BUTTON_MIDDLE)) != 0;
    mousePressed[0] = mousePressed[1] = mousePressed[2] = false;

#if SDL_HAS_CAPTURE_MOUSE && !defined(__EMSCRIPTEN__)
    SDL_Window* focused_window = SDL_GetKeyboardFocus();
    if (window == focused_window) {
      // SDL_GetMouseState() gives mouse position seemingly based on the last
      // window entered/focused(?) The creation of a new windows at runtime and
      // SDL_CaptureMouse both seems to severely mess up with that, so we
      // retrieve that position globally.s
      int wx, wy;
      SDL_GetWindowPosition(focused_window, &wx, &wy);
      SDL_GetGlobalMouseState(&mx, &my);
      mx -= wx;
      my -= wy;
      io.MousePos = ImVec2((float)mx, (float)my);
    }

    // SDL_CaptureMouse() let the OS know e.g. that our imgui drag outside the
    // SDL window boundaries shouldn't e.g. trigger the OS window resize cursor.
    // The function is only supported from SDL 2.0.4 (released Jan 2016)
    bool any_mouse_button_down = ImGui::IsAnyMouseDown();
    SDL_CaptureMouse(any_mouse_button_down ? SDL_TRUE : SDL_FALSE);
#else
    if (SDL_GetWindowFlags(window) & SDL_WINDOW_INPUT_FOCUS)
      io.MousePos = ImVec2((float)mx, (float)my);
#endif
  }

  void updateMouseCursor()
  {
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange)
      return;

    ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
    if (io.MouseDrawCursor || imgui_cursor == ImGuiMouseCursor_None) {
      // Hide OS mouse cursor if imgui is drawing it or if it wants no cursor
      SDL_ShowCursor(SDL_FALSE);
    } else {
      // Show OS mouse cursor
      SDL_SetCursor(mouseCursors[imgui_cursor]
                      ? mouseCursors[imgui_cursor]
                      : mouseCursors[ImGuiMouseCursor_Arrow]);
      SDL_ShowCursor(SDL_TRUE);
    }
  }
};

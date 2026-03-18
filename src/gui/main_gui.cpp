#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include "obj_handler/obj_output.hpp"
#include "obj_handler/obj_parser.hpp"
#include "raylib.h"
#include "voxelizer/octree.hpp"

// Helper untuk melakukan log ke terminal
static void appendLog(std::vector<std::string> &logs, const std::string &line) {
    std::cout << line << std::endl;
    logs.push_back(line);
    if (logs.size() > 500) {
        logs.erase(logs.begin());
    }
}

// Helper untuk setup kamera dari viewfinder
static void fitCameraToModel(Camera3D &camera, const std::vector<Vertex> &vertices, Vector3 &outCenter, float &outSpan, float &outBottomY) {
    if (vertices.empty()) {
        camera.target = {0.0f, 0.0f, 0.0f};
        camera.position = {6.0f, 6.0f, 6.0f};
        outCenter = {0.0f, 0.0f, 0.0f};
        outSpan = 10.0f;
        outBottomY = 0.0f;
        return;
    }

    Vertex minV = vertices[0];
    Vertex maxV = vertices[0];
    for (size_t i = 1; i < vertices.size(); i++) {
        const Vertex &v = vertices[i];
        if (v.x < minV.x) minV.x = v.x;
        if (v.y < minV.y) minV.y = v.y;
        if (v.z < minV.z) minV.z = v.z;

        if (v.x > maxV.x) maxV.x = v.x;
        if (v.y > maxV.y) maxV.y = v.y;
        if (v.z > maxV.z) maxV.z = v.z;
    }

    Vector3 center = {(minV.x + maxV.x) * 0.5f, (minV.y + maxV.y) * 0.5f, (minV.z + maxV.z) * 0.5f};
    float spanX = maxV.x - minV.x;
    float spanY = maxV.y - minV.y;
    float spanZ = maxV.z - minV.z;

    float span = spanX;
    if (spanY > span) span = spanY;
    if (spanZ > span) span = spanZ;
    if (span < 0.1f) span = 1.0f;

    camera.target = center;
    camera.position = {center.x + span * 1.8f, center.y + span * 1.2f, center.z + span * 1.8f};
    
    outCenter = center;
    outSpan = span;
    outBottomY = minV.y;
}

// Helper untuk load OBJ ke Viewfinder
static bool loadModelForPreview(const std::string &path, std::vector<Vertex> &showVertices, std::vector<Face> &showFaces, Camera3D &camera, Vector3 &modCenter, float &modSpan, float &modBottomY) {
    OBJ obj;
    if (!obj.loadOBJ(path)) {
        return false;
    }

    showVertices = obj.getVertices();
    showFaces = obj.getFaces();
    fitCameraToModel(camera, showVertices, modCenter, modSpan, modBottomY);
    return true;
}

// Helper untuk menggambar tombol interaktif
static bool drawButton(Rectangle r, const char *text, Color bg) {
    Vector2 mouse = GetMousePosition();
    bool hover = CheckCollisionPointRec(mouse, r);

    Color drawColor = bg;
    if (hover) drawColor = Color{(unsigned char)(bg.r + 18), (unsigned char)(bg.g + 18), (unsigned char)(bg.b + 18), 255};

    DrawRectangleRec(r, drawColor);
    DrawRectangleLinesEx(r, 1.0f, Color{20, 20, 20, 120});

    int tw = MeasureText(text, 18);
    DrawText(text, (int)(r.x + (r.width - tw) * 0.5f), (int)(r.y + 10), 18, WHITE);

    return hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}

// Helper untuk menggambar kotak input teks interaktif untuk input path dan depth
static void drawTextBox(const char *label, Rectangle r, std::string &value, bool &active, int maxChars, Color panelBg) {
    DrawText(label, (int)r.x, (int)r.y - 20, 18, WHITE);

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) active = CheckCollisionPointRec(GetMousePosition(), r);

    if (active) {
        // Handle untuk input karakter biasa
        int key = GetCharPressed();
        while (key > 0) {
            if (key >= 32 && key <= 126) {
                if ((int)value.size() < maxChars) value.push_back((char)key);
            }
            key = GetCharPressed();
        }

        // Handle untuk paste dari clipboard
        if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) {
            if (IsKeyPressed(KEY_V)) {
                const char* clipboard = GetClipboardText();
                if (clipboard != nullptr) {
                    std::string pasted(clipboard);
                    for (int i=0; i<(int)pasted.size(); i++) {
                        char c = pasted[i];
                        if (c >= 32 && c <= 126 && (int)value.size() < maxChars) value.push_back(c);
                    }
                }
            }
        }

        // Handle untuk backspace
        if ((IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE)) && !value.empty()) value.pop_back();
    }

    DrawRectangleRec(r, panelBg);
    DrawRectangleLinesEx(r, 1.0f, active ? SKYBLUE : GRAY);

    std::string shown = value;
    int maxTextPx = (int)r.width - 16;
    while (!shown.empty() && MeasureText(shown.c_str(), 18) > maxTextPx) shown.erase(shown.begin());
    DrawText(shown.c_str(), (int)r.x + 8, (int)r.y + 8, 18, WHITE);
}

// Helper untuk kontrol kamera viewfinder dengan mouse
static void handleCameraControls(Camera3D &camera, Rectangle viewfinder) {
    Vector2 mouse = GetMousePosition();
    bool mouseInView = CheckCollisionPointRec(mouse, viewfinder);

    // Bagian untuk mengatur rotasi pergerakan mouse
    if (mouseInView && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        Vector2 delta = GetMouseDelta();
        float rotSpeed = 0.003f;

        Vector3 diff = {camera.position.x - camera.target.x, camera.position.y - camera.target.y, camera.position.z - camera.target.z};
        float dist = sqrtf(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);

        float yaw = atan2f(diff.x, diff.z);
        float pitch = asinf(diff.y / dist);

        yaw -= delta.x * rotSpeed;
        pitch -= delta.y * rotSpeed;

        if (pitch > 1.5f) pitch = 1.5f;
        if (pitch < -1.5f) pitch = -1.5f;

        camera.position.x = camera.target.x + dist * cosf(pitch) * sinf(yaw);
        camera.position.y = camera.target.y + dist * sinf(pitch);
        camera.position.z = camera.target.z + dist * cosf(pitch) * cosf(yaw);
    }

    // Bagian untuk mengatur pergerakan dari scroll wheel mouse
    if (mouseInView) {
        float wheel = GetMouseWheelMove();
        if (wheel != 0.0f) {
            Vector3 diff = {camera.position.x - camera.target.x, camera.position.y - camera.target.y, camera.position.z - camera.target.z};
            float dist = sqrtf(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);
            
            dist -= wheel * dist * 0.1f;
            if (dist < 1.0f) dist = 1.0f;

            float scale = dist / sqrtf(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);
            camera.position.x = camera.target.x + diff.x * scale;
            camera.position.y = camera.target.y + diff.y * scale;
            camera.position.z = camera.target.z + diff.z * scale;
        }
    }
}

// Helper untuk mengatur rendering ketiga terjadi resizing windows
static void updateRenderTexture(RenderTexture2D &viewRt, int &rtW, int &rtH, int targetW, int targetH) {
    if (targetW < 64) targetW = 64;
    if (targetH < 64) targetH = 64;

    if (targetW != rtW || targetH != rtH) {
        UnloadRenderTexture(viewRt);
        viewRt = LoadRenderTexture(targetW, targetH);
        rtW = targetW;
        rtH = targetH;
    }
}

// Draws a dark bounding grid on the ground plane (Y=0)
static void drawGrid3D(int slices, float spacing, Vector3 center, float bottomY) {
    float half = (float)slices * spacing * 0.5f;
    for (int i = -slices; i <= slices; i++) {
        DrawLine3D({center.x + (float)i * spacing, bottomY, center.z - half},
                   {center.x + (float)i * spacing, bottomY, center.z + half},
                   BLACK);
        DrawLine3D({center.x - half, bottomY, center.z + (float)i * spacing},
                   {center.x + half, bottomY, center.z + (float)i * spacing},
                   BLACK);
    }
}

// Menggambar 3d model dengan shading dari lightning
static void drawModel3D(const std::vector<Vertex> &vertices, const std::vector<Face> &faces) {
    Vector3 lightDir = {0.4f, 0.7f, 0.5f};
    float lightLen = sqrtf(lightDir.x*lightDir.x + lightDir.y*lightDir.y + lightDir.z*lightDir.z);
    lightDir.x /= lightLen; lightDir.y /= lightLen; lightDir.z /= lightLen;

    for (size_t i = 0; i < faces.size(); i++) {
        const Face &f = faces[i];
        if (f.v1 < 0 || f.v2 < 0 || f.v3 < 0) continue;
        if (f.v1 >= (int)vertices.size() || f.v2 >= (int)vertices.size() || f.v3 >= (int)vertices.size()) continue;

        Vector3 a = {vertices[f.v1].x, vertices[f.v1].y, vertices[f.v1].z};
        Vector3 b = {vertices[f.v2].x, vertices[f.v2].y, vertices[f.v2].z};
        Vector3 c = {vertices[f.v3].x, vertices[f.v3].y, vertices[f.v3].z};

        Vector3 ab = {b.x - a.x, b.y - a.y, b.z - a.z};
        Vector3 ac = {c.x - a.x, c.y - a.y, c.z - a.z};
        Vector3 normal = {
            ab.y * ac.z - ab.z * ac.y,
            ab.z * ac.x - ab.x * ac.z,
            ab.x * ac.y - ab.y * ac.x
        };
        float nLen = sqrtf(normal.x*normal.x + normal.y*normal.y + normal.z*normal.z);
        if (nLen > 0.0001f) {
            normal.x /= nLen; 
            normal.y /= nLen; 
            normal.z /= nLen;
        }

        float dot = normal.x*lightDir.x + normal.y*lightDir.y + normal.z*lightDir.z;
        if (dot < 0.0f) dot = -dot;

        float brightness = 0.3f + 0.7f * dot;
        unsigned char shade = (unsigned char)(brightness * 255.0f);
        Color faceColor = {shade, shade, shade, 255};

        DrawTriangle3D(a, b, c, faceColor);
        DrawTriangle3D(a, c, b, faceColor);
    }
}

// Helper file name parsing
static std::string getFileName(const std::string &path) {
    size_t pos = path.find_last_of("/\\");
    if (pos == std::string::npos) {
        size_t pos = path.find_last_of("\\\\");
    }
    if (pos == std::string::npos) return path;
    return path.substr(pos + 1);
}

// Inisiasi dan Loop utama dari program
int main() {

    // Setup utama window
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(1366, 820, "Voxelizer");
    SetTargetFPS(60);

    const int leftPanelWidth = 360;

    // Inisiasi placeholder text
    std::string inputPath = "test/test3.obj";
    std::string outputPath = "output/output.obj";
    std::string depthText = "5";
    std::string shownModelPath = "(none)";

    // Variable untuk viewfinder dan menu
    Vector3 modCenter = {0.0f, 0.0f, 0.0f};
    float modSpan = 10.0f;
    float modBottomY = 0.0f;

    bool inputActive = false;
    bool outputActive = false;
    bool depthActive = false;

    // Logging data untuk preview
    std::vector<std::string> logs;
    appendLog(logs, "GUI sudah diinisasi!");
    int logScrollOffset = 0;

    std::vector<Vertex> showVertices;
    std::vector<Face> showFaces;

    std::vector<Vertex> voxelVertices;
    std::vector<Face> voxelFaces;

    // Konfigurasi kamera
    Camera3D camera = {};
    camera.position = {6.0f, 6.0f, 6.0f};
    camera.target = {0.0f, 0.0f, 0.0f};
    camera.up = {0.0f, 1.0f, 0.0f};
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    // Inisiasi framebuffer untuk logging text
    int initW = GetScreenWidth() - leftPanelWidth - 16;
    int initH = GetScreenHeight() - 16;
    if (initW < 64) initW = 64;
    if (initH < 64) initH = 64;
    RenderTexture2D viewRt = LoadRenderTexture(initW, initH);
    int rtW = initW;
    int rtH = initH;

    // Loop utama dari program
    while (!WindowShouldClose()) {
        
        // Setup untuk ukuran window
        int windowW = GetScreenWidth();
        int windowH = GetScreenHeight();
        
        Rectangle viewfinder = {(float)leftPanelWidth + 8.0f, 8.0f, (float)(windowW - leftPanelWidth - 16), (float)(windowH - 16)};

        updateRenderTexture(viewRt, rtW, rtH, (int)viewfinder.width, (int)viewfinder.height);

        handleCameraControls(camera, viewfinder);

        // Setup rendering dari viewfinder
        BeginTextureMode(viewRt);
        ClearBackground(Color{60, 60, 60, 255}); 
        BeginMode3D(camera);
        
        float gridSpacing = modSpan / 10.0f;
        if (gridSpacing < 0.001f) gridSpacing = 1.0f;
        float gridY = modBottomY - (modSpan * 0.05f); 
        drawGrid3D(20, gridSpacing, modCenter, gridY);

        drawModel3D(showVertices, showFaces);
        
        EndMode3D();
        EndTextureMode();

        BeginDrawing();
        ClearBackground(Color{9, 20, 19, 255});

        // Melakukan inisasi bagian viewfinder
        Rectangle src = {0.0f, 0.0f, (float)viewRt.texture.width, -(float)viewRt.texture.height};
        DrawTexturePro(viewRt.texture, src, viewfinder, Vector2{0.0f, 0.0f}, 0.0f, WHITE);
        
        std::string vfTitle = "Viewfinder: " + getFileName(shownModelPath);
        DrawText(vfTitle.c_str(), (int)viewfinder.x + 12, (int)viewfinder.y + 10, 20, WHITE);

        // Melakukan inisasi bagian panel kiri
        Rectangle leftPanel = {0.0f, 0.0f, (float)leftPanelWidth, (float)windowH};
        Color panelBg = Color{68, 68, 78, 255};
        DrawRectangleRec(leftPanel, Color{9, 20, 19, 255});
        DrawLine(leftPanelWidth, 0, leftPanelWidth, windowH, Color{210, 214, 220, 255});

        DrawText("Voxelizer", 16, 18, 28, WHITE);
        DrawText("by renuno-frinardi", 16, 52, 18, WHITE);

        drawTextBox("Input Path", Rectangle{16, 114, (float)leftPanelWidth - 32, 34}, inputPath, inputActive, 260, panelBg);
        drawTextBox("Output Path", Rectangle{16, 186, (float)leftPanelWidth - 32, 34}, outputPath, outputActive, 260, panelBg);

        drawTextBox("Depth", Rectangle{16, 258, 120, 34}, depthText, depthActive, 4, panelBg);

        bool clickLoad = drawButton(Rectangle{16, 310, 156, 38}, "Render Input", Color{63, 111, 214, 255});
        bool clickProcess = drawButton(Rectangle{188, 310, 156, 38}, "Process Input", Color{67, 170, 139, 255});

        // Memproses event ketika tombol "Render Input" diklik
        if (clickLoad) {
            logs.clear();
            logScrollOffset = 0;
            if (!loadModelForPreview(inputPath, showVertices, showFaces, camera, modCenter, modSpan, modBottomY)) appendLog(logs, "[ERROR] Gagal melakukan load input OBJ.");
            else {
                shownModelPath = inputPath;
                appendLog(logs, "[LOG] OBJ berhasil dimuat untuk preview!");
                appendLog(logs, "[LOG] Vertices: " + std::to_string(showVertices.size()));
                appendLog(logs, "[LOG] Faces: " + std::to_string(showFaces.size()));
            }
        }

        // Memproses event ketika tombol "Process Input" diklik
        if (clickProcess) {
            logs.clear();
            logScrollOffset = 0;
            int depth = std::atoi(depthText.c_str());

            OBJ obj;
            if (!obj.loadOBJ(inputPath)) appendLog(logs, "[ERROR] Gagal melakukan load input OBJ.");
            else {
                appendLog(logs, "[LOG] Memproses vokselisasi...");

                Octree tree(obj.getVertices(), obj.getFaces(), depth);
                tree.build();

                appendLog(logs, "[LOG] Octree berhasil dibuat");

                voxelVertices.clear();
                voxelFaces.clear();
                tree.generateVoxelMesh(voxelVertices, voxelFaces);

                appendLog(logs, "[LOG] Voxels terbentuk: " + std::to_string(tree.getTotalVoxels()));
                appendLog(logs, "[LOG] Vertex terbentuk: " + std::to_string(voxelVertices.size()));
                appendLog(logs, "[LOG] Face terbentuk: " + std::to_string(voxelFaces.size()));
                appendLog(logs, "[LOG] Depth: " + std::to_string(depth));
                if (tree.getTimeTakenMs() >= 1000) {
                    appendLog(logs, "[LOG] Waktu yang dibutuhkan: " + std::to_string(tree.getTimeTakenMs() / 1000) + " detik");
                } 
                else appendLog(logs, "[LOG] Waktu yang dibutuhkan: " + std::to_string(tree.getTimeTakenMs()) + " milidetik");
                for (int i = 1; i <= depth; i++) {
                    appendLog(logs, "[LOG] " + std::to_string(i) + ": " + "Terbentuk " + std::to_string(tree.getNodeCountAtDepth(i)) + " buah node");
                }
                for (int i = 1; i <= depth; i++) {
                    appendLog(logs, "[LOG] " + std::to_string(i) + ": " + std::to_string(tree.getLeafCountAtDepth(i)) + " buah node tidak ditelusuri");
                }
                appendLog(logs, "-----------------------------------");

                bool ok = OBJOutput::writeOBJ(outputPath, voxelVertices, voxelFaces);
                if (!ok) appendLog(logs, "[ERROR] Gagal memuat output OBJ ke: " + outputPath);
                else {
                    appendLog(logs, "[LOG] Success! File written to: " + outputPath);
                    if (loadModelForPreview(outputPath, showVertices, showFaces, camera, modCenter, modSpan, modBottomY)) shownModelPath = outputPath;
                }
            }
        }

        DrawText("Processing Log", 16, 368, 20, WHITE);
        Rectangle logArea = {16.0f, 396.0f, (float)(leftPanelWidth - 32), (float)(windowH - 412)};
        DrawRectangle(16, 396, leftPanelWidth - 32, windowH - 412, panelBg);
        DrawRectangleLines(16, 396, leftPanelWidth - 32, windowH - 412, BLACK);

        int maxLogWidth = leftPanelWidth - 48;
        int textHeight = 16;
        int lineSpacing = 4;
        std::vector<std::string> wrappedLogs;

        for (size_t i = 0; i < logs.size(); i++) {
            std::string currentLine = logs[i];
            
            while (!currentLine.empty() && MeasureText(currentLine.c_str(), textHeight) > maxLogWidth) {
                int cutLen = currentLine.length();
                while (cutLen > 0 && MeasureText(currentLine.substr(0, cutLen).c_str(), textHeight) > maxLogWidth) cutLen--;
                if (cutLen == 0) cutLen = 1;
                
                wrappedLogs.push_back(currentLine.substr(0, cutLen));
                currentLine = currentLine.substr(cutLen);
            }
            if (!currentLine.empty()) wrappedLogs.push_back(currentLine);
        }

        int logAreaHeight = windowH - 412 - 8;
        int maxLines = logAreaHeight / (textHeight + lineSpacing);
        
        Vector2 mouse = GetMousePosition();
        bool mouseInLog = CheckCollisionPointRec(mouse, logArea);
        if (mouseInLog) {
            float wheel = GetMouseWheelMove();
            if (wheel != 0.0f) {
                logScrollOffset -= (int)(wheel * (textHeight + lineSpacing));
                if (logScrollOffset < 0) logScrollOffset = 0;
                int maxScroll = (int)wrappedLogs.size() * (textHeight + lineSpacing) - logAreaHeight;
                if (maxScroll < 0) maxScroll = 0;
                if (logScrollOffset > maxScroll) logScrollOffset = maxScroll;
            }
        }
        
        int startIndex = logScrollOffset / (textHeight + lineSpacing);
        if (startIndex >= (int)wrappedLogs.size()) startIndex = (int)wrappedLogs.size() - 1;
        if (startIndex < 0) startIndex = 0;

        BeginScissorMode(16, 396, leftPanelWidth - 32, windowH - 412);
        int y = 402 - (logScrollOffset % (textHeight + lineSpacing));
        for (int i = startIndex; i < (int)wrappedLogs.size(); i++) {
            DrawText(wrappedLogs[i].c_str(), 24, y, textHeight, WHITE);
            y += (textHeight + lineSpacing);
        }
        EndScissorMode();
        EndDrawing();
    }

    UnloadRenderTexture(viewRt);
    CloseWindow();
    return 0;
}

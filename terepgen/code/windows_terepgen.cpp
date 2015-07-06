/*
    Terep generátor by Hidvégi Máté @2015

*/

#include <windows.h>
#include <d3dcompiler.h>
#include <d3d11.h>
#include <DirectXMath.h>

#include "terepgen_terrain.h"
#include "terepgen_types.h"
#include "terepgen_dxresources.h"

global_variable bool32 GlobalRunning = true;
global_variable input GlobalInput;
global_variable bool32 Resize;
global_variable bool32 DrawTerrain1;   
global_variable bool32 DrawTerrain2;   
global_variable uint32 GlobalSeed;  
global_variable real32 Persistence;   

internal screen_info 
GetWindowDimension(HWND Window)
{
    screen_info Result;

    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    Result.Height = ClientRect.bottom - ClientRect.top;
    Result.Width = ClientRect.right - ClientRect.left;

    return Result;
}
        
LRESULT CALLBACK
WindowProc(HWND Window, 
           UINT Message,
           WPARAM WParam,
           LPARAM LParam)
{
    LRESULT Result = 0;
    
    switch(Message)
    {
        case WM_DESTROY:
        {
            GlobalRunning = false;
        } break;

        case WM_CLOSE:
        {
            //TODO: Handle with message to the user?
            GlobalRunning = false;
        } break;
        
        case WM_SIZE:
        {
            Resize = true;
        } break;
        
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            uint32 KeyCode = (uint32)WParam;
            bool32 KeyIsUp = (LParam >> 31) & 1;
            bool32 KeyWasDown = (LParam >> 30) & 1;
            
            if(KeyIsUp == KeyWasDown)
            {
                if(KeyCode == 'W')
                {
                    GlobalInput.moveForward = !GlobalInput.moveForward;
                }
                else if(KeyCode == 'A')
                {
                    GlobalInput.moveLeft = !GlobalInput.moveLeft;
                }
                else if(KeyCode == 'S')
                {
                    GlobalInput.moveBack = !GlobalInput.moveBack;
                }
                else if(KeyCode == 'D')
                {
                    GlobalInput.moveRight = !GlobalInput.moveRight;
                }
                else if(KeyCode == 'E' || KeyCode == VK_SPACE)
                {
                    GlobalInput.moveUp = !GlobalInput.moveUp;
                }
                else if(KeyCode == 'Q')
                {
                    GlobalInput.moveDown = !GlobalInput.moveDown;
                }
                else if(KeyCode == VK_NUMPAD1 && !KeyIsUp)
                {
                   DrawTerrain1 = !DrawTerrain1;
                }
                else if((KeyCode == VK_NUMPAD2 || KeyCode == 'T') && !KeyIsUp)
                {
                   DrawTerrain2 = !DrawTerrain2;
                }
                else if(KeyCode == VK_ESCAPE && !KeyIsUp)
                {
                   GlobalRunning = false;
                }
                else if(KeyCode == VK_UP && !KeyIsUp)
                {
                   GlobalSeed++;
                }
                else if(KeyCode == VK_DOWN && !KeyIsUp)
                {
                   GlobalSeed--;
                }
                else if(KeyCode == VK_LEFT && !KeyIsUp)
                {
                   Persistence -= 0.05f;
                }
                else if(KeyCode == VK_RIGHT && !KeyIsUp)
                {
                   Persistence += 0.05f;
                }
                
            }
        } break;
        
        default:
        {
            Result = DefWindowProc (Window, Message, WParam, LParam);
        } break;
    }

    return Result;
}

int CALLBACK 
WinMain(HINSTANCE Instance,
        HINSTANCE PrevInstance,
        LPSTR CommandLine,
        int ShowCode)
{    
    WNDCLASS WindowClass = {};
    
    WindowClass.style = CS_HREDRAW|CS_VREDRAW;
    WindowClass.lpfnWndProc = WindowProc;
    WindowClass.hInstance = Instance;
    WindowClass.hCursor = LoadCursor(0, IDC_ARROW);
    WindowClass.lpszClassName = "TerepGenWindowClass";
    
    screen_info ScreenInfo;
    ScreenInfo.Width = 1280;
    ScreenInfo.Height = 800;
    
    if(RegisterClassA(&WindowClass))
    {
        HWND Window = CreateWindowExA(
                            0,
                            WindowClass.lpszClassName,
                            "TerepGen",
                            WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                            200,
                            150,
                            ScreenInfo.Width,
                            ScreenInfo.Height,
                            0, 0,
                            Instance,
                            0);
        
        if(Window)
        {
            DrawTerrain1 = true;
            GlobalSeed = 1000;
            dx_resource DXResources;
            DXResources.Initialize(Window, ScreenInfo.Width, ScreenInfo.Height);
            
            camera Camera;
            Camera.Initialize(DXResources, ScreenInfo);
            Camera.CameraSpeed = 0.5f;
            
            Persistence = 0.4f;
            
			// terrain Terrain;
            // Terrain.Color = color{1.0f, 1.0f, 1.0f, 1.0f};
            // Terrain.Initialize(GlobalSeed, Persistence);
                        
            terrain3D Terrain3D;
            Terrain3D.Initialize(GlobalSeed, Persistence);
            
            terrainRenderer TRenderer;
            // TRenderer.Initialize(DXResources, Terrain.FinalVertexCount);
            TRenderer.Initialize(DXResources, Terrain3D.FinalVertexCount);
            
            while(GlobalRunning)
            {
                MSG Message;
                while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
                {
                    TranslateMessage(&Message);
                    DispatchMessageA(&Message);
                }
                
                if(Resize)
                {
                    ScreenInfo = GetWindowDimension(Window);
                    DXResources.Resize(ScreenInfo.Width, ScreenInfo.Height);
                    Resize = false;
                }
        
                GlobalInput.OldMouseX = GlobalInput.MouseX;
                GlobalInput.OldMouseY = GlobalInput.MouseY;
                
                POINT MouseP;
                GetCursorPos(&MouseP);
                ScreenToClient(Window, &MouseP);
                GlobalInput.MouseX = MouseP.x;
                GlobalInput.MouseY = -MouseP.y;
                
                Camera.Update(GlobalInput);
                // Terrain.Update(GlobalSeed, Persistence);
                Terrain3D.Update(GlobalSeed, Persistence);
                
                DXResources.LoadResource(Camera.SceneConstantBuffer,
                              &Camera.SceneConstants, sizeof(Camera.SceneConstants));
                
                color BackgroundColor = {0.0f, 0.2f, 0.4f, 1.0f};
                //color BackgroundColor = {0.3f, 0.3f, 0.3f, 1.0f}; //grey
                DXResources.DeviceContext->ClearRenderTargetView(DXResources.BackBuffer, BackgroundColor.C);
                
                // TRenderer.DrawWireframe(DXResources, Terrain.Vertices);
                TRenderer.DrawWireframe(DXResources, Terrain3D.Vertices);
                // if(DrawTerrain2)Terrain3D.UpdateAndDrawPoints(DXResources, GlobalSeed, Persistence);
                
                DXResources.SwapChain->Present(0, 0);
            }
            
            Camera.Release();
            DXResources.Release();
        }
    }
    
    return 0;
}
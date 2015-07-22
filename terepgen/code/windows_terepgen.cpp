/*
    Terep generátor by Hidvégi Máté @2015

*/

#include <windows.h>

#include "terepgen_types.h"
#include "terepgen_terrain.h"
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
#if TEREPGEN_DEBUG
        OutputDebugStringA("[TEREPGEN_DEBUG] message arrived: WM_DESTROY \n");
#endif    
            GlobalRunning = false;
        } break;

        case WM_CLOSE:
        {
            //TODO: Handle with message to the user?
#if TEREPGEN_DEBUG
        OutputDebugStringA("[TEREPGEN_DEBUG] message arrived: WM_CLOSE \n");
#endif        
            GlobalRunning = false;
        } break;
        
        case WM_SIZE:
        {
#if TEREPGEN_DEBUG
        OutputDebugStringA("[TEREPGEN_DEBUG] message arrived: WM_SIZE \n");
#endif    
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
                    GlobalInput.MoveForward = !GlobalInput.MoveForward;
                }
                else if(KeyCode == 'A')
                {
                    GlobalInput.MoveLeft = !GlobalInput.MoveLeft;
                }
                else if(KeyCode == 'S')
                {
                    GlobalInput.MoveBack = !GlobalInput.MoveBack;
                }
                else if(KeyCode == 'D')
                {
                    GlobalInput.MoveRight = !GlobalInput.MoveRight;
                }
                else if(KeyCode == 'E' || KeyCode == VK_SPACE)
                {
                    GlobalInput.MoveUp = !GlobalInput.MoveUp;
                }
                else if(KeyCode == 'Q')
                {
                    GlobalInput.MoveDown = !GlobalInput.MoveDown;
                }
                else if(KeyCode == VK_ADD)
                {
                    GlobalInput.SpeedUp = !GlobalInput.SpeedUp;
                }
                else if(KeyCode == VK_SUBTRACT)
                {
                    GlobalInput.SpeedDown = !GlobalInput.SpeedDown;
                }
                else if(KeyCode == VK_NUMPAD1 && !KeyIsUp)
                {
                   DrawTerrain1 = !DrawTerrain1;
                }
                else if((KeyCode == VK_NUMPAD2 || KeyCode == 'T') && !KeyIsUp)
                {
                   DrawTerrain2 = !DrawTerrain2;
                }
                else if(KeyCode == 'R' && !KeyIsUp)
                {
                    GlobalInput.RenderMode++;
                    if(GlobalInput.RenderMode > 2) 
                    {
                        GlobalInput.RenderMode = 0;
                    }
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
    
    int32 A = (int32)-10.f;
    WNDCLASS WindowClass = {};
    
    WindowClass.style = CS_HREDRAW|CS_VREDRAW;
    WindowClass.lpfnWndProc = WindowProc;
    WindowClass.hInstance = Instance;
    WindowClass.hCursor = LoadCursor(0, IDC_ARROW);
    WindowClass.lpszClassName = "TerepGenWindowClass";
    
    screen_info ScreenInfo;
    ScreenInfo.Width = 1280;
	ScreenInfo.Height = 800;
	// ScreenInfo.Width = 800;
	// ScreenInfo.Height = 600;
    
    if(RegisterClassA(&WindowClass))
    {
        HWND Window = CreateWindowExA(
                            0,
                            WindowClass.lpszClassName,
                            "TerepGen",
                            WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                            200,
                            50,
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
            HRESULT HResult;
            HResult = DXResources.Initialize(Window, ScreenInfo.Width, ScreenInfo.Height);
            if(FAILED(HResult))
            {
                //MessageBox(NULL, DXGetErrorDescription(HResult), NULL, MB_OK);
                DXResources.Release();
                return 1;
            }
            
            camera Camera;
            Camera.Initialize(DXResources, ScreenInfo);
            Camera.CameraSpeed = 0.5f;
            
            Persistence = 0.4f;
            
			// terrain Terrain;
            // Terrain.Color = color{1.0f, 1.0f, 1.0f, 1.0f};
            // Terrain.Initialize(GlobalSeed, Persistence);
                        
            terrain3D Terrain3D0;
            Terrain3D0.Initialize(GlobalSeed, Persistence, v3{});
            terrain3D Terrain3D1;
            Terrain3D1.Initialize(GlobalSeed, Persistence, {-64.0f, 0.0f, 0.0f});
            terrain3D Terrain3D2;
            Terrain3D2.Initialize(GlobalSeed, Persistence, {-64.0f, 64.0f, 0.0f});
            terrain3D Terrain3D3;
            Terrain3D3.Initialize(GlobalSeed, Persistence, {0.0f, 64.0f, 0.0f});
            
            terrain3D Terrain3D4;
            Terrain3D4.Initialize(GlobalSeed, Persistence, {0.0f, 0.0f, 64.0f});
            terrain3D Terrain3D5;
            Terrain3D5.Initialize(GlobalSeed, Persistence, {0.0f, 64.0f, 64.0f});
            terrain3D Terrain3D6;
            Terrain3D6.Initialize(GlobalSeed, Persistence, {-64.0f, 64.0f, 64.0f});
            terrain3D Terrain3D7;
            Terrain3D7.Initialize(GlobalSeed, Persistence, {-64.0f, 0.0f, 64.0f});
            
            terrainRenderer TRenderer;
            // TRenderer.Initialize(DXResources, Terrain.FinalVertexCount);
            TRenderer.Initialize(DXResources, Terrain3D0.MaxVertexCount);
            
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
                    HResult = DXResources.Resize(ScreenInfo.Width, ScreenInfo.Height);
                    if(FAILED(HResult)) 
                    {
                        break;
                    }
                    Camera.Resize(ScreenInfo);
                    Resize = false;
                }
        
                // NOTE: Update
                GlobalInput.OldMouseX = GlobalInput.MouseX;
                GlobalInput.OldMouseY = GlobalInput.MouseY;
                
                POINT MouseP;
                GetCursorPos(&MouseP);
                ScreenToClient(Window, &MouseP);
                if(GetActiveWindow() == Window)
                {
                    GlobalInput.MouseX = MouseP.x;
                    GlobalInput.MouseY = -MouseP.y;
                }
                
                Camera.Update(GlobalInput);
                // Terrain.Update(GlobalSeed, Persistence);
                Terrain3D0.Update(GlobalSeed, Persistence, (terrain_render_mode)GlobalInput.RenderMode);
                Terrain3D1.Update(GlobalSeed, Persistence, (terrain_render_mode)GlobalInput.RenderMode);
                Terrain3D2.Update(GlobalSeed, Persistence, (terrain_render_mode)GlobalInput.RenderMode);
                Terrain3D3.Update(GlobalSeed, Persistence, (terrain_render_mode)GlobalInput.RenderMode);
                Terrain3D4.Update(GlobalSeed, Persistence, (terrain_render_mode)GlobalInput.RenderMode);
                Terrain3D5.Update(GlobalSeed, Persistence, (terrain_render_mode)GlobalInput.RenderMode);
                Terrain3D6.Update(GlobalSeed, Persistence, (terrain_render_mode)GlobalInput.RenderMode);
                Terrain3D7.Update(GlobalSeed, Persistence, (terrain_render_mode)GlobalInput.RenderMode);
                
                // NOTE: Rendering
                DXResources.LoadResource(Camera.SceneConstantBuffer,
                              &Camera.SceneConstants, sizeof(Camera.SceneConstants));
                
                color BackgroundColor = {0.0f, 0.2f, 0.4f, 1.0f};
                //color BackgroundColor = {0.3f, 0.3f, 0.3f, 1.0f}; //grey
                DXResources.DeviceContext->ClearDepthStencilView(DXResources.DepthStencilView, 
                    D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);
                DXResources.DeviceContext->ClearRenderTargetView(DXResources.BackBuffer, BackgroundColor.C);
                
                // TRenderer.DrawDebugTriangle();
                Terrain3D0.Draw(TRenderer);
                Terrain3D1.Draw(TRenderer);
                Terrain3D2.Draw(TRenderer);
                Terrain3D3.Draw(TRenderer);
                Terrain3D4.Draw(TRenderer);
                Terrain3D5.Draw(TRenderer);
                Terrain3D6.Draw(TRenderer);
                Terrain3D7.Draw(TRenderer);
                // if(DrawTerrain2)Terrain3D.UpdateAndDrawPoints(DXResources, GlobalSeed, Persistence);
                
                TRenderer.DrawAxis(100.0f);
                DXResources.SwapChain->Present(0, 0);
            }
            
            Camera.Release();
            DXResources.Release();
        }
    }
    
    return 0;
}
//
// Game.cpp
//

#include "pch.h"
#include "Game.h"

extern void ExitGame() noexcept;

using namespace DirectX;
//using namespace DirectX::SimpleMath; // WLJ add

using Microsoft::WRL::ComPtr;

Game::Game() noexcept :
    m_window(nullptr),
    m_outputWidth(800),
    m_outputHeight(600),
    m_featureLevel(D3D_FEATURE_LEVEL_9_1)
{
    pGolf = new Golf;
    pPlay = new GolfPlay;

    m_currentState = GameState::GAMESTATE_STARTSCREEN;
    //m_currentState = GameState::GAMESTATE_GAMEPLAY;

    //m_currentCamera = GameCamera::GAMECAMERA_CAMERA4;
    //m_currentCamera = GameCamera::GAMECAMERA_SWINGVIEW;
    m_currentCamera = GameCamera::GAMECAMERA_PROJECTILEFLIGHTVIEW;
    //m_currentCamera = GameCamera::GAMECAMERA_PRESWINGVIEW;
}

Game::~Game()
{
    delete pGolf;
    delete pPlay;
}

// Initialize the Direct3D resources required to run.
void Game::Initialize(HWND window, int width, int height)
{
    m_window = window;
    m_outputWidth = std::max(width, 1);
    m_outputHeight = std::max(height, 1);

    CreateDevice();

    CreateResources();

    // TODO: Change the timer settings if you want something other than the default variable timestep mode.
    // e.g. for 60 FPS fixed timestep update logic, call:
    /*
    m_timer.SetFixedTimeStep(true);
    m_timer.SetTargetElapsedSeconds(1.0 / 60);
    */

    // WLJ add for mouse and keybord interface
    m_keyboard = std::make_unique<DirectX::Keyboard>();
    //m_kbStateTracker = std::make_unique< Keyboard::KeyboardStateTracker>();

    m_mouse = std::make_unique<Mouse>();
    m_mouse->SetWindow(window);
}

// Executes the basic game loop.
void Game::Tick()
{
    m_timer.Tick([&]()
    {
        Update(m_timer);
    });

    m_flightStepTimer.Tick([&]()
        {
        });

    Render();
}

// Updates the world.
void Game::Update(DX::StepTimer const& timer)
{
    float elapsedTime = float(timer.GetElapsedSeconds());
    m_projectileTimer += elapsedTime;
    // TODO: Add your game logic here.

    if (m_currentState == GameState::GAMESTATE_CHARACTERSELECT)
    {
        if (m_menuSelect == 0)
        {
            m_character0->Update(elapsedTime);
        }
        if (m_menuSelect == 1)
        {
            m_character1->Update(elapsedTime);
        }
        if (m_menuSelect == 2)
        {
            m_character2->Update(elapsedTime);
        }
    }

    pPlay->Swing();

    if (pPlay->UpdateSwing() == true)
    {
        pPlay->ResetSwingUpdateReady();
        pGolf->UpdateImpact(pPlay->GetImpactData());
    }

    UpdateCamera(timer);

    // WLJ add for mouse and keybord interface   
    auto kb = m_keyboard->GetState();       
    m_kbStateTracker.Update(kb);

    if (kb.Escape)
    {
        m_currentState = GameState::GAMESTATE_MAINMENU;
    }   
    if (m_kbStateTracker.pressed.Enter)
    {
        if (m_currentState == GameState::GAMESTATE_CHARACTERSELECT)
        {
            if (m_menuSelect == 0)
            {
                pGolf->SetCharacter(0);
            }
            if (m_menuSelect == 1)
            {
                pGolf->SetCharacter(1);
            }
            if (m_menuSelect == 2)
            {
                pGolf->SetCharacter(2);
            }
            m_menuSelect = 0;
            //m_currentState = GameState::GAMESTATE_MAINMENU; // Return to Main Menu after selecting character, ToDo: using value of 1 doesn't return to main menu
            m_currentState = GameState::GAMESTATE_STARTSCREEN;// Return to Main Menu after selecting character, ToDo: using value of 1 doesn't return to main menu
        }

        if (m_currentState == GameState::GAMESTATE_MAINMENU)
        {
            if (m_menuSelect == 0) // GoTo Game State
            {
                m_currentState = GameState::GAMESTATE_GAMEPLAY;
            }
            if (m_menuSelect == 1) // GoTo Character Select State
            {
                m_currentState = GameState::GAMESTATE_CHARACTERSELECT;
            }
            if (m_menuSelect == 2) // Quit Game
            {
                ExitGame();
            }
            m_menuSelect = 0;
        }
        if (m_currentState == GameState::GAMESTATE_STARTSCREEN)
        {
            m_currentState = GameState::GAMESTATE_MAINMENU;
        }
    }
    if (m_kbStateTracker.pressed.Up)
    {
        if (m_currentState == GameState::GAMESTATE_MAINMENU)
        {
            --m_menuSelect;
        }
        if (m_currentState == GameState::GAMESTATE_CHARACTERSELECT)
        {
            --m_menuSelect;
        }
    }
    if (m_kbStateTracker.pressed.Down)
    {
        if (m_currentState == GameState::GAMESTATE_MAINMENU)
        {
            ++m_menuSelect;
        }
        if (m_currentState == GameState::GAMESTATE_CHARACTERSELECT)
        {
            ++m_menuSelect;
        }
    }  
    if (m_kbStateTracker.pressed.Left)
    {
        if (m_currentState == GameState::GAMESTATE_CHARACTERSELECT)
        {
            --m_menuSelect;
        }
    }
    if (m_kbStateTracker.pressed.Right)
    {
        if (m_currentState == GameState::GAMESTATE_CHARACTERSELECT)
        {
            ++m_menuSelect;
        }
    }
    if (kb.D1)
    {
        pGolf->SelectInputClub(1);
    }
    if (kb.D2)
    {
        pGolf->SelectInputClub(2);
    }
    if (kb.D3)
    {
        pGolf->SelectInputClub(3);
    }
    if (kb.D4)
    {
        pGolf->SelectInputClub(4);
    }
    if (kb.D5)
    {
        pGolf->SelectInputClub(5);
    }
    if (kb.D6)
    {
        pGolf->SelectInputClub(6);
    }
    if (kb.D7)
    {
        pGolf->SelectInputClub(7);
    }
    if (kb.D8)
    {
        pGolf->SelectInputClub(8);
    }
    if (kb.D9)
    {
        pGolf->SelectInputClub(9);
    }
    if (kb.D0)
    {
        pGolf->SelectInputClub(10);
    }
    if (kb.Z)
    {
        pPlay->StartSwing();
    }
    if (kb.X)
    {
        pPlay->SetPower();
    }
    if (kb.C)
    {
        pPlay->SetImpact();
    }
    if (kb.V)
    {
        pPlay->ResetPlayData();
        ResetPowerMeter();
    }
    if (kb.A)
    {
        if (pPlay->GetIsGameplayButtonReady() == true)
        {
            pPlay->UpdateSwingState();
            pPlay->SetGameplayButtonReadyFalse();
        }
    }
    
    if (kb.IsKeyUp(DirectX::Keyboard::Keys::A))
    {
        pPlay->ResetGamePlayButton();
    }
    if (m_kbStateTracker.pressed.Space)
    {
        if (m_currentState == GameState::GAMESTATE_GAMEPLAY)
        {
            pPlay->UpdateSwingState();
        }
    }
    if (kb.F1)
    {
        SetGameCamera(1);
    }
    if (kb.F2)
    {
        SetGameCamera(2);
    }
    if (kb.F3)
    {
        SetGameCamera(4);
    }
    if (kb.F4)
    {
        SetGameCamera(4);
    }
    if (kb.NumPad6)
    {
        m_cameraRotationX -= m_cameraMovementSpeed;
    }
    if (kb.NumPad4)
    {
        m_cameraRotationX += m_cameraMovementSpeed;
    }
    if (kb.NumPad2)
    {
        m_cameraRotationY -= m_cameraMovementSpeed;
    }
    if (kb.NumPad8)
    {
        m_cameraRotationY += m_cameraMovementSpeed;
    }
    if (kb.NumPad7)
    {
        m_cameraTargetX += m_cameraMovementSpeed;
    }
    if (kb.NumPad9)
    {
        m_cameraTargetX -= m_cameraMovementSpeed;
    }
    if (kb.NumPad1)
    {
        m_cameraTargetZ += m_cameraMovementSpeed;
    }
    if (kb.NumPad3)
    {
        m_cameraTargetZ -= m_cameraMovementSpeed;
    }
    if (kb.OemMinus)
    {
        m_cameraZoom -= m_cameraMovementSpeed + .3f;
    }
    if (kb.OemPlus)
    {
        m_cameraZoom += m_cameraMovementSpeed + .3f;
    }
    if (m_kbStateTracker.pressed.I)
    {
        m_currentCamera = GameCamera::GAMECAMERA_PRESWINGVIEW;
    }
    if (m_kbStateTracker.pressed.O)
    {
        m_currentCamera = GameCamera::GAMECAMERA_PROJECTILEFLIGHTVIEW;
    }
    if (m_kbStateTracker.pressed.P)
    {
        m_currentCamera = GameCamera::GAMECAMERA_SWINGVIEW;
    }
    
    auto mouse = m_mouse->GetState();

    elapsedTime;
}

void Game::UpdateCamera(DX::StepTimer const& timer)
{
    if (m_currentCamera == GameCamera::GAMECAMERA_DEFAULT)
    {
        m_world = DirectX::SimpleMath::Matrix::CreateRotationY(cosf(static_cast<float>(timer.GetTotalSeconds())));
    }
    if (m_currentCamera == GameCamera::GAMECAMERA_CAMERA1)
    {

    }
    if (m_currentCamera == GameCamera::GAMECAMERA_CAMERA2)
    {
        m_view = DirectX::SimpleMath::Matrix::CreateLookAt(DirectX::SimpleMath::Vector3(2.f, 2.f, 0.f), DirectX::SimpleMath::Vector3::Zero, DirectX::SimpleMath::Vector3::UnitY);
        m_world = DirectX::SimpleMath::Matrix::CreateRotationY(Utility::ToRadians(90));
        m_effect->SetView(m_view);
    }
    if (m_currentCamera == GameCamera::GAMECAMERA_CAMERA3)
    {
        m_world = DirectX::SimpleMath::Matrix::CreateRotationY(m_cameraRotationX);
        m_view = DirectX::SimpleMath::Matrix::CreateLookAt(DirectX::SimpleMath::Vector3(6.f, 0.f, 0.f), DirectX::SimpleMath::Vector3::Zero, DirectX::SimpleMath::Vector3::UnitY);
        //m_world = Matrix::CreateRotationY(Utility::ToRadians(90));
        m_effect->SetView(m_view);
    }
    if (m_currentCamera == GameCamera::GAMECAMERA_CAMERA4)
    {
        //m_view = Matrix::CreateLookAt(Vector3(2.f, m_cameraRotationY, 2.f), Vector3::Zero, Vector3::UnitY);
        m_view = DirectX::SimpleMath::Matrix::CreateLookAt(DirectX::SimpleMath::Vector3(2.f, m_cameraRotationY, 2.f), DirectX::SimpleMath::Vector3(m_cameraTargetX, 0.0, m_cameraTargetZ) , DirectX::SimpleMath::Vector3::UnitY);
        m_world = DirectX::SimpleMath::Matrix::CreateRotationY(m_cameraRotationX);

        m_effect->SetView(m_view);
        m_effect->SetProjection(m_proj);
    }
    if (m_currentCamera == GameCamera::GAMECAMERA_CAMERA5)
    {
        m_view = DirectX::SimpleMath::Matrix::CreateLookAt(DirectX::SimpleMath::Vector3(-6.f, 1.f, 2.f), DirectX::SimpleMath::Vector3::Zero, DirectX::SimpleMath::Vector3::UnitY);
        m_effect->SetView(m_view);
    }
    if (m_currentCamera == GameCamera::GAMECAMERA_CAMERA6)
    {
        m_view = DirectX::SimpleMath::Matrix::CreateLookAt(DirectX::SimpleMath::Vector3(2.f, 2.f, 2.f), DirectX::SimpleMath::Vector3::Zero, DirectX::SimpleMath::Vector3::UnitY);

        m_effect->SetView(m_view);
        m_effect->SetProjection(m_proj);
    }
    if (m_currentCamera == GameCamera::GAMECAMERA_CAMERA7)
    {
        m_view = DirectX::SimpleMath::Matrix::CreateLookAt(DirectX::SimpleMath::Vector3(.0f, 0.0f, 7.0f), DirectX::SimpleMath::Vector3::Zero, DirectX::SimpleMath::Vector3::UnitY);
        m_effect->SetView(m_view);
    }
    if (m_currentCamera == GameCamera::GAMECAMERA_PRESWINGVIEW)
    {
        DirectX::SimpleMath::Vector3 cameraPos = m_shootOrigin;
        cameraPos.x -= .9f;
        cameraPos.y += .5f;
        DirectX::SimpleMath::Vector3 cameraLookAt = m_shootOrigin;
        cameraLookAt.y += .3f;
        m_view = DirectX::SimpleMath::Matrix::CreateLookAt(cameraPos, cameraLookAt, DirectX::SimpleMath::Vector3::UnitY);
        m_world = DirectX::SimpleMath::Matrix::Identity;
        m_effect->SetView(m_view);
    }
    if (m_currentCamera == GameCamera::GAMECAMERA_SWINGVIEW)
    {
        m_view = DirectX::SimpleMath::Matrix::CreateLookAt(DirectX::SimpleMath::Vector3(-2.f, 0.0f, .5f), m_shootOrigin, DirectX::SimpleMath::Vector3::UnitY);
        m_world = DirectX::SimpleMath::Matrix::Identity;
        m_effect->SetView(m_view);
    }
    if (m_currentCamera == GameCamera::GAMECAMERA_PROJECTILEFLIGHTVIEW)
    {  
        m_view = DirectX::SimpleMath::Matrix::CreateLookAt(DirectX::SimpleMath::Vector3(-2.f, 0.3f, 2.f), m_ballPos, DirectX::SimpleMath::Vector3::UnitY);
        m_world = DirectX::SimpleMath::Matrix::Identity;
        m_effect->SetView(m_view);
    }
}

// Draws the scene.
void Game::Render()
{
    // Don't try to render anything before the first Update.
    if (m_timer.GetFrameCount() == 0)
    {
        return;
    }

    Clear();

    // TODO: Add your rendering code here.
    // WLJ start
    m_d3dContext->OMSetBlendState(m_states->Opaque(), nullptr, 0xFFFFFFFF);
    m_d3dContext->OMSetDepthStencilState(m_states->DepthNone(), 0);
    //m_d3dContext->RSSetState(m_states->CullNone());

    //world start
    m_d3dContext->RSSetState(m_raster.Get()); // WLJ anti-aliasing
    m_effect->SetWorld(m_world);
    //world end
    m_effect->Apply(m_d3dContext.Get());

    m_d3dContext->IASetInputLayout(m_inputLayout.Get());

    m_batch->Begin();

    if (m_currentState == GameState::GAMESTATE_GAMEPLAY)
    {
        DrawSwing();
        DrawWorld();
        DrawProjectile();
        DrawCameraFocus();
        //DrawProjectileRealTime();
    }

    m_batch->End();

    m_spriteBatch->Begin();

    //DrawShotTimerUI();

    if (m_currentState == GameState::GAMESTATE_STARTSCREEN)
    {
        DrawStartScreen();
    }
    if (m_currentState == GameState::GAMESTATE_MAINMENU)
    {
        DrawMenuMain();
    }
    if (m_currentState == GameState::GAMESTATE_CHARACTERSELECT)
    {
        DrawMenuCharacterSelect();
    }
    if (m_currentState == GameState::GAMESTATE_GAMEPLAY)
    {
        DrawPowerBarUI();
        //DrawSwingUI();
        DrawUI();
    }

    m_spriteBatch->End();

    Present();

    // Switch to next club in the bag after impact of previous shot
    /*
    if (toggleGetNextClub == 1)
    {
        xVec.clear();
        yVec.clear();
        zVec.clear();
        pGolf->SelectNextClub();
    }
    */
}

void Game::DrawSwingUI()
{
    std::vector<std::string> uiString = pPlay->GetDebugData();

    float fontOriginPosX = m_fontPosDebug.x;
    float fontOriginPosY = m_fontPosDebug.y;

    for (int i = 0; i < uiString.size(); ++i)
    {
        std::string uiLine = std::string(uiString[i]);
        DirectX::SimpleMath::Vector2 lineOrigin = m_font->MeasureString(uiLine.c_str());

        m_font->DrawString(m_spriteBatch.get(), uiLine.c_str(), m_fontPosDebug, Colors::White, 0.f, lineOrigin);
        m_fontPosDebug.y += 35;
    }
    m_fontPosDebug.y = fontOriginPosY;
}

void Game::DrawUI()
{
    std::vector<std::string> uiString = pGolf->GetUIstrings();

    std::string output = uiString[0];

    float fontOriginPosX = m_fontPos2.x;
    float fontOriginPosY = m_fontPos2.y;

    for (int i = 0; i < uiString.size(); ++i)
    {
        std::string uiLine = std::string(uiString[i]);
        //Vector2 lineOrigin = m_font->MeasureString(uiLine.c_str()) / 2.f;
        DirectX::SimpleMath::Vector2 lineOrigin = m_font->MeasureString(uiLine.c_str());
        //m_font->DrawString(m_spriteBatch.get(), output.c_str(), m_fontPos, Colors::White, 0.f, originText);
        m_font->DrawString(m_spriteBatch.get(), uiLine.c_str(), m_fontPos2, Colors::White, 0.f, lineOrigin);
        m_fontPos2.y += 35;
    }
    m_fontPos2.y = fontOriginPosY;
}

void Game::DrawPowerBarUI()
{
    if (pPlay->GetMeterPower() >= 0.0)
    {
        m_powerMeterBarRect.left = m_powerMeterImpactPoint - (m_powerMeterSize * ((pPlay->GetMeterPower() * m_powerMeterBarScale) * 0.01));
    }
    else
    {
        m_powerMeterBarRect.right = m_powerMeterImpactPoint - (m_powerMeterSize * ((pPlay->GetMeterPower() * m_powerMeterBarScale) * 0.01));
    }
    if (pPlay->GetIsBackswingSet() == false)
    {
        m_powerMeterBackswingRect.left = m_powerMeterImpactPoint - (m_powerMeterSize * ((pPlay->GetMeterPower() * m_powerMeterBarScale) * 0.01));
    }
    else
    {
        m_powerMeterBackswingRect.left = m_powerMeterImpactPoint - (m_powerMeterSize * ((pPlay->GetBackswingSet() * m_powerMeterBarScale) * 0.01));
    }

    m_spriteBatch->Draw(m_powerBackswingTexture.Get(), m_powerMeterBackswingRect, nullptr, Colors::White);
    m_spriteBatch->Draw(m_powerMeterTexture.Get(), m_powerMeterBarRect, nullptr, Colors::White);

    m_spriteBatch->Draw(m_powerFrameTexture.Get(), m_powerMeterFrameRect, nullptr, Colors::White);
    m_spriteBatch->Draw(m_powerImpactTexture.Get(), m_powerMeterImpactRect, nullptr, Colors::White);
}

void Game::DrawWorld()
{
    // draw world grid
    DirectX::SimpleMath::Vector3 xAxis(2.f, 0.f, 0.f);
    DirectX::SimpleMath::Vector3 xFarAxis(6.f, 0.f, 0.f);
    DirectX::SimpleMath::Vector3 zAxis(0.f, 0.f, 2.f);
    //DirectX::SimpleMath::Vector3 yAxis(0.f, 2.f, 0.f);
    DirectX::SimpleMath::Vector3 origin = DirectX::SimpleMath::Vector3::Zero;
    size_t divisions = 50;
    size_t extention = 50;
    for (size_t i = 0; i <= divisions + extention; ++i)
    {
        float fPercent = float(i) / float(divisions);
        fPercent = (fPercent * 2.0f) - 1.0f;
        DirectX::SimpleMath::Vector3 scale = xAxis * fPercent + origin;
        if (scale.x == 0.0f)
        {
            VertexPositionColor v1(scale - zAxis, Colors::Green);
            VertexPositionColor v2(scale + zAxis, Colors::Green);
            m_batch->DrawLine(v1, v2);
        }
        else
        {
            VertexPositionColor v1(scale - zAxis, Colors::Green);
            VertexPositionColor v2(scale + zAxis, Colors::Green);
            m_batch->DrawLine(v1, v2);
        }
    }
    /*
    for (size_t i = 0; i <= divisions + extention; ++i)
    {
        float fPercent = float(i) / float(divisions);
        fPercent = (fPercent * 2.0f) - 1.0f;
        DirectX::SimpleMath::Vector3 scale = xAxis * fPercent + origin;
        if (scale.x == 0.0f)
        {
            VertexPositionColor v1(scale - yAxis, Colors::Green);
            VertexPositionColor v2(scale + yAxis, Colors::Green);
            m_batch->DrawLine(v1, v2);
        }
        else
        {
            VertexPositionColor v1(scale - yAxis, Colors::Green);
            VertexPositionColor v2(scale + yAxis, Colors::Green);
            m_batch->DrawLine(v1, v2);
        }
    }
    for (size_t i = 0; i <= divisions + extention; ++i)
    {
        float fPercent = float(i) / float(divisions);
        fPercent = (fPercent * 2.0f) - 1.0f;
        DirectX::SimpleMath::Vector3 scale = yAxis * fPercent + origin;
        if (scale.x == 0.0f)
        {
            VertexPositionColor v1(scale - xAxis, Colors::Green);
            VertexPositionColor v2(scale + xAxis, Colors::Green);
            m_batch->DrawLine(v1, v2);
        }
        else
        {
            VertexPositionColor v1(scale - xAxis, Colors::Green);
            VertexPositionColor v2(scale + xAxis, Colors::Green);
            m_batch->DrawLine(v1, v2);
        }
    }
    */
    for (size_t i = 0; i <= divisions; i++)
    {
        float fPercent = float(i) / float(divisions);
        fPercent = (fPercent * 2.0f) - 1.0f;

        DirectX::SimpleMath::Vector3 scale = zAxis * fPercent + origin;

        if (scale.z == 0.0f)
        {
            VertexPositionColor v1(scale - xAxis, Colors::LawnGreen);
            VertexPositionColor v2(scale + xFarAxis, Colors::LawnGreen);
            m_batch->DrawLine(v1, v2);
        }
        else
        {
            VertexPositionColor v1(scale - xAxis, Colors::Green);
            VertexPositionColor v2(scale + xFarAxis, Colors::Green);
            m_batch->DrawLine(v1, v2);
        }
    }
}

void Game::SetGameCamera(int aCamera)
{
    if (aCamera == 1)
    {
        m_currentCamera = GameCamera::GAMECAMERA_CAMERA1;
    }
    if (aCamera == 2)
    {
        m_currentCamera = GameCamera::GAMECAMERA_CAMERA2;
    }
    if (aCamera == 3)
    {
        m_currentCamera = GameCamera::GAMECAMERA_CAMERA3;
    }
    if (aCamera == 4)
    {
        m_currentCamera = GameCamera::GAMECAMERA_CAMERA4;
    }
    if (aCamera == 5)
    {
        m_currentCamera = GameCamera::GAMECAMERA_CAMERA5;
    }
}

// Helper method to clear the back buffers.
void Game::Clear()
{
    // Clear the views.
    m_d3dContext->ClearRenderTargetView(m_renderTargetView.Get(), Colors::Black);
    m_d3dContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    m_d3dContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());

    // Set the viewport.
    CD3D11_VIEWPORT viewport(0.0f, 0.0f, static_cast<float>(m_outputWidth), static_cast<float>(m_outputHeight));
    m_d3dContext->RSSetViewports(1, &viewport);
}

// Presents the back buffer contents to the screen.
void Game::Present()
{
    // The first argument instructs DXGI to block until VSync, putting the application
    // to sleep until the next VSync. This ensures we don't waste any cycles rendering
    // frames that will never be displayed to the screen.
    HRESULT hr = m_swapChain->Present(1, 0);

    // If the device was reset we must completely reinitialize the renderer.
    if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
    {
        OnDeviceLost();
    }
    else
    {
        DX::ThrowIfFailed(hr);
    }
}

// Message handlers
void Game::OnActivated()
{
    // TODO: Game is becoming active window.
    m_kbStateTracker.Reset();
}

void Game::OnDeactivated()
{
    // TODO: Game is becoming background window.

}

void Game::OnSuspending()
{
    // TODO: Game is being power-suspended (or minimized).

}

void Game::OnResuming()
{
    m_timer.ResetElapsedTime();
    m_kbStateTracker.Reset();
    // TODO: Game is being power-resumed (or returning from minimize).
}

void Game::OnWindowSizeChanged(int width, int height)
{
    m_outputWidth = std::max(width, 1);
    m_outputHeight = std::max(height, 1);

    CreateResources();

    // TODO: Game window is being resized.
}

// Properties
void Game::GetDefaultSize(int& width, int& height) const noexcept
{
    // TODO: Change to desired default window size (note minimum size is 320x200).
    width = 800;
    height = 600;
}

// These are the resources that depend on the device.
void Game::CreateDevice()
{
    UINT creationFlags = 0;

#ifdef _DEBUG
    creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    static const D3D_FEATURE_LEVEL featureLevels[] =
    {
        // TODO: Modify for supported Direct3D feature levels
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_9_2,
        D3D_FEATURE_LEVEL_9_1,
    };

    // Create the DX11 API device object, and get a corresponding context.
    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11DeviceContext> context;
    DX::ThrowIfFailed(D3D11CreateDevice(
        nullptr,                            // specify nullptr to use the default adapter
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        creationFlags,
        featureLevels,
        _countof(featureLevels),
        D3D11_SDK_VERSION,
        device.ReleaseAndGetAddressOf(),    // returns the Direct3D device created
        &m_featureLevel,                    // returns feature level of device created
        context.ReleaseAndGetAddressOf()    // returns the device immediate context
    ));

#ifndef NDEBUG
    ComPtr<ID3D11Debug> d3dDebug;
    if (SUCCEEDED(device.As(&d3dDebug)))
    {
        ComPtr<ID3D11InfoQueue> d3dInfoQueue;
        if (SUCCEEDED(d3dDebug.As(&d3dInfoQueue)))
        {
#ifdef _DEBUG
            d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
            d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
#endif
            D3D11_MESSAGE_ID hide[] =
            {
                D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,
                // TODO: Add more message IDs here as needed.
            };
            D3D11_INFO_QUEUE_FILTER filter = {};
            filter.DenyList.NumIDs = _countof(hide);
            filter.DenyList.pIDList = hide;
            d3dInfoQueue->AddStorageFilterEntries(&filter);
        }
    }
#endif

    DX::ThrowIfFailed(device.As(&m_d3dDevice));
    DX::ThrowIfFailed(context.As(&m_d3dContext));

    // TODO: Initialize device dependent objects here (independent of window size).
    m_world = DirectX::SimpleMath::Matrix::Identity;
    m_states = std::make_unique<CommonStates>(m_d3dDevice.Get());
    m_effect = std::make_unique<BasicEffect>(m_d3dDevice.Get());
    m_effect->SetVertexColorEnabled(true);

    void const* shaderByteCode;
    size_t byteCodeLength;

    m_effect->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);

    DX::ThrowIfFailed(m_d3dDevice->CreateInputLayout(VertexType::InputElements, VertexType::InputElementCount, shaderByteCode, byteCodeLength, m_inputLayout.ReleaseAndGetAddressOf()));

    m_batch = std::make_unique<PrimitiveBatch<VertexType>>(m_d3dContext.Get());

    CD3D11_RASTERIZER_DESC rastDesc(D3D11_FILL_SOLID, D3D11_CULL_NONE, FALSE,
        D3D11_DEFAULT_DEPTH_BIAS, D3D11_DEFAULT_DEPTH_BIAS_CLAMP,
        D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS, TRUE, FALSE, FALSE, TRUE);

    DX::ThrowIfFailed(m_d3dDevice->CreateRasterizerState(&rastDesc, m_raster.ReleaseAndGetAddressOf()));

    m_font = std::make_unique<SpriteFont>(m_d3dDevice.Get(), L"myfile.spritefont");
    m_titleFont = std::make_unique<SpriteFont>(m_d3dDevice.Get(), L"titleFont.spritefont");
    m_bitwiseFont = std::make_unique<SpriteFont>(m_d3dDevice.Get(), L"bitwise24.spritefont");
    m_spriteBatch = std::make_unique<SpriteBatch>(m_d3dContext.Get());

    // Start Texture
    ComPtr<ID3D11Resource> resource;
    DX::ThrowIfFailed(CreateWICTextureFromFile(m_d3dDevice.Get(), L"PowerbarFrame.png", nullptr, m_powerFrameTexture.ReleaseAndGetAddressOf()));
    DX::ThrowIfFailed(CreateWICTextureFromFile(m_d3dDevice.Get(), L"PowerbarMeter.png", nullptr, m_powerMeterTexture.ReleaseAndGetAddressOf()));
    DX::ThrowIfFailed(CreateWICTextureFromFile(m_d3dDevice.Get(), L"PowerbarImpact.png", nullptr, m_powerImpactTexture.ReleaseAndGetAddressOf()));
    DX::ThrowIfFailed(CreateWICTextureFromFile(m_d3dDevice.Get(), L"PowerbarBackswing.png", nullptr, m_powerBackswingTexture.ReleaseAndGetAddressOf()));
    
    DX::ThrowIfFailed(CreateWICTextureFromFile(m_d3dDevice.Get(), L"PowerbarFrame.png", resource.GetAddressOf(), m_powerFrameTexture.ReleaseAndGetAddressOf()));
    DX::ThrowIfFailed(CreateWICTextureFromFile(m_d3dDevice.Get(), L"PowerbarMeter.png", resource.GetAddressOf(), m_powerMeterTexture.ReleaseAndGetAddressOf()));
    DX::ThrowIfFailed(CreateWICTextureFromFile(m_d3dDevice.Get(), L"PowerbarImpact.png", resource.GetAddressOf(), m_powerImpactTexture.ReleaseAndGetAddressOf()));
    DX::ThrowIfFailed(CreateWICTextureFromFile(m_d3dDevice.Get(), L"PowerbarBackswing.png", resource.GetAddressOf(), m_powerBackswingTexture.ReleaseAndGetAddressOf()));
    ComPtr<ID3D11Texture2D> PowerbarFrame;
    ComPtr<ID3D11Texture2D> PowerbarMeter;
    ComPtr<ID3D11Texture2D> PowerbarImpact;
    ComPtr<ID3D11Texture2D> PowerbarBackswing;
    DX::ThrowIfFailed(resource.As(&PowerbarFrame));
    DX::ThrowIfFailed(resource.As(&PowerbarMeter));
    DX::ThrowIfFailed(resource.As(&PowerbarImpact));
    DX::ThrowIfFailed(resource.As(&PowerbarBackswing));

    CD3D11_TEXTURE2D_DESC PowerbarFrameDesc;
    PowerbarFrame->GetDesc(&PowerbarFrameDesc);
    CD3D11_TEXTURE2D_DESC PowerbarMeterDesc;
    PowerbarMeter->GetDesc(&PowerbarMeterDesc);
    CD3D11_TEXTURE2D_DESC PowerbarImpactDesc;
    PowerbarImpact->GetDesc(&PowerbarImpactDesc);
    CD3D11_TEXTURE2D_DESC PowerbarBackswingDesc;
    PowerbarBackswing->GetDesc(&PowerbarBackswingDesc);

    m_powerBarFrameOrigin.x = float(PowerbarFrameDesc.Width / 2);
    m_powerBarFrameOrigin.y = float(PowerbarFrameDesc.Height / 2);
    m_powerBarMeterOrigin.x = float(PowerbarMeterDesc.Width / 2);
    m_powerBarMeterOrigin.y = float(PowerbarMeterDesc.Height / 2);
    m_powerBarImpactOrigin.x = float(PowerbarImpactDesc.Width / 2);
    m_powerBarImpactOrigin.y = float(PowerbarImpactDesc.Height / 2);
    m_powerBarBackswingOrigin.x = float(PowerbarBackswingDesc.Width / 2);
    m_powerBarBackswingOrigin.y = float(PowerbarBackswingDesc.Height / 2);

    // Character Textures
    DX::ThrowIfFailed(CreateWICTextureFromFile(m_d3dDevice.Get(), L"ChacterSpriteSheet.png", nullptr, m_characterTexture.ReleaseAndGetAddressOf()));
    m_character = std::make_unique<AnimatedTexture>();
    m_character->Load(m_characterTexture.Get(), 4, 6);

    DX::ThrowIfFailed(CreateWICTextureFromFile(m_d3dDevice.Get(), L"Chacter0SpriteSheet.png", nullptr, m_character0Texture.ReleaseAndGetAddressOf()));
    m_character0 = std::make_unique<AnimatedTexture>();
    m_character0->Load(m_character0Texture.Get(), 4, 6);

    DX::ThrowIfFailed(CreateWICTextureFromFile(m_d3dDevice.Get(), L"Chacter1SpriteSheet.png", nullptr, m_character1Texture.ReleaseAndGetAddressOf()));
    m_character1 = std::make_unique<AnimatedTexture>();
    m_character1->Load(m_character1Texture.Get(), 4, 6);

    DX::ThrowIfFailed(CreateWICTextureFromFile(m_d3dDevice.Get(), L"Chacter2SpriteSheet.png", nullptr, m_character2Texture.ReleaseAndGetAddressOf()));
    m_character2 = std::make_unique<AnimatedTexture>();
    m_character2->Load(m_character2Texture.Get(), 4, 6);

    DX::ThrowIfFailed(CreateWICTextureFromFile(m_d3dDevice.Get(), L"CharacterBackground.png", resource.GetAddressOf(), m_characterBackgroundTexture.ReleaseAndGetAddressOf()));
    ComPtr<ID3D11Texture2D> characterBackground;
    DX::ThrowIfFailed(resource.As(&characterBackground));
    CD3D11_TEXTURE2D_DESC characterBackgroundDesc;
    characterBackground->GetDesc(&characterBackgroundDesc);
    m_characterBackgroundOrigin.x = float(characterBackgroundDesc.Width / 2);
    m_characterBackgroundOrigin.y = float(characterBackgroundDesc.Height / 2);
}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateResources()
{
    // Clear the previous window size specific context.
    ID3D11RenderTargetView* nullViews[] = { nullptr };
    m_d3dContext->OMSetRenderTargets(_countof(nullViews), nullViews, nullptr);
    m_renderTargetView.Reset();
    m_depthStencilView.Reset();
    m_d3dContext->Flush();

    const UINT backBufferWidth = static_cast<UINT>(m_outputWidth);
    const UINT backBufferHeight = static_cast<UINT>(m_outputHeight);
    const DXGI_FORMAT backBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
    const DXGI_FORMAT depthBufferFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    constexpr UINT backBufferCount = 2;

    // If the swap chain already exists, resize it, otherwise create one.
    if (m_swapChain)
    {
        HRESULT hr = m_swapChain->ResizeBuffers(backBufferCount, backBufferWidth, backBufferHeight, backBufferFormat, 0);

        if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
        {
            // If the device was removed for any reason, a new device and swap chain will need to be created.
            OnDeviceLost();

            // Everything is set up now. Do not continue execution of this method. OnDeviceLost will reenter this method 
            // and correctly set up the new device.
            return;
        }
        else
        {
            DX::ThrowIfFailed(hr);
        }
    }
    else
    {
        // First, retrieve the underlying DXGI Device from the D3D Device.
        ComPtr<IDXGIDevice1> dxgiDevice;
        DX::ThrowIfFailed(m_d3dDevice.As(&dxgiDevice));

        // Identify the physical adapter (GPU or card) this device is running on.
        ComPtr<IDXGIAdapter> dxgiAdapter;
        DX::ThrowIfFailed(dxgiDevice->GetAdapter(dxgiAdapter.GetAddressOf()));

        // And obtain the factory object that created it.
        ComPtr<IDXGIFactory2> dxgiFactory;
        DX::ThrowIfFailed(dxgiAdapter->GetParent(IID_PPV_ARGS(dxgiFactory.GetAddressOf())));

        // Create a descriptor for the swap chain.
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.Width = backBufferWidth;
        swapChainDesc.Height = backBufferHeight;
        swapChainDesc.Format = backBufferFormat;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount = backBufferCount;

        DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsSwapChainDesc = {};
        fsSwapChainDesc.Windowed = TRUE;

        // Create a SwapChain from a Win32 window.
        DX::ThrowIfFailed(dxgiFactory->CreateSwapChainForHwnd(
            m_d3dDevice.Get(),
            m_window,
            &swapChainDesc,
            &fsSwapChainDesc,
            nullptr,
            m_swapChain.ReleaseAndGetAddressOf()
        ));

        // This template does not support exclusive fullscreen mode and prevents DXGI from responding to the ALT+ENTER shortcut.
        DX::ThrowIfFailed(dxgiFactory->MakeWindowAssociation(m_window, DXGI_MWA_NO_ALT_ENTER));
    }

    // Obtain the backbuffer for this window which will be the final 3D rendertarget.
    ComPtr<ID3D11Texture2D> backBuffer;
    DX::ThrowIfFailed(m_swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf())));

    // Create a view interface on the rendertarget to use on bind.
    DX::ThrowIfFailed(m_d3dDevice->CreateRenderTargetView(backBuffer.Get(), nullptr, m_renderTargetView.ReleaseAndGetAddressOf()));

    // Allocate a 2-D surface as the depth/stencil buffer and
    // create a DepthStencil view on this surface to use on bind.
    CD3D11_TEXTURE2D_DESC depthStencilDesc(depthBufferFormat, backBufferWidth, backBufferHeight, 1, 1, D3D11_BIND_DEPTH_STENCIL);

    ComPtr<ID3D11Texture2D> depthStencil;
    DX::ThrowIfFailed(m_d3dDevice->CreateTexture2D(&depthStencilDesc, nullptr, depthStencil.GetAddressOf()));

    CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D);
    DX::ThrowIfFailed(m_d3dDevice->CreateDepthStencilView(depthStencil.Get(), &depthStencilViewDesc, m_depthStencilView.ReleaseAndGetAddressOf()));

    // TODO: Initialize windows-size dependent objects here.

    ////********* WLJ world start ----- deactivate to turn off world spin
    m_view = DirectX::SimpleMath::Matrix::CreateLookAt(DirectX::SimpleMath::Vector3(2.f, 2.f, 2.f), DirectX::SimpleMath::Vector3::Zero, DirectX::SimpleMath::Vector3::UnitY);
    m_proj = DirectX::SimpleMath::Matrix::CreatePerspectiveFieldOfView(XM_PI / 4.f, float(backBufferWidth) / float(backBufferHeight), 0.1f, 10.f);

    m_effect->SetView(m_view);
    m_effect->SetProjection(m_proj);
    // world end
    
    // UI font positions
    m_fontPos.x = backBufferWidth / 2.f;
    m_fontPos.y = backBufferHeight / 2.f;
    m_fontPos2.x = backBufferWidth - 5;
    m_fontPos2.y = 35;
    m_fontPosDebug.x = 480;
    m_fontPosDebug.y = 35;
    m_fontMenuPos.x = backBufferWidth / 2.f;
    m_fontMenuPos.y = 35;
    m_bitwiseFontPos.x = backBufferWidth / 2.f;
    m_bitwiseFontPos.y = backBufferHeight / 2.f;

    // Start swing power bar
    m_powerMeterFrameRect.left = (backBufferWidth / 2) - m_powerBarFrameOrigin.x;
    m_powerMeterFrameRect.right = (backBufferWidth / 2) + m_powerBarFrameOrigin.x;
    m_powerMeterFrameRect.top = (backBufferHeight / 1.08) - m_powerBarFrameOrigin.y;
    m_powerMeterFrameRect.bottom = (backBufferHeight / 1.08) + m_powerBarFrameOrigin.y;

    m_powerMeterSize = m_powerMeterFrameRect.right - m_powerMeterFrameRect.left;

    float powerMeterScale = pPlay->GetMeterLength();
    float impactPointScale = pPlay->GetMeterImpactPoint();
    impactPointScale = impactPointScale * (m_powerMeterSize / powerMeterScale);
    m_powerMeterImpactPoint = m_powerMeterFrameRect.right - impactPointScale;

    m_powerMeterImpactRect.top = m_powerMeterFrameRect.top;
    m_powerMeterImpactRect.bottom = m_powerMeterFrameRect.bottom;
    m_powerMeterImpactRect.right = m_powerMeterFrameRect.right - impactPointScale + 20;
    m_powerMeterImpactRect.left = m_powerMeterFrameRect.right - impactPointScale - 20;

    m_powerMeterBarRect = m_powerMeterFrameRect;
    m_powerMeterBarRect.left = m_powerMeterFrameRect.right - impactPointScale;
    m_powerMeterBarRect.right = m_powerMeterFrameRect.right - impactPointScale;

    m_powerMeterBackswingRect = m_powerMeterFrameRect;
    m_powerMeterBackswingRect.left = m_powerMeterBarRect.left;
    m_powerMeterBackswingRect.right = m_powerMeterBarRect.right;

    m_powerMeterBarScale = 1.0 - (pPlay->GetMeterImpactPoint() / pPlay->GetMeterLength());

    // Character texture
    m_characterPos.x = 200.f;
    m_characterPos.y = float((backBufferHeight / 2) + (backBufferHeight / 4));
    m_character0Pos.x = 200.f;
    m_character0Pos.y = float((backBufferHeight * .25f));

    m_character1Pos.x = 200.f;
    m_character1Pos.y = float((backBufferHeight * .5f));

    m_character2Pos.x = 200.f;
    m_character2Pos.y = float((backBufferHeight * .75f));

    DX::ThrowIfFailed(CreateWICTextureFromFile(m_d3dDevice.Get(), L"CharacterBackground.png", nullptr, m_characterBackgroundTexture.ReleaseAndGetAddressOf()));
    m_characterBackgroundPos.x = backBufferWidth / 2.f;
    m_characterBackgroundPos.y = backBufferHeight / 2.f;
    // End Texture
}
void Game::DrawCameraFocus()
{
    float line = .25f;
    DirectX::SimpleMath::Vector3 focalPoint(m_cameraTargetX, m_cameraTargetY, m_cameraTargetZ);
    DirectX::SimpleMath::Vector3 yLine(m_cameraTargetX, m_cameraTargetY + line, m_cameraTargetZ);
    DirectX::SimpleMath::Vector3 xLine(m_cameraTargetX + line, m_cameraTargetY, m_cameraTargetZ);
    DirectX::SimpleMath::Vector3 zLine(m_cameraTargetX, m_cameraTargetY, m_cameraTargetZ + line);
    VertexPositionColor origin(focalPoint, Colors::Yellow);
    VertexPositionColor yOffset(yLine, Colors::Yellow);
    VertexPositionColor xOffset(xLine, Colors::Yellow);
    VertexPositionColor zOffset(zLine, Colors::Yellow);
    m_batch->DrawLine(origin, yOffset);
    m_batch->DrawLine(origin, xOffset);
    m_batch->DrawLine(origin, zOffset);
}

// working old version prior to impmenting real time match update
void Game::DrawProjectile() 
{
    std::vector<DirectX::SimpleMath::Vector3> shotPath = pGolf->GetShotPath();
    
    //draw tee box
    double originX = shotPath[0].x;
    double originZ = shotPath[0].z;
    DirectX::SimpleMath::Vector3 t1(originX - .05, 0.0f, -0.1f);
    DirectX::SimpleMath::Vector3 t2(originX + .05, 0.0f, -0.1f);
    DirectX::SimpleMath::Vector3 t3(originX - 0.05, 0.0f, 0.1f);
    DirectX::SimpleMath::Vector3 t4(originX + .05, 0.0f, 0.1f);
    VertexPositionColor vt1(t1, Colors::White);
    VertexPositionColor vt2(t2, Colors::White);
    VertexPositionColor vt3(t3, Colors::White);
    VertexPositionColor vt4(t4, Colors::White);
    m_batch->DrawLine(vt1, vt2);
    m_batch->DrawLine(vt1, vt3);
    m_batch->DrawLine(vt3, vt4);
    m_batch->DrawLine(vt4, vt2);
    // end tee box draw

    int stepCount = shotPath.size();

    if (m_projectilePathStep >= stepCount)
    {
        m_flightStepTimer.ResetElapsedTime();
        m_projectilePathStep = 0;
    }
    m_ballPos = shotPath[m_projectilePathStep];
    ++m_projectilePathStep;

    DirectX::SimpleMath::Vector3 prevPos = shotPath[0];
    for (int i = 0; i < m_projectilePathStep; ++i)
    {
        DirectX::SimpleMath::Vector3 p1(prevPos);
        DirectX::SimpleMath::Vector3 p2(shotPath[i]);

        VertexPositionColor aV(p1, Colors::White);
        VertexPositionColor bV(p2, Colors::White);
        VertexPositionColor aVRed(p1, Colors::Red);
        VertexPositionColor bVRed(p2, Colors::Red);
        VertexPositionColor aVBlue(p1, Colors::Blue);
        VertexPositionColor bVBlue(p2, Colors::Blue);
        VertexPositionColor aVYellow(p1, Colors::Yellow);
        VertexPositionColor bVYellow(p2, Colors::Yellow);
        std::vector<int> colorVec = pGolf->GetDrawColorVector();
        int vecIndex = pGolf->GetDrawColorIndex();

        if (vecIndex > 0)
        {
            if (i > colorVec[0])
            {
                aV = aVRed;
                bV = bVRed;
            }
        }
        if (vecIndex > 1)
        {
            if (i > colorVec[1])
            {
                aV = aVBlue;
                bV = bVBlue;
            }
        }
        if (vecIndex > 2)
        {
            if (i > colorVec[2])
            {
                aV = aVYellow;
                bV = bVYellow;
            }
        }

        m_batch->DrawLine(aV, bV);
        prevPos = shotPath[i];

    }
    

    //bool toggleGetNextClub = 0;
    ///// Landing explosion
    //if (arcCount == stepCount)
    //{
        /*
        Vector3 f1(prevPos);
        Vector3 f2(prevX, prevY + 0.2f, prevZ);
        Vector3 f3(prevX + 0.1f, prevY + 0.1f, prevZ + 0.1f);
        Vector3 f4(prevX - 0.1f, prevY + 0.1f, prevZ - 0.1f);
        Vector3 f5(prevX + 0.1f, prevY + 0.1f, prevZ - 0.1f);
        Vector3 f6(prevX - 0.1f, prevY + 0.1f, prevZ + 0.1f);
        Vector3 f7(prevX + 0.01f, prevY + 0.1f, prevZ + 0.01f);
        Vector3 f8(prevX - 0.01f, prevY + 0.1f, prevZ - 0.01f);
        Vector3 f9(prevX + 0.01f, prevY + 0.1f, prevZ - 0.01f);
        Vector3 f10(prevX - 0.01f, prevY + 0.1f, prevZ + 0.01f);
        VertexPositionColor ft1(f1, Colors::Red);
        VertexPositionColor ft2(f2, Colors::Red);
        VertexPositionColor ft3(f3, Colors::Red);
        VertexPositionColor ft4(f4, Colors::Red);
        VertexPositionColor ft5(f5, Colors::Red);
        VertexPositionColor ft6(f6, Colors::Red);
        VertexPositionColor ft7(f7, Colors::Red);
        VertexPositionColor ft8(f8, Colors::Red);
        VertexPositionColor ft9(f9, Colors::Red);
        VertexPositionColor ft10(f10, Colors::Red);
        m_batch->DrawLine(ft1, ft2);
        m_batch->DrawLine(ft1, ft3);
        m_batch->DrawLine(ft1, ft4);
        m_batch->DrawLine(ft1, ft5);
        m_batch->DrawLine(ft1, ft6);
        m_batch->DrawLine(ft1, ft7);
        m_batch->DrawLine(ft1, ft8);
        m_batch->DrawLine(ft1, ft9);
        m_batch->DrawLine(ft1, ft10);
        */
        //toggleGetNextClub = 1;
    //}
    // end landing explosion
}

void Game::DrawProjectileRealTime()
{
    std::vector<DirectX::SimpleMath::Vector3> shotPath = pGolf->GetShotPath();

    std::vector<float> shotTimeStep = pGolf->GetShotPathTimeSteps();
    int stepCount = shotPath.size();
    float shotTimeTotal = shotTimeStep.back();

    if (m_projectilePathStep >= stepCount)
    {
        m_projectilePathStep = 0;
    }
    m_ballPos = shotPath[m_projectilePathStep];
    ++m_projectilePathStep;
    
    DirectX::SimpleMath::Vector3 prevPos = shotPath[0];
    for (int i = 0; i < shotPath.size(); ++i)
    {
        DirectX::SimpleMath::Vector3 p1(prevPos);
        DirectX::SimpleMath::Vector3 p2(shotPath[i]);
        VertexPositionColor aV(p1, Colors::White);
        VertexPositionColor bV(p2, Colors::White);

        if (shotTimeStep[i] < m_projectileTimer)
        {
            m_batch->DrawLine(aV, bV);
        }
        prevPos = shotPath[i];
    }
    
    if (m_projectileTimer > shotTimeStep.back())
    {
        m_projectileTimer = 0.0;
    }
}

void Game::DrawShotTimerUI()
{
    std::string timerUI = "Timer = " + std::to_string(m_projectileTimer);
    DirectX::SimpleMath::Vector2 lineOrigin = m_font->MeasureString(timerUI.c_str());
    m_font->DrawString(m_spriteBatch.get(), timerUI.c_str(), m_fontPosDebug, Colors::White, 0.f, lineOrigin);
}

void Game::DrawMenuCharacterSelect()
{
    const UINT backBufferWidth = static_cast<UINT>(m_outputWidth);
    const UINT backBufferHeight = static_cast<UINT>(m_outputHeight);

    float lineDrawY = m_fontMenuPos.y + 25;
    float lineDrawSpacingY = 15;
    std::string menuTitle = "Character Select";
    float menuTitlePosX = m_fontMenuPos.x;
    float menuTitlePosY = lineDrawY;
    DirectX::SimpleMath::Vector2 menuTitlePos(menuTitlePosX, menuTitlePosY);
    DirectX::SimpleMath::Vector2 menuOrigin = m_titleFont->MeasureString(menuTitle.c_str()) / 2.f;
    m_titleFont->DrawString(m_spriteBatch.get(), menuTitle.c_str(), menuTitlePos + DirectX::SimpleMath::Vector2(4.f, 4.f), Colors::Green, 0.f, menuOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), menuTitle.c_str(), menuTitlePos + DirectX::SimpleMath::Vector2(3.f, 3.f), Colors::Green, 0.f, menuOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), menuTitle.c_str(), menuTitlePos + DirectX::SimpleMath::Vector2(2.f, 2.f), Colors::Green, 0.f, menuOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), menuTitle.c_str(), menuTitlePos + DirectX::SimpleMath::Vector2(1.f, 1.f), Colors::Green, 0.f, menuOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), menuTitle.c_str(), menuTitlePos + DirectX::SimpleMath::Vector2(-1.f, -1.f), Colors::LawnGreen, 0.f, menuOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), menuTitle.c_str(), menuTitlePos, Colors::White, 0.f, menuOrigin);
    
    float posY0 = 250.0f;
    float originY = posY0;
    float ySpacing = 50.f;
    lineDrawY += menuTitlePosY + lineDrawSpacingY;
    std::string menuObj0String = pGolf->GetCharacterName(0);

    float posX0 = backBufferWidth * .20f;
    //float posX0 = 200.0;
    DirectX::SimpleMath::Vector2 menuObj0Pos(posX0, posY0);
    DirectX::SimpleMath::Vector2 menuObj0Origin = m_font->MeasureString(menuObj0String.c_str()) / 2.f;


    //float ySpacing = menuObj0Origin.y * 2;
    
    //m_characterBackgroundPos.x = m_character0Pos.x + menuObj0Origin.x;
    float half = m_characterBackgroundOrigin.x / 2.f;
    //float half = m_characterBackgroundOrigin.x - 105;
    //float half = m_characterBackgroundOrigin.x - 105;
    m_characterBackgroundPos.x = posX0 + half + 25.f;
    m_characterBackgroundPos.y = m_character0Pos.y + 10;
    m_characterBackgroundOrigin = menuObj0Origin;

    //m_character0Pos.x = menuTitlePosX / 3;
    m_character0Pos.x = m_characterBackgroundPos.x -menuObj0Origin.x;
    m_character0Pos.y = lineDrawY - menuObj0Origin.y;
    
    posY0 += ySpacing;
    int i = 0;
    
    std::string dataString = "Data: ";
    DirectX::SimpleMath::Vector2 dataOrigin = m_bitwiseFont->MeasureString(dataString.c_str()) / 2.f;
    posY0 += ySpacing;
    DirectX::SimpleMath::Vector2 dataPos;
    dataPos.x = posX0;
    dataPos.y = posY0;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), dataString.c_str(), dataPos, Colors::White, 0.f, dataOrigin);

    std::string armLengthString0 = pGolf->GetCharacterArmLength(i);
    DirectX::SimpleMath::Vector2 armLengthOrigin0 = m_bitwiseFont->MeasureString(armLengthString0.c_str()) / 2.f;
    posY0 += ySpacing;
    DirectX::SimpleMath::Vector2 armLengthPos0;
    armLengthPos0.x = posX0;
    armLengthPos0.y = posY0;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), armLengthString0.c_str(), armLengthPos0, Colors::White, 0.f, armLengthOrigin0);
    
    std::string armMassString0 = pGolf->GetCharacterArmMass(i);
    DirectX::SimpleMath::Vector2 armMassOrigin0 = m_bitwiseFont->MeasureString(armMassString0.c_str()) / 2.f;
    DirectX::SimpleMath::Vector2 armMassPos0;
    posY0 += ySpacing;
    armMassPos0.x = posX0;
    armMassPos0.y = posY0;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), armMassString0.c_str(), armMassPos0, Colors::White, 0.f, armMassOrigin0);

    std::string clubLengthModString0 = pGolf->GetCharacterClubLengthMod(i);
    DirectX::SimpleMath::Vector2 clubLengthModOrigin0 = m_bitwiseFont->MeasureString(clubLengthModString0.c_str()) / 2.f;
    DirectX::SimpleMath::Vector2 clubLengthModPos0;
    posY0 += ySpacing;
    clubLengthModPos0.x = posX0;
    clubLengthModPos0.y = posY0;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), clubLengthModString0.c_str(), clubLengthModPos0, Colors::White, 0.f, clubLengthModOrigin0);

    std::string armBalancePointString0 = pGolf->GetCharacterArmBalancePoint(i);
    DirectX::SimpleMath::Vector2 armBalancePointOrigin0 = m_bitwiseFont->MeasureString(armBalancePointString0.c_str()) / 2.f;
    DirectX::SimpleMath::Vector2 armBalancePointPos0;
    posY0 += ySpacing;
    armBalancePointPos0.x = posX0;
    armBalancePointPos0.y = posY0;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), armBalancePointString0.c_str(), armBalancePointPos0, Colors::White, 0.f, armBalancePointOrigin0);

    std::string armMassMoIString0 = pGolf->GetCharacterArmMassMoI(i);
    DirectX::SimpleMath::Vector2 armMassMoIOrigin0 = m_bitwiseFont->MeasureString(armMassMoIString0.c_str()) / 2.f;
    DirectX::SimpleMath::Vector2 armMassMoIPos0;
    posY0 += ySpacing;
    armMassMoIPos0.x = posX0;
    armMassMoIPos0.y = posY0;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), armMassMoIString0.c_str(), armMassMoIPos0, Colors::White, 0.f, armMassMoIOrigin0);

    std::string bioString = "Bio:";
    DirectX::SimpleMath::Vector2 bioOrigin = m_bitwiseFont->MeasureString(bioString.c_str()) / 2.f;
    posY0 += ySpacing + ySpacing;
    DirectX::SimpleMath::Vector2 bioPos;
    bioPos.x = posX0;
    bioPos.y = posY0;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), bioString.c_str(), bioPos, Colors::White, 0.f, bioOrigin);

    std::string bioLine0String0 = pGolf->GetCharacterBioLine0(i);
    DirectX::SimpleMath::Vector2 bioLine0Origin0 = m_bitwiseFont->MeasureString(bioLine0String0.c_str()) / 2.f;
    posY0 += ySpacing;
    DirectX::SimpleMath::Vector2 bioLine0Pos0;
    bioLine0Pos0.x = posX0;
    bioLine0Pos0.y = posY0;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), bioLine0String0.c_str(), bioLine0Pos0, Colors::White, 0.f, bioLine0Origin0);

    std::string bioLine1String0 = pGolf->GetCharacterBioLine1(i);
    DirectX::SimpleMath::Vector2 bioLine1Origin0 = m_bitwiseFont->MeasureString(bioLine1String0.c_str()) / 2.f;
    posY0 += ySpacing;
    DirectX::SimpleMath::Vector2 bioLine1Pos0;
    bioLine1Pos0.x = posX0;
    bioLine1Pos0.y = posY0;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), bioLine1String0.c_str(), bioLine1Pos0, Colors::White, 0.f, bioLine1Origin0);

    std::string bioLine2String0 = pGolf->GetCharacterBioLine2(i);
    DirectX::SimpleMath::Vector2 bioLine2Origin0 = m_bitwiseFont->MeasureString(bioLine2String0.c_str()) / 2.f;
    posY0 += ySpacing;
    DirectX::SimpleMath::Vector2 bioLine2Pos0;
    bioLine2Pos0.x = posX0;
    bioLine2Pos0.y = posY0;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), bioLine2String0.c_str(), bioLine2Pos0, Colors::White, 0.f, bioLine2Origin0);

    std::string bioLine3String0 = pGolf->GetCharacterBioLine3(i);
    DirectX::SimpleMath::Vector2 bioLine3Origin0 = m_bitwiseFont->MeasureString(bioLine3String0.c_str()) / 2.f;
    posY0 += ySpacing;
    DirectX::SimpleMath::Vector2 bioLine3Pos0;
    bioLine3Pos0.x = posX0;
    bioLine3Pos0.y = posY0;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), bioLine3String0.c_str(), bioLine3Pos0, Colors::White, 0.f, bioLine3Origin0);

 ///////////////////////////////////////////////////////////

    if (m_menuSelect == 0)
    {
        m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos + DirectX::SimpleMath::Vector2(4.f, 4.f), nullptr, Colors::LawnGreen, 0.f, m_characterBackgroundOrigin);
        m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos + DirectX::SimpleMath::Vector2(3.f, 3.f), nullptr, Colors::LawnGreen, 0.f, m_characterBackgroundOrigin);
        m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos + DirectX::SimpleMath::Vector2(2.f, 2.f), nullptr, Colors::LawnGreen, 0.f, m_characterBackgroundOrigin);
        m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos + DirectX::SimpleMath::Vector2(1.f, 1.f), nullptr, Colors::LawnGreen, 0.f, m_characterBackgroundOrigin);
    }
    else
    {
        m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos + DirectX::SimpleMath::Vector2(-4.f, -4.f), nullptr, Colors::Gray, 0.f, m_characterBackgroundOrigin);
        m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos + DirectX::SimpleMath::Vector2(-3.f, -3.f), nullptr, Colors::Gray, 0.f, m_characterBackgroundOrigin);
        m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos + DirectX::SimpleMath::Vector2(-2.f, -2.f), nullptr, Colors::Gray, 0.f, m_characterBackgroundOrigin);
        m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos + DirectX::SimpleMath::Vector2(-1.f, -1.f), nullptr, Colors::Gray, 0.f, m_characterBackgroundOrigin);
    }
    m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos, nullptr, Colors::White, 0.f, m_characterBackgroundOrigin);
    m_character0->Draw(m_spriteBatch.get(), m_character0Pos);

    float posX1 = backBufferWidth / 2.0;
    float posY1 = 250.0f;
    lineDrawY += menuObj0Pos.y;
    std::string menuObj1String = pGolf->GetCharacterName(1);
    //Vector2 menuObj1Pos(menuTitlePosX, menuObj0Pos.y + menuOrigin.x + 0);

   
    DirectX::SimpleMath::Vector2 menuObj1Pos(posX1, posY1);
    DirectX::SimpleMath::Vector2 menuObj1Origin = m_font->MeasureString(menuObj1String.c_str()) / 2.f;

    //half = m_characterBackgroundOrigin.x;
    m_characterBackgroundPos.x = posX1 + half + 10.f;
    //m_characterBackgroundPos.y = m_character1Pos.y + 10;
    //m_characterBackgroundOrigin = menuObj1Origin;
    m_character1Pos.x = m_characterBackgroundPos.x - menuObj0Origin.x;
    m_character1Pos.y = m_character0Pos.y;

    posY1 += ySpacing + ySpacing;
    ++i;

    dataPos.x = posX1;
    dataPos.y = posY1;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), dataString.c_str(), dataPos, Colors::White, 0.f, dataOrigin);

    std::string armLengthString1 = pGolf->GetCharacterArmLength(i);
    DirectX::SimpleMath::Vector2 armLengthOrigin1 = m_bitwiseFont->MeasureString(armLengthString1.c_str()) / 2.f;
    DirectX::SimpleMath::Vector2 armLengthPos1;
    posY1 += ySpacing;
    armLengthPos1.x = posX1;
    armLengthPos1.y = posY1;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), armLengthString1.c_str(), armLengthPos1, Colors::White, 0.f, armLengthOrigin1);

    std::string armMassString1 = pGolf->GetCharacterArmMass(i);
    DirectX::SimpleMath::Vector2 armMassOrigin1 = m_bitwiseFont->MeasureString(armMassString1.c_str()) / 2.f;
    DirectX::SimpleMath::Vector2 armMassPos1;
    posY1 += ySpacing;
    armMassPos1.x = posX1;
    armMassPos1.y = posY1;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), armMassString1.c_str(), armMassPos1, Colors::White, 0.f, armMassOrigin1);

    std::string clubLengthModString1 = pGolf->GetCharacterClubLengthMod(i);
    DirectX::SimpleMath::Vector2 clubLengthModOrigin1 = m_bitwiseFont->MeasureString(clubLengthModString1.c_str()) / 2.f;
    DirectX::SimpleMath::Vector2 clubLengthModPos1;
    posY1 += ySpacing;
    clubLengthModPos1.x = posX1;
    clubLengthModPos1.y = posY1;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), clubLengthModString1.c_str(), clubLengthModPos1, Colors::White, 0.f, clubLengthModOrigin1);

    std::string armBalancePointString1 = pGolf->GetCharacterArmBalancePoint(i);
    DirectX::SimpleMath::Vector2 armBalancePointOrigin1 = m_bitwiseFont->MeasureString(armBalancePointString1.c_str()) / 2.f;
    DirectX::SimpleMath::Vector2 armBalancePointPos1;
    posY1 += ySpacing;
    armBalancePointPos1.x = posX1;
    armBalancePointPos1.y = posY1;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), armBalancePointString1.c_str(), armBalancePointPos1, Colors::White, 0.f, armBalancePointOrigin1);

    std::string armMassMoIString1 = pGolf->GetCharacterArmMassMoI(i);
    DirectX::SimpleMath::Vector2 armMassMoIOrigin1 = m_bitwiseFont->MeasureString(armMassMoIString1.c_str()) / 2.f;
    DirectX::SimpleMath::Vector2 armMassMoIPos1;
    posY1 += ySpacing;
    armMassMoIPos1.x = posX1;
    armMassMoIPos1.y = posY1;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), armMassMoIString1.c_str(), armMassMoIPos1, Colors::White, 0.f, armMassMoIOrigin1);

    posY1 += ySpacing + ySpacing;
    bioPos.x = posX1;
    bioPos.y = posY1;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), bioString.c_str(), bioPos, Colors::White, 0.f, bioOrigin);

    std::string bioLine0String1 = pGolf->GetCharacterBioLine0(i);
    DirectX::SimpleMath::Vector2 bioLine0Origin1 = m_bitwiseFont->MeasureString(bioLine0String1.c_str()) / 2.f;
    DirectX::SimpleMath::Vector2 bioLine0Pos1;
    posY1 += ySpacing;
    bioLine0Pos1.x = posX1;
    bioLine0Pos1.y = posY1;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), bioLine0String1.c_str(), bioLine0Pos1, Colors::White, 0.f, bioLine0Origin1);

    std::string bioLine1String1 = pGolf->GetCharacterBioLine1(i);
    DirectX::SimpleMath::Vector2 bioLine1Origin1 = m_bitwiseFont->MeasureString(bioLine1String1.c_str()) / 2.f;
    DirectX::SimpleMath::Vector2 bioLine1Pos1;
    posY1 += ySpacing;
    bioLine1Pos1.x = posX1;
    bioLine1Pos1.y = posY1;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), bioLine1String1.c_str(), bioLine1Pos1, Colors::White, 0.f, bioLine1Origin1);

    std::string bioLine2String1 = pGolf->GetCharacterBioLine2(i);
    DirectX::SimpleMath::Vector2 bioLine2Origin1 = m_bitwiseFont->MeasureString(bioLine2String1.c_str()) / 2.f;
    DirectX::SimpleMath::Vector2 bioLine2Pos1;
    posY1 += ySpacing;
    bioLine2Pos1.x = posX1;
    bioLine2Pos1.y = posY1;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), bioLine2String1.c_str(), bioLine2Pos1, Colors::White, 0.f, bioLine2Origin1);

    std::string bioLine3String1 = pGolf->GetCharacterBioLine3(i);
    DirectX::SimpleMath::Vector2 bioLine3Origin1 = m_bitwiseFont->MeasureString(bioLine3String1.c_str()) / 2.f;
    DirectX::SimpleMath::Vector2 bioLine3Pos1;
    posY1 += ySpacing;
    bioLine3Pos1.x = posX1;
    bioLine3Pos1.y = posY1;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), bioLine3String1.c_str(), bioLine3Pos1, Colors::White, 0.f, bioLine3Origin1);

    if (m_menuSelect == 1)
    {
        m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos + DirectX::SimpleMath::Vector2(4.f, 4.f), nullptr, Colors::LawnGreen, 0.f, m_characterBackgroundOrigin);
        m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos + DirectX::SimpleMath::Vector2(3.f, 3.f), nullptr, Colors::LawnGreen, 0.f, m_characterBackgroundOrigin);
        m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos + DirectX::SimpleMath::Vector2(2.f, 2.f), nullptr, Colors::LawnGreen, 0.f, m_characterBackgroundOrigin);
        m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos + DirectX::SimpleMath::Vector2(1.f, 1.f), nullptr, Colors::LawnGreen, 0.f, m_characterBackgroundOrigin);
    }
    else
    {
        m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos + DirectX::SimpleMath::Vector2(-4.f, -4.f), nullptr, Colors::Gray, 0.f, m_characterBackgroundOrigin);
        m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos + DirectX::SimpleMath::Vector2(-3.f, -3.f), nullptr, Colors::Gray, 0.f, m_characterBackgroundOrigin);
        m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos + DirectX::SimpleMath::Vector2(-2.f, -2.f), nullptr, Colors::Gray, 0.f, m_characterBackgroundOrigin);
        m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos + DirectX::SimpleMath::Vector2(-1.f, -1.f), nullptr, Colors::Gray, 0.f, m_characterBackgroundOrigin);
    }
    m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos, nullptr, Colors::White, 0.f, m_characterBackgroundOrigin);
    m_character1->Draw(m_spriteBatch.get(), m_character1Pos);

    float posX2 = backBufferWidth * .80f;
    float posY2 = 250.0f;
    lineDrawY += menuObj0Pos.y;
    std::string menuObj2String = pGolf->GetCharacterName(2);

    DirectX::SimpleMath::Vector2 menuObj2Pos(posX2, posY2);
    DirectX::SimpleMath::Vector2 menuObj2Origin = m_font->MeasureString(menuObj2String.c_str()) / 2.f;

    //m_characterBackgroundPos.x = posX2 + half;
    m_characterBackgroundPos.x = posX2 + half + 25.f;
    //m_characterBackgroundPos.y = m_character2Pos.y + 10;
    //m_characterBackgroundOrigin = menuObj2Origin;
    m_character2Pos.x = m_characterBackgroundPos.x - 135.f;
    m_character2Pos.y = m_character0Pos.y;

    ++i;
    posY2 += ySpacing + ySpacing;

    dataPos.x = posX2;
    dataPos.y = posY2;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), dataString.c_str(), dataPos, Colors::White, 0.f, dataOrigin);

    std::string armLengthString2 = pGolf->GetCharacterArmLength(i);
    DirectX::SimpleMath::Vector2 armLengthOrigin2 = m_bitwiseFont->MeasureString(armLengthString2.c_str()) / 2.f;
    DirectX::SimpleMath::Vector2 armLengthPos2;
    posY2 += ySpacing;
    armLengthPos2.x = posX2;
    armLengthPos2.y = posY2;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), armLengthString2.c_str(), armLengthPos2, Colors::White, 0.f, armLengthOrigin2);

    std::string armMassString2 = pGolf->GetCharacterArmMass(i);
    DirectX::SimpleMath::Vector2 armMassOrigin2 = m_bitwiseFont->MeasureString(armMassString2.c_str()) / 2.f;
    DirectX::SimpleMath::Vector2 armMassPos2;
    posY2 += ySpacing;
    armMassPos2.x = posX2;
    armMassPos2.y = posY2;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), armMassString2.c_str(), armMassPos2, Colors::White, 0.f, armMassOrigin2);

    std::string clubLengthModString2 = pGolf->GetCharacterClubLengthMod(i);
    DirectX::SimpleMath::Vector2 clubLengthModOrigin2 = m_bitwiseFont->MeasureString(clubLengthModString2.c_str()) / 2.f;
    DirectX::SimpleMath::Vector2 clubLengthModPos2;
    posY2 += ySpacing;
    clubLengthModPos2.x = posX2;
    clubLengthModPos2.y = posY2;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), clubLengthModString2.c_str(), clubLengthModPos2, Colors::White, 0.f, clubLengthModOrigin2);

    std::string armBalancePointString2 = pGolf->GetCharacterArmBalancePoint(i);
    DirectX::SimpleMath::Vector2 armBalancePointOrigin2 = m_bitwiseFont->MeasureString(armBalancePointString2.c_str()) / 2.f;
    DirectX::SimpleMath::Vector2 armBalancePointPos2;
    posY2 += ySpacing;
    armBalancePointPos2.x = posX2;
    armBalancePointPos2.y = posY2;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), armBalancePointString2.c_str(), armBalancePointPos2, Colors::White, 0.f, armBalancePointOrigin2);

    std::string armMassMoIString2 = pGolf->GetCharacterArmMassMoI(i);
    DirectX::SimpleMath::Vector2 armMassMoIOrigin2 = m_bitwiseFont->MeasureString(armMassMoIString2.c_str()) / 2.f;
    DirectX::SimpleMath::Vector2 armMassMoIPos2;
    posY2 += ySpacing;
    armMassMoIPos2.x = posX2;
    armMassMoIPos2.y = posY2;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), armMassMoIString2.c_str(), armMassMoIPos2, Colors::White, 0.f, armMassMoIOrigin2);

    posY2 += ySpacing + ySpacing;
    bioPos.x = posX2;
    bioPos.y = posY2;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), bioString.c_str(), bioPos, Colors::White, 0.f, bioOrigin);

    std::string bioLine0String2 = pGolf->GetCharacterBioLine0(i);
    DirectX::SimpleMath::Vector2 bioLine0Origin2 = m_bitwiseFont->MeasureString(bioLine0String2.c_str()) / 2.f;
    DirectX::SimpleMath::Vector2 bioLine0Pos2;
    posY2 += ySpacing;
    bioLine0Pos2.x = posX2;
    bioLine0Pos2.y = posY2;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), bioLine0String2.c_str(), bioLine0Pos2, Colors::White, 0.f, bioLine0Origin2);

    std::string bioLine1String2 = pGolf->GetCharacterBioLine1(i);
    DirectX::SimpleMath::Vector2 bioLine1Origin2 = m_bitwiseFont->MeasureString(bioLine1String2.c_str()) / 2.f;
    DirectX::SimpleMath::Vector2 bioLine1Pos2;
    posY2 += ySpacing;
    bioLine1Pos2.x = posX2;
    bioLine1Pos2.y = posY2;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), bioLine1String2.c_str(), bioLine1Pos2, Colors::White, 0.f, bioLine1Origin2);

    std::string bioLine2String2 = pGolf->GetCharacterBioLine2(i);
    DirectX::SimpleMath::Vector2 bioLine2Origin2 = m_bitwiseFont->MeasureString(bioLine2String2.c_str()) / 2.f;
    DirectX::SimpleMath::Vector2 bioLine2Pos2;
    posY2 += ySpacing;
    bioLine2Pos2.x = posX2;
    bioLine2Pos2.y = posY2;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), bioLine2String2.c_str(), bioLine2Pos2, Colors::White, 0.f, bioLine2Origin2);

    std::string bioLine3String2 = pGolf->GetCharacterBioLine3(i);
    DirectX::SimpleMath::Vector2 bioLine3Pos2;
    DirectX::SimpleMath::Vector2 bioLine3Origin2;
    posY2 += ySpacing;
    bioLine3Pos2.x = posX2;
    bioLine3Pos2.y = posY2;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), bioLine3String2.c_str(), bioLine3Pos2, Colors::White, 0.f, bioLine3Origin2);

    if (m_menuSelect == 2)
    {
        m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos + DirectX::SimpleMath::Vector2(4.f, 4.f), nullptr, Colors::LawnGreen, 0.f, m_characterBackgroundOrigin);
        m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos + DirectX::SimpleMath::Vector2(3.f, 3.f), nullptr, Colors::LawnGreen, 0.f, m_characterBackgroundOrigin);
        m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos + DirectX::SimpleMath::Vector2(2.f, 2.f), nullptr, Colors::LawnGreen, 0.f, m_characterBackgroundOrigin);
        m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos + DirectX::SimpleMath::Vector2(1.f, 1.f), nullptr, Colors::LawnGreen, 0.f, m_characterBackgroundOrigin);
    }
    else
    {
        m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos + DirectX::SimpleMath::Vector2(-4.f, -4.f), nullptr, Colors::Gray, 0.f, m_characterBackgroundOrigin);
        m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos + DirectX::SimpleMath::Vector2(-3.f, -3.f), nullptr, Colors::Gray, 0.f, m_characterBackgroundOrigin);
        m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos + DirectX::SimpleMath::Vector2(-2.f, -2.f), nullptr, Colors::Gray, 0.f, m_characterBackgroundOrigin);
        m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos + DirectX::SimpleMath::Vector2(-1.f, -1.f), nullptr, Colors::Gray, 0.f, m_characterBackgroundOrigin);
    }
    m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos, nullptr, Colors::White, 0.f, m_characterBackgroundOrigin);
    m_character2->Draw(m_spriteBatch.get(), m_character2Pos);

    if (m_menuSelect < 0 || m_menuSelect > 2)
    {
        m_menuSelect = 0;
    }

    // Start Menu Select Highlight
    if (m_menuSelect == 0)
    {
        m_font->DrawString(m_spriteBatch.get(), menuObj0String.c_str(), menuObj0Pos + DirectX::SimpleMath::Vector2(4.f, 4.f), Colors::White, 0.f, menuObj0Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj0String.c_str(), menuObj0Pos + DirectX::SimpleMath::Vector2(-4.f, 4.f), Colors::White, 0.f, menuObj0Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj0String.c_str(), menuObj0Pos + DirectX::SimpleMath::Vector2(-4.f, -4.f), Colors::White, 0.f, menuObj0Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj0String.c_str(), menuObj0Pos + DirectX::SimpleMath::Vector2(4.f, -4.f), Colors::White, 0.f, menuObj0Origin);

        m_font->DrawString(m_spriteBatch.get(), menuObj0String.c_str(), menuObj0Pos + DirectX::SimpleMath::Vector2(2.f, 2.f), Colors::Black, 0.f, menuObj0Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj0String.c_str(), menuObj0Pos + DirectX::SimpleMath::Vector2(-2.f, 2.f), Colors::Black, 0.f, menuObj0Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj0String.c_str(), menuObj0Pos + DirectX::SimpleMath::Vector2(-2.f, -2.f), Colors::Black, 0.f, menuObj0Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj0String.c_str(), menuObj0Pos + DirectX::SimpleMath::Vector2(2.f, -2.f), Colors::Black, 0.f, menuObj0Origin);

        m_font->DrawString(m_spriteBatch.get(), menuObj0String.c_str(), menuObj0Pos, Colors::White, 0.f, menuObj0Origin);
    }
    else
    {
        m_font->DrawString(m_spriteBatch.get(), menuObj0String.c_str(), menuObj0Pos, Colors::White, 0.f, menuObj0Origin);
    }

    if (m_menuSelect == 1)
    {
        m_font->DrawString(m_spriteBatch.get(), menuObj1String.c_str(), menuObj1Pos + DirectX::SimpleMath::Vector2(4.f, 4.f), Colors::White, 0.f, menuObj1Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj1String.c_str(), menuObj1Pos + DirectX::SimpleMath::Vector2(-4.f, 4.f), Colors::White, 0.f, menuObj1Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj1String.c_str(), menuObj1Pos + DirectX::SimpleMath::Vector2(-4.f, -4.f), Colors::White, 0.f, menuObj1Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj1String.c_str(), menuObj1Pos + DirectX::SimpleMath::Vector2(4.f, -4.f), Colors::White, 0.f, menuObj1Origin);

        m_font->DrawString(m_spriteBatch.get(), menuObj1String.c_str(), menuObj1Pos + DirectX::SimpleMath::Vector2(2.f, 2.f), Colors::Black, 0.f, menuObj1Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj1String.c_str(), menuObj1Pos + DirectX::SimpleMath::Vector2(-2.f, 2.f), Colors::Black, 0.f, menuObj1Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj1String.c_str(), menuObj1Pos + DirectX::SimpleMath::Vector2(-2.f, -2.f), Colors::Black, 0.f, menuObj1Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj1String.c_str(), menuObj1Pos + DirectX::SimpleMath::Vector2(2.f, -2.f), Colors::Black, 0.f, menuObj1Origin);

        m_font->DrawString(m_spriteBatch.get(), menuObj1String.c_str(), menuObj1Pos, Colors::White, 0.f, menuObj1Origin);
    }
    else
    {
        m_font->DrawString(m_spriteBatch.get(), menuObj1String.c_str(), menuObj1Pos, Colors::White, 0.f, menuObj1Origin);
    }

    if (m_menuSelect == 2)
    {
        m_font->DrawString(m_spriteBatch.get(), menuObj2String.c_str(), menuObj2Pos + DirectX::SimpleMath::Vector2(4.f, 4.f), Colors::White, 0.f, menuObj2Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj2String.c_str(), menuObj2Pos + DirectX::SimpleMath::Vector2(-4.f, 4.f), Colors::White, 0.f, menuObj2Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj2String.c_str(), menuObj2Pos + DirectX::SimpleMath::Vector2(-4.f, -4.f), Colors::White, 0.f, menuObj2Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj2String.c_str(), menuObj2Pos + DirectX::SimpleMath::Vector2(4.f, -4.f), Colors::White, 0.f, menuObj2Origin);

        m_font->DrawString(m_spriteBatch.get(), menuObj2String.c_str(), menuObj2Pos + DirectX::SimpleMath::Vector2(2.f, 2.f), Colors::Black, 0.f, menuObj2Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj2String.c_str(), menuObj2Pos + DirectX::SimpleMath::Vector2(-2.f, 2.f), Colors::Black, 0.f, menuObj2Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj2String.c_str(), menuObj2Pos + DirectX::SimpleMath::Vector2(-2.f, -2.f), Colors::Black, 0.f, menuObj2Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj2String.c_str(), menuObj2Pos + DirectX::SimpleMath::Vector2(2.f, -2.f), Colors::Black, 0.f, menuObj2Origin);

        m_font->DrawString(m_spriteBatch.get(), menuObj2String.c_str(), menuObj2Pos, Colors::White, 0.f, menuObj2Origin);
    }
    else
    {
        m_font->DrawString(m_spriteBatch.get(), menuObj2String.c_str(), menuObj2Pos, Colors::White, 0.f, menuObj2Origin);
    }
    
}

void Game::DrawMenuMain()
{
    float lineDrawY = m_fontMenuPos.y + 15;
    float lineDrawSpacingY = 15;
    std::string menuTitle = "Main Menu";
    float menuTitlePosX = m_fontMenuPos.x;
    //float menuTitlePosY = m_fontPos.y / 2.f;
    float menuTitlePosY = lineDrawY;
    DirectX::SimpleMath::Vector2 menuTitlePos(menuTitlePosX, menuTitlePosY);
    DirectX::SimpleMath::Vector2 menuOrigin = m_titleFont->MeasureString(menuTitle.c_str()) / 2.f;
    m_titleFont->DrawString(m_spriteBatch.get(), menuTitle.c_str(), menuTitlePos + DirectX::SimpleMath::Vector2(4.f, 4.f), Colors::Green, 0.f, menuOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), menuTitle.c_str(), menuTitlePos + DirectX::SimpleMath::Vector2(3.f,3.f), Colors::Green, 0.f, menuOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), menuTitle.c_str(), menuTitlePos + DirectX::SimpleMath::Vector2(2.f, 2.f), Colors::Green, 0.f, menuOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), menuTitle.c_str(), menuTitlePos + DirectX::SimpleMath::Vector2(1.f, 1.f), Colors::Green, 0.f, menuOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), menuTitle.c_str(), menuTitlePos + DirectX::SimpleMath::Vector2(-1.f, -1.f), Colors::LawnGreen, 0.f, menuOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), menuTitle.c_str(), menuTitlePos, Colors::White, 0.f, menuOrigin);  

    lineDrawY += menuTitlePosY + lineDrawSpacingY;
    std::string menuObj0String = "Driving Range";
    DirectX::SimpleMath::Vector2 menuObj0Pos(menuTitlePosX, lineDrawY);
    DirectX::SimpleMath::Vector2 menuObj0Origin = m_font->MeasureString(menuObj0String.c_str()) / 2.f;

    lineDrawY += menuObj0Pos.y;
    std::string menuObj1String = "Charachter Select";
    //Vector2 menuObj1Pos(menuTitlePosX, menuObj0Pos.y + menuOrigin.x + 0);
    DirectX::SimpleMath::Vector2 menuObj1Pos(menuTitlePosX, lineDrawY);
    DirectX::SimpleMath::Vector2 menuObj1Origin = m_font->MeasureString(menuObj1String.c_str()) / 2.f;

    lineDrawY += menuObj0Pos.y;
    std::string menuObj2String = "Quit";
    DirectX::SimpleMath::Vector2 menuObj2Pos(menuTitlePosX, lineDrawY);
    DirectX::SimpleMath::Vector2 menuObj2Origin = m_font->MeasureString(menuObj2String.c_str()) / 2.f;

    if (m_menuSelect < 0 || m_menuSelect > 2)
    {
        m_menuSelect = 0;
    }
    if (m_menuSelect == 0)
    {
        m_font->DrawString(m_spriteBatch.get(), menuObj0String.c_str(), menuObj0Pos + DirectX::SimpleMath::Vector2(4.f, 4.f), Colors::White, 0.f, menuObj0Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj0String.c_str(), menuObj0Pos + DirectX::SimpleMath::Vector2(-4.f, 4.f), Colors::White, 0.f, menuObj0Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj0String.c_str(), menuObj0Pos + DirectX::SimpleMath::Vector2(-4.f, -4.f), Colors::White, 0.f, menuObj0Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj0String.c_str(), menuObj0Pos + DirectX::SimpleMath::Vector2(4.f, -4.f), Colors::White, 0.f, menuObj0Origin);

        m_font->DrawString(m_spriteBatch.get(), menuObj0String.c_str(), menuObj0Pos + DirectX::SimpleMath::Vector2(2.f, 2.f), Colors::Black, 0.f, menuObj0Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj0String.c_str(), menuObj0Pos + DirectX::SimpleMath::Vector2(-2.f, 2.f), Colors::Black, 0.f, menuObj0Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj0String.c_str(), menuObj0Pos + DirectX::SimpleMath::Vector2(-2.f, -2.f), Colors::Black, 0.f, menuObj0Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj0String.c_str(), menuObj0Pos + DirectX::SimpleMath::Vector2(2.f, -2.f), Colors::Black, 0.f, menuObj0Origin);

        m_font->DrawString(m_spriteBatch.get(), menuObj0String.c_str(), menuObj0Pos, Colors::White, 0.f, menuObj0Origin);
    }
    else
    {
        m_font->DrawString(m_spriteBatch.get(), menuObj0String.c_str(), menuObj0Pos, Colors::White, 0.f, menuObj0Origin);
    }

    if (m_menuSelect == 1)
    {
        m_font->DrawString(m_spriteBatch.get(), menuObj1String.c_str(), menuObj1Pos + DirectX::SimpleMath::Vector2(4.f, 4.f), Colors::White, 0.f, menuObj1Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj1String.c_str(), menuObj1Pos + DirectX::SimpleMath::Vector2(-4.f, 4.f), Colors::White, 0.f, menuObj1Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj1String.c_str(), menuObj1Pos + DirectX::SimpleMath::Vector2(-4.f, -4.f), Colors::White, 0.f, menuObj1Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj1String.c_str(), menuObj1Pos + DirectX::SimpleMath::Vector2(4.f, -4.f), Colors::White, 0.f, menuObj1Origin);

        m_font->DrawString(m_spriteBatch.get(), menuObj1String.c_str(), menuObj1Pos + DirectX::SimpleMath::Vector2(2.f, 2.f), Colors::Black, 0.f, menuObj1Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj1String.c_str(), menuObj1Pos + DirectX::SimpleMath::Vector2(-2.f, 2.f), Colors::Black, 0.f, menuObj1Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj1String.c_str(), menuObj1Pos + DirectX::SimpleMath::Vector2(-2.f, -2.f), Colors::Black, 0.f, menuObj1Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj1String.c_str(), menuObj1Pos + DirectX::SimpleMath::Vector2(2.f, -2.f), Colors::Black, 0.f, menuObj1Origin);

        m_font->DrawString(m_spriteBatch.get(), menuObj1String.c_str(), menuObj1Pos, Colors::White, 0.f, menuObj1Origin);
    }
    else
    {
        m_font->DrawString(m_spriteBatch.get(), menuObj1String.c_str(), menuObj1Pos, Colors::White, 0.f, menuObj1Origin);
    }

    if (m_menuSelect == 2)
    {
        m_font->DrawString(m_spriteBatch.get(), menuObj2String.c_str(), menuObj2Pos + DirectX::SimpleMath::Vector2(4.f, 4.f), Colors::White, 0.f, menuObj2Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj2String.c_str(), menuObj2Pos + DirectX::SimpleMath::Vector2(-4.f, 4.f), Colors::White, 0.f, menuObj2Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj2String.c_str(), menuObj2Pos + DirectX::SimpleMath::Vector2(-4.f, -4.f), Colors::White, 0.f, menuObj2Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj2String.c_str(), menuObj2Pos + DirectX::SimpleMath::Vector2(4.f, -4.f), Colors::White, 0.f, menuObj2Origin);

        m_font->DrawString(m_spriteBatch.get(), menuObj2String.c_str(), menuObj2Pos + DirectX::SimpleMath::Vector2(2.f, 2.f), Colors::Black, 0.f, menuObj2Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj2String.c_str(), menuObj2Pos + DirectX::SimpleMath::Vector2(-2.f, 2.f), Colors::Black, 0.f, menuObj2Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj2String.c_str(), menuObj2Pos + DirectX::SimpleMath::Vector2(-2.f, -2.f), Colors::Black, 0.f, menuObj2Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj2String.c_str(), menuObj2Pos + DirectX::SimpleMath::Vector2(2.f, -2.f), Colors::Black, 0.f, menuObj2Origin);

        m_font->DrawString(m_spriteBatch.get(), menuObj2String.c_str(), menuObj2Pos, Colors::White, 0.f, menuObj2Origin);
    }
    else
    {
        m_font->DrawString(m_spriteBatch.get(), menuObj2String.c_str(), menuObj2Pos, Colors::White, 0.f, menuObj2Origin);
    }
}

/* Testing different start screen designs
void Game::DrawStartScreen()
{
    std::string title = "Golf Gm 1989";
    float lineDraw = m_fontMenuPos.y + 10;
    float fontTitlePosX = m_fontPosDebug.x - 100;
    //float fontTitlePosX = m_fontPos.x;
    float fontTitlePosY = lineDraw;
    Vector2 titlePos(fontTitlePosX, fontTitlePosY);


    Vector2 titleOrigin = m_titleFont->MeasureString(title.c_str()) / 2.f;
    
    float space = 35;

    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos, Colors::LimeGreen, 0.f, titleOrigin);
    titlePos.y += titleOrigin.y + space;

    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(6.f, 6.f), Colors::White, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-6.f, -6.f), Colors::ForestGreen, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(4.f, 4.f), Colors::Black, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-4.f, -4.f), Colors::Green, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(2.f, 2.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-2.f, -2.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos, Colors::LimeGreen, 0.f, titleOrigin);

    titlePos.y += titleOrigin.y + space;

    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(4.f, 4.f), Colors::ForestGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-4.f, 4.f), Colors::ForestGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(2.f, 2.f), Colors::Green, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-2.f, 2.f), Colors::Green, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos, Colors::Lime, 0.f, titleOrigin);

    titlePos.y += titleOrigin.y + space;

    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(6.f, 6.f), Colors::White, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-6.f, -6.f), Colors::ForestGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(4.f, 4.f), Colors::Black, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-4.f, -4.f), Colors::Green, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(2.f, 2.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-2.f, -2.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos, Colors::LimeGreen, 0.f, titleOrigin);

    titlePos.y += titleOrigin.y + space;

    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(6.f, 6.f), Colors::White, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-6.f, 6.f), Colors::ForestGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(4.f, 4.f), Colors::Black, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-4.f, 4.f), Colors::Green, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(2.f, 2.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-2.f, 2.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos, Colors::LimeGreen, 0.f, titleOrigin);

    titlePos.y += titleOrigin.y + space;

    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(4.f, 4.f), Colors::White, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-4.f, 4.f), Colors::White, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(4.f, -4.f), Colors::White, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-4.f, -4.f), Colors::White, 0.f, titleOrigin);

    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(2.f, 2.f), Colors::Black, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-2.f, 2.f), Colors::Black, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(2.f, -2.f), Colors::Black, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-2.f, -2.f), Colors::Black, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos, Colors::LimeGreen, 0.f, titleOrigin);

    titlePos.y += titleOrigin.y + space;

    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(4.f, 4.f), Colors::White, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-4.f, 4.f), Colors::White, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(4.f, -4.f), Colors::White, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-4.f, -4.f), Colors::White, 0.f, titleOrigin);

    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(2.f, 2.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-2.f, 2.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(2.f, -2.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-2.f, -2.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos, Colors::LimeGreen, 0.f, titleOrigin);

    titlePos.y += titleOrigin.y + space;

    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(4.f, 4.f), Colors::Green, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-4.f, 4.f), Colors::Green, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(4.f, -4.f), Colors::Green, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-4.f, -4.f), Colors::Green, 0.f, titleOrigin);

    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(2.f, 2.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-2.f, 2.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(2.f, -2.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-2.f, -2.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos, Colors::LimeGreen, 0.f, titleOrigin);

    titlePos.y += titleOrigin.y + space;

    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(4.f, 4.f), Colors::Green, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-4.f, 4.f), Colors::Green, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(4.f, -4.f), Colors::Green, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-4.f, -4.f), Colors::Green, 0.f, titleOrigin);

    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(2.f, 2.f), Colors::Black, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-2.f, 2.f), Colors::Black, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(2.f, -2.f), Colors::Black, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-2.f, -2.f), Colors::Black, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos, Colors::LimeGreen, 0.f, titleOrigin);

    titlePos.y += titleOrigin.y + space;

    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(6.f, 6.f), Colors::White, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-6.f, -6.f), Colors::ForestGreen, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(4.f, 4.f), Colors::Black, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-4.f, -4.f), Colors::Green, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(2.f, 2.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-2.f, -2.f), Colors::White, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos, Colors::LimeGreen, 0.f, titleOrigin);

    titlePos.y += titleOrigin.y + space;

    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-8.f, -8.f), Colors::DarkGreen, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(6.f, 6.f), Colors::White, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-6.f, -6.f), Colors::ForestGreen, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(4.f, 4.f), Colors::Black, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-4.f, -4.f), Colors::Green, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(2.f, 2.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-2.f, -2.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos, Colors::LimeGreen, 0.f, titleOrigin);

    titlePos.y += titleOrigin.y + space;

    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-8.f, -8.f), Colors::DarkGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-7.f, -7.f), Colors::DarkGreen, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(6.f, 6.f), Colors::White, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-6.f, -6.f), Colors::ForestGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-5.f, -5.f), Colors::ForestGreen, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(4.f, 4.f), Colors::Black, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-4.f, -4.f), Colors::Green, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-3.f, -3.f), Colors::Green, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(2.f, 2.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-2.f, -2.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-1.f, -1.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos, Colors::LimeGreen, 0.f, titleOrigin);

    titlePos.y = m_fontMenuPos.y + 15;
    titlePos.x = m_fontPos2.x - 450;/////////////////////////////Break

    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-8.f, -8.f), Colors::DarkGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-7.f, -7.f), Colors::DarkGreen, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(6.f, 6.f), Colors::White, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-6.f, -6.f), Colors::ForestGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-5.f, -5.f), Colors::ForestGreen, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(4.f, 4.f), Colors::Black, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-4.f, -4.f), Colors::Green, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-3.f, -3.f), Colors::Green, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(2.f, 2.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-2.f, -2.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-1.f, -1.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos, Colors::LimeGreen, 0.f, titleOrigin);

    titlePos.y += titleOrigin.y + space;

    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-10.f, -10.f), Colors::Green, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-9.f, -9.f), Colors::Green, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-8.f, -8.f), Colors::Green, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-7.f, -7.f), Colors::Green, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(6.f, 6.f), Colors::White, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-6.f, -6.f), Colors::Green, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-5.f, -5.f), Colors::Green, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(4.f, 4.f), Colors::Black, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-4.f, -4.f), Colors::Green, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-3.f, -3.f), Colors::Green, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(2.f, 2.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-2.f, -2.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-1.f, -1.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos, Colors::LimeGreen, 0.f, titleOrigin);

    titlePos.y += titleOrigin.y + space;

    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-10.f, -10.f), Colors::Green, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-9.f, -9.f), Colors::Green, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-8.f, -8.f), Colors::Green, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-7.f, -7.f), Colors::Green, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(6.f, 6.f), Colors::White, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-6.f, -6.f), Colors::Green, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-5.f, -5.f), Colors::Green, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(4.f, 4.f), Colors::Black, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-4.f, -4.f), Colors::Green, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-3.f, -3.f), Colors::Green, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(2.f, 2.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-2.f, -2.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-1.f, -1.f), Colors::White, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos, Colors::LimeGreen, 0.f, titleOrigin);

    titlePos.y += titleOrigin.y + space;

    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-8.f, -8.f), Colors::DarkGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-7.f, -7.f), Colors::DarkGreen, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(6.f, 6.f), Colors::White, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-6.f, -6.f), Colors::ForestGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-5.f, -5.f), Colors::ForestGreen, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(4.f, 4.f), Colors::Black, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-4.f, -4.f), Colors::Green, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-3.f, -3.f), Colors::Green, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(2.f, 2.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-2.f, -2.f), Colors::Lime, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-1.f, -1.f), Colors::White, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos, Colors::LimeGreen, 0.f, titleOrigin);

    titlePos.y += titleOrigin.y + space;

    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-8.f, -8.f), Colors::DarkGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-7.f, -7.f), Colors::DarkGreen, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(6.f, 6.f), Colors::White, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-6.f, -6.f), Colors::ForestGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-5.f, -5.f), Colors::ForestGreen, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(4.f, 4.f), Colors::Black, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-4.f, -4.f), Colors::Green, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-3.f, -3.f), Colors::Green, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(2.f, 2.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-2.f, -2.f), Colors::White, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-1.f, -1.f), Colors::Lime, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos, Colors::LimeGreen, 0.f, titleOrigin);

    titlePos.y += titleOrigin.y + space;

    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(8.f, 8.f), Colors::DarkGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(7.f, 7.f), Colors::DarkGreen, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(6.f, 6.f), Colors::White, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(6.f, 6.f), Colors::ForestGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(5.f, 5.f), Colors::ForestGreen, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(4.f, 4.f), Colors::Black, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(4.f, 4.f), Colors::Green, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(3.f, 3.f), Colors::Green, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(2.f, 2.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(2.f, 2.f), Colors::Gray, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(1.f, 1.f), Colors::Lime, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos, Colors::LimeGreen, 0.f, titleOrigin);

    titlePos.y += titleOrigin.y + space;

    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(8.f, 8.f), Colors::DarkGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(7.f, 7.f), Colors::DarkGreen, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(6.f, 6.f), Colors::White, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(6.f, 6.f), Colors::ForestGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(5.f, 5.f), Colors::ForestGreen, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(4.f, 4.f), Colors::Black, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(4.f, 4.f), Colors::Green, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(3.f, 3.f), Colors::Green, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(2.f, 2.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(2.f, 2.f), Colors::Black, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(1.f, 1.f), Colors::Lime, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos, Colors::LimeGreen, 0.f, titleOrigin);

    titlePos.y += titleOrigin.y + space;

    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(8.f, 8.f), Colors::DarkGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(7.f, 7.f), Colors::DarkGreen, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(6.f, 6.f), Colors::White, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(6.f, 6.f), Colors::ForestGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(5.f, 5.f), Colors::ForestGreen, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(4.f, 4.f), Colors::Black, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(4.f, 4.f), Colors::Green, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(3.f, 3.f), Colors::Green, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(2.f, 2.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(2.f, 2.f), Colors::Black, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(1.f, 1.f), Colors::Lime, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos, Colors::White, 0.f, titleOrigin);

    titlePos.y += titleOrigin.y + space;

    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(8.f, 8.f), Colors::DarkGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(7.f, 7.f), Colors::DarkGreen, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(6.f, 6.f), Colors::White, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(6.f, 6.f), Colors::ForestGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(5.f, 5.f), Colors::ForestGreen, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(4.f, 4.f), Colors::Black, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(4.f, 4.f), Colors::White, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(3.f, 3.f), Colors::White, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(2.f, 2.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(2.f, 2.f), Colors::Black, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(1.f, 1.f), Colors::Lime, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos, Colors::LawnGreen, 0.f, titleOrigin);

    titlePos.y += titleOrigin.y + space;

    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-8.f, -8.f), Colors::DarkGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-7.f, -7.f), Colors::DarkGreen, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(6.f, 6.f), Colors::White, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-6.f, -6.f), Colors::ForestGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-5.f, -5.f), Colors::ForestGreen, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(4.f, 4.f), Colors::Black, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-4.f, -4.f), Colors::White, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-3.f, -3.f), Colors::White, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(2.f, 2.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-2.f, -2.f), Colors::Black, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-1.f, -1.f), Colors::Lime, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos, Colors::LawnGreen, 0.f, titleOrigin);

    titlePos.y += titleOrigin.y + space;

    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-8.f, -8.f), Colors::White, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-7.f, -7.f), Colors::DarkGreen, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(6.f, 6.f), Colors::White, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-6.f, -6.f), Colors::DarkGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-5.f, -5.f), Colors::ForestGreen, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(4.f, 4.f), Colors::Black, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-4.f, -4.f), Colors::ForestGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-3.f, -3.f), Colors::White, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(2.f, 2.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-2.f, -2.f), Colors::Black, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-1.f, -1.f), Colors::Lime, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos, Colors::LawnGreen, 0.f, titleOrigin);

    titlePos.y += titleOrigin.y + space;

    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-8.f, -8.f), Colors::White, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-7.f, -7.f), Colors::DarkGreen, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(6.f, 6.f), Colors::White, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-6.f, -6.f), Colors::DarkGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-5.f, -5.f), Colors::ForestGreen, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(4.f, 4.f), Colors::Black, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-4.f, -4.f), Colors::ForestGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-3.f, -3.f), Colors::ForestGreen, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(2.f, 2.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-2.f, -2.f), Colors::Black, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-1.f, -1.f), Colors::Lime, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos, Colors::LawnGreen, 0.f, titleOrigin);

    titlePos.y += titleOrigin.y + space;
}
*/

void Game::DrawStartScreen()
{
    std::string title = "GolfGame1989";
    std::string author = "By Lehr Jackson";
    std::string startText = "Press Enter to Start";
    float fontTitlePosX = m_fontPos.x;
    float fontTitlePosY = m_fontPos.y / 2.f;
    DirectX::SimpleMath::Vector2 titlePos(fontTitlePosX, fontTitlePosY);
    float fontAuthorPosX = m_fontPos.x;
    float fontAuthorPosY = m_fontPos.y;
    DirectX::SimpleMath::Vector2 authorPos(fontAuthorPosX, fontAuthorPosY);
    DirectX::SimpleMath::Vector2 startTextPos(m_fontPos.x, m_fontPos.y + fontTitlePosY);

    DirectX::SimpleMath::Vector2 titleOrigin = m_titleFont->MeasureString(title.c_str()) / 2.f;
    DirectX::SimpleMath::Vector2 authorOrigin = m_font->MeasureString(author.c_str()) / 2.f;
    DirectX::SimpleMath::Vector2 startTextOrigin = m_font->MeasureString(startText.c_str()) / 2.f;

    /*
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(6.f, 6.f), Colors::White, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-6.f, -6.f), Colors::ForestGreen, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(4.f, 4.f), Colors::Black, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-4.f, -4.f), Colors::Green, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(2.f, 2.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-2.f, -2.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos, Colors::LimeGreen, 0.f, titleOrigin);
    */
//////////////////////////////////////////////////////////////////////
    /*
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(11.f, 11.f), Colors::Green, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(10.f, 10.f), Colors::Green, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(9.f, 9.f), Colors::Green, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(8.f, 8.f), Colors::Green, 0.f, titleOrigin);
    */
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + DirectX::SimpleMath::Vector2(7.f, 7.f), Colors::Green, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(6.f, 6.f), Colors::White, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + DirectX::SimpleMath::Vector2(6.f, 6.f), Colors::Green, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + DirectX::SimpleMath::Vector2(5.f, 5.f), Colors::Green, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(4.f, 4.f), Colors::Black, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + DirectX::SimpleMath::Vector2(4.f, 4.f), Colors::Green, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + DirectX::SimpleMath::Vector2(3.f, 3.f), Colors::Green, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(2.f, 2.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + DirectX::SimpleMath::Vector2(2.f, 2.f), Colors::Green, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + DirectX::SimpleMath::Vector2(-1.f, -1.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos, Colors::LimeGreen, 0.f, titleOrigin);
    
/////////////////////////////////////////////////////////////
    /*
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(8.f, 8.f), Colors::DarkGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(7.f, 7.f), Colors::DarkGreen, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(6.f, 6.f), Colors::White, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(6.f, 6.f), Colors::ForestGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(5.f, 5.f), Colors::ForestGreen, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(4.f, 4.f), Colors::Black, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(4.f, 4.f), Colors::Green, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(3.f, 3.f), Colors::Green, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(2.f, 2.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(2.f, 2.f), Colors::DarkOliveGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-1.f, -1.f), Colors::White, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos, Colors::LimeGreen, 0.f, titleOrigin);
    */
/////////////////////////////////////////////////////////////////////////////////////////////////////    

    m_font->DrawString(m_spriteBatch.get(), author.c_str(), authorPos, Colors::White, 0.f, authorOrigin);
    m_font->DrawString(m_spriteBatch.get(), startText.c_str(), startTextPos, Colors::White, 0.f, startTextOrigin);
}

void Game::DrawSwing()
{
    float shoulderAccel = .98;

    std::vector<DirectX::SimpleMath::Vector3> angles;
    angles = pGolf->GetRawSwingAngles();
    DirectX::SimpleMath::Vector3 origin;
    origin.Zero;
    origin += m_swingOrigin;
    DirectX::SimpleMath::Vector3 thetaOrigin;
    thetaOrigin.Zero;
    thetaOrigin.y = -.02;

    VertexPositionColor shoulder(origin, Colors::White);

    int swingStepCount = angles.size();
    if (m_swingPathStep >= swingStepCount)
    {
        m_swingPathStep = 0;
    }
    ++m_swingPathStep;

    int impactPoint = pGolf->GetImpactStep();

    for (int i = 0; i < m_swingPathStep; ++i)
    {
        if (i < impactPoint)
        {
            DirectX::SimpleMath::Vector3 theta = DirectX::SimpleMath::Vector3::Transform(thetaOrigin, DirectX::SimpleMath::Matrix::CreateRotationZ(-angles[i].z));
            DirectX::SimpleMath::Vector3 beta = DirectX::SimpleMath::Vector3::Transform(theta, DirectX::SimpleMath::Matrix::CreateRotationZ(-angles[i].y));
            theta += m_swingOrigin;
                       
            //beta += m_swingOrigin;
            beta += theta;
            VertexPositionColor thetaColor(theta, Colors::Blue);
            VertexPositionColor betaColor(beta, Colors::Red);
            m_batch->DrawLine(shoulder, thetaColor);
            m_batch->DrawLine(thetaColor, betaColor);
        }
    }
}

void Game::OnDeviceLost()
{
    // TODO: Add Direct3D resource cleanup here.
    m_states.reset();
    m_effect.reset();
    m_batch.reset();
    m_inputLayout.Reset();
    m_font.reset();
    m_titleFont.reset();
    m_bitwiseFont.reset();
    m_spriteBatch.reset();
    m_kbStateTracker.Reset();
    
    //Powerbar
    m_powerFrameTexture.Reset();
    m_powerMeterTexture.Reset();
    m_powerImpactTexture.Reset();
    m_powerBackswingTexture.Reset();

    // Charcter
    m_character.reset();
    m_characterTexture.Reset();
    m_character0.reset();
    m_character0Texture.Reset();
    m_character1.reset();
    m_character1Texture.Reset();
    m_character2.reset();
    m_character2Texture.Reset();
    m_characterBackgroundTexture.Reset();

    m_depthStencilView.Reset();
    m_renderTargetView.Reset();
    m_swapChain.Reset();
    m_d3dContext.Reset();
    m_d3dDevice.Reset();

    CreateDevice();
    CreateResources();
}
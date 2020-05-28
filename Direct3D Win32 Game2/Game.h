//
// Game.h
//

#pragma once

#include "StepTimer.h"
#include "Golf.h"
//#include "TextConsole.h"

// A basic game implementation that creates a D3D11 device and
// provides a game loop.
class Game
{
public:

    
    Game() noexcept;
    ~Game() = default;
    

    Game(Game&&) = default;
    Game& operator= (Game&&) = default;

    Game(Game const&) = delete;
    Game& operator= (Game const&) = delete;

    // Initialization and management
    void Initialize(HWND window, int width, int height);

    // Basic game loop
    void Tick();

    // Messages
    void OnActivated();
    void OnDeactivated();
    void OnSuspending();
    void OnResuming();
    void OnWindowSizeChanged(int width, int height);

    // Properties
    void GetDefaultSize( int& width, int& height ) const noexcept;

private:

    void Update(DX::StepTimer const& timer);
    void Render();

    void Clear();
    void Present();

    void CreateDevice();
    void CreateResources();

    void OnDeviceLost();

    // Device resources.
    HWND                                            m_window;
    int                                             m_outputWidth;
    int                                             m_outputHeight;

    D3D_FEATURE_LEVEL                               m_featureLevel;
    Microsoft::WRL::ComPtr<ID3D11Device1>           m_d3dDevice;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext1>    m_d3dContext;

    Microsoft::WRL::ComPtr<IDXGISwapChain1>         m_swapChain;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView>  m_renderTargetView;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView>  m_depthStencilView;

    // Rendering loop timer.
    DX::StepTimer                                   m_timer;

    // WLJ add start
    using VertexType = DirectX::VertexPositionColor;

    std::unique_ptr<DirectX::CommonStates> m_states;
    std::unique_ptr<DirectX::BasicEffect> m_effect;
    std::unique_ptr<DirectX::PrimitiveBatch<VertexType>> m_batch;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;

    //std::unique_ptr<DX::TextConsole> m_console; // ????? WLJ add Text Console 


    // End

    //world start
    DirectX::SimpleMath::Matrix m_world;
    DirectX::SimpleMath::Matrix m_view;
    DirectX::SimpleMath::Matrix m_proj;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_raster; // WLJ anti-aliasing
    //world end

    // WLJ added for drawing projectile and swing
    Golf* pGolf;
    int arcCount = 0;

    // WLJ added for displaying text
    std::unique_ptr<DirectX::SpriteFont> m_font;
    DirectX::SimpleMath::Vector2 m_fontPos;
    DirectX::SimpleMath::Vector2 m_fontPos2;
    std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;

};
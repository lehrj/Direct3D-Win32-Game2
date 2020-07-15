#pragma once

struct Environ
{
    std::string                         name;
    DirectX::XMVECTORF32                terainColor;
    double                              airDensity;        // in kg/m^3
    double                              gravity;           // in m/s^2
    double                              windX;             // in m/s
    double                              windY;             // in m/s
    double                              windZ;             // in m/s
    double                              landingFrictionScale;
    double                              landingHardnessScale;
};

class Environment
{
public:
    Environment();

    double GetAirDensity() const { return m_airDensity; };
    double GetAirDensity(const int aEnvironmentIndex) const { return m_environs[aEnvironmentIndex].airDensity; };
    double GetGravity() const { return m_gravity; };
    double GetGravity(const int aEnvironmentIndex) const { return m_environs[aEnvironmentIndex].gravity; };
    double GetLandingHeight() const { return m_landingHeight; };
    double GetLauchHeight() const { return m_launchHeight; };
    DirectX::XMVECTORF32 GetEnvironColor() const { return m_currentEnviron.terainColor; };
    std::string GetEnvironName(const int aEnvironmentIndex) const {return m_environs[aEnvironmentIndex].name;};
    int GetNumerOfEnvirons() const { return m_environsAvailable; };
    int GetNumberOfEnvironSelectDisplayVariables() const { return m_environSelectDisplayDataPoints; };
    double GetWindX() const { return m_windX; };
    double GetWindX(const int aEnvironmentIndex) const { return m_environs[aEnvironmentIndex].windX; };
    double GetWindY() const { return m_windY; };
    double GetWindY(const int aEnvironmentIndex) const { return m_environs[aEnvironmentIndex].windY; };
    double GetWindZ() const { return m_windZ; };
    double GetWindZ(const int aEnvironmentIndex) const { return m_environs[aEnvironmentIndex].windZ; };

    void PrintEnvironmentData();
    void ReadInEnvironmentData();
    void SetDefaultEnvironment();
    void UpdateEnvironment(const int aIndex);

private:
    void InputEnvironmentData();
    void InputEnvironmentDataBasic();
    void LoadEnvironmentData();
    void SetAirDensity(double aDensity);   
    void SetGravity(double aGravity);
    void SetLandingHeight(double aLandingHeight);
    void SetLauchHeight(double aLaunchHeight);
    void SetWindX(const double& aWindX);
    void SetWindY(const double& aWindY);
    void SetWindZ(const double& aWindZ);

    double                              m_airDensity = 1.225;        // in kg/m^3
    double                              m_gravity = -9.8;           // in m/s^2
    double                              m_landingHeight = 0.0;     // in meters
    double                              m_launchHeight = 0.0;      // in meters
    double                              m_windX = 0.0;             // in m/s
    double                              m_windY = 0.0;             // in m/s
    double                              m_windZ = 0.0;             // in m/s

    // variables for bounce and roll functionality not yet implemented
    double                              m_landingFriction;
    double                              m_landingHardness;
    double                              m_landingXslope;
    double                              m_landingZslope;

    // min max consts
    const double                        m_minAirDensity = 0.0;
    const double                        m_maxAirDensity = 68.0; // just above the air density of Venus
    const double                        m_minGravity = 0.1;
    const double                        m_maxGravity = 28.0; // approximate value for the mass of the sun
    const double                        m_minMaxHeight = 450.0; // Launch & Landing min/max heights is just above the largest elevation change (>400 meters) of any real golf course which is the Extreme 19 in Limpopo Province South Africa
    const double                        m_minMaxWind = 667.0;// highest know wind speed on Neptune

    std::vector<Environ>                m_environs;
    Environ                             m_currentEnviron;
    const int                           m_environsAvailable = 3;
    const int                           m_environSelectDisplayDataPoints = 5;
};


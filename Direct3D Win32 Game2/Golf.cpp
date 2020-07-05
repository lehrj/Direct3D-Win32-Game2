#include "pch.h"
#include "Golf.h"
#include <fstream>

Golf::Golf()
{
    //BuildVector();
    pEnvironment = new Environment();
    pEnvironment->SetDefaultEnvironment();
    pCharacter = new GolfCharacter();
    pSwing = new GolfSwing();
    pSwing->SetDefaultSwingValues(pEnvironment->GetGravity());
    
    pSwing->UpdateGolfSwingValues();
    pBall = new GolfBall();
    pBall->SetDefaultBallValues(pEnvironment);
    pPlay = new GolfPlay();
    SetCharacter(0);
    BuildVector();
    BuildUIstrings();
}

Golf::~Golf()
{
    delete pBall;
    delete pSwing;
    delete pEnvironment;
    delete pCharacter;
    delete pPlay;
}

void Golf::BuildVector()
{
    pBall->FireProjectile(pSwing->CalculateLaunchVector(), pEnvironment);
    InputData();
    ScaleCordinates();
}

void Golf::CopyShotPath(std::vector<DirectX::SimpleMath::Vector3>& aPath)
{
    m_shotPath = aPath;
}

void Golf::BuildUIstrings()
{
    m_uiStrings.clear();
    m_uiStrings.push_back("Air Density = " + std::to_string(pEnvironment->GetAirDensity()) + " kg/m cubed"); // WLJ need to find sweet font that supports "^"
    m_uiStrings.push_back("Wind X = " + std::to_string(pEnvironment->GetWindX()) + " m/s");
    m_uiStrings.push_back("Wind Y = " + std::to_string(pEnvironment->GetWindY()) + " m/s");
    m_uiStrings.push_back("Wind Z = " + std::to_string(pEnvironment->GetWindZ()) + " m/s");
    m_uiStrings.push_back("Shot Power = " + std::to_string(pSwing->GetBackSwingPercentage()) + " percent");
    m_uiStrings.push_back("Club Name = " + pSwing->GetClubName());
    m_uiStrings.push_back("Club Angle = " + std::to_string(pSwing->GetClubAngle()) + " degrees");
    m_uiStrings.push_back("Club Length = " + std::to_string(pSwing->GetClubLength()) + " meters");
    m_uiStrings.push_back("Club Mass = " + std::to_string(pSwing->GetClubMass()) + " kg");
    m_uiStrings.push_back("Launch Angle = " + std::to_string(pSwing->GetLaunchAngle()) + " degrees");
    m_uiStrings.push_back("Launch Velocity = " + std::to_string(pSwing->GetLaunchVelocity()) + " m/s");
    m_uiStrings.push_back("Initial Spin Rate = " + std::to_string(pBall->GetInitialSpinRate()) + " m/s");
    m_uiStrings.push_back("Landing Spin Rate = " + std::to_string(pBall->GetLandingSpinRate()) + " m/s");
    m_uiStrings.push_back("Travel Distance = " + std::to_string(pBall->GetShotDistance()) + " meters");
    m_uiStrings.push_back("Max Height = " + std::to_string(pBall->GetMaxHeight()) + " meters");
    m_uiStrings.push_back("Landing Height = " + std::to_string(pBall->GetLandingHeight()) + " meters");
    m_uiStrings.push_back("Bounce Count = " + std::to_string(pBall->GetBounceCount()));
    m_uiStrings.push_back("Character Name = " + pCharacter->GetName(m_selectedCharacter));
}

void Golf::InputData()
{
    m_shotPath.clear();
    CopyShotPath(pBall->OutputShotPath());
}

std::string Golf::GetCharacterArmBalancePoint(const int aCharacterIndex) const
{
    std::string armBalancePointString = "Arm Balance Point = " + std::to_string(pCharacter->GetArmBalancePoint(aCharacterIndex)) + " %";
    return armBalancePointString;
}
std::string Golf::GetCharacterArmLength(const int aCharacterIndex) const
{
    std::string armLengthString = "Arm Length = " + std::to_string(pCharacter->GetArmLength(aCharacterIndex)) + " m";
    return armLengthString;
}

std::string Golf::GetCharacterArmMass(const int aCharacterIndex) const
{
    std::string armMassString = "Arm Mass = " + std::to_string(pCharacter->GetArmMass(aCharacterIndex)) + " kg";
    return armMassString;
}

std::string Golf::GetCharacterClubLengthMod(const int aCharacterIndex) const
{
    std::string clubLengthModString = "Club Length = x" + std::to_string(pCharacter->GetClubLenghtModifier(aCharacterIndex)) + " m";
    return clubLengthModString;
}

std::string Golf::GetCharacterArmMassMoI(const int aCharacterIndex) const
{
    std::string armMassMoIString = "Arm MoI = " + std::to_string(pCharacter->GetArmMassMoI(aCharacterIndex)) + " kg Squared";
    return armMassMoIString;
}

std::string Golf::GetCharacterBioLine0(const int aCharacterIndex) const
{
    return pCharacter->GetBioLine0(aCharacterIndex);
}

std::string Golf::GetCharacterBioLine1(const int aCharacterIndex) const
{
    return pCharacter->GetBioLine1(aCharacterIndex);
}

std::string Golf::GetCharacterBioLine2(const int aCharacterIndex) const
{
    return pCharacter->GetBioLine2(aCharacterIndex);
}

std::string Golf::GetCharacterBioLine3(const int aCharacterIndex) const
{
    return pCharacter->GetBioLine3(aCharacterIndex);
}

std::string Golf::GetCharacterName(const int aCharacterIndex) const
{
    return pCharacter->GetName(aCharacterIndex);
}

int Golf::GetDrawColorIndex()
{
    return pBall->GetColorIndex();
}

std::vector<int> Golf::GetDrawColorVector()
{
    return pBall->GetColorVector();
}

void Golf::LoadCharacterTraits()
{
    pSwing->SetArmBalancePoint(pCharacter->GetArmBalancePoint(m_selectedCharacter));
    pSwing->SetArmLength(pCharacter->GetArmLength(m_selectedCharacter));
    pSwing->SetArmMass(pCharacter->GetArmMass(m_selectedCharacter));
    pSwing->SetArmMassMoI(pCharacter->GetArmMassMoI(m_selectedCharacter));
    pSwing->SetClubLengthModifier(pCharacter->GetClubLenghtModifier(m_selectedCharacter));
    pSwing->UpdateGolfer();
}

void Golf::ScaleCordinates()
{
    DirectX::SimpleMath::Matrix scaleMatrix = DirectX::SimpleMath::Matrix::Identity;
    double scaleFactor = .02;
    double sX = scaleFactor;
    double sY = scaleFactor;
    double sZ = scaleFactor;
    scaleMatrix = DirectX::SimpleMath::Matrix::CreateScale(sX, sY, sZ);

    for (int i = 0; i < m_shotPath.size(); ++i)
    {
        m_shotPath[i] = DirectX::SimpleMath::Vector3::Transform(m_shotPath[i], scaleMatrix);
        TransformCordinates(i);
    }  
}

/*
void Golf::SelectNextClub()
{
    pSwing->SetDefaultSwingValues(pEnvironment->GetGravity());
    pBall->ResetBallData();
    pSwing->CycleClub();
    pSwing->ResetAlphaBeta();
    pSwing->UpdateGolfSwingValues();
    BuildVector();

    BuildUIstrings();
}
*/

void Golf::SelectInputClub(int aInput)
{
    pBall->ResetBallData();
    pSwing->SetDefaultSwingValues(pEnvironment->GetGravity());

    pSwing->InputClub(aInput);
    pSwing->ResetAlphaBeta();
    pSwing->UpdateGolfSwingValues();
    BuildVector();
    BuildUIstrings();
}

void Golf::SetCharacter(const int aCharacterIndex)
{
    if (aCharacterIndex < 0 || aCharacterIndex > pCharacter->GetCharacterCount() - 1)
    {
        m_selectedCharacter = 0;
    }
    else
    {
        m_selectedCharacter = aCharacterIndex;
    }
    LoadCharacterTraits();
}

void Golf::SetShotCordMax()
{
    double maxX = 0.0;
    double maxY = 0.0;
    double maxZ = 0.0;
    for (int i = 0; i < m_shotPath.size(); ++i)
    {
        if (maxX < m_shotPath[i].x)
        {
            maxX = m_shotPath[i].x;
        }
        if (maxY < m_shotPath[i].y)
        {
            maxY = m_shotPath[i].y;
        }
        if (maxZ < m_shotPath[i].z)
        {
            maxZ = m_shotPath[i].z;
        }
    }
    m_maxX = maxX;
    m_maxY = maxY;
    m_maxZ = maxZ;
}

//Transform shotpath to start at edge of world grid
void Golf::TransformCordinates(const int aIndex)
{
    m_shotPath[aIndex].x -= 2;
}

void Golf::UpdateImpact(Utility::ImpactData aImpact)
{
    pSwing->UpdateImpactData(aImpact);
    pBall->ResetBallData();
    pSwing->ResetAlphaBeta();
    pSwing->UpdateGolfSwingValues();
    BuildVector();
    BuildUIstrings();
}
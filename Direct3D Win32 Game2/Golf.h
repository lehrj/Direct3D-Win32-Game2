#pragma once
#include <vector>
#include "GolfBag.h"
#include "GolfSwing.h"
#include "GolfBall.h"
#include "Environment.h"
#include "Utility.h"
#include "Vector4d.h"


class Golf
{
public:
    Golf();
    ~Golf();

    void BuildVector();
    Vector4d GetLaunchVector();
    std::vector<Vector4d> GetSwingData();
    const int GetSwingStepIncCount() { return pSwing->GetSwingStepIncrementCount(); };

    void CalculateData();
    void SetMaxX();
    void SetMaxY();
    void SetMaxZ();

    std::vector<double> GetVect(const int aInput);
    void InputData();
    void NormalizeData();

private:
    Environment* pEnvironment;
    GolfSwing* pSwing;
    GolfBall* pBall;

    void CopyXvec(std::vector<double> aVec);
    void CopyYvec(std::vector<double> aVec);
    void CopyZvec(std::vector<double> aVec);
    double m_maxX;
    double m_maxY;
    double m_maxZ;
    double m_xWindow;
    double m_yWindow;
    double m_zWindow;

    std::vector<double> m_xNorm;
    std::vector<double> m_yNorm;
    std::vector<double> m_zNorm;

    std::vector<double> m_xVals;
    std::vector<double> m_yVals;
    std::vector<double> m_zVals;

};


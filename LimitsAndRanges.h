#pragma once

//********************************************************************
//
//  Notes:
// 
//  -   'value01': valid range only from 0.0 to 1.0
//  -   'value11': valid range only from -1.0 to 1.0
//  -   if 'u' is added, e.g. 'value01u' it means 'unconstrained':
//      normal part of the range is still between 0 and 1 but value 
//      can be outside of that
//
//********************************************************************

void normalizeRange(double& min, double& max);
void normalizeRange(double& min, double& max, double& val);

double value11To01(double value11u);
double value01To11(double value01u);

//returns 0 if value is at left, returns 1 if value is at right. Result clamped to [0,1]
double getValue01Clamped(double value, double left, double right);
double getValue11Clamped(double value, double left, double right);

template<typename T>
T mix(T a, T b, double b_ratio01)
{
    return a + (b - a) * b_ratio01;
}
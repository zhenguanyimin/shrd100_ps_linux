#ifndef _DIRCALC_H__
#define _DIRCALC_H__

#define MAXnAngle 128
#define UnitAngle 0.01
#define PI_BY_180 0.01745329252
#define PitchCircMin 0
#define PitchCircMax 30.0

int dirCalc(int *ampDat, int *angleDat, int nAngle, int *ampMax, int *angleCen);
int pitchCalc(int *ampDat, int *pitchDat, int nPitch, int *ampMax, int *pitchCen);
int pitchCalcMax(int *ampDat, int *pitchDat, int nPitch, int *ampMax, int *pitchCen);

#endif



#include <stdlib.h>

#include "dirCalc.h"
#include "math.h"

int dirCalc(int *ampDat, int *angleDat, int nAngle, int *ampMax, int *angleCen)
{
	// if fail
	if (nAngle <= 0)
	{
		return(0);
	}
	if (nAngle > MAXnAngle)
	{
		return(0);
	}
	
	// if only 1 data
	if (nAngle == 1)
	{
		*ampMax = ampDat[0];
		*angleCen = angleDat[0];
		return(1);
	}

	// initialization
	int *angleDatSort = (int *)malloc(nAngle * sizeof(int));
	int *ampDatSort = (int *)malloc(nAngle * sizeof(int));
	int *angleDatDiff = (int *)malloc(nAngle * sizeof(int));
	int *wCoef = (int *)malloc(nAngle * sizeof(int));
	int ampDatTmp = 0, angleDatTmp = 0, ampDatMin = 0;
	int iAngle = 0, jAngle = 0;
	int maxAngleDiff = 0, iMaxAngleDiff = 0;
	int tmpAngleDiff = 0, sumAngle = 0;
	double sumNorth = 0.0, sumWest = 0.0;

	// sort angleDat
	*ampMax = 0;
	for (iAngle = 0; iAngle < nAngle; iAngle++)
	{
		angleDatSort[iAngle] = angleDat[iAngle];
		ampDatSort[iAngle] = ampDat[iAngle];

		// find min amp
		if (ampDat[iAngle] > 0)
		{
			if (ampDat[iAngle] < ampDatMin || ampDatMin == 0)
			ampDatMin = ampDat[iAngle];
		}

		// find max amp
		if (ampDat[iAngle] > *ampMax)
		{
			*ampMax = ampDat[iAngle];
		}
	}
	for (iAngle = 0; iAngle < nAngle; iAngle++)
	{
		if (ampDatSort[iAngle] > 0)
		{
			ampDatSort[iAngle] -= ampDatMin;
		}
	}
	// max - 6dB
	for (iAngle = 0; iAngle < nAngle; iAngle++)
	{
		if (*ampMax - ampDatMin > 510)
		{
			if (ampDatSort[iAngle] > *ampMax - ampDatMin - 510)
			{
				ampDatSort[iAngle] -= *ampMax - ampDatMin - 510;
			}
			else
			{
				ampDatSort[iAngle] = 0;
			}
		}
	}

	jAngle = nAngle - 1;
	while (jAngle > 0)
	{
		for (iAngle = 0; iAngle < jAngle; iAngle++)
		{
			if (angleDatSort[iAngle] > angleDatSort[iAngle + 1])
			{
				angleDatTmp = angleDatSort[iAngle + 1];
				angleDatSort[iAngle + 1] = angleDatSort[iAngle];
				angleDatSort[iAngle] = angleDatTmp;
				ampDatTmp = ampDatSort[iAngle + 1];
				ampDatSort[iAngle + 1] = ampDatSort[iAngle];
				ampDatSort[iAngle] = ampDatTmp;
			}
		}
		jAngle--;
	}

	// calculate angle diff
	for (iAngle = 0; iAngle < nAngle - 1; iAngle++)
	{
		angleDatDiff[iAngle] = angleDatSort[iAngle + 1] - angleDatSort[iAngle];
	}
	angleDatDiff[nAngle - 1] = angleDatSort[0] - angleDatSort[nAngle - 1] + (int)(360.0 / UnitAngle);

	// calculate max angle diff
	for (iAngle = 0; iAngle < nAngle; iAngle++)
	{
		if (angleDatDiff[iAngle] > maxAngleDiff)
		{
			maxAngleDiff = angleDatDiff[iAngle];
			iMaxAngleDiff = iAngle;
		}
	}

	// calculate weighting coefficient
	wCoef[0] = (angleDatDiff[0] + angleDatDiff[nAngle - 1]) / 2;
	for (iAngle = 1; iAngle < nAngle; iAngle++)
	{
		wCoef[iAngle] = (angleDatDiff[iAngle] + angleDatDiff[iAngle - 1]) / 2;
	}
	/*
	if (iMaxAngleDiff == 0)
	{
		wCoef[0] = angleDatDiff[nAngle - 1];
		wCoef[1] = angleDatDiff[1];
	}
	else if (iMaxAngleDiff == nAngle - 1)
	{
		wCoef[nAngle - 1] = angleDatDiff[nAngle - 2];
		wCoef[0] = angleDatDiff[0];
	}
	else
	{
		wCoef[iMaxAngleDiff] = angleDatDiff[iMaxAngleDiff - 1];
		wCoef[iMaxAngleDiff + 1] = angleDatDiff[iMaxAngleDiff + 1];
	}
	*/

	// calculate centrol angle
	for (iAngle = 0; iAngle < nAngle; iAngle++)
	{
		sumNorth += cos((double)(angleDatSort[iAngle]) * UnitAngle * PI_BY_180)
				  * (double)(ampDatSort[iAngle]) * (double)(wCoef[iAngle]);
		sumWest += sin((double)(angleDatSort[iAngle]) * UnitAngle * PI_BY_180)
			     * (double)(ampDatSort[iAngle]) * (double)(wCoef[iAngle]);
	}
	*angleCen = (int)(atan2(sumWest, sumNorth) / PI_BY_180 / UnitAngle);

	for (iAngle = 0; iAngle < nAngle - 1; iAngle++)
	{
		tmpAngleDiff = angleDat[iAngle + 1] - angleDat[iAngle];

		if (tmpAngleDiff >(int)(180.0 / UnitAngle))
		{
			tmpAngleDiff -= (int)(360.0 / UnitAngle);
		}
		if (tmpAngleDiff < (int)(-180.0 / UnitAngle))
		{
			tmpAngleDiff += (int)(360.0 / UnitAngle);
		}

		sumAngle += tmpAngleDiff;
	}
	*angleCen += (int)((float)sumAngle / (float)nAngle / 0.210 * 0.2); // T = 210ms, Delay = 500ms
	if (*angleCen < 0)
	{
		*angleCen += (int)(360.0 / UnitAngle);
	}

	return(1);
}

int pitchCalc(int *ampDat, int *pitchDat, int nPitch, int *ampMax, int *pitchCen)
{
	// if fail
	if (nPitch <= 0)
	{
		return(0);
	}
	if (nPitch > MAXnAngle)
	{
		return(0);
	}

	// if only 1 data
	if (nPitch == 1)
	{
		*ampMax = ampDat[0];
		*pitchCen = pitchDat[0];
		return(1);
	}

	// initialization
	int *pitchDatSort = (int *)malloc(nPitch * sizeof(int));
	int *ampDatSort = (int *)malloc(nPitch * sizeof(int));
	int *pitchDatDiff = (int *)malloc(nPitch * sizeof(int));
	int *wCoef = (int *)malloc(nPitch * sizeof(int));
	int ampDatTmp = 0, pitchDatTmp = 0, ampDatMin = 0;
	int iPitch = 0, jPitch = 0;
	int maxPitchDiff = 0, iMaxPitchDiff = 0;
	double sumFront = 0.0, sumUp = 0.0;

	// sort angleDat
	*ampMax = 0;
	for (iPitch = 0; iPitch < nPitch; iPitch++)
	{
		pitchDatSort[iPitch] = pitchDat[iPitch];
		ampDatSort[iPitch] = ampDat[iPitch];

		// find min amp
		if (ampDat[iPitch] > 0)
		{
			if (ampDat[iPitch] < ampDatMin || ampDatMin == 0)
				ampDatMin = ampDat[iPitch];
		}

		// find max amp
		if (ampDat[iPitch] > *ampMax)
		{
			*ampMax = ampDat[iPitch];
		}
	}
	for (iPitch = 0; iPitch < nPitch; iPitch++)
	{
		if (ampDatSort[iPitch] > 0)
		{
			ampDatSort[iPitch] -= ampDatMin;
		}
	}

	jPitch = nPitch - 1;
	while (jPitch > 0)
	{
		for (iPitch = 0; iPitch < jPitch; iPitch++)
		{
			if (pitchDatSort[iPitch] > pitchDatSort[iPitch + 1])
			{
				pitchDatTmp = pitchDatSort[iPitch + 1];
				pitchDatSort[iPitch + 1] = pitchDatSort[iPitch];
				pitchDatSort[iPitch] = pitchDatTmp;
				ampDatTmp = ampDatSort[iPitch + 1];
				ampDatSort[iPitch + 1] = ampDatSort[iPitch];
				ampDatSort[iPitch] = ampDatTmp;
			}
		}
		jPitch--;
	}

	// calculate angle diff
	for (iPitch = 0; iPitch < nPitch - 1; iPitch++)
	{
		pitchDatDiff[iPitch] = pitchDatSort[iPitch + 1] - pitchDatSort[iPitch];
	}


	// calculate weighting coefficient
	wCoef[0] = (pitchDatDiff[0]) / 2;
	for (iPitch = 1; iPitch < nPitch - 1; iPitch++)
	{
		wCoef[iPitch] = (pitchDatDiff[iPitch] + pitchDatDiff[iPitch - 1]) / 2;
	}
	wCoef[nPitch - 1] = (pitchDatDiff[nPitch - 2]) / 2;

	// calculate centrol angle
	for (iPitch = 0; iPitch < nPitch; iPitch++)
	{
		if (pitchDatSort[iPitch] < (int)(PitchCircMin / UnitAngle))
		{
			continue;
		}
		if (pitchDatSort[iPitch] > (int)(PitchCircMax / UnitAngle))
		{
			continue;
		}
		sumFront += cos((double)(pitchDatSort[iPitch]) * UnitAngle * PI_BY_180)
			* (double)(ampDatSort[iPitch]) * (double)(wCoef[iPitch]);
		sumUp += sin((double)(pitchDatSort[iPitch]) * UnitAngle * PI_BY_180)
			* (double)(ampDatSort[iPitch]) * (double)(wCoef[iPitch]);
	}
	*pitchCen = (int)(atan2(sumUp, sumFront) / PI_BY_180 / UnitAngle);
	if (*pitchCen < (int)(PitchCircMin / UnitAngle))
	{
		*pitchCen = (int)(PitchCircMin / UnitAngle);
	}
	if (*pitchCen > (int)(PitchCircMax / UnitAngle))
	{
		*pitchCen = (int)(PitchCircMax / UnitAngle);
	}

	return(1);
}

// search max amp for pitch
int pitchCalcMax(int *ampDat, int *pitchDat, int nPitch, int *ampMax, int *pitchCen)
{
	// if fail
	if (nPitch <= 0)
	{
		return(0);
	}
	if (nPitch > MAXnAngle)
	{
		return(0);
	}

	int ampDatMax = 0;
	int iPitchMax = 0;
	int iPitch = 0;
	int nValid = 0;

	for (iPitch = 0; iPitch < nPitch; iPitch++)
	{
		if (pitchDat[iPitch] < (int)(PitchCircMin / UnitAngle))
		{
			continue;
		}
		if (pitchDat[iPitch] > (int)(PitchCircMax / UnitAngle))
		{
			continue;
		}

		// find max amp
		if (ampDat[iPitch] > ampDatMax)
		{
			iPitchMax = iPitch;
			ampDatMax = ampDat[iPitch];
		}

		nValid++;
	}

	if (ampDatMax > 0)
	{
		*ampMax = ampDatMax;

		*pitchCen = pitchDat[iPitchMax];

		if (*pitchCen < 200)
		{
			*pitchCen = 200;
		}

		if (*pitchCen > 1500)
		{
			*pitchCen = (*pitchCen - 1000) / 4 + 1000;
		}

		return(1);
	}
	else
	{
		*pitchCen = 1500;
		return(0);
	}
}


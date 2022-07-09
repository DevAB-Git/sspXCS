////////////////////////////////////////////////////////////////
//  xcsenvironment.cpp
//  environment file for xcs based single step problems
//  developed by AB Siddique
//  School of Engineering and Computer Science
//  Victoria University of Wellington
/////////////////////////////////////////////////////////////////
#include "xcsenvironment.hpp"
#include <math.h>
#include "xcsconfig.hpp"

namespace xcs
{

env::env()
{
    m_pnPosRlvntBits = new int[m_stGEnvConfig.m_nRlvntBits];
    for(int i=0; i<m_stGEnvConfig.m_nRlvntBits; i++)
        m_pnPosRlvntBits[i]=i;
}

env::~env()
{
    delete []m_pnPosRlvntBits;
}

bool env::isDV1Term(int nNum)
{
    for(int i=0; i<nSizeDV1; i++)
    {
		if(nNum == DV1[i]){
			return true;
		}
	}
	return false;
}

void env::resetState(char pczState[])
{
    //Generates a new random problem instance.
	for(int i=0; i<m_stGEnvConfig.m_nCondLen; i++)
	{
		if(fRand()<0.5){
			pczState[i]='0';
		}
		else{
			pczState[i]='1';
		}
	}
}

// Executes the action and determines the reward.
double env::prfmAction(int nAction, char pczState[], bool &bReset)
{
    switch(m_stGEnvConfig.m_eEnv)
    {
		case evenParity:
			return prfmAction(nAction,pczState,bReset);
		case carry:
			return prfmCarryAction(nAction,pczState,bReset);
        case multiplexer:
            return prfmMuxAction(nAction,pczState,bReset);
        case hiddenEvenParity:
            return prfmHiddenEvenParityAction(nAction,pczState,bReset);
        case hiddenOddParity:
            return prfmHiddenOddParityAction(nAction,pczState,bReset);
        case countOnes:
            return prfmCountOnesAction(nAction,pczState,bReset);
		case majorityOn:
            return prfmMajorityOnAction(nAction,pczState,bReset);
		case dv1:
            return prfmDV1Action(nAction,pczState,bReset);
        default:
            printf("\nEnvironment not supported!\n");
            exit(0);
    }
}

double env::evlAction(int nAction, int nGenunAct, bool &bReset)
{
    int nRes=0;
    if(nAction == nGenunAct)
	{
	    bReset=true;
	    nRes = m_stGEnvConfig.m_nMaxPayoff;
	}
	else
	{
	    bReset=false;
	    nRes = 0;
	}

	return (double)nRes;
}

double env::prfmEvnParityAction(int nAction, char pczState[], bool &bReset)
{
    int nGenunAct = 0;
	int nNumOnes = 0;

	for(int i=0; i<m_stGEnvConfig.m_nCondLen; i++)
	{
		if(pczState[i] == '1')
			nNumOnes++;
	}

	if (nNumOnes%2 == 0)
		nGenunAct = 1;

    return evlAction(nAction, nGenunAct, bReset);
}

double env::prfmCarryAction(int nAction, char pczState[], bool &bReset)
{
    int nCarry = 0;
	int nHalfCondLen = m_stGEnvConfig.m_nCondLen/2;

	for(int i=nHalfCondLen-1; i>=0; i--)
	{
		nCarry = ((pczState[i]-'0') + (pczState[i+nHalfCondLen]-'0') + nCarry)/2;
	}

    return evlAction(nAction, nCarry, bReset);
}

double env::prfmMuxAction(int nAction, char pczState[], bool &bReset)
{
    int nPlace=m_stGEnvConfig.m_nPosBits;

	for(int i=0; i<m_stGEnvConfig.m_nPosBits; i++)
	{
	    if(pczState[i]=='1')
			nPlace += (int)pow(2.0, (double)(m_stGEnvConfig.m_nPosBits-1-i));

	}

    return evlAction(nAction, pczState[nPlace]-'0', bReset);
}

double env::prfmHiddenEvenParityAction(int nAction, char pczState[], bool &bReset)
{
    int nGenunAct = 0;
	int nNumOnes = 0;

	for(int i=0; i<m_stGEnvConfig.m_nRlvntBits; i++)
	{
		if(pczState[m_pnPosRlvntBits[i]] == '1')
			nNumOnes++;

	}
	if (nNumOnes%2 == 0)
		nGenunAct = 1;

	return evlAction(nAction, nGenunAct, bReset);
}

double env::prfmHiddenOddParityAction(int nAction, char pczState[], bool &bReset)
{
    int nGenunAct = 0;
	int nNumOnes = 0;

	for(int i=0; i<m_stGEnvConfig.m_nRlvntBits; i++)
	{
		if(pczState[m_pnPosRlvntBits[i]] == '1')
			nNumOnes++;

	}
	if (nNumOnes%2 == 0)
		nGenunAct = 1;

	return evlAction(nAction, nGenunAct, bReset);
}

double env::prfmCountOnesAction(int nAction, char pczState[], bool &bReset)
{
    int nGenunAct = 0;
	int nNumOnes = 0;

	for(int i=0; i<m_stGEnvConfig.m_nRlvntBits; i++)
	{
		if(pczState[m_pnPosRlvntBits[i]] == '1')
			nNumOnes++;

	}
	if (nNumOnes > m_stGEnvConfig.m_nRlvntBits/2)
		nGenunAct = 1;

	return evlAction(nAction, nGenunAct, bReset);
}
double env::prfmMajorityOnAction(int nAction, char pczState[], bool &bReset)
{
    int nGenunAct = 0;
	int nNumOnes = 0;

	for(int i=0; i<m_stGEnvConfig.m_nCondLen; i++)
	{
		if(pczState[i] == '1')
			nNumOnes++;

	}

	if (nNumOnes > m_stGEnvConfig.m_nCondLen/2)
		nGenunAct = 1;

    return evlAction(nAction, nGenunAct, bReset);
}
double env::prfmDV1Action(int nAction, char pczState[], bool &bReset)
{
    int nGenunAct = 0;
	int nNumOnes = 0;
	int p = 0;

	for(int i=m_stGEnvConfig.m_nCondLen-1; i>=0; i--)
	{
		nNumOnes += (pczState[i]-'0')*pow(2,p);
		p++;
	}
	if (isDV1Term(nNumOnes))
		nGenunAct = 1;

	return evlAction(nAction, nGenunAct, bReset);
}


} //end namespace xcs

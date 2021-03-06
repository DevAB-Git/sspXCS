////////////////////////////////////////////////////////////////
//  xcsclassifier.cpp
//  xcsclassifier file for xcs based single step problems
//  developed by AB Siddique
//  School of Engineering and Computer Science
//  Victoria University of Wellington
/////////////////////////////////////////////////////////////////
#include "xcsclassifierharness.hpp"
#include <cassert>
#include <math.h>
#include <string.h>

namespace xcs
{

clfrharness::clfrharness()
{
    m_pstPopulation = NULL;
    m_pstMatchSet = NULL;
    m_pstActionSet = NULL;
    m_pstKillSet = NULL;
    m_pfPredictArr = new double[m_stGEnvConfig.m_nActions];
    m_pfSumClfrFtnsPredictArr = new double[m_stGEnvConfig.m_nActions];
}

clfrharness::~clfrharness()
{
    delete []m_pfPredictArr;
    delete []m_pfSumClfrFtnsPredictArr;
}

void clfrharness::initClassifierSets()
{
    m_pstPopulation = NULL;
    m_pstMatchSet = NULL;
    m_pstActionSet = NULL;
    m_pstKillSet = NULL;
}

ClassifierSet**  clfrharness::getPopSet()
{
    return &m_pstPopulation;
}
ClassifierSet**  clfrharness::getActSet()
{
    return &m_pstActionSet;
}
ClassifierSet**  clfrharness::getKilSet()
{
    return &m_pstKillSet;
}

void clfrharness::initClassifier(Classifier *pstClfr, double fActSetSize, int nTimeStamp)
{
    pstClfr->m_nNumerosity = 1;
	pstClfr->m_nExprnc = 0;
	pstClfr->m_nTimeStamp = nTimeStamp;
	pstClfr->m_fPredict = m_stGClsCFG.m_fInitPredict;
	pstClfr->m_fPredictErr = m_stGClsCFG.m_fInitPredictErr;
	pstClfr->m_fAccuracy = m_stGClsCFG.m_fInitFitness;
	pstClfr->m_fFitness = m_stGClsCFG.m_fInitFitness;
	pstClfr->m_fActSetSize = fActSetSize;
}

int clfrharness::getNumerositySum(ClassifierSet *pstClfrSet)
{
    int nSum = 0;
    for(; pstClfrSet!=NULL; pstClfrSet=pstClfrSet->pstNext)
          nSum += pstClfrSet->pstClfr->m_nNumerosity;

    return nSum;
}
int clfrharness::getSetSize(ClassifierSet *pstClfrSet)
{
    int nSize = 0;
    for(; pstClfrSet!=NULL; pstClfrSet=pstClfrSet->pstNext)
        nSize++;

    return nSize;
}
int clfrharness::getSetSize(ClfrSetType eClfrSetType)
{
    switch(eClfrSetType)
    {
        case POPSET:
            return getSetSize(m_pstPopulation);
            break;
        case MSET:
            return getSetSize(m_pstMatchSet);
            break;
        case ACTSET:
            return getSetSize(m_pstActionSet);
            break;
        case KILLSET:
            return getSetSize(m_pstKillSet);
            break;
        default:
            printf("\n Classifier Set type doesn't match\n");
            return -1;
    }
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Match Set
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Gets the match-set that matches state from pop.
// If a classifier was deleted, record its address in killset to be
// able to update former actionsets.
// The iteration time 'itTime' is used when creating a new classifier
// due to covering. Covering occurs when not all possible actions are
//  present in the match set. Thus, it is made sure that all actions
// are present in the match set.

void clfrharness::getMatchSet(char pczState[], int nTimeStamp)
{
    //Match set should be NULL
    assert(m_pstMatchSet==NULL);

    ClassifierSet *pstClfrSet;
    Classifier *pstClfrKilled, *pstClfrCover;
    int nPopSize=0, nSetSize=0, nActions;
    bool bCvrdActions[m_stGEnvConfig.m_nActions];

	for(pstClfrSet= m_pstPopulation; pstClfrSet!=NULL; pstClfrSet=pstClfrSet->pstNext)
	{
	    // calculate the population size
		nPopSize += pstClfrSet->pstClfr->m_nNumerosity;
		if(isCondMatched(pstClfrSet->pstClfr->m_pczCondition,pczState))
		{
		    // add matching classifier to the matchset
			addClassifier(pstClfrSet->pstClfr, &m_pstMatchSet);
			// calculate size of the match set
			nSetSize+=pstClfrSet->pstClfr->m_nNumerosity;
		}
	}

	nActions = getActions(m_pstMatchSet,bCvrdActions);

	while(nActions < m_stGEnvConfig.m_nActions)
	{
	    // create covering classifiers, if not all actions are covered
		for(int i=0; i<m_stGEnvConfig.m_nActions; i++)
		{
			if(bCvrdActions[i]==false)
			{
			    // make sure that all actions are covered!
				pstClfrCover = createClfr(pczState,i,nSetSize+1,nTimeStamp);
				addClassifier(pstClfrCover,&m_pstMatchSet);
				nSetSize++;
				addClassifier(pstClfrCover,&m_pstPopulation);
				nPopSize++;
			}
		}

	    // Delete classifier if population is too big and record it in killset
		 while( nPopSize > m_stGEnvConfig.m_nMaxPopSize )
		 {
			//PL
			pstClfrKilled = delStochClassifier();
			if(pstClfrKilled!=NULL)
			{
				delClfrPointerFromSet(&m_pstMatchSet, pstClfrKilled);
  				addClassifier(pstClfrKilled, &m_pstKillSet, false);
			}
			nPopSize--;
		}
		nActions = getActions(m_pstMatchSet,bCvrdActions);
	}

}

int clfrharness::getActions(ClassifierSet *pstClfrSet, bool arrCvrdActions[])
{
    int nActions;

	for(int i=0; i<m_stGEnvConfig.m_nActions; i++){
		arrCvrdActions[i] = false;
	}

	for(nActions=0; nActions<m_stGEnvConfig.m_nActions && pstClfrSet!=NULL; pstClfrSet=pstClfrSet->pstNext)
	{
		if(arrCvrdActions[pstClfrSet->pstClfr->m_nAction]==false)
		{
			arrCvrdActions[pstClfrSet->pstClfr->m_nAction] = true;
			nActions++;
		}
	}
	return nActions;
}

bool clfrharness::isCondMatched(char pczClfrCond[], char pczCond[])
{
    for(int i=0; i<m_stGEnvConfig.m_nCondLen; i++)
    {
        if(pczClfrCond[i]!=m_stGClsCFG.m_cDntCare && pczClfrCond[i]!=pczCond[i])
            return false;
    }

    return true;
}

void clfrharness::createMatchCond(char pczCond[], char pczState[])
{
    for(int i=0; i<m_stGEnvConfig.m_nCondLen; i++)
    {
		if(fRand()<m_stGClsCFG.m_fProbDntCare)
			pczCond[i] = m_stGClsCFG.m_cDntCare;
		else
			pczCond[i] = pczState[i];

	}
}

void clfrharness::createMatchCond(Classifier *pstClfr, char pczState[])
{
    for(int i=0; i<m_stGEnvConfig.m_nCondLen; i++)
    {
		if(fRand()<m_stGClsCFG.m_fProbDntCare)
			pstClfr->m_pczCondition[i] = m_stGClsCFG.m_cDntCare;
		else
			pstClfr->m_pczCondition[i] = pczState[i];

	}
}

Classifier* clfrharness::createClfr(char pczState[], int nAction, int nSetSize, int nTimeStamp)
{
    Classifier *pstClfr;
    // get memory for the new classifier
	pstClfr = new Classifier;
	pstClfr->m_pczCondition = new char[m_stGEnvConfig.m_nCondLen];
	//temp chg AB
	createMatchCond(pstClfr->m_pczCondition,pczState);
	pstClfr->m_nAction = nAction;
	initClassifier(pstClfr,nSetSize,nTimeStamp);
	//createMatchCond(pstClfr,pczState);

	return pstClfr;
}

void clfrharness::getPredictArr()
{
    // here match set should never be NULL (because of covering)
    assert(m_pstMatchSet!=NULL);
    ClassifierSet* pstMatchSet = m_pstMatchSet;

	for(int i=0; i<m_stGEnvConfig.m_nActions; i++)
	{
		m_pfPredictArr[i]=0.0;
	    m_pfSumClfrFtnsPredictArr[i]=0.0;
	}

	for(; pstMatchSet!=NULL ; pstMatchSet=pstMatchSet->pstNext)
	{
	    int nActVal = pstMatchSet->pstClfr->m_nAction;
	    m_pfPredictArr[nActVal]+= pstMatchSet->pstClfr->m_fPredict*pstMatchSet->pstClfr->m_fFitness;
	    m_pfSumClfrFtnsPredictArr[nActVal]+= pstMatchSet->pstClfr->m_fFitness;
	}
	for(int i=0; i<m_stGEnvConfig.m_nActions; i++)
	{
	    if(m_pfSumClfrFtnsPredictArr[i]!=0)
			m_pfPredictArr[i] /= m_pfSumClfrFtnsPredictArr[i];
//temp chg AB the below else is extra, controll will never come here because above code for calculating m_pfSumClfrFtnsPredictArr
		else
			m_pfPredictArr[i]=0;
	}
}

//Returns the highest value in the prediction array.
double clfrharness::getBestValue()
{
	double fMax = m_pfPredictArr[0];

	for(int i=1; i<m_stGEnvConfig.m_nActions; i++)
	{
		if(fMax<m_pfPredictArr[i])
			fMax = m_pfPredictArr[i];
	}

	return fMax;
}

//Selects an action randomly.
//The function assures that the chosen action is represented by at least one classifier in the prediction array.
int clfrharness::randActionWinner()
{
	int nAct=0;
	do
	{
	    nAct = nRand(m_stGEnvConfig.m_nActions);
	}while(m_pfSumClfrFtnsPredictArr[nAct]==0);

	return nAct;
}

//Selects the action in the prediction array with the best value.
int clfrharness::bestActionWinner()
{
	int nAct=0;
	for(int i=1; i<m_stGEnvConfig.m_nActions; i++)
	{
		if(m_pfPredictArr[nAct]<m_pfPredictArr[i]){
			nAct=i;
		}
	}
	return nAct;
}

//Selects an action in the prediction array by roulette wheel selection.
int clfrharness::rouletteActionWinner()
{
	double fBidSum=0.0;
	int i;

	for(i=0; i<m_stGEnvConfig.m_nActions; i++)
	    fBidSum += m_pfPredictArr[i];

	fBidSum *= fRand();
	double fBidC=0.0;

	for(i=0; fBidC<fBidSum; i++)
	{
	    //temp chg AB
	    if(i==m_stGEnvConfig.m_nActions)
	    {
	        printf("\nOut of bound m_pfPredictArr ");
	        exit(0);
	    }
	    fBidC += m_pfPredictArr[i];
	}


	return i;
}

/////////////////////////////////////////////////////////////////
//action set operations
/////////////////////////////////////////////////////////////////

void clfrharness::getActionSet(int nAction)
{
    //Action set should be NULL
    assert(m_pstActionSet==NULL);

    ClassifierSet *pstSetP;

    for(pstSetP=m_pstMatchSet; pstSetP!=NULL; pstSetP=pstSetP->pstNext)
	{
		if(nAction == pstSetP->pstClfr->m_nAction)
			addClassifier(pstSetP->pstClfr,&m_pstActionSet);

    }

}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// Updates all parameters in the action set.
// Essentially, reinforcement Learning as well as the fitness evaluation takes place in this set.
// Moreover, the prediction error and the action set size estimate is updated. Also,
// action set subsumption takes place if selected. As in the algorithmic description, the fitness is updated
// after prediction and prediction error. However, in order to be more conservative the prediction error is
// updated before the prediction.
// @param fMaxPredict The maximum prediction value in the successive prediction array (should be set to zero in single step environments).
// @param fReward The actual resulting reward after the execution of an action.
//////////////////////////////////////////////////////////////////////////////////////////////////////

void clfrharness::updateActionSet(double fMaxPredict, double fReward)
{
    double fPredict, fSetSize=0.0;
	ClassifierSet *pstSetP;
    fPredict = fReward + m_stGClsCFG.m_fGama*fMaxPredict;

	for(pstSetP=m_pstActionSet; pstSetP!=NULL; pstSetP=pstSetP->pstNext)
	{
		fSetSize += pstSetP->pstClfr->m_nNumerosity;
		pstSetP->pstClfr->m_nExprnc++;
	}

	for(pstSetP=m_pstActionSet; pstSetP!=NULL; pstSetP=pstSetP->pstNext)
	{
	    // update prediction, prediction error and action set size estimate
		if((double)pstSetP->pstClfr->m_nExprnc < 1.0/m_stGClsCFG.m_fBeta)
		{
			// !first adjustments! -> simply calculate the average
			pstSetP->pstClfr->m_fPredictErr = (pstSetP->pstClfr->m_fPredictErr * ((double)pstSetP->pstClfr->m_nExprnc - 1.0) + absValue(fPredict - pstSetP->pstClfr->m_fPredict)) / (double)pstSetP->pstClfr->m_nExprnc;
			pstSetP->pstClfr->m_fPredict = (pstSetP->pstClfr->m_fPredict * ((double)pstSetP->pstClfr->m_nExprnc - 1.0) + fPredict) / (double)pstSetP->pstClfr->m_nExprnc;
			pstSetP->pstClfr->m_fActSetSize = (pstSetP->pstClfr->m_fActSetSize *((double)(pstSetP->pstClfr->m_nExprnc - 1))+fSetSize)/(double)pstSetP->pstClfr->m_nExprnc;
		}
		else
		{
			// normal adjustment -> use widrow hoff delta rule
			pstSetP->pstClfr->m_fPredictErr += m_stGClsCFG.m_fBeta * (absValue(fPredict - pstSetP->pstClfr->m_fPredict) - pstSetP->pstClfr->m_fPredictErr);
			pstSetP->pstClfr->m_fPredict += m_stGClsCFG.m_fBeta * (fPredict - pstSetP->pstClfr->m_fPredict);
			pstSetP->pstClfr->m_fActSetSize += m_stGClsCFG.m_fBeta * (fSetSize - pstSetP->pstClfr->m_fActSetSize);
		}
	}

	updateFitness(m_pstActionSet);

	if(m_stGClsCFG.m_bActSetSubSump)
		actSetSubsumption();


}

// update the fitnesses of an action set (the previous [A] in multi-step envs or the current [A] in single-step envs.)
void clfrharness::updateFitness(ClassifierSet *pstClfrSet)
{
    ClassifierSet *pstSetP;
	double fKSum=0.0;

	if(pstClfrSet==NULL)
	{
	    // if the action set got NULL (due to deletion) return
		return;
	}

	//First, calculate the accuracies of the classifier and the accuracy sums
	for(pstSetP=pstClfrSet; pstSetP!=NULL; pstSetP=pstSetP->pstNext) {
		if(pstSetP->pstClfr->m_fPredictErr <= m_stGClsCFG.m_fEpsilon0) {
			pstSetP->pstClfr->m_fAccuracy = 1.0;
		}
		else{
			pstSetP->pstClfr->m_fAccuracy = m_stGClsCFG.m_fAlpha * pow(pstSetP->pstClfr->m_fPredictErr / m_stGClsCFG.m_fEpsilon0 , -m_stGClsCFG.m_fNu);
		}
		fKSum += pstSetP->pstClfr->m_fAccuracy*(double)pstSetP->pstClfr->m_nNumerosity;
	}

  //Next, update the fitnesses accordingly
  for(pstSetP=pstClfrSet; pstSetP!=NULL; pstSetP=pstSetP->pstNext)
	pstSetP->pstClfr->m_fFitness += m_stGClsCFG.m_fBeta * ( (pstSetP->pstClfr->m_fAccuracy * pstSetP->pstClfr->m_nNumerosity) / fKSum - pstSetP->pstClfr->m_fFitness );


}

bool clfrharness::addClassifier(Classifier *pstClfr, ClassifierSet **pstClfrSet)
{
    ClassifierSet *pstSetP;
	pstSetP = new ClassifierSet;
	pstSetP->pstClfr=pstClfr;
	pstSetP->pstNext=*pstClfrSet;
	*pstClfrSet=pstSetP;
	return true;
}

bool clfrharness::addClassifier(Classifier *pstClfr, ClassifierSet **pstClfrSet, bool bIncNum)
{
    ClassifierSet *pstSetP;
    //Add in population
    if(bIncNum)
	{
	    // Check if classifier exists already. If so, just increase the numerosity and free the space of the new classifier
	    for(pstSetP=*pstClfrSet; pstSetP!=NULL; pstSetP=pstSetP->pstNext)
        {
            if( isEquals(pstSetP->pstClfr,pstClfr) )
            {
                pstSetP->pstClfr->m_nNumerosity++;
                freeClassifier(pstClfr);
                return true;
            }
        }
	}
	else
	{
	    for(pstSetP=*pstClfrSet; pstSetP!=NULL; pstSetP=pstSetP->pstNext)
        {
            if(pstSetP->pstClfr == pstClfr)
            {
                // classifier is already in Set
                return false;
            }
        }
	}

	// add the classifier, as it is not already in the pointerset
	pstSetP = new ClassifierSet;
	pstSetP->pstClfr=pstClfr;
	pstSetP->pstNext=*pstClfrSet;
	*pstClfrSet=pstSetP;
	return true;
}

bool clfrharness::isEquals(Classifier *pstClfr1, Classifier *pstClfr2)
{
    if( pstClfr1->m_nAction==pstClfr2->m_nAction && isSameCond(pstClfr1->m_pczCondition,pstClfr2->m_pczCondition) )
        return true;

    return false;
}

bool clfrharness::isSameCond(char pczCond1[], char pczCond2[])
{
    for(int i=0; i<m_stGEnvConfig.m_nCondLen; i++)
    {
		if(pczCond1[i] != pczCond2[i])
			return false;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////////////
//concrete deletion of a classifier or a whole classifier set
//////////////////////////////////////////////////////////////////////////////////////
//Frees only the complete ClassifierSet (not the Classifiers itself)!
void clfrharness::freeSet(ClassifierSet **pstClfrSet){
	ClassifierSet *clp;
  	while(*pstClfrSet!=NULL)
  	{
		clp=(*pstClfrSet)->pstNext;
		delete *pstClfrSet;
		*pstClfrSet=clp;
	}
}

void clfrharness::freeSet(ClfrSetType eClfrSetType)
{
    switch(eClfrSetType)
    {
        case POPSET:
            freeSet(&m_pstPopulation);
            break;
        case MSET:
            freeSet(&m_pstMatchSet);
            break;
        case ACTSET:
            freeSet(&m_pstActionSet);
            break;
        case KILLSET:
            if(m_pstKillSet!=NULL)
                freeSet(&m_pstKillSet);
            break;
        default:
            printf("\n Classifier Set type for deletion doesn't match\n");
    }
}

//Frees the complete ClassifierSet with the corresponding Classifiers.
void clfrharness::freeClassifierSet(ClassifierSet **pstClfrSet)
{
	ClassifierSet *clp;
	while(*pstClfrSet!=NULL)
	{
		freeClassifier((*pstClfrSet)->pstClfr);
		clp=(*pstClfrSet)->pstNext;
		delete *pstClfrSet;
		*pstClfrSet=clp;
    }
}

//Frees the complete ClassifierSet with the corresponding Classifiers.
void clfrharness::freeClassifierSet(ClfrSetType eClfrSetType)
{
    switch(eClfrSetType)
    {
        case POPSET:
            freeClassifierSet(&m_pstPopulation);
            break;
        case MSET:
            freeClassifierSet(&m_pstMatchSet);
            break;
        case ACTSET:
            freeClassifierSet(&m_pstActionSet);
            break;
        case KILLSET:
            if(m_pstKillSet!=NULL)
                freeClassifierSet(&m_pstKillSet);
            break;
        default:
            printf("\n Classifier Set type for deletion doesn't match\n");
    }
}


//Frees one classifier.
void clfrharness::freeClassifier(Classifier *pstClfr)
{

    if(pstClfr->m_pczCondition)
    {
       delete [] pstClfr->m_pczCondition;
       pstClfr->m_pczCondition=NULL;
    }

	delete pstClfr;
}

//Deletes a classifier from the population.
Classifier* clfrharness::delStochClassifier()
{
    ClassifierSet *pstSetP,*pstSetPl;
	Classifier *pstkildP=NULL;
	double fSum=0.0;
	double fMeanFitness=0.0;
	double fDelP;
	int nSize=0;

	//get the sum of the fitness and the numerosity
	for(pstSetP=m_pstPopulation; pstSetP!=NULL; pstSetP=pstSetP->pstNext)
	{
		fMeanFitness+=pstSetP->pstClfr->m_fFitness;
		nSize+=pstSetP->pstClfr->m_nNumerosity;
	}
	fMeanFitness/=(double)nSize;

	//get the delete proportion, which depends on the average fitness
	for(pstSetP=m_pstPopulation; pstSetP!=NULL; pstSetP=pstSetP->pstNext)
		fSum += getDelProp(pstSetP->pstClfr,fMeanFitness);

	//choose the classifier that will be deleted
	fDelP=fRand()*fSum;

	//look for the classifier
	pstSetP=pstSetPl=m_pstPopulation;

	fSum = getDelProp(pstSetP->pstClfr,fMeanFitness);
	while(fSum < fDelP) {
		pstSetPl=pstSetP;
		pstSetP=pstSetP->pstNext;
		fSum += getDelProp(pstSetP->pstClfr,fMeanFitness);
	}

	//delete the classifier
	pstkildP=delClassifier(pstSetP, pstSetPl);

	//return the pointer to the deleted classifier, to be able to update other sets
	return pstkildP;
}

//Returns the vote for deletion of the classifier.
double clfrharness::getDelProp(Classifier *pstClfr, double fMeanFitness)
{
    if(pstClfr->m_fFitness/(double)pstClfr->m_nNumerosity >= m_stGClsCFG.m_fDelta*fMeanFitness || pstClfr->m_nExprnc < m_stGClsCFG.m_nThetaDel)
		return (double)(pstClfr->m_fActSetSize*pstClfr->m_nNumerosity);
    else
		return (double)pstClfr->m_fActSetSize*(double)pstClfr->m_nNumerosity*fMeanFitness / (pstClfr->m_fFitness/(double)pstClfr->m_nNumerosity);

}

//Deletes the classifier pstSetP from the population, pstSetPl points to the classifier that is before pstSetP in the list
Classifier* clfrharness::delClassifier(ClassifierSet *pstSetP, ClassifierSet *pstSetPl)
{
    Classifier *pstKilledp=NULL;

	//pstSetP must point to some classifier!
	assert(pstSetP!=NULL);

	if(pstSetP->pstClfr->m_nNumerosity>1)
	{
	    // if the numerosity is greater than one -> just decrease it
		pstSetP->pstClfr->m_nNumerosity--;
	}
	else
	{
		//Otherwise, delete it and record it in pstKilledp
		if(pstSetP==pstSetPl)
			m_pstPopulation=pstSetP->pstNext;

		else
			pstSetPl->pstNext=pstSetP->pstNext;

		pstKilledp=pstSetP->pstClfr;
		freeClassifier(pstSetP->pstClfr);
		delete pstSetP;
	}
	// return a pointer to a deleted classifier (NULL if the numerosity was just decreased)
	return pstKilledp;
}

//Check if the classifier pointers that are in killset are in pstUset - delete the pointers,
//if they are inside. Note that the classifiers in killset are already deleted, so do not
//read their values or try to delete them again here!

bool clfrharness::updateSet(ClassifierSet **pstUset)
{
	ClassifierSet *pstSetP,*pstSetPl,*pstKillP,*pstUsetP;
	bool bUpdated = true;

     // if one of the sets is empty -> do not do anything
	if(*pstUset==NULL || m_pstKillSet==NULL)
		return false;

	// check all classifiers in pstUset
	pstSetP=*pstUset;
	while(bUpdated && pstSetP!=NULL)
	{
		pstSetP=*pstUset;
		pstSetPl=*pstUset;
		bUpdated = false;
		while(pstSetP!=NULL && !bUpdated)
		{
			for(pstKillP=m_pstKillSet; pstKillP!=NULL; pstKillP=pstKillP->pstNext)
			{
				if(pstKillP->pstClfr == pstSetP->pstClfr)
				{
					// if killed classifier found, delete the struct classifier set in pstUset
					bUpdated = true;
					if(pstSetP==pstSetPl)
					{ // first entry in set
						pstUsetP=*pstUset;
						*pstUset=pstUsetP->pstNext;
						delete pstUsetP;
						break;
					}
					else
					{
						pstSetPl->pstNext=pstSetP->pstNext;
						delete pstSetP;
						pstSetP = *pstUset;
						pstSetPl= *pstUset;
						break;
					}
				}
			}
			if(bUpdated)
			{
			    // check the whole uset again, if one pointer was deleted
				break;
			}
			pstSetPl=pstSetP;
			pstSetP=pstSetP->pstNext;
		}
	}
    return bUpdated;
}

bool clfrharness::delClfrPointerFromSet(ClassifierSet **pstUset, Classifier *pstClfr)
{
    ClassifierSet *pstSetP, *pstSetPl;
	for(pstSetP=*pstUset, pstSetPl=*pstUset; pstSetP!=NULL; pstSetP=pstSetP->pstNext)
	{
		if(pstSetP->pstClfr==pstClfr)
		{
			if(pstSetPl==pstSetP)
			{
				*pstUset=(*pstUset)->pstNext;
				delete pstSetP;
			}
			else
			{
				pstSetPl->pstNext=pstSetP->pstNext;
				delete pstSetP;
			}
			return true;
		}
		pstSetPl=pstSetP;
	}
	return false;
}


//Subsumption
void clfrharness::actSetSubsumption()
{
    Classifier *pstSubSumer=NULL;
	ClassifierSet *pstSetP, *pstSetPl;

	// Find the most general subsumer
	for(pstSetP=m_pstActionSet; pstSetP!=NULL; pstSetP=pstSetP->pstNext)
	{
		if(isSubSumer(pstSetP->pstClfr))
		{
			if(pstSubSumer==NULL || isMoreGeneral(pstSetP->pstClfr->m_pczCondition, pstSubSumer->m_pczCondition))
				pstSubSumer = pstSetP->pstClfr;

		}
	}

	//If a subsumer was found, subsume all classifiers that are more specific.
	if(pstSubSumer!=NULL)
	{
		for(pstSetP=m_pstActionSet, pstSetPl=m_pstActionSet; pstSetP!=NULL; pstSetP=pstSetP->pstNext)
		{
			while(isMoreGeneral(pstSubSumer->m_pczCondition, pstSetP->pstClfr->m_pczCondition))
			{
				pstSubSumer->m_nNumerosity += pstSetP->pstClfr->m_nNumerosity;

				if(pstSetPl==pstSetP)
				{
					m_pstActionSet=pstSetP->pstNext;
					delClfrPointerFromSet(&m_pstPopulation,pstSetP->pstClfr);
					freeClassifier(pstSetP->pstClfr);
					addClassifier(pstSetP->pstClfr, &m_pstKillSet, false);
					delete pstSetP;
					pstSetP=m_pstActionSet;
					pstSetPl=m_pstActionSet;

				}
				else
				{

					pstSetPl->pstNext=pstSetP->pstNext;
					delClfrPointerFromSet(&m_pstPopulation,pstSetP->pstClfr);
					freeClassifier(pstSetP->pstClfr);
					addClassifier(pstSetP->pstClfr, &m_pstKillSet, false);
					delete pstSetP;
					pstSetP=pstSetPl;

				}
			}
			pstSetPl=pstSetP;
		}
	}

}


//////////////////////////////////////////////////////////////////////////
//Check if the first condition is more general than the second.
//It is made sure that the classifier is indeed more general and not equally general
//as well as that the more specific classifier is completely included in the more general one (do not specify overlapping regions).
//////////////////////////////////////////////////////////////////////////
bool clfrharness::isMoreGeneral(char pczFirst[], char pczSecond[])
{
    	bool bRes = false;
	for(int i=0; i<m_stGEnvConfig.m_nCondLen; i++){
		if(pczFirst[i] != m_stGClsCFG.m_cDntCare && pczFirst[i] != pczSecond[i]){
			return false;
		}
		else if(pczFirst[i] !=  pczSecond[i]){
			bRes = true;
		}
	}
	return bRes;
}

bool clfrharness::isSubSumes(Classifier *pstClfr1, Classifier * pstClfr2)
{
    return pstClfr1->m_nAction==pstClfr2->m_nAction && isSubSumer(pstClfr1) && isMoreGeneral(pstClfr1->m_pczCondition,pstClfr2->m_pczCondition);
}

bool clfrharness::isSubSumer(Classifier *pstClfr)
{
   return pstClfr->m_nExprnc > m_stGClsCFG.m_nThetaSub && pstClfr->m_fPredictErr <= m_stGClsCFG.m_fEpsilon0;
}

//Tries to subsume the parents.
void clfrharness::subsumeClassifier(Classifier *pstClfr, Classifier **pstParents, ClassifierSet *pstLocSet)
{
    int i;
	for(i=0; i<2; i++)
	{
		if(pstParents[i]!=NULL && isSubSumes(pstParents[i],pstClfr))
		{
			pstParents[i]->m_nNumerosity++;
			freeClassifier(pstClfr);
			return;
		}
	}
  	if(subsumeClassifierToSet(pstClfr,pstLocSet))
  		return;

	addClassifier(pstClfr, &m_pstPopulation, true);
}

bool clfrharness::subsumeClassifierToSet(Classifier *pstClfr, ClassifierSet *pstSet)
{
    ClassifierSet *pstSetP;
	Classifier *pstSubCl[m_stGEnvConfig.m_nMaxPopSize];
	int nNumSub=0;

	for(pstSetP=pstSet; pstSetP!=NULL; pstSetP=pstSetP->pstNext)
	{
		if(isSubSumes(pstSetP->pstClfr,pstClfr))
		{
			pstSubCl[nNumSub]=pstSetP->pstClfr;
			nNumSub++;
		}
	}
	// if there were classifiers found to subsume, then choose randomly one and subsume
	if(nNumSub>0)
	{
		nNumSub = nRand(nNumSub);
		pstSubCl[nNumSub]->m_nNumerosity++;
		freeClassifier(pstClfr);
		return true;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Discovery mechanisms
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

//The discovery conmponent with the genetic algorithm
// note: some classifiers in set could be deleted !
void clfrharness::discoveryComponent(ClassifierSet **pstSet, int nItTime, char pczSituation[])
{
	ClassifierSet *pstSetP;
	Classifier *pstClfr[2], *pstParents[2];
	double fFitSum=0.0;
	int i, nLen, nSetSum=0, nGaitSum=0;

	if(*pstSet==NULL)
	{ // if the classifier set is empty, return (due to deletion)
		return;
	}

    // get all sums that are needed to do the discovery
	getDiscoversSums(*pstSet, &fFitSum, &nSetSum, &nGaitSum);

	// do not do a GA if the average number of time-steps in the set since the last GA is less or equal than thetaGA
	if( nItTime - (double)nGaitSum / (double)nSetSum < m_stGClsCFG.m_fThetaGA)
		return;

	setTimeStamps(*pstSet, nItTime);

    // select two classifiers (tournament selection) and copy them
	selectTwoClassifiers(pstClfr, pstParents, *pstSet, fFitSum, nSetSum);
  	// do crossover on the two selected classifiers
  	crossover(pstClfr,m_stGClsCFG.m_nCrssoverType);
  	// do mutation
	for(i=0; i<2; i++)
		mutation(pstClfr[i], pczSituation);


	pstClfr[0]->m_fPredict   = (pstClfr[0]->m_fPredict + pstClfr[1]->m_fPredict) / 2.0;
	pstClfr[0]->m_fPredictErr = m_stGClsCFG.m_fPredictErrReduction * ( (pstClfr[0]->m_fPredictErr + pstClfr[1]->m_fPredictErr) / 2.0 );
	pstClfr[0]->m_fFitness = m_stGClsCFG.m_fFitnessReduction * ( (pstClfr[0]->m_fFitness + pstClfr[1]->m_fFitness) / 2.0 );

	pstClfr[1]->m_fPredict = pstClfr[0]->m_fPredict;
	pstClfr[1]->m_fPredictErr = pstClfr[0]->m_fPredictErr;
	pstClfr[1]->m_fFitness = pstClfr[0]->m_fFitness;

  	// get the length of the population to check if clasifiers have to be deleted
	for(nLen=0, pstSetP=m_pstPopulation; pstSetP!=NULL; pstSetP=pstSetP->pstNext)
		nLen += pstSetP->pstClfr->m_nNumerosity;


	// insert the new two classifiers and delete two if necessary
	insertDiscoveredClassifier(pstClfr, pstParents, pstSet, nLen);
}

// Calculate all necessary sums in the set for the discovery component.
void clfrharness::getDiscoversSums(ClassifierSet *pstSet, double *fFitSum, int *nSetSum, int *nGaitSum)
{
    ClassifierSet *pstSetP;
	*fFitSum=0.0;
	*nSetSum=0;
	*nGaitSum=0;
	for(pstSetP=pstSet; pstSetP!=NULL; pstSetP=pstSetP->pstNext){
		(*fFitSum)+=pstSetP->pstClfr->m_fFitness;
		(*nSetSum)+=pstSetP->pstClfr->m_nNumerosity;
		(*nGaitSum) += pstSetP->pstClfr->m_nTimeStamp*pstSetP->pstClfr->m_nNumerosity;
	}
}

// Sets the time steps of all classifiers in the set to itTime (because a GA application is occurring in this set!).
void clfrharness::setTimeStamps(ClassifierSet *pstSet, int nItTime)
{
   for( ; pstSet!=NULL; pstSet=pstSet->pstNext)
		pstSet->pstClfr->m_nTimeStamp = nItTime;

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//selection mechanism
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Select two classifiers using the chosen selection mechanism and copy them as offspring.
void clfrharness::selectTwoClassifiers(Classifier **pstClfr, Classifier **pstParents, ClassifierSet *pstSet, double fFitSum, int nSetSum)
{
    Classifier *pstClP;

	assert(pstSet!=NULL);

	for(int i=0;i<2;i++)
	{
		if(m_stGClsCFG.m_fThetaSel <= 0)
            pstClP = rouletteWheelSelection(pstSet,fFitSum);
		else
		{
			if(i==0)
				pstClP = tournamentSelection(pstSet, nSetSum, 0);
			else
				pstClP = tournamentSelection(pstSet, nSetSum, pstParents[0]);
		}

		pstParents[i]=pstClP;

		pstClfr[i] = new Classifier;
        pstClfr[i]->m_pczCondition = new char[m_stGEnvConfig.m_nCondLen];
		for(int j=0; j<m_stGEnvConfig.m_nCondLen; j++)
            pstClfr[i]->m_pczCondition[j]=pstClP->m_pczCondition[j];

		pstClfr[i]->m_nAction = pstClP->m_nAction;
		pstClfr[i]->m_fPredict = pstClP->m_fPredict;
		pstClfr[i]->m_fPredictErr = pstClP->m_fPredictErr;
		pstClfr[i]->m_fAccuracy = pstClP->m_fAccuracy;
		pstClfr[i]->m_fFitness = pstClP->m_fFitness /(double)pstClP->m_nNumerosity;
		pstClfr[i]->m_nNumerosity = 1;
		pstClfr[i]->m_nExprnc = 0;
		pstClfr[i]->m_fActSetSize = pstClP->m_fActSetSize;
		pstClfr[i]->m_nTimeStamp = pstClP->m_nTimeStamp;
	}
}


//Selects a classifier from 'set' using tournament selection.
//If 'pstNotMe' is not the NULL pointer and forceDifferentInTournament is set to a value larger 0,
//this classifier is not selected except if it is the only classifier.

Classifier* clfrharness::tournamentSelection(ClassifierSet *pstSet, int nSetSum, Classifier *pstNotMe)
{
	ClassifierSet *pstSetP, *pstWinnerSet=NULL;
	Classifier *pstWinner=NULL;
	double fFitness=-1.0, fVal;
	int i, j, *pSel, nSize=0;

    //There must be at least one classifier in the set
	assert(pstSet!=NULL);

	if(pstNotMe!=0)
	{
		if(fRand() < m_stGClsCFG.m_fForceDiffInTornmnt)
			nSetSum -= pstNotMe->m_nNumerosity;
		else
            //picking the same guy is allowed
			pstNotMe=0;

	}

	//only one classifier in set
	if(nSetSum<=0)
		return pstSet->pstClfr;


	if(m_stGClsCFG.m_fThetaSel>1)
	{
	    //tournament with fixed size
		pSel = new int[nSetSum];
		for(i=0; i<m_stGClsCFG.m_fThetaSel; i++)
		{ // (with replacement)
			pSel[nRand(nSetSum)]=1;
		}
		if(i-m_stGClsCFG.m_fThetaSel != 0 && fRand() > i-m_stGClsCFG.m_fThetaSel)
		{
			//possible probabilistic selection of the last guy
			pSel[nRand(nSetSum)]=1;
		}

		for(pstSetP=pstSet, i=0; pstSetP!=NULL; pstSetP=pstSetP->pstNext)
		{
			if(pstSetP->pstClfr != pstNotMe)
			{
				if(fFitness < pstSetP->pstClfr->m_fFitness/pstSetP->pstClfr->m_nNumerosity)
				{
					for(j=0; j<pstSetP->pstClfr->m_nNumerosity; j++)
					{
						if(pSel[i+j])
						{
							freeSet(&pstWinnerSet);
							addClassifier(pstSetP->pstClfr, &pstWinnerSet, false);
							fFitness = pstSetP->pstClfr->m_fFitness/pstSetP->pstClfr->m_nNumerosity;
							break; // go to next classifier since this one is already a winner
						}
					}
				}
				i += pstSetP->pstClfr->m_nNumerosity;
			}
		}
		delete[] pSel;
		assert(pstWinnerSet!=NULL);
		nSize=1;
	}
	else
	{
	    // tournament selection with the tournament size approx. equal to tournamentSize*setsum
		pstWinnerSet=NULL;
		while(pstWinnerSet==NULL) {
			nSize=0;
			for(pstSetP=pstSet; pstSetP!=NULL; pstSetP=pstSetP->pstNext)
			{
				if(pstSetP->pstClfr != pstNotMe)
				{
				    //do not reselect the same classifier -> this only applies if forcedDifferentInTournament is set!
					fVal = pstSetP->pstClfr->m_fPredictErr;
					if(pstWinnerSet==NULL || (!m_stGClsCFG.m_bGAErrBasedSel && fFitness - m_stGClsCFG.m_fSelTolrnc <= pstSetP->pstClfr->m_fFitness/pstSetP->pstClfr->m_nNumerosity) || (m_stGClsCFG.m_bGAErrBasedSel && fFitness + m_stGClsCFG.m_fSelTolrnc * m_stGEnvConfig.m_nMaxPayoff >= fVal))
                    {
						// if his fitness is worse then do not bother
						for(i=0; i<pstSetP->pstClfr->m_nNumerosity; i++)
						{
							if(fRand() < m_stGClsCFG.m_fThetaSel)
							{
								// this classifier is a selection candidate and
								// his fitness/error is higher/lower or similar to the other classifier
								if(pstWinnerSet==NULL)
								{
									//the first guy in the tournament
									addClassifier(pstSetP->pstClfr, &pstWinnerSet, false);
									if(m_stGClsCFG.m_bGAErrBasedSel)
										fFitness = fVal;

									else
										fFitness = pstSetP->pstClfr->m_fFitness/pstSetP->pstClfr->m_nNumerosity;

									nSize=1;
								}
								else
								{
									// another guy in the tournament
									if( (!m_stGClsCFG.m_bGAErrBasedSel && fFitness + m_stGClsCFG.m_fSelTolrnc > pstSetP->pstClfr->m_fFitness/pstSetP->pstClfr->m_nNumerosity) ||(m_stGClsCFG.m_bGAErrBasedSel && fFitness - m_stGClsCFG.m_fSelTolrnc * m_stGEnvConfig.m_nMaxPayoff < fVal))
									{
										// both classifiers in tournament have a similar fitness/error
										nSize += addClassifier(pstSetP->pstClfr, &pstWinnerSet, false);
									}
									else
									{
										//new classifier in tournament is clearly better
										freeSet(&pstWinnerSet);
										pstWinnerSet=NULL;
										addClassifier(pstSetP->pstClfr, &pstWinnerSet, false);
										if(m_stGClsCFG.m_bGAErrBasedSel)
											fFitness = fVal;
										else
											fFitness = pstSetP->pstClfr->m_fFitness/pstSetP->pstClfr->m_nNumerosity;

										nSize=1;
									}
								}
								break; // go to next classifier since this one is already a winner
							}
						}
					}
				}
			}
		}
	}
	// choose one of the equally best winners at random
	nSize = nRand(nSize);
	for(pstSetP=pstWinnerSet; pstSetP!=NULL; pstSetP=pstSetP->pstNext)
	{
		if(nSize==0)
		{
			break;
		}
		nSize--;
	}
	pstWinner = pstSetP->pstClfr;
	freeSet(&pstWinnerSet);
	return pstWinner;
}

//Select a classifier for the discovery mechanism using roulette wheel selection
Classifier* clfrharness::rouletteWheelSelection(ClassifierSet *pstSet, double fFitSum)
{
    ClassifierSet *pstSetP;
	double fChoiceP;

	fChoiceP=fRand()*fFitSum;
	pstSetP=pstSet;
	fFitSum=pstSetP->pstClfr->m_fFitness;
	while(fChoiceP>fFitSum)
	{
		pstSetP=pstSetP->pstNext;
		fFitSum+=pstSetP->pstClfr->m_fFitness;
	}

	return pstSetP->pstClfr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Crossover and mutation
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void clfrharness::crossover(Classifier **pstClfr, int nType)
{
    // Determines if crossover is applied and calls then the selected crossover type.
	if(fRand()<m_stGClsCFG.m_fpX)
	{
		if(nType == 0)
			uniformCrossover(pstClfr);
		else if(nType == 1)
			onePointCrossover(pstClfr);

		else
			twoPointCrossover(pstClfr);
	}
}

void clfrharness::uniformCrossover(Classifier **pstClfr)
{
    // Crosses the two received classifiers using uniform crossover.
	char chTemp;
	for(int i=0; i<m_stGEnvConfig.m_nCondLen; i++)
	{
		if(fRand() < 0.5)
		{
			chTemp = pstClfr[0]->m_pczCondition[i];
			pstClfr[0]->m_pczCondition[i] = pstClfr[1]->m_pczCondition[i];
			pstClfr[1]->m_pczCondition[i] = chTemp;
		}
	}
}

void clfrharness::onePointCrossover(Classifier **pstClfr)
{
    // Crosses the two received classifiers using one-point crossover.
	char chTemp;
	int nSep = nRand(m_stGEnvConfig.m_nCondLen);
	if(nSep<0 || nSep>=m_stGEnvConfig.m_nCondLen){
		printf("\ninvalid crossover point\n");
		exit(0);
	}
	for(int i=0; i<=nSep; i++){
		chTemp=pstClfr[0]->m_pczCondition[i];
		pstClfr[0]->m_pczCondition[i]=pstClfr[1]->m_pczCondition[i];
		pstClfr[1]->m_pczCondition[i]=chTemp;
	}
}

void clfrharness::twoPointCrossover(Classifier **pstClfr)
{
    // Crosses the two received classifiers using two-point crossover.
	char chTemp;
	int nSep1 = nRand(m_stGEnvConfig.m_nCondLen);
	int nSep2 = nRand(m_stGEnvConfig.m_nCondLen);
	if(nSep1<0 || nSep1>=m_stGEnvConfig.m_nCondLen || nSep2<0 || nSep2>=m_stGEnvConfig.m_nCondLen)
	{
		printf("\ninvalid crossover point\n");
		exit(0);
	}
	if(nSep1>nSep2)
	{
		int nTemp=nSep1;
		nSep1=nSep2;
		nSep2=nTemp;
	}
	for(int i=nSep1; i<=nSep2; i++)
	{
		chTemp=pstClfr[0]->m_pczCondition[i];
		pstClfr[0]->m_pczCondition[i]=pstClfr[1]->m_pczCondition[i];
		pstClfr[1]->m_pczCondition[i]=chTemp;
	}
}

 //Apply mutation to classifier 'pstClfr'.
 //If niche mutation is applied, 'pczState' is considered to constrain mutation.
 //returns if the condition was changed.

void clfrharness::mutation(Classifier *pstClfr, char pczState[])
{
	if(m_stGClsCFG.m_nMutationType == 0)
		applyNicheMutation(pstClfr,pczState);
	else
		applyGeneralMutation(pstClfr,pczState);

	mutateAction(pstClfr);
}

//Mutates the condition of the classifier. If one allele is mutated depends on the constant pM.
//This mutation is a niche mutation. It assures that the resulting classifier still matches the current situation.

void clfrharness::applyNicheMutation(Classifier *pstClfr, char pczState[])
{
	for(int i=0; i<m_stGEnvConfig.m_nCondLen; i++)
	{
	   if(fRand()<m_stGClsCFG.m_fpM)
	   {
			if(pstClfr->m_pczCondition[i]==m_stGClsCFG.m_cDntCare)
				pstClfr->m_pczCondition[i]=pczState[i];
			else
				pstClfr->m_pczCondition[i]=m_stGClsCFG.m_cDntCare;
        }
	}
}

//Mutates the condition of the classifier. If one allele is mutated depends on the constant pM.
//This mutation is a general mutation.

void clfrharness::applyGeneralMutation(Classifier *pstClfr, char pczState[])
{
	for(int i=0; i<m_stGEnvConfig.m_nCondLen; i++)
	{
	    if(fRand()<m_stGClsCFG.m_fpM)
	    {
			if(pstClfr->m_pczCondition[i]==m_stGClsCFG.m_cDntCare)
				pstClfr->m_pczCondition[i]= (fRand() < 0.5)? '0' : '1';
			else
				pstClfr->m_pczCondition[i]=m_stGClsCFG.m_cDntCare;
		}
	}
}

void clfrharness::mutateAction(Classifier *pstClfr)
{
    //Mutates the action of the classifier.
	if(fRand()<m_stGClsCFG.m_fpM)
	{
		int nAct = pstClfr->m_nAction;
		do
		{
		    pstClfr->m_nAction = nRand(m_stGEnvConfig.m_nActions);
		}while(nAct==pstClfr->m_nAction);
	}
}

//Insert a discovered classifier into the population and respects the population size.
void clfrharness::insertDiscoveredClassifier(Classifier **pstClfr, Classifier **pstParents, ClassifierSet **pstSet, int nLen)
{
    Classifier *pstKilledP;
	nLen+=2;
	if(m_stGClsCFG.m_bGASubSump)
	{
		subsumeClassifier(pstClfr[0],pstParents,*pstSet);
		subsumeClassifier(pstClfr[1],pstParents,*pstSet);
	}
	else
	{
		addClassifier(pstClfr[0],&m_pstPopulation, true);
		addClassifier(pstClfr[1],&m_pstPopulation, true);
	}

	while(nLen > m_stGEnvConfig.m_nMaxPopSize)
	{
		nLen--;
		pstKilledP=delStochClassifier();

		//record the deleted classifier to update other sets
		if(pstKilledP!=NULL)
		{
			addClassifier(pstKilledP, &m_pstKillSet, false);
			//update the set
			updateSet(pstSet);
		}
	}
}

void clfrharness::printClassifierSet(ClassifierSet *pstHead, FILE *pFile)
{
    if(pFile)
    {
        for(; pstHead!=NULL; pstHead=pstHead->pstNext)
            printClassifier(pstHead->pstClfr, pFile);

        fwrite("\n\n",strlen("\n\n"),1,pFile);
    }
    else
    {
        for(; pstHead!=NULL; pstHead=pstHead->pstNext)
            printClassifier(pstHead->pstClfr);

        printf("\n\n");
    }
}

void clfrharness::printClassifierSet(ClfrSetType eClfrSetType, FILE *pFile)
{
    switch(eClfrSetType)
    {
        case POPSET:
            printClassifierSet(m_pstPopulation, pFile);
            break;
/*        case MSET:
            return getSetSize(m_pstMatchSet);
            break;
        case ACTSET:
            return getSetSize(m_pstActionSet);
            break;
        case KILLSET:
            return getSetSize(m_pstKillSet);
            break;
*/
        default:
            printf("\n Classifier Set type doesn't match\n");
    }
}

void clfrharness::printClassifier(Classifier *pstClfr, FILE *pFile)
{
    if(pFile)
    {
        char pczBuf[100];
        for(int i=0; i<m_stGEnvConfig.m_nCondLen; i++)
        {
            sprintf(pczBuf,"%c",pstClfr->m_pczCondition[i]);
            fwrite(pczBuf,strlen(pczBuf),1,pFile);
        }

        sprintf(pczBuf," : %d ",pstClfr->m_nAction);
        fwrite(pczBuf,strlen(pczBuf),1,pFile);
        sprintf(pczBuf," ----> Numerosity: %d ",pstClfr->m_nNumerosity);
        fwrite(pczBuf,strlen(pczBuf),1,pFile);
        sprintf(pczBuf,"Accuracy: %0.2f ",pstClfr->m_fAccuracy);
        fwrite(pczBuf,strlen(pczBuf),1,pFile);
        sprintf(pczBuf,"Fitness: %0.2f ",pstClfr->m_fFitness);
        fwrite(pczBuf,strlen(pczBuf),1,pFile);
        sprintf(pczBuf,"Prediction Error: %0.2f ",pstClfr->m_fPredictErr);
        fwrite(pczBuf,strlen(pczBuf),1,pFile);
        sprintf(pczBuf,"Prediction: %0.2f ",pstClfr->m_fPredict);
        fwrite(pczBuf,strlen(pczBuf),1,pFile);
        sprintf(pczBuf,"Experience: %d ",pstClfr->m_nExprnc);
        fwrite(pczBuf,strlen(pczBuf),1,pFile);
        sprintf(pczBuf,"ActionSetSize: %0.2f\n",pstClfr->m_fActSetSize);
        fwrite(pczBuf,strlen(pczBuf),1,pFile);
    }
    else
    {
        for(int i=0; i<m_stGEnvConfig.m_nCondLen; i++)
            printf("%c",pstClfr->m_pczCondition[i]);

        printf(" : %d ",pstClfr->m_nAction);
        printf(" ----> Numerosity: %d ",pstClfr->m_nNumerosity);
        printf("Accuracy: %f ",pstClfr->m_fAccuracy);
        printf("Fitness: %f ",pstClfr->m_fFitness);
        printf("Prediction Error: %f ",pstClfr->m_fPredictErr);
        printf("Prediction: %f ",pstClfr->m_fPredict);
        printf("Experience: %d\n",pstClfr->m_nExprnc);
    }
}

//Sort the classifier set pstClfrSet in numerosity, prediction, fitness, or error order.
//type 0 = numerosity order, type 1 = prediction order, type 2 = fitness order, type 3=error order

ClassifierSet* clfrharness::sortClassifierSet(ClassifierSet **pstSet, int nType)
{
	ClassifierSet *pstSetP, *pstMaxSet, *pstNewSet, *pstNewSetHead;
	double fMax;

	fMax=0.0;
	pstNewSetHead = new ClassifierSet;

	pstNewSet=pstNewSetHead;
	do
	{
		fMax=-100000.0;
		// check the classifier set cls for the next maximum -> already inserted classifier are referenced by the NULL pointer */
		for( pstSetP=*pstSet, pstMaxSet=NULL; pstSetP!=NULL; pstSetP=pstSetP->pstNext )
		{
			if(pstSetP->pstClfr!=NULL && (pstMaxSet==NULL || ((nType==0 && pstSetP->pstClfr->m_nNumerosity>fMax) || (nType==1 && pstSetP->pstClfr->m_fPredict>fMax) || (nType==2 && pstSetP->pstClfr->m_fFitness/pstSetP->pstClfr->m_nNumerosity > fMax) || (nType==3 && -1.0*(pstSetP->pstClfr->m_fPredictErr) > fMax))))
			{
				if(nType==0){
					fMax=pstSetP->pstClfr->m_nNumerosity;
				}
				else if (nType==1){
					fMax=pstSetP->pstClfr->m_fPredict;
				}
				else if (nType==2){
					fMax=pstSetP->pstClfr->m_fFitness/pstSetP->pstClfr->m_nNumerosity;
				}
				else if(nType==3){
					fMax=-1.0*(pstSetP->pstClfr->m_fPredictErr);
				}
				pstMaxSet=pstSetP;
			}
		}

		if(fMax>-100000.0)
		{
			pstNewSet->pstNext = new ClassifierSet;
			pstNewSet=pstNewSet->pstNext;
			pstNewSet->pstNext=NULL;
			pstNewSet->pstClfr=pstMaxSet->pstClfr;
			pstMaxSet->pstClfr=NULL; // do not delete the classifier itself, as it will be present in the new, sorted classifier list
		}
	}while(fMax>-100000.0);

	// set the new ClassifierSet pointer and free the old stuff
	pstNewSet=pstNewSetHead->pstNext;
	delete pstNewSetHead;
	freeSet(pstSet);

	return pstNewSet; // return the pointer to the new ClassifierSet
}


void clfrharness::sortClassifierSetAndUpdatePtr(ClfrSetType eClfrSetType, int nType)
{
    switch(eClfrSetType)
    {
        case POPSET:
            m_pstPopulation = sortClassifierSet(&m_pstPopulation,nType);
            break;
/*        case MSET:
            return getSetSize(m_pstMatchSet);
            break;
        case ACTSET:
            return getSetSize(m_pstActionSet);
            break;
        case KILLSET:
            return getSetSize(m_pstKillSet);
            break;
*/
        default:
            printf("\n Classifier Set type doesn't match\n");
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Simplify
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool clfrharness::qualifyForSimplification(Classifier *pstClfr1, Classifier *pstClfr2)
{
	if(isSameCond(pstClfr1->m_pczCondition,pstClfr2->m_pczCondition) && pstClfr1->m_nAction != pstClfr2->m_nAction)
	{
		if(pstClfr1->m_fPredict == 0.0 && pstClfr2->m_fPredict == (double)m_stGEnvConfig.m_nMaxPayoff)
		{ //swap the classifiers
			Classifier pstTemp;
			memmove(&pstTemp,pstClfr1,sizeof(Classifier));
			memmove(pstClfr1,pstClfr2,sizeof(Classifier));
			memmove(pstClfr2,&pstTemp,sizeof(Classifier));

            //swap their conditions

            for(int i=0; i<m_stGEnvConfig.m_nCondLen; i++)
            {
                char chTemp = pstClfr1->m_pczCondition[i];
                pstClfr1->m_pczCondition[i] = pstClfr2->m_pczCondition[i];
                pstClfr2->m_pczCondition[i] = chTemp;
            }
            //memmove(&pczTemp, pstClfr1->m_pczCondition,sizeof(char)*m_stGEnvConfig.m_nCondLen);
            //memmove(pstClfr1->m_pczCondition, pstClfr2->m_pczCondition,sizeof(char)*m_stGEnvConfig.m_nCondLen);
            //memmove(pstClfr2->m_pczCondition, &pczTemp, sizeof(char)*m_stGEnvConfig.m_nCondLen);

			return true;
		}

		if(pstClfr1->m_fPredict == (double)m_stGEnvConfig.m_nMaxPayoff && pstClfr2->m_fPredict == 0.0)
			return true;

	}

	return false;
}

//condense the population of classifiers by deleting classifier rules with "numerosity < limit"
void clfrharness::condensePopulation()
{
    //population sorted according to numerosity
	int nFinalPopSize=0, nMaxNumerosityDiff=0, nLBNumerosity=0, nHBNumerosity=0;
	ClassifierSet *pstIt, *pstBoundary=NULL, *pstDummy;

	pstDummy = new ClassifierSet; // ending symbol
	pstDummy->pstClfr = new Classifier;
	pstDummy->pstClfr->m_nNumerosity = 0;
	pstDummy->pstNext = NULL;
	for(pstIt = m_pstPopulation; pstIt->pstNext!=NULL; pstIt=pstIt->pstNext)
		; //empty statement, pointer go to the end

	pstIt->pstNext = pstDummy; // insert at the end

	for(pstIt = m_pstPopulation; pstIt->pstNext!=NULL; pstIt=pstIt->pstNext)
	{
		if(pstIt->pstClfr->m_nNumerosity - pstIt->pstNext->pstClfr->m_nNumerosity > nMaxNumerosityDiff)
		{
			pstBoundary = pstIt;
			nHBNumerosity = pstBoundary->pstClfr->m_nNumerosity;
			nLBNumerosity = pstBoundary->pstNext->pstClfr->m_nNumerosity;
			nMaxNumerosityDiff = nHBNumerosity - nLBNumerosity;
		}
	}

	freeClassifierSet(&(pstBoundary->pstNext));
	pstBoundary->pstNext = NULL;

	printf("\nLBN: %d, HBN: %d\n",nLBNumerosity, nHBNumerosity);

	for(pstIt=m_pstPopulation; pstIt!=NULL; pstIt=pstIt->pstNext)
		nFinalPopSize++;

	printf("\nNumber of classifiers in final population: %d\n",nFinalPopSize);
}

void clfrharness::simplifyPopulation()
{
    ClassifierSet *pstSet,*pstSetP, *pstKillSet = NULL;

	//consider only well experienced and accurate classifier rules
	for(pstSet=m_pstPopulation,pstSetP=m_pstPopulation ; pstSet!=NULL; pstSet=pstSet->pstNext)
	{
		//printClassifier(pstSet->pstClfr);
		while(pstSet->pstClfr->m_nExprnc <= 1.0/m_stGClsCFG.m_fBeta || pstSet->pstClfr->m_fPredictErr > m_stGClsCFG.m_fEpsilon0)
		{
			//printClassifier(pstSet->pstClfr);
			if(pstSetP==pstSet)
			{
				m_pstPopulation=pstSet->pstNext;
				delClfrPointerFromSet(&m_pstPopulation, pstSet->pstClfr);
				freeClassifier(pstSet->pstClfr);
				addClassifier(pstSet->pstClfr,&pstKillSet,false);
				delete pstSet;
				pstSet=m_pstPopulation;
				pstSetP=m_pstPopulation;
			}
			else
			{
				pstSetP->pstNext=pstSet->pstNext;
				delClfrPointerFromSet(&m_pstPopulation,pstSet->pstClfr);
				freeClassifier(pstSet->pstClfr);
				addClassifier(pstSet->pstClfr,&pstKillSet,false);
				delete pstSet;
				pstSet=pstSetP;
			}
		}
		pstSetP=pstSet;
	}
	freeSet(&pstKillSet);

	//simplify the population of classifiers by deleting redundant classifier rules
	ClassifierSet *pstSet2;
	for(pstSet=m_pstPopulation; pstSet->pstNext!=NULL; pstSet=pstSet->pstNext)
	{
		for(pstSetP=pstSet,pstSet2=pstSet->pstNext; pstSet2!=NULL; pstSet2=pstSet2->pstNext)
		{
			if(qualifyForSimplification(pstSet->pstClfr,pstSet2->pstClfr))
			{
				pstSet->pstClfr->m_nNumerosity += pstSet2->pstClfr->m_nNumerosity;
				if(pstSet->pstClfr->m_nExprnc < pstSet2->pstClfr->m_nExprnc)
					pstSet->pstClfr->m_nExprnc = pstSet2->pstClfr->m_nExprnc;

				if(pstSet->pstClfr->m_fFitness < pstSet2->pstClfr->m_fFitness)
					pstSet->pstClfr->m_fFitness = pstSet2->pstClfr->m_fFitness;

				pstSetP->pstNext=pstSet2->pstNext;
				delClfrPointerFromSet(&m_pstPopulation,pstSet2->pstClfr);
				freeClassifier(pstSet2->pstClfr);
				addClassifier(pstSet2->pstClfr,&pstKillSet,false);
				delete pstSet2;
				pstSet2=pstSetP;
			}
			pstSetP=pstSet2;
		}
		if(pstSet->pstNext==NULL){
			break;
		}
	}
	freeSet(&pstKillSet);
}


} //end namespace xcs

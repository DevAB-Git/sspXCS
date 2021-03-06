////////////////////////////////////////////////////////////////////////////////////////////////////
//  xcsclassifierharness.hpp
//  xcsclassifier file for xcs based single step problems
//  Developer: AB Siddique
//  School of Engineering and Computer Science
//  Victoria University of Wellington
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef XCSCLASSIFIER_HARNESS_HPP__
#define XCSCLASSIFIER_HARNESS_HPP__
#include "xcsconfig.hpp"
#include "xcsenvironment.hpp"

namespace xcs
{

struct Classifier
{
	int         m_nAction;
	int         m_nNumerosity;
	int         m_nExprnc;
	int         m_nTimeStamp;
	double      m_fPredict;
	double      m_fPredictErr;
	double      m_fAccuracy;
	double      m_fFitness;
	double      m_fActSetSize;
	//temp chg AB
	//char        m_pczCondition[11];
	char        *m_pczCondition;     //condition[condLength];

}; //temp chg AB ALIGN(64);

struct ClassifierSet
{
	Classifier  *pstClfr;
	ClassifierSet *pstNext;
};//ALIGN(64);

class clfrharness
{
public:
    clfrharness();
    ~clfrharness();

    void initClassifierSets();
    //initialize classifier configuration and other settings
    void initClassifier(Classifier *pstClfr, double fActSetSize, int nTimeStamp);
    int getNumerositySum(ClassifierSet *pstClfrSet);
    int getSetSize(ClassifierSet *pstClfrSet);
    int getSetSize(ClfrSetType eClfrSetType);

    void getMatchSet(char pczState[], int nTimeStamp);
    int getActions(ClassifierSet *pstClfrSet, bool arrCvrdActions[]);
    bool isCondMatched(char pczClfrCond[], char pczCond[]);
    void createMatchCond(char pczCond[], char pczState[]);
    void createMatchCond(Classifier *pstClfr, char pczState[]);
    Classifier* createClfr(char pczState[], int nAction, int nSetSize, int nTimeStamp);

    //determines the prediction array out of the match set ms
    void getPredictArr();
    //Returns the highest value in the prediction array.
    double getBestValue();
    int randActionWinner();
    int bestActionWinner();
    int rouletteActionWinner();

    void getActionSet(int nAction);
    void updateActionSet(double fMaxPredict, double fReward);
    void updateFitness(ClassifierSet *pstClfrSet);

    void discoveryComponent(ClassifierSet **pstSet, int nItTime, char pczSituation[]);
    void getDiscoversSums(ClassifierSet *pstSet, double *fFitSum, int *nSetSum, int *nGaitSum);
    void setTimeStamps(ClassifierSet *pstSet, int nItTime);

    void selectTwoClassifiers(Classifier **pstClfr, Classifier **pstParents, ClassifierSet *pstSet, double fFitSum, int nSetSum);
    Classifier* tournamentSelection(ClassifierSet *pstSet, int nSetSum, Classifier *pstNotMe);
    Classifier* rouletteWheelSelection(ClassifierSet *pstSet, double fFitSum);

    void crossover(Classifier **pstClfr, int nType);
    void uniformCrossover(Classifier **pstClfr);
    void onePointCrossover(Classifier **pstClfr);
    void twoPointCrossover(Classifier **pstClfr);
    void mutation(Classifier *pstClfr, char pczState[]);
    void applyNicheMutation(Classifier *pstClfr, char pczState[]);
    void applyGeneralMutation(Classifier *pstClfr, char pczState[]);
    void mutateAction(Classifier *pstClfr);

    void insertDiscoveredClassifier(Classifier **pstClfr, Classifier **pstParents, ClassifierSet **pstSet, int nLen);

    void actSetSubsumption();
    void subsumeClassifier(Classifier *pstClfr, Classifier **pstParents, ClassifierSet *pstLocSet);
    bool subsumeClassifierToSet(Classifier *pstClfr, ClassifierSet *pstSet);
    bool isSubSumes(Classifier *pstClfr1, Classifier * pstClfr2);
    bool isSubSumer(Classifier *pstClfr);
    bool isMoreGeneral(char pczFirst[], char pczSecond[]);

    // it is similar to addNewClassifierToSet
    bool addClassifier(Classifier *pstClfr, ClassifierSet **pstClfrSet);
    //it is similar to addClassifierToPointerSet or addClassifierToSet for  false or true value of bIncNum respectivelly
    bool addClassifier(Classifier *pstClfr, ClassifierSet **pstClfrSet, bool bIncNum);
    bool isEquals(Classifier *pstClfr1, Classifier *pstClfr2);
    bool isSameCond(char pczCond1[], char pczCond2[]);

    //Deletes a classifier from the population.
    Classifier* delStochClassifier();
    //Returns the vote for deletion of the classifier.
    double getDelProp(Classifier *pstClfr, double fMeanFitness);
    //Deletes the classifier setp from the population pop, setpl points to the classifier that is before setp in the list
    Classifier* delClassifier(ClassifierSet *pstSetP, ClassifierSet *pstSetPl);
    bool updateSet(ClassifierSet **pstUset);
    bool delClfrPointerFromSet(ClassifierSet **pstUset, Classifier *pstClfr);


    void freeClassifier(Classifier *pstClfr);
    void freeSet(ClfrSetType eClfrSetType);
    void freeSet(ClassifierSet **pstClfrSet);
    void freeClassifierSet(ClfrSetType eClfrSetType);
    void freeClassifierSet(ClassifierSet **pstClfrSet);

    void printClassifierSet(ClassifierSet *pstHead, FILE *pFile=NULL);
    void printClassifierSet(ClfrSetType eClfrSetType, FILE *pFile=NULL);
    void printClassifier(Classifier *pstClfr, FILE *pFile=NULL);

    ClassifierSet* sortClassifierSet(ClassifierSet **pstSet, int nType);
    void sortClassifierSetAndUpdatePtr(ClfrSetType eClfrSetType, int nType);


    //condense the population of classifiers by deleting classifier rules with "numerosity < limit"
    void condensePopulation();
    void simplifyPopulation();
    bool qualifyForSimplification(Classifier *pstClfr1, Classifier *pstClfr2);

    ClassifierSet**  getPopSet();
    ClassifierSet**  getActSet();
    ClassifierSet**  getKilSet();

private:
    env                  m_stEnv;
    ClassifierSet       *m_pstPopulation;
    ClassifierSet       *m_pstMatchSet;
    ClassifierSet       *m_pstActionSet;
    ClassifierSet       *m_pstKillSet;
    double              *m_pfPredictArr;               //prediction array
    double              *m_pfSumClfrFtnsPredictArr;    //The sum of the fitnesses of classifiers that represent each entry in the prediction array.

};

}
#endif //XCSCLASSIFIER_HARNESS_HPP__

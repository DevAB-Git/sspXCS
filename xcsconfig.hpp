////////////////////////////////////////////////////////////////////////////////////////////////////
//  xcsconfig.hpp
//  It contains configuration settings for xcs based single step problems
//  Developer: AB Siddique
//  School of Engineering and Computer Science
//  Victoria University of Wellington
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef XCSCONFIG_HPP__
#define XCSCONFIG_HPP__
#include "xcsdefs.hpp"

using namespace std;

namespace xcs
{

struct ClsConfig
{

    char        m_cDntCare;                 //The don't care symbol (normally '#')
    bool        m_bGASubSump;               //Specifies if GA subsumption should be executed.
    bool        m_bActSetSubSump;           //Specifies if action set subsumption should be executed.
    bool        m_bGAErrBasedSel;
    double      m_fAlpha;                   //The fall of rate in the fitness evaluation.
    double      m_fBeta;                    //The learning rate for updating fitness, prediction, prediction error, and action set size estimate in XCS's classifiers.
    double      m_fGama;                    //The discount rate in multi-step problems.
    double      m_fDelta;                   //The fraction of the mean fitness of the population below which the fitness of a classifier may be considered in its vote for deletion.
    double      m_fNu;                      //Specifies the exponent in the power function for the fitness evaluation.
    double      m_fThetaGA;                 //The threshold for the GA application in an action set.
    double      m_fEpsilon0;                //The error threshold under which the accuracy of a classifier is set to one.
    double      m_fpX;                      //The probability of applying crossover in an offspring classifier.
    double      m_fpM;                      //The probability of mutating one allele and the action in an offspring classifier.
    double      m_fProbDntCare;             //The probability of using a don't care symbol in an allele when covering.
    double      m_fPredictErrReduction;     //The reduction of the prediction error when generating an offspring classifier.
    double      m_fFitnessReduction;        //The reduction of the fitness when generating an offspring classifier.
    double      m_fInitPredict;             //The initial prediction value when generating a new classifier (e.g in covering).
    double      m_fInitPredictErr;          //The initial prediction error value when generating a new classifier (e.g in covering).
    double      m_fInitFitness;             //The initial fitness value when generating a new classifier (e.g in covering).
    double      m_fThetaSel;                //The fraction of classifiers participating in a tournament from an action set.
    double      m_fForceDiffInTornmnt;
    double      m_fSelTolrnc;
    int         m_nThetaDel;                //Specified the threshold over which the fitness of a classifier may be considered in its deletion probability.
    int         m_nCrssoverType;            // 0 uniform, 1 onePoint, and 2 twoPoint crossover.
    int         m_nMutationType;            // 0 niche, and 1 general mutation.
    int         m_nThetaSub;                //The experience of a classifier required to be a subsumer.

    ClsConfig();

};

struct envConfig
{
    // multiplexer environment settings
    int         m_nPosBits;                    //2, 3, 4, 5, 6, and 7 for 6-, 11-, 20-, 37-, 70-, and 135-bits MUX respectively
    int         m_nCondLen;
// countOnes, hiddenEvenParity and hiddenOddParity environments settings
    int         m_nRlvntBits;
    int         m_nMaxPopSize;                 //Specifies the maximal number of micro-classifiers in the population.
    int         m_nMaxProblems;                //training set
    int         m_nMaxPayoff;
    int         m_nActions; //0 or 1
// Environment type
    Environment m_eEnv;

    envConfig();
};

//structure to log the behavior during execution
struct LogExec
{
    uint32_t    m_nTotalRuns;
    uint32_t    m_nTestFrequency;
    string      m_strLogPath;              //Path for output log files
    string      m_strPerformance;       //Output file for Performance
    string      m_strCndnsdClsPop;      //Output file for condensed classifier population
    string      m_strSmplClsPop;        //Output file for simplified classifier population
    string      m_strWholeClsPop;      //Output file for wole classifier population
    FILE*       m_pfPerformance;
    FILE*       m_pfCndnsdClsPop;
    FILE*       m_pfSmplClsPop;
    FILE*       m_pfWholeClsPop;
    LogExec();
    void OpenLogFiles();
    void CloseLogFiles();
    FILE* getFilePtr(LogFileType eFileType);
};

//load configurations from file
struct loadConfig
{
    bool isComment(string& strLine);
    string lineTrim(string& strLine);
    string removeFinalSpaces(string& strLine);
    bool readLine(ifstream& ifsFile, string& strLine);
    string getNextToken(string& strLine );
    void loadConfigFromFile(string strFileName);
    void parseargs(int argc, char** argv);
    void printConfig();
    //void printConfigImpOnly();
    string m_strConfigFile;
};

// declare classifiers config object
extern ClsConfig m_stGClsCFG;// ALIGN(64);
//declare LogExe object
extern LogExec m_stGLogExec;// ALIGN(64);
//declare environment setting object
extern envConfig m_stGEnvConfig;
//declare loadConfig object
extern loadConfig m_stGLoadConfig;

} //~end namespace xcs
#endif // XCSCONFIG_HPP__


////////////////////////////////////////////////////////////////
//  xcssystem.cpp
//  xcssystem file for xcs based single step problems
//  developed by AB Siddique
//  School of Engineering and Computer Science
//  Victoria University of Wellington
/////////////////////////////////////////////////////////////////
#include "xcssystem.hpp"
#include <string.h>

namespace xcs
{

xcsSys::xcsSys()
{

}

xcsSys::~xcsSys()
{

}

void xcsSys::startXCS()
{
    m_tXcsTime.startTimer();
    m_stClfrHrns.initClassifierSets();
    //Status
    printf("XCS is busy! Please wait ....\n");
    singleStepExp();
    //Sort and print classifier set
    //m_stClfrHrns.printClassifierSet(m_stClfrHrns.sortClassifierSet(m_stClfrHrns.getPopSet(), 0), m_stGLogExec.getFilePtr(WHOLECLSPOP));
    m_stClfrHrns.sortClassifierSetAndUpdatePtr(POPSET, 0);
    m_stClfrHrns.printClassifierSet(POPSET, m_stGLogExec.getFilePtr(WHOLECLSPOP));

    //Free pop set
    m_stClfrHrns.freeClassifierSet(POPSET);
}

//Executes one single-step experiment and monitor the performance.
void xcsSys::singleStepExp()
{
    int nExplore=0;
    char pczState[m_stGEnvConfig.m_nCondLen];
    int arrCorrect[m_stGLogExec.m_nTestFrequency];
    double arrSysErr[m_stGLogExec.m_nTestFrequency];
    //run the program for total number of instances
    for(int nExplrInst=0; nExplrInst <= m_stGEnvConfig.m_nMaxProblems; nExplrInst+=nExplore)
    {
		nExplore = (nExplore+1)%2;
		m_stEnv.resetState(pczState);

		if(nExplore==1)
            singleStepExplore(pczState,nExplrInst);
		else
            singleStepExploit(pczState,nExplrInst, arrCorrect, arrSysErr);

		if(nExplrInst%m_stGLogExec.m_nTestFrequency==0 && nExplore==0 && nExplrInst>0)
			writePerformance(arrCorrect, arrSysErr, nExplrInst, POPSET, PERFORMANCE);
	}

}

// Executes one main learning loop for a single step problem.
void xcsSys::singleStepExplore(char pczState[], int nInsts)
{
    bool bWasCorrect = false;
	m_stClfrHrns.getMatchSet(pczState,nInsts);
	m_stClfrHrns.freeSet(KILLSET);

	m_stClfrHrns.getPredictArr();
	int nActWinner = m_stClfrHrns.randActionWinner();
	m_stClfrHrns.getActionSet(nActWinner);
	double fReward = m_stEnv.prfmAction(nActWinner,pczState,bWasCorrect);

	m_stClfrHrns.updateActionSet(0.0,fReward);
	m_stClfrHrns.freeSet(KILLSET);

	m_stClfrHrns.discoveryComponent(m_stClfrHrns.getActSet(),nInsts,pczState);
	m_stClfrHrns.freeSet(KILLSET);

	m_stClfrHrns.freeSet(MSET);
	m_stClfrHrns.freeSet(ACTSET);

}

void xcsSys::singleStepExploit(char pczState[], int nInsts, int arrCorrect[], double arrSysErr[])
{
    bool bWasCorrect = false;

    m_stClfrHrns.getMatchSet(pczState,nInsts);
	m_stClfrHrns.freeSet(KILLSET);

	m_stClfrHrns.getPredictArr();
	int nActWinner = m_stClfrHrns.bestActionWinner();
	double fReward = m_stEnv.prfmAction(nActWinner,pczState,bWasCorrect);

	if(bWasCorrect)
		arrCorrect[nInsts%m_stGLogExec.m_nTestFrequency]=1;
	else
		arrCorrect[nInsts%m_stGLogExec.m_nTestFrequency]=0;

    arrSysErr[nInsts%m_stGLogExec.m_nTestFrequency] = absValue(fReward - m_stClfrHrns.getBestValue());

	m_stClfrHrns.freeSet(MSET);
}

void xcsSys::writePerformance(int arrCorrect[], double arrSysErr[], int nInsts, ClfrSetType eClfrSetType, LogFileType eFileType)
{
    char pczBuf[100];
	double fPerf=0.0, fSysErr=0.0;
	FILE* fWriteFile = m_stGLogExec.getFilePtr(eFileType);

    if(!fWriteFile)
    {
        printf("\nError in opening the write file ");
		exit(1);
    }

	for(uint32_t i=0; i<m_stGLogExec.m_nTestFrequency; i++)
	{
		fPerf+=arrCorrect[i];
        fSysErr+=arrSysErr[i];
	}

	fPerf/=m_stGLogExec.m_nTestFrequency;
    fSysErr/=m_stGLogExec.m_nTestFrequency;

	int nSetSize = m_stClfrHrns.getSetSize(eClfrSetType);

    sprintf(pczBuf,"%d ",nInsts); fwrite(pczBuf,strlen(pczBuf),1,fWriteFile);
	sprintf(pczBuf,"%f ",fPerf); fwrite(pczBuf,strlen(pczBuf),1,fWriteFile);
	sprintf(pczBuf,"%f ",fSysErr); fwrite(pczBuf,strlen(pczBuf),1,fWriteFile);
	sprintf(pczBuf,"%d ",nSetSize); fwrite(pczBuf,strlen(pczBuf),1,fWriteFile);
	fwrite("\n",strlen("\n"),1,fWriteFile);

	//int numerositySum = getNumerositySum(population); printf("%d %f %f %d %d\n",exploreProbC,perf,serr,setSize,numerositySum);
	//printf("%d %f %f %d\n",exploreProbC,perf,serr,setSize);
}

void xcsSys::writePerformanceHeader(int nExpNo, LogFileType eFileType)
{
    char pczBuf[1000];
	FILE* fWriteFile = m_stGLogExec.getFilePtr(eFileType);

    if(!fWriteFile)
    {
        printf("\nError in opening the write file ");
		exit(1);
    }

    sprintf(pczBuf,"%s ","Experiment No: "); fwrite(pczBuf,strlen(pczBuf),1,fWriteFile);
	sprintf(pczBuf,"%d ",nExpNo); fwrite(pczBuf,strlen(pczBuf),1,fWriteFile);
	fwrite("\n",strlen("\n"),1,fWriteFile);
	//string strTemp = "ProbInsts, Accuracy, SysError, PopSet ";
	//SGK;;;;;;;;;;;;;JTHHKGRRRRRpJ*78
	+- dfb+sprintf(pczBuf,"%s ",strTemp); fwrite(pczBuf,strlen(pczBuf),1,fWriteFile);
}

void xcsSys::expExit(LogFileType eFileType)
{
    printf("Shutting down...\n");
	m_tXcsTime.getCpuElapsedTime();
   	logHeader();

	//compute elapsed time
	string strTime;
	m_tXcsTime.getClkElapsedTime(strTime);
	FILE* fWriteFile = m_stGLogExec.getFilePtr(eFileType);
	fwrite(strTime.c_str(),strlen(strTime.c_str()),1,fWriteFile);
	fwrite("\n",strlen("\n"),1,fWriteFile);

	printf("Elapsed Time: %s", strTime.c_str());
	printf(" Secs\n\n");
}

void xcsSys::logHeader()
{
    time_t t;
    //char hostName[1024]; //[MAXHOSTNAMELEN];
    time(&t);
    //comiplation error only at KDE & code block
    //gethostname(hostName,sizeof(hostName));
    //printf("%s\nSeed: %ld\n%s",hostName,getSeed(),ctime(&t));
    printf("%s\nSeed: %ld\n%s","ABSystem",getSeed(),ctime(&t));
}

}//~end namespace xcs

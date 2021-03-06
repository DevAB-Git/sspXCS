////////////////////////////////////////////////////////////////
//  xcsenvironment.hpp
//  environment file for xcs based single step problems
//  developed by AB Siddique
//  School of Engineering and Computer Science
//  Victoria University of Wellington
/////////////////////////////////////////////////////////////////
#ifndef XCSENVIRONMENT_HPP__
#define XCSENVIRONMENT_HPP__
#include "xcsdefs.hpp"

namespace xcs
{

class xcsSys;
class clfrharness;

class env
{
//friend class
    friend class xcsSys;
    friend class clfrharness;
public:
    env();
    ~env();

    bool isDV1Term(int nNum);
    void resetState(char pczState[]);
    double evlAction(int nAction, int nGenunAct, bool &bReset);
    // Executes the action and determines the reward.
    double prfmAction(int nAction, char pczState[], bool &bReset);
    double prfmEvnParityAction(int nAction, char pczState[], bool &bReset);
    double prfmCarryAction(int nAction, char pczState[], bool &bReset);
    double prfmMuxAction(int nAction, char pczState[], bool &bReset);
    double prfmHiddenEvenParityAction(int nAction, char pczState[], bool &bReset);
    double prfmHiddenOddParityAction(int nAction, char pczState[], bool &bReset);
    double prfmCountOnesAction(int nAction, char pczState[], bool &bReset);
    double prfmMajorityOnAction(int nAction, char pczState[], bool &bReset);
    double prfmDV1Action(int nAction, char pczState[], bool &bReset);


private:
    int         *m_pnPosRlvntBits;          //if it gives an error due to a bug in codeblock, just specify the size

};


} //end namespace xcs

#endif //XCSENVIRONMENT_HPP__

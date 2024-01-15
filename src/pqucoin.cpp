// Copyright (c) 2015-2021 The Pqucoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/random/uniform_int.hpp>
#include <boost/random/mersenne_twister.hpp>

#include "policy/policy.h"
#include "arith_uint256.h"
#include "pqucoin.h"
#include "txmempool.h"
#include "util.h"
#include "validation.h"
#include "pqucoin-fees.h"


//wh_include
#include <unordered_map>
#include <string>
using namespace std;

vector<unsigned int> btc_nBits;
vector<int> btc_block_counts;
vector<unsigned int> ltc_nBits;
vector<int> ltc_block_counts;

unordered_map<string, unsigned int> hash_to_nBits;


int static generateMTRandom(unsigned int s, int range)
{
    boost::mt19937 gen(s);
    boost::uniform_int<> dist(1, range);
    return dist(gen);
}

// Pqucoin: Normally minimum difficulty blocks can only occur in between
// retarget blocks. However, once we introduce Digishield every block is
// a retarget, so we need to handle minimum difficulty on all blocks.
bool AllowDigishieldMinDifficultyForBlock(const CBlockIndex* pindexLast, const CBlockHeader *pblock, const Consensus::Params& params)
{
    // check if the chain allows minimum difficulty blocks
    if (!params.fPowAllowMinDifficultyBlocks)
        return false;

    // check if the chain allows minimum difficulty blocks on recalc blocks
    if (pindexLast->nHeight < 157500)
    // if (!params.fPowAllowDigishieldMinDifficultyBlocks)
        return false;

    // Allow for a minimum block time if the elapsed time > 2*nTargetSpacing
    return (pblock->GetBlockTime() > pindexLast->GetBlockTime() + params.nPowTargetSpacing*2);
}

unsigned int CalculatePqucoinNextWorkRequired(const CBlockIndex* pindexLast, int64_t nFirstBlockTime, const Consensus::Params& params)
{
    int nHeight = pindexLast->nHeight + 1;
    const int64_t retargetTimespan = params.nPowTargetTimespan;
    const int64_t nActualTimespan = pindexLast->GetBlockTime() - nFirstBlockTime;
    int64_t nModulatedTimespan = nActualTimespan;
    int64_t nMaxTimespan;
    int64_t nMinTimespan;

    if (params.fDigishieldDifficultyCalculation) //DigiShield implementation - thanks to RealSolid & WDC for this code
    {
        // amplitude filter - thanks to daft27 for this code
        nModulatedTimespan = retargetTimespan + (nModulatedTimespan - retargetTimespan) / 8;

        nMinTimespan = retargetTimespan - (retargetTimespan / 4);
        nMaxTimespan = retargetTimespan + (retargetTimespan / 2);
    } else if (nHeight > 10000) {
        nMinTimespan = retargetTimespan / 4;
        nMaxTimespan = retargetTimespan * 4;
    } else if (nHeight > 5000) {
        nMinTimespan = retargetTimespan / 8;
        nMaxTimespan = retargetTimespan * 4;
    } else {
        nMinTimespan = retargetTimespan / 16;
        nMaxTimespan = retargetTimespan * 4;
    }

    // Limit adjustment step
    if (nModulatedTimespan < nMinTimespan)
        nModulatedTimespan = nMinTimespan;
    else if (nModulatedTimespan > nMaxTimespan)
        nModulatedTimespan = nMaxTimespan;

    // Retarget
    const arith_uint256 bnPowLimit = UintToArith256(params.powLimit);
    arith_uint256 bnNew;
    arith_uint256 bnOld;
    bnNew.SetCompact(pindexLast->nBits);
    bnOld = bnNew;
    bnNew *= nModulatedTimespan;
    bnNew /= retargetTimespan;

    if (bnNew > bnPowLimit)
        bnNew = bnPowLimit;

    return bnNew.GetCompact();
}

void CalculatecoinNextWorkRequired_only_aux(const CBlockIndex* pindexLast, const Consensus::Params& params)
{
    int nHeight = pindexLast->nHeight + 1;
    const CBlockIndex* pindexFirst;
    if (nHeight > 4) pindexFirst = pindexLast->GetAncestor(nHeight - 4);
    else pindexFirst = pindexLast->GetAncestor(nHeight - 1);

    const int64_t retargetTimespan = params.nPowTargetTimespan; //目标时间间隔，即当前网络规定的时间跨度
    const int64_t nActualTimespan = pindexLast->GetBlockTime() - pindexFirst->GetBlockTime(); // 选择的父链实际的时间跨度

    vector<int> y_n(4);
    vector<int> x_n(4);
    for(int i=0; i<4; i++){
        x_n[i]=*(btc_block_counts.end()-i-1);
        y_n[i]=*(ltc_block_counts.end()-i-1);
    }

    if (nActualTimespan != retargetTimespan){
        // Limit adjustment step
        // 用float的精度存储中间变量
        double temp_btc_1;
        double temp_ltc_1;

        temp_btc_1 = 10.0*y_n[0]/x_n[0];

        if(temp_btc_1 >1.0) temp_btc_1 = 1.0;
        else if(temp_btc_1 <0.25) temp_btc_1 = 0.25;
        
        temp_ltc_1 = 1.0/temp_btc_1;

        int diff_1[3];
        int diff_2[3];
        for(int i=0;i<3;i++){
            diff_1[i] = (x_n[i]-x_n[i+1])>0? (x_n[i]-x_n[i+1]):(x_n[i+1]-x_n[i]);
            diff_2[i] = (y_n[i]-y_n[i+1])>0? (y_n[i]-y_n[i+1]):(y_n[i+1]-y_n[i]);

        } 

        double temp_x_a = 0.5*diff_1[0]+0.3*diff_1[1]+0.2*diff_1[2];
        double temp_y_b = 0.5*diff_2[0]+0.3*diff_2[1]+0.2*diff_2[2];

        double temp_btc_2;
        double temp_ltc_2;
        if(temp_x_a==0.0){
            temp_btc_2 =1.0;
        }
        else{
            temp_btc_2 = (x_n[1]-x_n[0])/temp_x_a;
        }

        if(temp_y_b==0.0){
            temp_ltc_2 =1.0;
        }
        else{
            temp_ltc_2 = (y_n[1]-y_n[0])/temp_y_b;
        }

        temp_btc_2 = pow(1.2,temp_btc_2);
        temp_ltc_2 = pow(1.2,temp_ltc_2);

        if(temp_btc_2 >1.5) temp_btc_2=1.5;
        else if(temp_btc_2 <0.67) temp_btc_2 = 0.67;
        if(temp_ltc_2 >1.5) temp_ltc_2=1.5;
        else if(temp_ltc_2 <0.67) temp_ltc_2 = 0.67;

        //nActualTimespan != retargetTimespan 的两种情况只有这个不同，为避免冗余合并
        double temp_btc_3;
        double temp_ltc_3;
        if(nActualTimespan>retargetTimespan){

            temp_btc_3 = (temp_btc_1*temp_btc_2 >1.0)? (temp_btc_1*temp_btc_2 ):1.0;
            temp_ltc_3 = (temp_ltc_1*temp_ltc_2 >1.0)? (temp_ltc_1*temp_ltc_2 ):1.0;
        }
        else if(nActualTimespan<retargetTimespan){


            temp_btc_3 = (temp_btc_1*temp_btc_2 >1.0)? 1.0:(temp_btc_1*temp_btc_2);
            temp_ltc_3 = (temp_ltc_1*temp_ltc_2 >1.0)? 1.0:(temp_ltc_1*temp_ltc_2);
        }

        // Retarget
        const arith_uint256 bnPowLimit = UintToArith256(params.powLimit);
        arith_uint256 bnNew_btc;
        arith_uint256 bnNew_ltc;
        bnNew_btc.SetCompact(btc_nBits.back());
        bnNew_ltc.SetCompact(ltc_nBits.back());
        bnNew_btc *=nActualTimespan;
        bnNew_btc *=temp_btc_3;
        bnNew_btc /=retargetTimespan;
        bnNew_ltc *=nActualTimespan;
        bnNew_ltc *=temp_ltc_3;
        bnNew_ltc /=retargetTimespan;

        if (bnNew_btc > bnPowLimit)
            bnNew_btc = bnPowLimit;
        if(bnNew_ltc > bnPowLimit) bnNew_ltc=bnPowLimit;

        btc_nBits.push_back(bnNew_btc.GetCompact());
        ltc_nBits.push_back(bnNew_ltc.GetCompact());
    } else if(nActualTimespan == retargetTimespan){
        // return pindexLast->nBits;
        btc_nBits.push_back(btc_nBits.back());
        ltc_nBits.push_back(ltc_nBits.back());
    }
}



bool CheckAuxPowProofOfWork(const CBlockHeader& block, const Consensus::Params& params)
{
    /* Except for legacy blocks with full version 1, ensure that
       the chain ID is correct.  Legacy blocks are not allowed since
       the merge-mining start, which is checked in AcceptBlockHeader
       where the height is known.  */
    if (!block.IsLegacy() && params.fStrictChainId && block.GetChainId() != params.nAuxpowChainId)
        return error("%s : block does not have our chain ID"
                     " (got %d, expected %d, full nVersion %d)",
                     __func__, block.GetChainId(),
                     params.nAuxpowChainId, block.nVersion);

    /* If there is no auxpow, just check the block hash.  */
    if (!block.auxpow) {
        if (block.IsAuxpow())
            return error("%s : no auxpow on block with auxpow version",
                         __func__);

        if (!CheckProofOfWork(block.GetPoWHash(), block.nBits, params))
            return error("%s : non-AUX proof of work failed", __func__);

        return true;
    }

    /* We have auxpow.  Check it.  */

    //wh_check_auxpow
    if (!block.IsAuxpow())
        return error("%s : auxpow on block with non-auxpow version", __func__);

    if (!block.auxpow->check(block.GetHash(), block.GetChainId(), params))
        return error("%s : AUX POW is not valid", __func__);
    if (block.auxpow->parentBlock.nVersion == 0x10000000) {
        if (!CheckProofOfWork(block.auxpow->getParentBlockPoWHash(), hash_to_nBits[block.GetHash().GetHex()], params))
        return error("%s : AUX proof of work failed", __func__);
        //std::cout << "in checkpow BlockHash: " << block.GetHash().GetHex() << " nBits: " << hash_to_nBits[block.GetHash().GetHex()] << std::endl;
    } else if (block.auxpow->parentBlock.nVersion == 0x20000000) {
        if (!CheckProofOfWork(block.auxpow->getParentBlockHash(), hash_to_nBits[block.GetHash().GetHex()], params))
        return error("%s : AUX proof of work failed", __func__);
    }
    return true;
}

CAmount GetPqucoinBlockSubsidy(int nHeight, const Consensus::Params& consensusParams, uint256 prevHash)
{
    int halvings = nHeight / consensusParams.nSubsidyHalvingInterval;

    if (!consensusParams.fSimplifiedRewards)
    {
        // Old-style rewards derived from the previous block hash
        const std::string cseed_str = prevHash.ToString().substr(7, 7);
        const char* cseed = cseed_str.c_str();
        char* endp = NULL;
        long seed = strtol(cseed, &endp, 16);
        CAmount maxReward = (1000000 >> halvings) - 1;
        int rand = generateMTRandom(seed, maxReward);

        return (1 + rand) * COIN;
    } else if (nHeight < (6 * consensusParams.nSubsidyHalvingInterval)) {
        // New-style constant rewards for each halving interval
        return (500000 * COIN) >> halvings;
    } else {
        // Constant inflation
        return 10000 * COIN;
    }
}



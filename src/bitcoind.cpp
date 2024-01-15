// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#if defined(HAVE_CONFIG_H)
#include "config/bitcoin-config.h"
#endif

#include "chainparams.h"
#include "clientversion.h"
#include "compat.h"
#include "rpc/server.h"
#include "init.h"
#include "noui.h"
#include "scheduler.h"
#include "util.h"
#include "httpserver.h"
#include "httprpc.h"
#include "utilstrencodings.h"

#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>

#include <stdio.h>

//wh_include
#include <pthread.h>
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <iostream>
#include <deque>
#include <vector>
#include <sys/time.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <pqucoin.h>
#include <unordered_map>
#include <string>
using namespace std;

extern vector<unsigned int> btc_nBits;
extern vector<int> btc_block_counts;
extern vector<unsigned int> ltc_nBits;
extern vector<int> ltc_block_counts;
//extern unordered_map<string, pair<uint32_t, uint32_t>> hash_to_nBits;

class BlockNumberCount {
public:
	BlockNumberCount() {}
    static void BlockNumberCountStart () {
        //注册信号捕捉
        struct sigaction act;
        act.sa_flags = 0;
        act.sa_handler = resetTimer;
        sigemptyset(&act.sa_mask);
        sigaction(SIGALRM, &act, NULL);
        cout << "timer start" << endl;
        setTimer();
    }
	static void add(int id) {
        if (id == 1) {
            ++count1;
        } else if (id == 2) {
            ++count2;
        } 
	}
	static vector<uint32_t> get(int which) {
        if (which == 1) return vector<uint32_t>(blockNumber1.begin(), blockNumber1.end());
		else if (which == 2) return vector<uint32_t>(blockNumber2.begin(), blockNumber2.end());
        return {};
	}
	static void resetTimer(int alarm) {

        cout << "ltc_nBits: ";
        for (auto i : ltc_nBits) {

            int nShift = (i >> 24) & 0xff;

            double dDiff =
                (double)0x0000ffff / (double)(i & 0x00ffffff);
            while (nShift < 29)
            {
                dDiff *= 256.0;
                nShift++;
            }
            while (nShift > 29)
            {
                dDiff /= 256.0;
                nShift--;
            }
            cout << dDiff << ' ';
        }
        cout << endl;

        blockNumber1.push_back(count1);
        ltc_block_counts.push_back(count1 ? 1 : count1);
        btc_block_counts.push_back(!count2 ? 1 : count2);
        if (blockNumber1.size() > 5) blockNumber1.pop_front();
        blockNumber2.push_back(count2);
        if (blockNumber2.size() > 5) blockNumber2.pop_front();
        count1 = 0;
        count2 = 0;
        setTimer();
        cout << "===============================================================" << endl;
        cout << "litecoin submitauxblock num: ";
        for (int i = 0; i < blockNumber1.size(); ++i) {
            cout << blockNumber1[i] << ' ';
        }
        cout << endl;
        cout << "bitcoind submitauxblock num: ";
        for (int i = 0; i < blockNumber2.size(); ++i) {
            cout << blockNumber2[i] << ' ';
        }
        cout << endl;
        cout << "===============================================================" << endl;
    }
private:
	static void setTimer() {
        struct itimerval new_value;
        new_value.it_interval.tv_sec = 0;
        new_value.it_interval.tv_usec = 0;
        new_value.it_value.tv_sec = 40;
        new_value.it_value.tv_usec = 0;
        int ret = setitimer(ITIMER_REAL, &new_value, NULL);
        if(ret == -1) {
            perror("setitimer");
            exit(0);
        }
    }
	static uint32_t count1; //最近一分钟
	static deque<uint32_t> blockNumber1; //最近五分钟
	static uint32_t count2;
	static deque<uint32_t> blockNumber2;
};
uint32_t BlockNumberCount::count1 = 0;
deque<uint32_t> BlockNumberCount::blockNumber1;
uint32_t BlockNumberCount::count2 = 0;
deque<uint32_t> BlockNumberCount::blockNumber2;

//cmx_createauxblock_define
extern UniValue createauxblock(const JSONRPCRequest& request);
// //wh_getAuxBlockThread_define
extern bool createauxblock(const std::string& address, uint256 &hash, uint32_t &nbits, int id);
extern bool SubmitAuxBlock(const char buf[]);
void* getAuxBlockThread(void* arg) {
    int skfd = socket(PF_INET, SOCK_DGRAM, 0);
    if (skfd == -1) {
        perror("socket failed in getAuxBlockthread!");
        exit(-1);
    }
    //wh_ip_getAuxBlockThread
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9988); //指定端口号
    addr.sin_addr.s_addr = htonl(INADDR_ANY); //默认获取本机ip
    //inet_pton(AF_INET, "10.162.196.131", &addr.sin_addr.s_addr); // 指定ip

    int ret = bind(skfd, (struct sockaddr*)&addr, sizeof(addr));
    if (ret == -1) {
        perror("bind failed in getAuxBlockthread!");
        exit(-1);
    }

    char recvbuf[1024] = {0}; //接收缓冲区
    char address[35] = {0}; //地址

    struct sockaddr_in client; //父链某个节点的socket信息
    socklen_t len = sizeof(client);
    int id; //父链id

    //返回的hash和nbits
    uint256 hash; //要返回给父链的 hash
    uint32_t nbits; //要返回给父链的 nbits
    while (1) {
        //memset(recvbuf, 0, sizeof(recvbuf));

        //会阻塞
        int ret = recvfrom(skfd, recvbuf, sizeof(recvbuf), 0, (struct sockaddr*)&client, &len);
        if (ret != sizeof(recvbuf)) {
            perror("recvfrom failed in getAuxBlockthread!\n");
            exit(-1);
        }

        //从父链发来的数据获取 id 和 address
        memcpy(&id, recvbuf, sizeof(id));
        memcpy(address, recvbuf + sizeof(id), sizeof(address));

        if (!createauxblock(address, hash, nbits, id)) { // id传入参数， address 和 hash 传出参数
            perror("createauxblock failed!\n");
            exit(-1);
        }
        // if (id == 1) {
        //     hash_to_nBits[hash.GetHex()].first = nbits;
        // } else {
        //     hash_to_nBits[hash.GetHex()].second = nbits;
        // }
        

        //std::cout << "BlockHash: " << hash.GetHex() << " nBits: " << nbits << std::endl;

        //memset(recvbuf, 0, sizeof(recvbuf));
        memcpy(recvbuf, hash.begin(), 32);
        memcpy(recvbuf + 32, &nbits, sizeof(nbits));
        // std::cout << "recvbuf: " << recvbuf << std::endl;
        // std::cout << "getauxblockhash: " << hash.GetHex() << std::endl;
        // std::cout << "getauxblocknbits: " << nbits << std::endl;
        // memcpy(hash.begin(), recvbuf, 32);
        // memcpy(&nbits, recvbuf + 32, 4);
        // std::cout << "getauxblockhash: " << hash.GetHex() << std::endl;
        // std::cout << "getauxblocknbits: " << nbits << std::endl;

        ret = sendto(skfd, recvbuf, sizeof(recvbuf), 0, (struct sockaddr*)&client, sizeof(client));
        if (ret != sizeof(recvbuf)) {
            perror("sendto failed!\n");
            exit(-1);
        }
        // struct in_addr inp;
        // inp.s_addr = client.sin_addr.s_addr;
        //std::cout << "client ip: " << inet_ntoa(inp) << std::endl;
        //std::cout << "send ret: " << ret << std::endl;
    }
    close(skfd);
}


void* submitAuxBlockThread(void* arg) __attribute__((optimize("O0")));
void* submitAuxBlockThread(void* arg) {
    int skfd = socket(PF_INET, SOCK_DGRAM, 0);
    if (skfd == -1) {
        perror("socket");
        exit(-1);
    }
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9999);
    //wh_ip_submitAuxBlockThread
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    //inet_pton(AF_INET, "10.162.144.73", &addr.sin_addr.s_addr);
    int ret = bind(skfd, (struct sockaddr*)&addr, sizeof(addr));
    if (ret == -1) {
        perror("bind");
        exit(-1);
    }    
    char recvbuf[1024] = {0};
    sockaddr_in client;
    socklen_t len = sizeof(client);
    BlockNumberCount::BlockNumberCountStart();
    int accept = 0; 
    while (1) {
        memset(recvbuf, 0, sizeof(recvbuf));
        int num = recvfrom(skfd, recvbuf, sizeof(recvbuf), 0, (sockaddr*)&client, &len);
        accept = SubmitAuxBlock(recvbuf);
        BlockNumberCount::add(accept);
    }
}

/* Introduction text for doxygen: */

/*! \mainpage Developer documentation
 *
 * \section intro_sec Introduction
 *
 * This is the developer documentation of the reference client for an experimental new digital currency called Bitcoin (https://www.bitcoin.org/),
 * which enables instant payments to anyone, anywhere in the world. Bitcoin uses peer-to-peer technology to operate
 * with no central authority: managing transactions and issuing money are carried out collectively by the network.
 *
 * The software is a community-driven open source project, released under the MIT license.
 *
 * \section Navigation
 * Use the buttons <code>Namespaces</code>, <code>Classes</code> or <code>Files</code> at the top of the page to start navigating the code.
 */

void WaitForShutdown(boost::thread_group* threadGroup)
{
    bool fShutdown = ShutdownRequested();
    // Tell the main threads to shutdown.
    while (!fShutdown)
    {
        MilliSleep(200);
        fShutdown = ShutdownRequested();
    }
    if (threadGroup)
    {
        Interrupt(*threadGroup);
        threadGroup->join_all();
    }
}

//////////////////////////////////////////////////////////////////////////////
//
// Start
//
bool AppInit(int argc, char* argv[])
{
    //wh_AppInit
    pthread_t tid, tid2;
    pthread_create(&tid, NULL, getAuxBlockThread, NULL);
    pthread_detach(tid);
    pthread_create(&tid2, NULL, submitAuxBlockThread, NULL);
    pthread_detach(tid2);

    boost::thread_group threadGroup;
    CScheduler scheduler;

    bool fRet = false;

    //
    // Parameters
    //
    // If Qt is used, parameters/bitcoin.conf are parsed in qt/bitcoin.cpp's main()
    ParseParameters(argc, argv);

    // Process help and version before taking care about datadir
    if (IsArgSet("-?") || IsArgSet("-h") ||  IsArgSet("-help") || IsArgSet("-version"))
    {
        std::string strUsage = strprintf(_("%s Daemon"), _(PACKAGE_NAME)) + " " + _("version") + " " + FormatFullVersion() + "\n";

        if (IsArgSet("-version"))
        {
            strUsage += FormatParagraph(LicenseInfo());
        }
        else
        {
            strUsage += "\n" + _("Usage:") + "\n" +
                  "  pqucoind [options]                     " + strprintf(_("Start %s Daemon"), _(PACKAGE_NAME)) + "\n";

            strUsage += "\n" + HelpMessage(HMM_BITCOIND);
        }

        fprintf(stdout, "%s", strUsage.c_str());
        return true;
    }

    try
    {
        if (!boost::filesystem::is_directory(GetDataDir(false)))
        {
            fprintf(stderr, "Error: Specified data directory \"%s\" does not exist.\n", GetArg("-datadir", "").c_str());
            return false;
        }
        try
        {
            ReadConfigFile(GetArg("-conf", BITCOIN_CONF_FILENAME));
        } catch (const std::exception& e) {
            fprintf(stderr,"Error reading configuration file: %s\n", e.what());
            return false;
        }
        // Check for -testnet or -regtest parameter (Params() calls are only valid after this clause)
        try {
            SelectParams(ChainNameFromCommandLine());
        } catch (const std::exception& e) {
            fprintf(stderr, "Error: %s\n", e.what());
            return false;
        }

        // Command-line RPC
        bool fCommandLine = false;
        for (int i = 1; i < argc; i++)
            if (!IsSwitchChar(argv[i][0]) && !boost::algorithm::istarts_with(argv[i], "pqucoin:"))
                fCommandLine = true;

        if (fCommandLine)
        {
            fprintf(stderr, "Error: There is no RPC client functionality in pqucoind anymore. Use the pqucoin-cli utility instead.\n");
            exit(EXIT_FAILURE);
        }
        // -server defaults to true for bitcoind but not for the GUI so do this here
        SoftSetBoolArg("-server", true);
        // Set this early so that parameter interactions go to console
        InitLogging();
        InitParameterInteraction();
        if (!AppInitBasicSetup())
        {
            // InitError will have been called with detailed error, which ends up on console
            exit(1);
        }
        if (!AppInitParameterInteraction())
        {
            // InitError will have been called with detailed error, which ends up on console
            exit(1);
        }
        if (!AppInitSanityChecks())
        {
            // InitError will have been called with detailed error, which ends up on console
            exit(1);
        }
        if (GetBoolArg("-daemon", false))
        {
#if HAVE_DECL_DAEMON
            fprintf(stdout, "Pqucoin server starting\n");

            // Daemonize
            if (daemon(1, 0)) { // don't chdir (1), do close FDs (0)
                fprintf(stderr, "Error: daemon() failed: %s\n", strerror(errno));
                return false;
            }
#else
            fprintf(stderr, "Error: -daemon is not supported on this operating system\n");
            return false;
#endif // HAVE_DECL_DAEMON
        }

        fRet = AppInitMain(threadGroup, scheduler);
    }
    catch (const std::exception& e) {
        PrintExceptionContinue(&e, "AppInit()");
    } catch (...) {
        PrintExceptionContinue(NULL, "AppInit()");
    }

    if (!fRet)
    {
        Interrupt(threadGroup);
        // threadGroup.join_all(); was left out intentionally here, because we didn't re-test all of
        // the startup-failure cases to make sure they don't result in a hang due to some
        // thread-blocking-waiting-for-another-thread-during-startup case
    } else {
        WaitForShutdown(&threadGroup);
    }
    Shutdown();

    return fRet;
}

int main(int argc, char* argv[])
{
    SetupEnvironment();
    // Connect bitcoind signal handlers
    noui_connect();
    return (AppInit(argc, argv) ? EXIT_SUCCESS : EXIT_FAILURE);
}

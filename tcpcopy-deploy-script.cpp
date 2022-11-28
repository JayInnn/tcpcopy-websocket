// It's a script for deploying and running TCPCOPY
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <string>
#include <fstream>

using std::string;
using std::vector;

struct OptionItems
{
    string toolName;
    string runningCommand;
    int helpMode;
    int paramNum;
    bool isIdempotent;
    bool isOnline;
    bool uninstall;
} item;

static bool checkParams(const vector<string> &);
static bool checkStartItem(const vector<string> &);
static bool procParams();
static bool checkTcpcopyOptions(const vector<string> &);
static bool checkInterceptOptions(const vector<string> &);
static void help();

int main(int argc, char **argv)
{
    item.helpMode = 0;
    vector<string> params;
    item.paramNum = argc - 1;
    for (int i = 1; i < argc; i++)
    {
        params.push_back(string(argv[i]));
    }
    if (checkParams(params))
    {
        procParams();
    }
    else
    {
        std::cerr << "you can enter '-h' or '--help' to get help\n";
    }
    return 0;
}

static bool checkParams(const vector<string> &params)
{
    if (params.empty())
    {
        std::cerr << "Need params\n";
        return false;
    }

    if (!checkStartItem(params))
    {
        std::cerr << "Invalid start item\n";
        return false;
    }

    if (item.toolName == "tcpcopy")
    {
        return checkTcpcopyOptions(params);
    }

    if (item.toolName == "intercept")
    {
        return checkInterceptOptions(params);
    }

    return true;
}

static bool checkStartItem(const vector<string> &params)
{
    string start_item = params[0];
    if (start_item == "--help" || start_item == "-h")
    {
        item.helpMode = 1;
        return true;
    }
    else if (start_item == "clean")
    {
        if (params.size() == 1)
        {
            item.uninstall = true;
            return true;
        }
        std::cerr << "There must be no other parameters after 'clean'\n";
    }
    else if (start_item == "tcpcopy" || start_item == "intercept")
    {
        item.toolName = start_item;
        if (item.paramNum != 1)
        {
            return true;
        }
        std::cerr << "There must be more parameters after 'tcpcopy/intercept'\n";
    }
    return false;
}
static bool checkTcpcopyOptions(const vector<string> &param)
{
    string isOnline,isIdempotent;
    if (item.paramNum > 1)
            isOnline = param[1];
    if (item.paramNum > 2)
            isIdempotent = param[2];

    //check online/offline option
    if (isOnline == "-h" || isOnline == "--help")
    {
        item.helpMode = 2;
        return true;
    }
    else if (isOnline == "--online" || isOnline == "--offline")
    {
        item.isOnline = (isOnline == "--online");
    }
    else
    {
        std::cerr << "Invalid option for online/offline"<< std::endl;
        return false;
    }

    // check idempotent/non-idempotent option
    if (isIdempotent == "-h" || isIdempotent == "--help")
    {
        item.helpMode = 4;
        return true;
    }
    else if (isIdempotent == "--idempotent" || isIdempotent == "--non-idempotent")
    {
        item.isIdempotent = (isIdempotent == "--idempotent");
    }
    else
    {
        std::cerr << "Invalid option for idempotent/non-idempotent" << std::endl;
        return false;
    }

    //generate running command
    for (int i = 3; i < param.size(); i++)
    {
        item.runningCommand = item.runningCommand + " " + param[i];
    }
    return true;
}
static bool checkInterceptOptions(const vector<string> &param)
{
    if (param[1] == "-h" || param[1] == "--help")
    {
        item.helpMode = 3;
        return true;
    }

    for (int i = 1; i < param.size(); i++)
    {
        item.runningCommand = item.runningCommand + " " + param[i];
    }
    return true;
}
static bool procParams()
{
    if (item.helpMode)
    {
        help();
        return true;
    }
    if (item.uninstall)
    {
        system("cd tcpcopy && make clean");
        system("cd intercept && make clean");
        system("rm -f tcpcopy && rm -f intercept");
        return true;
    }

    std::ifstream readPackageName;
    string packageName;
    if (item.toolName == "intercept")
    {
        //Find installation package and decompression
        system("rm -f intercept");
        system("find ./ -name \"intercept.zip\" >> temp");
        system("find ./ -name \"intercept.tar\" >> temp");
        readPackageName.open("temp", std::ios::in);
        readPackageName >> packageName;
        if (packageName == "./intercept.zip")
        {
            // system("unzip -oq intercept.zip | ln -s `unzip -l intercept.zip | awk '{if(NR==4){print $4}}'` intercept");
            system("unzip -oq intercept.zip | ln -s `unzip -l intercept.zip |egrep \"intercept.*/$\"| awk '{if(NR==1){print $4}}'` intercept");
        }
        else if (packageName == "./intercept.tar")
        {
            // system("tar -xf intercept.tar | ln -s `tar -tvf intercept.tar | awk '{if(NR==1)print($NF)}' intercept`");
            system("tar -xf intercept.tar | ln -s `tar -tvf intercept.tar |egrep \"d.*intercept.*/$\"| awk '{if(NR==1)print($NF)}'` intercept");
        }
        else
        {
            std::cerr << "can not find the package to install intercept\n";
            return false;
        }
        readPackageName.close();
        system("rm -f temp");

        //Build
        if (0 != system("cd intercept && bash -x configure --single --with-debug && make && make install"))
        {
            std::cerr << "Build error!\n";
            return false;
        }


        // run intercept
        item.runningCommand = "/usr/local/intercept/sbin/intercept " + item.runningCommand;
        std::cout << "running command: " << item.runningCommand << "\n";
        if (0 != system(item.runningCommand.c_str()))
        {
            std::cerr << "run intercept fail\n";
            return false;
        }
        std::cout << "run intercept successfully!\n";
    }
    else if (item.toolName == "tcpcopy")
    {
        //find the package and decompression

        system("rm -f tcpcopy");
        system("find ./ -name \"tcpcopy.zip\" >> temp");
        system("find ./ -name \"tcpcopy.tar\" >> temp");
        readPackageName.open("temp", std::ios::in);
        readPackageName >> packageName;
        if (packageName == "./tcpcopy.zip")
        {
            system("unzip -oq tcpcopy.zip | ln -s `unzip -l tcpcopy.zip |egrep \"tcpcopy.*/$\"| awk '{if(NR==1){print $4}}'` tcpcopy");
        }
        else if (packageName == "./tcpcopy.tar")
        {
            system("tar -xf tcpcopy.tar | ln -s `tar -tvf tcpcopy.tar |egrep \"d.*tcpcopy.*/$\"| awk '{if(NR==1)print($NF)}'` tcpcopy");
        }
        else
        {
            std::cerr << "can not find the package to install tcpcopy\n";
            return false;
        }
        readPackageName.close();
        system("rm -f temp");

        // build
        if (item.isOnline)
        {
            if (0 != system("cd tcpcopy && bash -x configure --pcap-capture --single --with-debug && make && make install"))
            {
                std::cerr << "Build error!\n";
                return false;
            }
        }
        else
        {
            if (0 != system("cd tcpcopy && bash -x configure --offline --single --with-debug && make && make install"))
            {
                std::cerr << "Build error！\n";
                return false;
            }
        }

        // run tcpcopy
        if (!item.isIdempotent)
        {
            item.runningCommand += " -W";
        }
        item.runningCommand = "/usr/local/tcpcopy/sbin/tcpcopy " + item.runningCommand;
        std::cout << "running command: " << item.runningCommand << std::endl;
        if (0 != system(item.runningCommand.c_str()))
        {
            std::cerr << "run tcpcopy fail\n";
            return false;
        }
        std::cout << "run tcpcopy successfully!" << std::endl;
    }
    return true;
}

void help()
{
    if (item.helpMode==1)
    {
        printf("Format: \n"
               "./tcpcopyDS <start_option> (<online_option> <idempotent_option>) <running command>\n\n");
        printf("Example: \n"
               "./tcpcopyDS tcpcopy --online --idempotent -i em3 -x 38002-10.12.x.x:58008 -s 10.12.x.x -c 10.13.x.x -t 20 -d\n"
               "./tcpcopyDS tcpcopy --online --non-idempotent -i em3 -x 38002-10.12.x.x:58008 -s 10.12.x.x -c 10.13.x.x -t 20 -d\n"
               "./tcpcopyDS tcpcopy --offline --idempotent -i em3 -x 38002-10.12.x.x:58008 -s 10.12.x.x -c 10.13.x.x -t 20 -d\n"
               "./tcpcopyDS tcpcopy --offline --non-idempotent -i em3 -x 38002-10.12.x.x:58008 -s 10.12.x.x -c 10.13.x.x -t 20 -d\n"
               "./tcpcopyDS intercept -i ens192 -F tcp and src port 58008 -d\n\n");
        printf("start_option:\n"
               "-h/--help     print this help and exit\n"
               "clean         clean compiled files\n"
               "tcpcopy       TCPCOPY would be compiled and running, please ensure the installation\n"
               "              package named 'tcpcopy.zip/tar' has been exist.And get more details by\n"
               "              './tcpcopyDS tcpcopy -h/--help'\n"
               "intercept     INTERCEPT would be compiled and running, please ensure the installation\n"
               "              package named 'intercept.zip/tar' has been exist.And get more details by\n"
               "              './tcpcopyDS intercept -h/--help'\n");
    }
    else if (item.helpMode==2)
    {
        printf("Format: \n"
               "./tcpcopyDS tcpcopy <online_option> <idempotent_option> <running command>\n\n");
        printf("online_option:\n"
               "--online            online real-time traffic copy function would be activated\n"
               "--offline           offline traffic copy function would be activated\n");
        printf("idempotent_option:\n"
               "--idempotent        if the respose of the target_server is exactly the same as\n"
               "                    online_server when receive the same request,using this param \n"
               "--non-idempotent    otherwise, please use this param\n");
        printf("running command --> get running commands by './tcpcopyDS tcpcopy --online -h/--help'\n"
               "                    or './tcpcopyDS tcpcopy --offline -h/--help'");
    }
    else if (item.helpMode==3)
    {
        printf("Format: \n"
               "./tcpcopyDS intercept <running command>\n\n");
        printf("Example: \n"
               "./tcpcopyDS intercept -i ens192 -F tcp and src port 58008 -d\n\n");
        printf("running command:\n");
        printf("-i <device,>        The name of the interface to listen on.  This is usually a driver\n"
               "                    name followed by a unit number,for example eth0 for the first\n"
               "                    Ethernet interface.\n");
        printf("-F <filter>         user filter(same as pcap filter)\n");
        printf("-p <num>            set the TCP port number to listen on. The default number is 36524.\n"
               "-s <num>            set the hash table size for intercept. The default value is 65536.\n"
               "-l <file>           save log information in <file>\n");
        printf("-P <file>           save PID in <file>, only used with -d option\n"
               "-b <ip_addr>        interface to listen on (default: INADDR_ANY, all addresses)\n");
        printf("-c                  set connections protected\n");
        printf("-v                  intercept version\n"
               "-h                  print this help and exit\n"
               "-d                  run as a daemon\n");
    }
    else
    {
        if (item.isOnline)
        {
            printf("Format: \n"
                   "./tcpcopyDS tcpcopy --online <idempotent_option> <running command>\n\n");
            printf("Example: \n"
                   "./tcpcopyDS tcpcopy --online --idempotent -i em3 -x 38002-10.12.7.9:58008 -s 10.12.7.8 -c 10.13.9.142 -t 20 -d\n\n");
            printf("-x <transfer,> use <transfer,> to specify the IPs and ports of the source and target\n"
                   "               servers. Suppose 'sourceIP' and 'sourcePort' are the IP and port \n"
                   "               number of the source server you want to copy from, 'targetIP' and \n");
            printf("               'targetPort' are the IP and port number of the target server you want\n"
                   "               to send requests to, the format of <transfer,> could be as follows:\n"
                   "               'sourceIP:sourcePort-targetIP:targetPort,...'. Most of the time,\n");
            printf("               sourceIP could be omitted and thus <transfer,> could also be:\n"
                   "               'sourcePort-targetIP:targetPort,...'. As seen, the IP address and the\n"
                   "               port number are segmented by ':' (colon), the sourcePort and the\n");
            printf("               targetIP are segmented by '-', and two 'transfer's are segmented by\n"
                   "               ',' (comma). For example, './tcpcopy -x 80-192.168.0.2:18080' would\n"
                   "               copy requests from port '80' on current server to the target port\n"
                   "               '18080' of the target IP '192.168.0.2'.\n");
            printf("-H <ip_addr>   change the localhost IP address to the given IP address\n");
            printf("-c <ip_addr,>  change the client IP to one of IP addresses when sending to the\n"
                   "               target server. For example,\n"
                   "               './tcpcopy -x 8080-192.168.0.2:8080 -c 62.135.200.x' would copy\n"
                   "               requests from port '8080' of current online server to the target port\n"
                   "               '8080' of target server '192.168.0.2' and modify the client IP to be\n"
                   "               one of net 62.135.200.0/24.\n");
            printf("-i <device,>   The name of the interface to listen on. This is usually a driver\n"
                   "               name followed by a unit number, for example eth0 for the first\n"
                   "               Ethernet interface.\n");
            printf("-F <filter>    user filter (same as pcap filter)\n");
            printf("-B <num>       buffer size for pcap capture in megabytes(default 16M)\n");
            printf("-S <snaplen>   capture <snaplen> bytes per packet\n");
            printf("-n <num>       use <num> to set the replication times when you want to get a \n"
                   "               copied data stream that is several times as large as the online data.\n"
                   "               The maximum value allowed is 1023. As multiple copying is based on \n"
                   "               port number modification, the ports may conflict with each other,\n");
            printf("               in particular in intranet applications where there are few source IPs\n"
                   "               and most connections are short. Thus, tcpcopy would perform better \n"
                   "               when less copies are specified. For example, \n"
                   "               './tcpcopy -x 80-192.168.0.2:8080 -n 3' would copy data flows from \n");
            printf("               port 80 on the current server, generate data stream that is three\n"
                   "               times as large as the source data, and send these requests to the\n"
                   "               target port 8080 on '192.168.0.2'.\n");
            printf("-f <num>       use this parameter to control the port number modification process\n"
                   "               and reduce port conflications when multiple tcpcopy instances are\n"
                   "               running. The value of <num> should be different for different tcpcopy\n"
                   "               instances. The maximum value allowed is 1023.\n");
            printf("-m <num>       set the maximum memory allowed to use for tcpcopy in megabytes, \n"
                   "               to prevent tcpcopy occupying too much memory and influencing the\n"
                   "               online system. When the memory exceeds this limit, tcpcopy would quit\n"
                   "               automatically. The parameter is effective only when the kernel \n");
            printf("-M <num>       MTU value sent to backend (default 1500)\n");
            printf("-D <num>       MSS value sent back(default 1460)\n");
            printf("-R <num>       set default rtt value\n");
            printf("-U <num>       set user session pool size in kilobytes(default 1).\n"
                   "               The maximum value allowed is 63.\n");
            printf("-C <num>       parallel connections between tcpcopy and intercept.\n"
                   "               The maximum value allowed is 11(default 2 connections).\n");
            printf("-s <server,>   intercept server list\n"
                   "               Format:\n"
                   "               ip_addr1:port1, ip_addr2:port2, ...\n");
            printf("-t <num>       set the session timeout limit. If tcpcopy does not receive response\n"
                   "               from the target server within the timeout limit, the session would \n"
                   "               be dropped by tcpcopy. When the response from the target server is\n"
                   "               slow or the application protocol is context based, the value should \n"
                   "               be set larger. The default value is 120 seconds.\n");
            printf("-k <num>       set the session keepalive timeout limit.\n");
            printf("-l <file>      save the log information in <file>\n"
                   "-r <num>       set the percentage of sessions transfered (integer range:1~100)\n"
                   "-p <num>       set the target server listening port. The default value is 36524.\n");
            printf("-P <file>      save PID in <file>, only used with -d option\n");
            printf("-O             only replay full session\n");
            printf("-g             gradully replay\n");
            printf("-W             don't wait response when having a new req\n");
            printf("-L             lonely for tcpcopy when intercept is closed\n");
            printf("-h             print this help and exit\n"
                   "-v             version\n"
                   "-d             run as a daemon\n");
        }
        else
        {
            printf("Format: \n"
                   "./tcpcopyDS tcpcopy --offline <idempotent_option> <running command>\n\n");
            printf("-x <transfer,> use <transfer,> to specify the IPs and ports of the source and target\n"
                   "               servers. Suppose 'sourceIP' and 'sourcePort' are the IP and port \n"
                   "               number of the source server you want to copy from, 'targetIP' and \n");
            printf("               'targetPort' are the IP and port number of the target server you want\n"
                   "               to send requests to, the format of <transfer,> could be as follows:\n"
                   "               'sourceIP:sourcePort-targetIP:targetPort,...'. Most of the time,\n");
            printf("               sourceIP could be omitted and thus <transfer,> could also be:\n"
                   "               'sourcePort-targetIP:targetPort,...'. As seen, the IP address and the\n"
                   "               port number are segmented by ':' (colon), the sourcePort and the\n");
            printf("               targetIP are segmented by '-', and two 'transfer's are segmented by\n"
                   "               ',' (comma). For example, './tcpcopy -x 80-192.168.0.2:18080' would\n"
                   "               copy requests from port '80' on current server to the target port\n"
                   "               '18080' of the target IP '192.168.0.2'.\n");
            printf("-H <ip_addr>   change the localhost IP address to the given IP address\n");
            printf("-c <ip_addr,>  change the client IP to one of IP addresses when sending to the\n"
                   "               target server. For example,\n"
                   "               './tcpcopy -x 8080-192.168.0.2:8080 -c 62.135.200.x' would copy\n"
                   "               requests from port '8080' of current online server to the target port\n"
                   "               '8080' of target server '192.168.0.2' and modify the client IP to be\n"
                   "               one of net 62.135.200.0/24.\n");
            printf("-i <file>      set the pcap file used for tcpcopy to <file> (only valid for the\n"
                   "               offline version of tcpcopy when it is configured to run at\n"
                   "               --offline mode).\n");
            printf("-a <num>       accelerated times for offline replay\n");
            printf("-I <num>       set the threshold interval for offline replay acceleration\n"
                   "               in millisecond.\n");
            printf("-T <num>       replay times for offline replay\n");
            printf("-n <num>       use <num> to set the replication times when you want to get a \n"
                   "               copied data stream that is several times as large as the online data.\n"
                   "               The maximum value allowed is 1023. As multiple copying is based on \n"
                   "               port number modification, the ports may conflict with each other,\n");
            printf("               in particular in intranet applications where there are few source IPs\n"
                   "               and most connections are short. Thus, tcpcopy would perform better \n"
                   "               when less copies are specified. For example, \n"
                   "               './tcpcopy -x 80-192.168.0.2:8080 -n 3' would copy data flows from \n");
            printf("               port 80 on the current server, generate data stream that is three\n"
                   "               times as large as the source data, and send these requests to the\n"
                   "               target port 8080 on '192.168.0.2'.\n");
            printf("-f <num>       use this parameter to control the port number modification process\n"
                   "               and reduce port conflications when multiple tcpcopy instances are\n"
                   "               running. The value of <num> should be different for different tcpcopy\n"
                   "               instances. The maximum value allowed is 1023.\n");
            printf("-m <num>       set the maximum memory allowed to use for tcpcopy in megabytes, \n"
                   "               to prevent tcpcopy occupying too much memory and influencing the\n"
                   "               online system. When the memory exceeds this limit, tcpcopy would quit\n"
                   "               automatically. The parameter is effective only when the kernel \n");
            printf("-M <num>       MTU value sent to backend (default 1500)\n");
            printf("-D <num>       MSS value sent back(default 1460)\n");
            printf("-R <num>       set default rtt value\n");
            printf("-U <num>       set user session pool size in kilobytes(default 1).\n"
                   "               The maximum value allowed is 63.\n");
            printf("-C <num>       parallel connections between tcpcopy and intercept.\n"
                   "               The maximum value allowed is 11(default 2 connections).\n");
            printf("-s <server,>   intercept server list\n"
                   "               Format:\n"
                   "               ip_addr1:port1, ip_addr2:port2, ...\n");
            printf("-t <num>       set the session timeout limit. If tcpcopy does not receive response\n"
                   "               from the target server within the timeout limit, the session would \n"
                   "               be dropped by tcpcopy. When the response from the target server is\n"
                   "               slow or the application protocol is context based, the value should \n"
                   "               be set larger. The default value is 120 seconds.\n");
            printf("-k <num>       set the session keepalive timeout limit.\n");
            printf("-l <file>      save the log information in <file>\n"
                   "-r <num>       set the percentage of sessions transfered (integer range:1~100)\n"
                   "-p <num>       set the target server listening port. The default value is 36524.\n");
            printf("-P <file>      save PID in <file>, only used with -d option\n");
            printf("-O             only replay full session\n");
            printf("-g             gradully replay\n");
            printf("-W             don't wait response when having a new req\n");
            printf("-L             lonely for tcpcopy when intercept is closed\n");
            printf("-h             print this help and exit\n"
                   "-v             version\n"
                   "-d             run as a daemon\n");
        }
    }
}
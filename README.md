# [TCPCopy](https://github.com/session-replay-tools/tcpcopy) - A TCP Stream Replay Tool

## About
Based on TCPCopy, this tool can push client pakages of Websocket protocol to target server.

## How to work
This tool could only copy ```ws://``` client pakages to target server, and ignore the return pakage from intercept. As for online copy, the problem which the ```-w``` param can lead to blocking has been solved. As for offline copy, this tool gurantees to hold time difference like raw pakages. BTW, some params should be balanced in order to simulate concurrency, especially in ```streaming speech scenarios ```.

## Run Command:
You can also refer to ```tcpcopy-deploy-script.cpp```, in this project, to deploy online/offline tcpcopy tool.
```
tcpcopy -i <network card> -x <OS_service_port-TS_ip_addr:TS_service_port> -s <AS_ip_addr> -c <faked_ip_addr> -t 20 -W -d
```

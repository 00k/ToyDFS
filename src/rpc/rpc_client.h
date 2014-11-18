// Copyright (c) 2014, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Author: yanshiguang02@baidu.com

#ifndef  BFS_RPC_CLIENT_H_
#define  BFS_RPC_CLIENT_H_

#include <sofa/pbrpc/pbrpc.h>
#include "common/mutex.h"

namespace bfs {
    
class RpcClient {
public:
    template <class T>
    bool GetStub(const std::string server, T** stub) {
        MutexLock lock(&_host_map_lock);
        sofa::pbrpc::RpcChannel* channel = NULL;
        HostMap::iterator it = _host_map.find(server);
        if (it != _host_map.end()) {
            channel = it->second;
        } else {
            // ���� channel������ͨѶͨ����ÿ����������ַ��Ӧһ�� channel
            // ����ͨ�� channel_options ָ��һЩ���ò���
            sofa::pbrpc::RpcChannelOptions channel_options;
            channel = new sofa::pbrpc::RpcChannel(&rpc_client, server, channel_options);
            _host_map[server] = channel;
        }
        *stub = new T(channel);
        return true;
    }
    template <class Stub, class Request, class Response, class Callback>
    bool SendRequest(Stub* stub, void(Stub::*func)(
                    google::protobuf::RpcController*,
                    const Request*, Response*, Callback*),
                    const Request* request, Response* response,
                    int32_t rpc_timeout, int retry_times) {
        // ���� controller ���ڿ��Ʊ��ε��ã����趨��ʱʱ�䣨Ҳ���Բ����ã�ȱʡΪ10s��
        sofa::pbrpc::RpcController controller;
        controller.SetTimeout(rpc_timeout * 1000L);
        for (int32_t retry = 0; retry < retry_times; ++retry) {
            (stub->*func)(&controller, request, response, NULL);
            if (controller.Failed()) {
                if (retry < retry_times - 1) {
                    printf("Send failed, retry ...\n");
                    usleep(1000000);
                } else {
                    printf("SendRequest fail: %s\n", controller.ErrorText().c_str());
                }
            } else {
                return true;
            }
            controller.Reset();
        }
        return false;
    }
private:
    // ���� client ����һ�� client ����ֻ��Ҫһ�� client ����
    // ����ͨ�� client_options ָ��һЩ���ò�����Ʃ���߳��������ص�
    sofa::pbrpc::RpcClientOptions client_options;
    sofa::pbrpc::RpcClient rpc_client;
    typedef std::map<std::string, sofa::pbrpc::RpcChannel*> HostMap;
    HostMap _host_map;
    Mutex _host_map_lock;
};

} // namespace bfs

#endif  // BFS_RPC_CLIENT_H_

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */

#include "ClientOP.h"
#include <json/json.h>
#include <fstream>
#include <sstream>
#include "RequestFactory.h"
#include "RequestCodec.h"
#include "RespondFactory.h"
#include "RespondMsgCodec.h"
#include "Message.pb.h"
#include "RsaCrypto.h"
#include "Hash.h"
#include "tcpSocket.h"

using namespace std;
using namespace Json;

ClientOP::ClientOP(string jsonFile)
{
	cout << "9999999999" << endl;
    // 读取json文件
    ifstream ifs(jsonFile);
    Reader r;
    Value root;
    r.parse(ifs, root);

    m_info.serverID = root["ServerID"].asString();
    m_info.clientID = root["ClientID"].asString();
    m_info.ip = root["ServerIP"].asString();
    m_info.port = root["Port"].asInt();

    cout << "服务器端地址:" << m_info.ip << "\t端口:" << m_info.port << endl;
}

ClientOP::~ClientOP()
{
}

bool ClientOP::seckeyAgree()
{
    // 创建rsa对象
    RsaCrypto rsa;
    // 创建密钥对
    rsa.generateRsakey(1024);
    // 读取公钥
    ifstream ifs("public.pem");
    stringstream str;
    str << ifs.rdbuf();

    // 初始化序列化数据
    RequestInfo reqInfo;
    reqInfo.clientID = m_info.clientID;
    reqInfo.serverID = m_info.serverID;
    reqInfo.cmd = 1; // 密钥协商
    reqInfo.data = str.str(); // 非对称加密的公钥

    Hash ha(H_SHA1);
    ha.addData(str.str());
    reqInfo.sign = rsa.rsaSign(ha.result()); // 公钥签名
    //reqInfo.sign = ha.result();
    cout << "cmd:" << reqInfo.cmd << " clientID:"
    << reqInfo.clientID << " serverID:" 
    << reqInfo.serverID << endl;
    cout<<" sign:"<<reqInfo.sign <<endl;
    cout<< "data:" << reqInfo.data << endl;
    // 编码
    //RequestFactory request_enc(&info);
    CodecFactory* factory = (CodecFactory*)new RequestFactory(&reqInfo);
    Codec* c =NULL;
    c = factory->CreateCodec();
    string encstr = c->encodeMsg();
    cout << "req编码之后的结果:" << encstr << endl;
    delete factory;
    delete c;
    c = NULL;
    // 解码
    factory = (CodecFactory*)new RequestFactory(encstr);
    c = factory->CreateCodec();
    RequestMsg* reqDecMsg = (RequestMsg*)c->decodeMsg();
    cout << "cmdType: "<< reqDecMsg->cmdtype()
        <<"\tclientID: "<<reqDecMsg->clientid()
        <<"\tserverID: "<<reqDecMsg->serverid()
        <<"\tsign: "<<reqDecMsg->sign()
        <<"\tdata: "<<reqDecMsg->data() <<endl;
    delete factory;
    delete c;
    c = NULL;
    /*// 校验签名
    RsaCrypto rsa_test("public.pem", false);
    Hash h(H_SHA1);
    string aaa = reqDecMsg->data();
    cout << "aaa:" << aaa << endl;
    h.addData(aaa);
    bool s = rsa_test.rsaVerify(h.result(), reqDecMsg->sign());
    if(s)
    {
        cout << "签名校验成功！！！" << endl;
    }
    else
    {
        cout << "签名校验失败..." << endl;
    }*/
    // 套接字通信
    tcpSocket *tcp = new tcpSocket;
    // 链接服务器
    cout << m_info.ip << ",端口:" << m_info.port << endl;
    int ret = tcp->connectToHost(m_info.ip, m_info.port); 
    if(ret != 0)
    {
        cout << "服务器链接失败..." << endl;
        return false;
    }
    cout << "链接服务器成功..." << endl;
    
    tcp->sendMsg(encstr); // 给服务器发送数据

    string recv_msg = tcp->recvMsg(); // 接收服务器端返回的数据

    // 解码
    factory = (CodecFactory*)new RespondFactory(recv_msg);
    c = factory->CreateCodec();
    RespondMsg* res_data = (RespondMsg*)c->decodeMsg(); // 解码
    // 释放资源
    delete factory;
    delete c;
    c = NULL;
    // 检测密钥协商是否成功
    if(!res_data->status())
    {
        cout << "密钥协商失败..." << endl;
        return false;
    }

    // 获取对称加密密钥
    string key = rsa.rsaPriKeyDecrypt(res_data->data());
    cout << "对称加密密钥:" << key << endl;

    // 释放资源
    delete tcp;
    tcp = NULL;
    return true;
}

void ClientOP::seckeyCheck()
{

}

void ClientOP::seckeyCancel()
{

}

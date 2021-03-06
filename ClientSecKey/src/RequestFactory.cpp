#include "RequestFactory.h"

RequestFactory::RequestFactory(string encstr)
{
    flag = false;
    m_encStr = encstr;
}

RequestFactory::RequestFactory(RequestInfo *info)
{
    flag = true;
    m_info = info;
}

Codec* RequestFactory::CreateCodec()
{
    Codec* codec = NULL;
    if(flag)
    {
        codec = (Codec*)new RequestCodec(m_info);
    }
    else
    {
        codec = (Codec*)new RequestCodec(m_encStr);
    }

    return codec;
}

RequestFactory::~RequestFactory()
{

}

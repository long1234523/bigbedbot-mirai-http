#include <sstream>

#include "msg.h"
#include "api.h"

namespace mirai
{

MsgMetadata parseMsgMetadata(const json& v)
{
    MsgMetadata meta;

    if (v.contains("type"))
    {
        const std::string type = v.at("type").get<std::string>();
        if (v == "FriendMessage") meta.source = MsgMetadata::FRIEND;
        else if (v == "TempMessage") meta.source = MsgMetadata::TEMP;
        else if (v == "GroupMessage") meta.source = MsgMetadata::GROUP;
    }

    if (v.contains("messageChain") && v.at("messageChain").contains("Source"))
    {
        const json& s = v.at("messageChain").at("Source");
        meta.msgid = s.at("id").get<int>();
        meta.time = s.at("time").get<int>();
    }

    if (v.contains("sender"))
    {
        const json& s = v.at("sender");
        if (s.contains("id"))
            meta.qqid = s.at("id").get<int>();
        if (s.contains("group"))
        {
            const json& g = s.at("group");
            if (g.contains("id"))
                meta.groupid = g.at("id").get<int>();
        }
    }

    return meta;
}

std::string convertMessageChainObject(const json& e)
{
    if (!e.contains("type")) return "";

    auto type = e.at("type").get<std::string>();
    std::stringstream ss;

    if (type == "Plain" && e.contains("text"))
        ss << e.at("text").get<std::string>();
    else if (type == "At" && e.contains("target"))
        ss << " @" << e.at("target").get<int64_t>() << " ";

    return ss.str();
}
    
std::string messageChainToStr(const json& v)
{
    if (!v.contains("messageChain")) return "";

    const auto& m = v.at("messageChain");
    std::stringstream ss;
    for (const auto& e: m)
    {
        ss << convertMessageChainObject(e);
    }
    return ss.str();
}

std::vector<std::string> messageChainToArgs(const json& v, unsigned max_count)
{
    auto ss = std::stringstream(messageChainToStr(v));

    std::vector<std::string> args;
    while (!ss.eof() && args.size() < max_count)
    {
        std::string s;
        ss >> s;
        args.push_back(s);
    }
    if (!ss.eof()) args.push_back(ss.str());
    return args;
}

int sendMsgRespStr(const MsgMetadata& meta, const std::string& str, int64_t quoteMsgId)
{
    if (meta.source != MsgMetadata::GROUP || meta.groupid == 0) 
        return sendPrivateMsgStr(meta.qqid, str, quoteMsgId);
    else 
        return sendGroupMsgStr(meta.groupid, str, quoteMsgId);
}

int sendMsgResp(const MsgMetadata& meta, const json& messageChain, int64_t quoteMsgId)
{
    if (meta.source != MsgMetadata::GROUP || meta.groupid == 0) 
        return sendPrivateMsg(meta.qqid, messageChain, quoteMsgId);
    else 
        return sendGroupMsg(meta.groupid, messageChain, quoteMsgId);
}

}
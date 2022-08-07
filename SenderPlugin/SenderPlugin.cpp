
#include "ISender.h"
#include <Poco/ClassLibrary.h>
#include "pcap.h"

class SenderPlugin:public ISender
{
private:
    std::string str_interfaces;
public:

    std::string routine() override{
        char error[PCAP_ERRBUF_SIZE];
        pcap_if_t *interfaces,*temp;

        if(pcap_findalldevs(&interfaces,error)==-1)
        {
            log_error("error in pcap findall devs");
            str_interfaces="error";
            return "error";
        }
        int i=0;
        std::string res = "[";
        for(temp=interfaces;temp->next;temp=temp->next)
        {
            res +="\""+ std::to_string(++i)+":"+ std::string(temp->name) + "\",\n";
        }
        res+= "\""+ std::to_string(++i)+":"+std::string(temp->name) + "\"]";
        pcap_freealldevs(interfaces);
        std::replace(res.begin(),res.end(),'\\','/');
        str_interfaces=res;
        return res;
    }

    void clear(){
        str_interfaces="\"None\"";
    }

    std::string getInterfacesString(){
        return str_interfaces;
    }
};

POCO_BEGIN_MANIFEST(ISender)
POCO_EXPORT_CLASS(SenderPlugin)
POCO_END_MANIFEST
 
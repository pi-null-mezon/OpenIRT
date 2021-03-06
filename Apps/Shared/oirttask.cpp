#include "oirttask.h"

OIRTTask::OIRTTask(QTcpSocket *_tcpsocket) :
    tcpsocket(_tcpsocket),
    taskcode(OIRTTask::TaskCode::UnknownTask),
    labelinfobytes(-1),
    labelaccepted(false),
    encimgbytes(-1),
    encimgaccepted(false),
    vencimgbytes(-1),
    vencimgaccepted(false),
    whitelistbytes(-1),
    whitelistaccepted(false)
{    
}

OIRTTask::~OIRTTask()
{
}

OIRTTask::TaskCode OIRTTask::getTaskCode(quint8 _val)
{
    switch(_val) {
        case 1:
            return OIRTTask::RememberLabel;
        case 2:
            return OIRTTask::DeleteLabel;
        case 3:
            return OIRTTask::IdentifyImage;
        case 4:
            return OIRTTask::AskLabelsList;
        case 5:
            return OIRTTask::VerifyImage;
        case 6:
            return OIRTTask::UpdateWhitelist;
        case 7:
            return OIRTTask::RecognizeImage;
        case 8:
            return OIRTTask::DropWhitelist;
        default:
            return OIRTTask::UnknownTask;
    }
}

quint8 OIRTTask::getTaskCodeValue(OIRTTask::TaskCode _taskcode)
{
    switch(_taskcode) {
        case OIRTTask::RememberLabel:
            return 1;
        case OIRTTask::DeleteLabel:
            return 2;
        case OIRTTask::IdentifyImage:
            return 3;
        case OIRTTask::AskLabelsList:
            return 4;
        case OIRTTask::VerifyImage:
            return 5;
        case OIRTTask::UpdateWhitelist:
            return 6;
        case OIRTTask::RecognizeImage:
            return 7;
        case OIRTTask::DropWhitelist:
            return 8;
        default:
            return 0;
    }
}

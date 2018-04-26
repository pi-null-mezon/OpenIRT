#include "oirttask.h"

OIRTTask::OIRTTask(QTcpSocket *_tcpsocket) :
    tcpsocket(_tcpsocket),
    taskcode(OIRTTask::TaskCode::UnknownTask),
    labelinfobytes(-1),
    labelaccepted(false),
    encimgbytes(-1),
    encimgaccepted(false)
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
        default:
            return 0;
    }
}

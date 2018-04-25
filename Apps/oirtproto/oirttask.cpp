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
    qDebug("OIRTTask::~OIRTTask()");
    if(tcpsocket != 0)
        tcpsocket->deleteLater();
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

#include "oICTTask.h"

OICTTask::OICTTask(QTcpSocket *_tcpsocket) :
    tcpsocket(_tcpsocket),
    taskcode(OICTTask::TaskCode::UnknownTask),
    encimgbytes(-1),
    encimgaccepted(false)
{    
}

OICTTask::~OICTTask()
{
}

OICTTask::TaskCode OICTTask::getTaskCode(quint8 _val)
{
    switch(_val) {
        case 1:
            return OICTTask::Classify;
        case 2:
            return OICTTask::Predict;
        case 3:
            return OICTTask::AskLabelsList;
        default:
            return OICTTask::UnknownTask;
    }
}

quint8 OICTTask::getTaskCodeValue(OICTTask::TaskCode _taskcode)
{
    switch(_taskcode) {
        case OICTTask::Classify:
            return 1;
        case OICTTask::Predict:
            return 2;
        case OICTTask::AskLabelsList:
            return 3;        
        default:
            return 0;
    }
}

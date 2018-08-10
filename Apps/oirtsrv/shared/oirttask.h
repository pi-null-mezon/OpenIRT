#ifndef OIRTTASK_H
#define OIRTTASK_H

#include <QTcpSocket>

struct OIRTTask
{
    /**
     * @brief The TaskCode enum describes wht server should do to process task
     * RememberLabel - get input image along with input label, make enrollment template, save template for the label, return result code
     * DeleteLabel   - get input label, remove it from the known labels list, return result code
     * IdentifyImage - get input image, make identification template, find nearest label, return result code
     */
    enum TaskCode {UnknownTask, RememberLabel, DeleteLabel, IdentifyImage, AskLabelsList};

    OIRTTask(QTcpSocket *_tcpsocket=0);
    ~OIRTTask();

    static TaskCode getTaskCode(quint8 _val);
    static quint8   getTaskCodeValue(TaskCode _taskcode);

    QTcpSocket *tcpsocket;

    TaskCode    taskcode;

    QByteArray  labeinfo;
    qint32      labelinfobytes;
    bool        labelaccepted;

    QByteArray  encimg;
    qint32      encimgbytes;
    bool        encimgaccepted;
};

#endif // OIRTTASK_H

#ifndef OICTTASK_H
#define OICTTASK_H

#include <QTcpSocket>

struct OICTTask
{
    /**
     * @brief describes what server should do to process task
     * Classify      - return vector of labels with computede confidences
     * Predict       - return label with max confidence
     * AskLabelsList - return result code with list of known labels
     */
    enum TaskCode {UnknownTask, Classify, Predict, AskLabelsList};

    OICTTask(QTcpSocket *_tcpsocket=nullptr);
    ~OICTTask();

    static TaskCode getTaskCode(quint8 _val);
    static quint8   getTaskCodeValue(TaskCode _taskcode);

    QTcpSocket *tcpsocket;

    TaskCode    taskcode;

    QByteArray  encimg;
    qint32      encimgbytes;
    bool        encimgaccepted;
};

#endif // OICTTASK_H

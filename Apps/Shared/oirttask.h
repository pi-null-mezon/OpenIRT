#ifndef OIRTTASK_H
#define OIRTTASK_H

#include <QTcpSocket>

struct OIRTTask
{
    /**
     * @brief describes what server should do to process task
     * RememberLabel    - get input image along with input label, make enrollment template, save template for the label, return result code
     * DeleteLabel      - get input label, remove it from the known labels list, return result code
     * IdentifyImage    - get input image, make identification template, find nearest label, return result code
     * AskLabelsList    - return result code with list of known labels
     * VerifyImage      - get two images, compare them, return result code
     * UpdateWhitelist  - get list of the labels that should be used for identification, update classifier, return result code
     * RecognizeImage   - get input image, make identiifcation template, find labels for all templates with distance lower that distance thresh
     * DropWhitelist    - add all labels to whitelist
     */
    enum TaskCode {UnknownTask, RememberLabel, DeleteLabel, IdentifyImage, AskLabelsList, VerifyImage, UpdateWhitelist, RecognizeImage, DropWhitelist};

    OIRTTask(QTcpSocket *_tcpsocket=nullptr);
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

    QByteArray  vencimg;
    qint32      vencimgbytes;
    bool        vencimgaccepted;

    QByteArray  whitelist;
    qint32      whitelistbytes;
    bool        whitelistaccepted;
};

#endif // OIRTTASK_H

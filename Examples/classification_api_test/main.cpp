#include <QCoreApplication>
#include <QStringList>
#include <QDir>
#include <QMap>

#include "qupdatethread.h"
#include "qconfusionaggregator.h"

int main(int argc, char *argv[])
{
#ifdef Q_OS_WIN
    setlocale(LC_CTYPE,"Rus");
#endif
    int _argc = argc;
    char **_argv = argv;
    char *dirname=nullptr, *apiurl=nullptr;
    unsigned int maxthreads = QThread::idealThreadCount();
    bool asklabelinfo =  false;

    while(--argc > 0 && (*++argv)[0] == '-') {
        switch(*++argv[0]) {
            case 't':
                maxthreads = QString(++argv[0]).toUInt();
                break;
            case 'i':
                dirname = ++argv[0];
                break;
            case 'a':
                apiurl = ++argv[0];
                break;            
            case 'h':
                qInfo("%s v.%s\n this application has been designed for automation of the classification api testing", APP_NAME, APP_VERSION);
                qInfo(" -a[apiurl]  - recognition web-server api url");                
                qInfo(" -i[dirname] - directory with labels and pictures in the subdirs");
                qInfo(" -t[uint]    - maximum number of worker threads");
                qInfo(" -h          - this help");
                qInfo("designed by %s in 2019", APP_DESIGNER);
                return 0;
        }
    }
    if(apiurl == nullptr) {
        qWarning("Empty api url! Abort...");
        return 1;
    }
    if(!dirname) {
        qWarning("Empty labels directory name! Abort...");
        return 2;
    }
    if(maxthreads == 0) {
        qWarning("You should specify non zero positive quantity of worker threads! Abort...");
        return 3;
    }
    QDir dir(dirname);
    if(dir.exists() == false) {
        qWarning("Empty labels directory name! Abort...");
        return 4;
    }

    QCoreApplication a(_argc,_argv);
    QStringList apilabels = askLabelsInfoFrom(apiurl);
    QConfusionAggregator confusiontable(apilabels);
    QList<QPair<QString,QString>> files;
    files.reserve(1024);
    qInfo("Analyzing local directory:");
    qInfo("%s", dir.absolutePath().toUtf8().constData());
    QStringList filefilters;
    filefilters << "*.jpg" << "*.jpeg" << "*.png";
    for(int i = 0; i < apilabels.size(); ++i) {
        QDir subdir(dir.absolutePath().append("/%1").arg(apilabels.at(i)));
        qInfo(" / %s", apilabels.at(i).toUtf8().constData());
        QStringList lfiles = subdir.entryList(filefilters, QDir::Files | QDir::NoDotAndDotDot);
        for(int j = 0; j < lfiles.size(); ++j) {
            qInfo("     / %s", lfiles.at(j).toUtf8().constData());
            files.push_back(qMakePair(apilabels.at(i),subdir.absoluteFilePath(lfiles.at(j))));
        }
    }

    unsigned int threadcounter = 0;
    QString url(apiurl);
    if(files.size() > 0) {
        qInfo("Processing tasks:");
        for(int i = 0; i < files.size(); ++i) {
            qInfo("%d from %d",i,files.size());
            if(i != files.size()-1) {
                while(threadcounter >= maxthreads)
                    QCoreApplication::processEvents();
                QUpdateThread *thread = new QUpdateThread(&threadcounter,url,QUpdateThread::Predict,files.at(i).first,files.at(i).second);
                QObject::connect(thread,SIGNAL(predicted(QString,QString)),&confusiontable,SLOT(update(QString,QString)));
                QObject::connect(thread,SIGNAL(finished()),thread,SLOT(deleteLater()));
                thread->start();
            } else { // last task should be executed differently, basically we need wait untill all other task will be accomplished
                while(threadcounter > 0)
                    QCoreApplication::processEvents();
                QUpdateThread *thread = new QUpdateThread(&threadcounter,url,QUpdateThread::Predict,files.at(i).first,files.at(i).second);
                QObject::connect(thread,SIGNAL(predicted(QString,QString)),&confusiontable,SLOT(update(QString,QString)));
                QObject::connect(thread,SIGNAL(finished()),thread,SLOT(deleteLater()));
                QObject::connect(thread,SIGNAL(finished()),&confusiontable,SLOT(printResults()));
                QObject::connect(thread,SIGNAL(finished()),&a,SLOT(quit()));
                thread->start();
            }
        }        
        return a.exec();
    }    
    return 0;
}

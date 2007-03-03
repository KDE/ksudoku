

#include <kapplication.h>
#include <dcopclient.h>
#include <qdatastream.h>
#include <qstring.h>

int main(int argc, char **argv)
{
    KApplication app(argc, argv, "ksudoku_client", false);

    // get our DCOP client and attach so that we may use it
    DCOPClient *client = app.dcopClient();
    client->attach();

    // do a 'send' for now
    QByteArray data;
    QDataStream ds(data, QIODevice::WriteOnly);
    if (argc > 1)
        ds << QString(argv[1]);
    else
        ds << QString("http://www.kde.org");
    client->send("ksudoku", "ksudokuIface", "openURL(QString)", data);

    return app.exec();
}

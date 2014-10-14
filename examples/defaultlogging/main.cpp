#include <QtCore>
#include <QtGui>
#include <QtQuick>

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);

    qSetMessagePattern("[%{if-debug}D%{endif}%{if-warning}W%{endif}%{if-critical}C%{endif}%{if-fatal}F%{endif}] %{file}:%{line} - Category: %{if-category}%{category}%{endif} - %{message}");

    QQuickView view;
    view.setSource(QUrl::fromLocalFile("../Logging.qml"));

    view.show();

    return app.exec();
}
